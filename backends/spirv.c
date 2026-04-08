#include "spirv.h"
#include "../src/common.h"
#include "../src/ast.h"
#include "../src/eval.h"

#undef FUNCTION
#define FUNCTION(...) (&b->functions.items[(b->current_function, ## __VA_ARGS__)])

typedef struct {
    Array(size_t, uint32_t) code;
    SpvId spv_id;
    bool function_did_return;
} Spirv_Function;

typedef struct {
    Array(size_t, uint32_t) code_header;
    Array(size_t, uint32_t) code_debug;
    Array(size_t, uint32_t) code_annotations;
    Array(size_t, uint32_t) code_types;
    // Array(size_t, uint32_t) code_body;
    Array(size_t, Spirv_Function) functions;
    Array(size_t, uint32_t) temp;
    SpvId next_result_id;
    SpvId bool_id;

    size_t current_function;
    bool do_debug;

    SpvId entry_id;
    string entry_name;
    SpvId push_const_id;

    SpvId vertex_index;
    SpvId index_index;
    SpvId instance_index;
    SpvId per_vertex;

    SpvId frag_color_typeid;
    SpvId frag_color_index;

    Array(uint32_t, SpvId) input_variable_ids;
    Array(uint32_t, SpvId) output_variable_ids;

    Array(uint32_t, SpvId)* current_access_chain_tmp;
} LL_Backend_Spirv;

#define SPIRV_INVALID_FUNCTION 0u
#define SPIRV_INVALID_VARIABLE 0u

#define get_size_by_array(...) (sizeof(((uint32_t[]) { __VA_ARGS__ } )) / sizeof(uint32_t))
#define emit_header(...) for (size_t ___i = 1, ___size = get_size_by_array(__VA_ARGS__); ___i; ___i = 0)

#define emit_header_raw(op, ...)     emit_header((op), __VA_ARGS__) oc_array_append_many(&cc->arena, &b->code_header, ((uint32_t[]) { (op), __VA_ARGS__ } ), ___size)
#define emit_header_op(op, ...)     emit_header((op), __VA_ARGS__) oc_array_append_many(&cc->arena, &b->code_header, ((uint32_t[]) { (___size << 16) | (op), __VA_ARGS__ } ), ___size)
#define emit_debug_op(op, ...)      emit_header((op), __VA_ARGS__) oc_array_append_many(&cc->arena, &b->code_debug, ((uint32_t[]) { (___size << 16) | (op), __VA_ARGS__ } ), ___size)
#define emit_annotation_op(op, ...) emit_header((op), __VA_ARGS__) oc_array_append_many(&cc->arena, &b->code_annotations, ((uint32_t[]) { (___size << 16) | (op), __VA_ARGS__ } ), ___size)
#define emit_type_op(op, ...)       emit_header((op), __VA_ARGS__) oc_array_append_many(&cc->arena, &b->code_types, ((uint32_t[]) { (___size << 16) | (op), __VA_ARGS__ } ), ___size)
#define emit_op(op, ...)            emit_header((op), __VA_ARGS__) oc_array_append_many(&cc->arena, &FUNCTION()->code, ((uint32_t[]) { (___size << 16) | (op), __VA_ARGS__ } ), ___size)

static inline
void _emit_debug_name(Compiler_Context* cc, LL_Backend_Spirv* b, SpvId target, string name) {
    size_t num_words = (name.len + 1 /* null terminated */ + 4 - 1) / 4 + 2;
    size_t count = b->code_debug.count;
    oc_array_reserve(&cc->arena, &b->code_debug, b->code_debug.count + num_words);
    b->code_debug.count = count;
    oc_array_append(&cc->arena, &b->code_debug, SpvOpName | (num_words << 16));
    oc_array_append(&cc->arena, &b->code_debug, target);

    memcpy(b->code_debug.items + b->code_debug.count, name.ptr, name.len);
    size_t padding_count = (num_words - 2) * 4 - name.len - 1;
    memset((uint8_t*)(b->code_debug.items + b->code_debug.count) + name.len, 0, 1 + padding_count);
    b->code_debug.count += num_words - 2;
}

#define emit_debug_name(target, name) do { if (b->do_debug) _emit_debug_name(cc, b, (target), (name)); } while (0)

static inline
void _emit_entry_point(Compiler_Context* cc, LL_Backend_Spirv* b, SpvExecutionModel execution_model, SpvId entry_point_id, string entry_point_name, SpvId* interfaces, uint32_t interface_count) {
    size_t num_name_words = (entry_point_name.len + 1 /* null terminated */ + 4 - 1) / 4;
    size_t num_words = num_name_words + 3 + interface_count;
    size_t count = b->code_header.count;
    oc_array_reserve(&cc->arena, &b->code_header, b->code_header.count + num_words);
    b->code_header.count = count;
    oc_array_append(&cc->arena, &b->code_header, SpvOpEntryPoint | (num_words << 16));
    oc_array_append(&cc->arena, &b->code_header, execution_model);
    oc_array_append(&cc->arena, &b->code_header, entry_point_id);

    memcpy(b->code_header.items + b->code_header.count, entry_point_name.ptr, entry_point_name.len);
    size_t padding_count = num_name_words * 4 - entry_point_name.len;
    memset((uint8_t*)(b->code_header.items + b->code_header.count) + entry_point_name.len, 0, padding_count);
    b->code_header.count += num_name_words;

    oc_array_append_many(&cc->arena, &b->code_header, interfaces, interface_count);
}

#define emit_entry_point(execution_model, entry_point_id, entry_point_name, ...) _emit_entry_point(cc, b, (execution_model), (entry_point_id), (entry_point_name), ((SpvId[]) { __VA_ARGS__ } ), get_size_by_array(__VA_ARGS__))

static inline
SpvId emit(Compiler_Context* cc, LL_Backend_Spirv* b, typeof(b->code_header)* code, SpvId op, SpvId* ptr, size_t size) {
    size_t count = code->count;
    oc_array_reserve(&cc->arena, code, code->count + size + 2);
    code->count = count;
    oc_array_append(&cc->arena, code, op | ((size + 2) << 16));
    oc_array_append(&cc->arena, code, b->next_result_id);
    oc_array_append_many(&cc->arena, code, ptr, size);
    return b->next_result_id++;
}
static inline
SpvId emit_rev(Compiler_Context* cc, LL_Backend_Spirv* b, typeof(b->code_header)* code, SpvId op, SpvId typeid, SpvId* ptr, size_t size) {
    size_t count = code->count;
    oc_array_reserve(&cc->arena, code, code->count + size + 3);
    code->count = count;
    oc_array_append(&cc->arena, code, op | ((size + 3) << 16));
    oc_array_append(&cc->arena, code, typeid);
    oc_array_append(&cc->arena, code, b->next_result_id);
    oc_array_append_many(&cc->arena, code, ptr, size);
    return b->next_result_id++;
}

inline bool spirv_storage_class_needs_explicit(SpvStorageClass sc) {
    switch (sc) {
    case SpvStorageClassUniform:
    case SpvStorageClassStorageBuffer:
    case SpvStorageClassPhysicalStorageBuffer:
        return true;
    default: return false;
    }
}


#define emit_type_op_dst(op, ...) emit(cc, b, (typeof(b->code_header)*)&b->code_types, (op), ((uint32_t[]) { __VA_ARGS__ }), get_size_by_array(__VA_ARGS__))
#define emit_type_op_dst_rev(op, typeid, ...) emit_rev(cc, b, (typeof(b->code_header)*)&b->code_types, (op), (typeid), ((uint32_t[]) { __VA_ARGS__ }), get_size_by_array(__VA_ARGS__))
#define emit_op_dst(op, typeid, ...)      emit_rev(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, (op), (typeid), ((uint32_t[]) { __VA_ARGS__ }), get_size_by_array(__VA_ARGS__))
#define emit_op_dst_noarg(op, ...)      emit(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, (op), ((uint32_t[]) { __VA_ARGS__ }), get_size_by_array(__VA_ARGS__))

#define reserve_id() (b->next_result_id++)

LL_Backend_Layout spirv_get_layout(LL_Type* ty);
void spirv_calculate_struct_offsets(LL_Type* type);
SpvId spirv_get_pointer_type(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type, SpvStorageClass storage_class);
SpvId spirv_generate_constant(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type, void* value);
bool spirv_determine_if_explicit(Compiler_Context* cc, LL_Backend_Spirv* b, Code* expression);

typedef struct {
    // bool omit_array_stride;
    bool needs_explicit_layout;

    bool* is_invariant;
} Spirv_Type_Parameters;

#define Spirv_Type_Parameters_Default_Values .omit_array_stride = false, .needs_explicit_layout = false
#define Spirv_Type_Parameters_Default ((Spirv_Type_Parameters) { Spirv_Type_Parameters_Default_Values })

SpvId spirv_generate_type(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type);
SpvId spirv_generate_type_with_parameters(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type, Spirv_Type_Parameters parameters);
SpvId spirv_generate_expression(Compiler_Context* cc, LL_Backend_Spirv* b, Code* expr, bool lvalue);
SpvId spirv_explicit_copy_fix(Compiler_Context* cc, LL_Backend_Spirv* b, Code* from_expression, Code* to_expression, SpvId from_id);

void spirv_init(Compiler_Context* cc, LL_Backend_Spirv* b) {
    memset(b, 0, sizeof(*b));
    b->next_result_id = 1;
    emit_header_raw(SpvMagicNumber);
    emit_header_raw(SpvVersion);
    emit_header_raw(0);
    emit_header_raw(0);
    emit_header_raw(0);

    emit_header_op(SpvOpCapability, SpvCapabilityShader);
    emit_header_op(SpvOpCapability, SpvCapabilityInt8);
    emit_header_op(SpvOpCapability, SpvCapabilityInt16);
    emit_header_op(SpvOpCapability, SpvCapabilityInt64);
    emit_header_op(SpvOpCapability, SpvCapabilityPhysicalStorageBufferAddresses);
    emit_header_op(SpvOpCapability, SpvCapabilityVulkanMemoryModel);
    emit_header_op(SpvOpCapability, SpvCapabilityDrawParameters);

    emit_header_op(SpvOpMemoryModel, SpvAddressingModelPhysicalStorageBuffer64, SpvMemoryModelVulkan);

    oc_array_extend_count_unint(&cc->arena, &b->functions, 1);
    b->do_debug = true;

    b->bool_id = emit_type_op_dst(SpvOpTypeBool);

    
    if (cc->vertex) {
        // SpvId sint = spirv_get_pointer_type(cc, b, cc->typer->ty_int32, SpvStorageClassInput);
        LL_Type* sint_type = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, cc->typer->ty_int32, SpvStorageClassInput);
        bool is_invariant = false;
        SpvId sint = spirv_generate_type_with_parameters(cc, b, sint_type, (Spirv_Type_Parameters) { .is_invariant = &is_invariant });

        b->vertex_index = emit_type_op_dst_rev(SpvOpVariable, sint, SpvStorageClassInput);
        emit_annotation_op(SpvOpDecorate, b->vertex_index, SpvDecorationBuiltIn, SpvBuiltInVertexIndex);
        b->index_index = emit_type_op_dst_rev(SpvOpVariable, sint, SpvStorageClassInput);
        emit_annotation_op(SpvOpDecorate, b->index_index, SpvDecorationBuiltIn, SpvBuiltInDrawIndex);
        b->instance_index = emit_type_op_dst_rev(SpvOpVariable, sint, SpvStorageClassInput);
        emit_annotation_op(SpvOpDecorate, b->instance_index, SpvDecorationBuiltIn, SpvBuiltInInstanceIndex);

        SpvId per_vertex_id = emit_type_op_dst(SpvOpTypeStruct,
            spirv_generate_type(cc, b, ll_typer_get_vector_type(cc, cc->typer, cc->typer->ty_float32, 1, 4)),
            // spirv_generate_type(cc, b, cc->typer->ty_float32),
            // spirv_generate_type(cc, b, cc->typer->ty_uint32),
            // spirv_generate_type(cc, b, cc->typer->ty_uint32)
        );
        emit_annotation_op(SpvOpDecorate, per_vertex_id, SpvDecorationBlock);
        emit_annotation_op(SpvOpMemberDecorate, per_vertex_id, 0, SpvDecorationBuiltIn, SpvBuiltInPosition);
        // emit_annotation_op(SpvOpMemberDecorate, per_vertex_id, 1, SpvDecorationBuiltIn, SpvBuiltInPointSize);
        // emit_annotation_op(SpvOpMemberDecorate, per_vertex_id, 2, SpvDecorationBuiltIn, SpvBuiltInClipDistance);
        // emit_annotation_op(SpvOpMemberDecorate, per_vertex_id, 3, SpvDecorationBuiltIn, SpvBuiltInCullDistance);
        SpvId per_vertex_id_ptr = emit_type_op_dst(SpvOpTypePointer, SpvStorageClassOutput, per_vertex_id);
        b->per_vertex = emit_type_op_dst_rev(SpvOpVariable, per_vertex_id_ptr, SpvStorageClassOutput);
    } else if (cc->fragment) {
        bool is_invariant;
        Code_Scope* frag_type_scope;
        LL_Type* frag_type = ll_get_base_type_and_scope(cc->typer->fragment_input, &frag_type_scope);

        if (frag_type->kind == LL_TYPE_STRUCT) {
            LL_Type_Struct* struct_type = (LL_Type_Struct*)frag_type;
            Code_Struct* decl = (Code_Struct*)frag_type_scope->decl;
            oc_assert(decl->base.base.kind == CODE_KIND_STRUCT);

            oc_array_resize(&cc->arena, &b->input_variable_ids, struct_type->field_count);
            memset(b->input_variable_ids.items, 0, struct_type->field_count * sizeof(*b->input_variable_ids.items));

            for (uint32_t i = 0; i < decl->block->statements.count; ++i) {
                Code_Variable_Declaration* var_decl = (Code_Variable_Declaration*)decl->block->statements.items[i];
                if (var_decl->base.base.kind != CODE_KIND_VARIABLE_DECLARATION) continue;

                LL_Type* field_type = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, var_decl->base.ident->base.type, SpvStorageClassInput);
                SpvId field_type_id = spirv_generate_type_with_parameters(cc, b, field_type, (Spirv_Type_Parameters) { .is_invariant = &is_invariant });

                SpvId input_id = emit_type_op_dst_rev(SpvOpVariable, field_type_id, SpvStorageClassInput);
                emit_annotation_op(SpvOpDecorate, input_id, SpvDecorationLocation, var_decl->ordered_index);
                var_decl->ir_index = input_id;
                b->input_variable_ids.items[var_decl->ordered_index] = input_id;
            }
        } else {
            LL_Type* field_type = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, frag_type, SpvStorageClassInput);
            SpvId field_type_id = spirv_generate_type_with_parameters(cc, b, field_type, (Spirv_Type_Parameters) { .is_invariant = &is_invariant });

            SpvId input_id = emit_type_op_dst_rev(SpvOpVariable, field_type_id, SpvStorageClassInput);
            emit_annotation_op(SpvOpDecorate, input_id, SpvDecorationLocation, 0);
            oc_array_append(&cc->arena, &b->output_variable_ids, input_id);
        }





        SpvId color_typeid = spirv_generate_type(cc, b, ll_typer_get_vector_type(cc, cc->typer, cc->typer->ty_float32, 1, 4));
        SpvId color_ptr_typeid = emit_type_op_dst(SpvOpTypePointer, SpvStorageClassOutput, color_typeid);
        b->frag_color_typeid = color_typeid;
        b->frag_color_index = emit_type_op_dst_rev(SpvOpVariable, color_ptr_typeid, SpvStorageClassOutput);
        emit_annotation_op(SpvOpDecorate, b->frag_color_index, SpvDecorationLocation, 0);
    }
}

bool spirv_write_to_file(Compiler_Context* cc, LL_Backend_Spirv* b, char* filepath) {
    (void)cc;
    b->code_header.items[3] = b->next_result_id; // write id bound

    Array(uint32, SpvId) entry_interface = { 0 };
    oc_array_append(&cc->tmp_arena, &entry_interface, b->push_const_id);
    if (cc->vertex) {
        oc_array_append(&cc->tmp_arena, &entry_interface, b->vertex_index);
        oc_array_append(&cc->tmp_arena, &entry_interface, b->index_index);
        oc_array_append(&cc->tmp_arena, &entry_interface, b->instance_index);
        oc_array_append(&cc->tmp_arena, &entry_interface, b->per_vertex);
        oc_array_append_many(&cc->tmp_arena, &entry_interface, b->output_variable_ids.items, b->output_variable_ids.count);
    } else if (cc->fragment) {
        oc_array_append(&cc->tmp_arena, &entry_interface, b->frag_color_index);
        oc_array_append_many(&cc->tmp_arena, &entry_interface, b->input_variable_ids.items, b->input_variable_ids.count);
    }

    _emit_entry_point(cc, b, cc->vertex ? SpvExecutionModelVertex : SpvExecutionModelFragment, b->entry_id, b->entry_name, entry_interface.items, entry_interface.count);

    if (cc->fragment) {
        emit_header_op(SpvOpExecutionMode, b->entry_id, SpvExecutionModeOriginUpperLeft);
    }

    FILE* fptr;
    if (fopen_s(&fptr, filepath, "wb")) {
        eprint("Unable to open output file: {}\n", filepath);
        return false;
    }

    bool s = fwrite(b->code_header.items, 1, b->code_header.count * 4, fptr) == b->code_header.count * 4;
    s = s && fwrite(b->code_debug.items, 1, b->code_debug.count * 4, fptr) == b->code_debug.count * 4;
    s = s && fwrite(b->code_annotations.items, 1, b->code_annotations.count * 4, fptr) == b->code_annotations.count * 4;
    s = s && fwrite(b->code_types.items, 1, b->code_types.count * 4, fptr) == b->code_types.count * 4;
    // @Note: startat 1 bc 0 is nil value
    for (size_t i = 1; i < b->functions.count; ++i) {
        Spirv_Function* fn = &b->functions.items[i];
        s = s && fwrite(fn->code.items, 1, fn->code.count * 4, fptr) == fn->code.count * 4;
    }
    
    // s = s && fwrite(b->code_body.items, 1, b->code_body.count * 4, fptr) == b->code_body.count * 4;
    fclose(fptr);


    return s;
}

void spirv_generate_statement(Compiler_Context* cc, LL_Backend_Spirv* b, Code* stmt) {
    bool is_invariant;
    switch (stmt->kind) {
    case CODE_KIND_BLOCK: {
        Code_Scope* blk = CODE_AS(stmt, Code_Scope);
        for (size_t i = 0; i < blk->declarations.capacity; ++i) {
            if (blk->declarations.entries[i].filled) {
                Code_Declaration* decl = blk->declarations.entries[i]._value;
                if (decl->base.kind == CODE_KIND_VARIABLE_DECLARATION) {
                } else {
                    spirv_generate_statement(cc, b, (Code*)blk->declarations.entries[i]._value);
                }
            }
        }

        if (blk->flags & CODE_SCOPE_FLAG_DECLARATIVE) {
        } else {
            for (size_t i = 0; i < blk->statements.count; ++i) {
                spirv_generate_statement(cc, b, blk->statements.items[i]);
            }
        }

    } break;
    case CODE_KIND_VARIABLE_DECLARATION: {
        Code_Variable_Declaration* var_decl = CODE_AS(stmt, Code_Variable_Declaration);
        // if (ll_symbol_not_used(var_decl->base.usage)) return;
        if (var_decl->base.within_scope->flags & CODE_SCOPE_FLAG_DECLARATIVE) return;

        if (var_decl->storage_class & LL_STORAGE_CLASS_EXTERN) break;
        if (var_decl->storage_class & LL_STORAGE_CLASS_CONST) break;
        oc_assert(b->current_function != SPIRV_INVALID_FUNCTION);

        oc_assert(var_decl->ir_index != SPIRV_INVALID_VARIABLE); // we handle this in the block
        // SpvId typeid = spirv_generate_type(cc, b, var_decl->base.declared_type);
        // SpvId variable_id = emit_op_dst(SpvOpVariable, typeid);

        // #if 0
        if (var_decl->initializer) {
            SpvId init_id = spirv_generate_expression(cc, b, var_decl->initializer, false);
            emit_op(SpvOpStore, var_decl->ir_index, init_id);
        }
        // #endif

    } break;
    case CODE_KIND_FUNCTION_DECLARATION: {
        Code_Function_Declaration* fn_decl = CODE_AS(stmt, Code_Function_Declaration);
        bool is_main = string_eql(fn_decl->base.ident->str, lit("main"));
        if (!is_main) {
            // don't dead code elim entry
            if (ll_symbol_not_used(fn_decl->base.usage)) return;
        }

        if (fn_decl->storage_class & LL_STORAGE_CLASS_MACRO) return;
        if (fn_decl->storage_class & LL_STORAGE_CLASS_POLYMORPHIC) return;
        if (fn_decl->ir_index != 0) return;

        size_t last_function = b->current_function;
        size_t new_function_id = b->functions.count;
        b->current_function = new_function_id;
        fn_decl->ir_index = new_function_id;

        oc_array_extend_count_unint(&cc->arena, &b->functions, 1);
        Spirv_Function* fn = FUNCTION();
        memset(fn, 0, sizeof(*fn));

        LL_Type_Function* fn_type;
        SpvId return_type;
        if (is_main) {
            fn_type = (LL_Type_Function*)ll_typer_get_fn_type(cc, cc->typer, cc->typer->ty_void, NULL, 0, false);
            return_type = spirv_generate_type(cc, b, cc->typer->ty_void);
        } else {
            fn_type = (LL_Type_Function*)fn_decl->base.ident->base.type;
            return_type = spirv_generate_type(cc, b, fn_type->return_type);
        }

        SpvId spirv_function_type = spirv_generate_type(cc, b, (LL_Type*)fn_type);

        SpvId function_id = emit_op_dst(SpvOpFunction, return_type, 0, spirv_function_type);
        fn->spv_id = function_id;
        emit_debug_name(function_id, fn_decl->base.ident->str);

        if (is_main) {
            b->entry_id = function_id;
            b->entry_name = fn_decl->base.ident->str;
        }

        if (fn_decl->body) {
            SpvId parameter_ids[fn_decl->parameters.count];
            b->push_const_id = 0;

            if (is_main) {
                LL_Type* return_type = fn_decl->base.type->type;
                Code_Scope* return_type_scope = NULL;
                return_type = ll_get_base_type_and_scope(return_type, &return_type_scope);

                if (return_type->kind == LL_TYPE_STRUCT) {
                    LL_Type_Struct* struct_type = (LL_Type_Struct*)return_type;
                    Code_Struct* decl = (Code_Struct*)return_type_scope->decl;
                    oc_assert(decl->base.base.kind == CODE_KIND_STRUCT);

                    oc_array_resize(&cc->arena, &b->output_variable_ids, struct_type->field_count);
                    memset(b->output_variable_ids.items, 0, struct_type->field_count * sizeof(*b->output_variable_ids.items));

                    for (uint32_t i = 0; i < decl->block->statements.count; ++i) {
                        Code_Variable_Declaration* var_decl = (Code_Variable_Declaration*)decl->block->statements.items[i];
                        if (var_decl->base.base.kind != CODE_KIND_VARIABLE_DECLARATION) continue;


                        // SpvId field_type_id = spirv_get_pointer_type(cc, b, var_decl->base.ident->base.type, SpvStorageClassOutput);

                        LL_Type* field_type = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, var_decl->base.ident->base.type, SpvStorageClassOutput);
                        SpvId field_type_id = spirv_generate_type_with_parameters(cc, b, field_type, (Spirv_Type_Parameters) { .is_invariant = &is_invariant });

                        SpvId output_id = emit_type_op_dst_rev(SpvOpVariable, field_type_id, SpvStorageClassOutput);
                        emit_annotation_op(SpvOpDecorate, output_id, SpvDecorationLocation, var_decl->ordered_index);
                        var_decl->ir_index = output_id;
                        b->output_variable_ids.items[var_decl->ordered_index] = output_id;
                    }

                    // LL_Type_Struct* struct_type = (LL_Type_Struct*)return_type;
                    // for (size_t i = 0; i < struct_type->field_count; ++i) {
                    //     SpvId field_type_id = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, struct_type->fields[i], SpvStorageClassOutput);
                    //     SpvId output_id = emit_type_op_dst_rev(SpvOpVariable, field_type_id, SpvStorageClassOutput);
                    //     emit_annotation_op(SpvOpDecorate, output_id, SpvDecorationLocation, (uint32_t)i);
                    // }
                } else if (return_type->kind != LL_TYPE_VOID) {
                    LL_Type* field_type = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, return_type, SpvStorageClassOutput);
                    SpvId field_type_id = spirv_generate_type_with_parameters(cc, b, field_type, (Spirv_Type_Parameters) { .is_invariant = &is_invariant });

                    SpvId output_id = emit_type_op_dst_rev(SpvOpVariable, field_type_id, SpvStorageClassOutput);
                    emit_annotation_op(SpvOpDecorate, output_id, SpvDecorationLocation, 0);
                    oc_array_append(&cc->arena, &b->output_variable_ids, output_id);
                }
            }

            // Generate SpvOpParameters (needs to be before first block)
            if (is_main) {
                SpvId push_const_typeid = reserve_id();

                uint32_t offset = 0;

                // main parameters are handled as inputs
                for (uint32 i = 0; i < fn_decl->parameters.count; ++i) {
                    Code_Variable_Declaration* decl = &fn_decl->parameters.items[i];

                    SpvId typeid = spirv_generate_type_with_parameters(cc, b, decl->base.ident->base.type, (Spirv_Type_Parameters) { .needs_explicit_layout = true, .is_invariant = &is_invariant });
                    LL_Backend_Layout l = spirv_get_layout(decl->base.ident->base.type);

                    offset = oc_align_forward(offset, l.alignment);
                    emit_annotation_op(SpvOpMemberDecorate, push_const_typeid, i, SpvDecorationOffset, offset);
                    offset = oc_align_forward(offset + max(l.size, l.alignment), l.alignment);

                    parameter_ids[i] = typeid;
                }

                oc_array_append(&cc->arena, &b->code_types, (SpvOpTypeStruct) | ((2 + fn_decl->parameters.count) << 16));
                oc_array_append(&cc->arena, &b->code_types, push_const_typeid);
                oc_array_append_many(&cc->arena, &b->code_types, parameter_ids, fn_decl->parameters.count);

                emit_annotation_op(SpvOpDecorate, push_const_typeid, SpvDecorationBlock);

                SpvId ptr_push_const_typeid = emit_type_op_dst(SpvOpTypePointer, SpvStorageClassPushConstant, push_const_typeid);
                b->push_const_id = emit_type_op_dst_rev(SpvOpVariable, ptr_push_const_typeid, SpvStorageClassPushConstant);
            } else {
                // normal function parameters
                for (uint32 i = 0; i < fn_decl->parameters.count; ++i) {
                    Code_Variable_Declaration* decl = &fn_decl->parameters.items[i];

                    SpvId typeid = spirv_generate_type(cc, b, decl->base.type->type);
                    SpvId parameter_id = emit_op_dst(SpvOpFunctionParameter, typeid);

                    parameter_ids[i] = parameter_id;
                    emit_debug_name(parameter_id, decl->base.ident->str);
                }
            }

            // first block label
            (void)emit_op_dst_noarg(SpvOpLabel);

            // Create variables to store paramters if needed (needs to be at beginning of first block)
            if (is_main) {
                for (uint32 i = 0; i < fn_decl->parameters.count; ++i) {
                    Code_Variable_Declaration* decl = &fn_decl->parameters.items[i];

                    LL_Type* variable_type = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, decl->base.ident->base.type, SpvStorageClassFunction);
                    SpvId variable_type_id = spirv_generate_type_with_parameters(cc, b, variable_type, (Spirv_Type_Parameters) { .is_invariant = &is_invariant });
                    SpvId variable_id = emit_op_dst(SpvOpVariable, variable_type_id, SpvStorageClassFunction);
                    emit_debug_name(variable_id, decl->base.ident->str);

                    decl->ir_index = variable_id;
                }
            } else {
                for (uint32 i = 0; i < fn_decl->parameters.count; ++i) {
                    Code_Variable_Declaration* decl = &fn_decl->parameters.items[i];

                    if (decl->base.usage.direct_stores + decl->base.usage.pointers_created) {
                        // SpvId typeid = spirv_get_pointer_type(cc, b, decl->base.type->type, SpvStorageClassFunction);
                        LL_Type* field_type = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, decl->base.type->type, SpvStorageClassFunction);
                        SpvId field_type_id = spirv_generate_type_with_parameters(cc, b, field_type, (Spirv_Type_Parameters) { .is_invariant = &is_invariant });
                        SpvId variable_id = emit_op_dst(SpvOpVariable, field_type_id, SpvStorageClassFunction);

                        decl->ir_index = variable_id;
                        emit_debug_name(variable_id, decl->base.ident->str);
                    } else {
                        decl->ir_index = 0;
                    }
                }
            }

            // Generate local variables
            for (uint32 i = 0; i < fn_decl->all_local_variables.count; ++i) {
                Code_Variable_Declaration* decl = fn_decl->all_local_variables.items[i];

                // SpvId typeid = spirv_get_pointer_type(cc, b, decl->base.type->type, SpvStorageClassFunction);
                LL_Type* field_type = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, decl->base.type->type, SpvStorageClassFunction);
                SpvId field_type_id = spirv_generate_type_with_parameters(cc, b, field_type, (Spirv_Type_Parameters) { .is_invariant = &is_invariant });
                SpvId variable_id = emit_op_dst(SpvOpVariable, field_type_id, SpvStorageClassFunction);
                decl->ir_index = variable_id;
                emit_debug_name(variable_id, decl->base.ident->str);
            }

            // Copy parameters to variables if needed
            if (is_main) {
                for (uint32 i = 0; i < fn_decl->parameters.count; ++i) {
                    Code_Variable_Declaration* decl = &fn_decl->parameters.items[i];

                    SpvId index = spirv_generate_constant(cc, b, cc->typer->ty_uint32, &i);
                    SpvId typeid = emit_type_op_dst(SpvOpTypePointer, SpvStorageClassPushConstant, parameter_ids[i]);
                    SpvId ptr = emit_op_dst(SpvOpAccessChain, typeid, b->push_const_id, index);
                    SpvId loaded = emit_op_dst(SpvOpLoad, parameter_ids[i], ptr);

                    LL_Type* base_type = ll_get_base_type(decl->base.ident->base.type); 
                    if (base_type->kind == LL_TYPE_STRUCT) {
                        SpvId variable_typeid = spirv_generate_type_with_parameters(cc, b, decl->base.ident->base.type, (Spirv_Type_Parameters) { .is_invariant = &is_invariant });
                        loaded = emit_op_dst(SpvOpCopyLogical, variable_typeid, loaded);
                    }

                    emit_op(SpvOpStore, decl->ir_index, loaded);
                    decl->base.base.kind = CODE_KIND_VARIABLE_DECLARATION; // @Robustness: how bad is this
                }

            } else {
                for (uint32 i = 0; i < fn_decl->parameters.count; ++i) {
                    Code_Variable_Declaration* decl = &fn_decl->parameters.items[i];
                    if (decl->ir_index) {
                        decl->base.base.kind = CODE_KIND_VARIABLE_DECLARATION; // @Robustness: how bad is this
                        emit_op(SpvOpStore, decl->ir_index, parameter_ids[i]);
                    } else {
                        decl->ir_index = parameter_ids[i];
                    }
                }
            }

            spirv_generate_statement(cc, b, (Code*)fn_decl->body);
        }
        if (!FUNCTION()->function_did_return) {
            emit_op(SpvOpReturn);
        }
        emit_op(SpvOpFunctionEnd);

        b->current_function = last_function;
    } break;
    case CODE_KIND_STRUCT: {} break;
    default: {
        spirv_generate_expression(cc, b, stmt, false);
    } break;
    }
}

void spirv_generate_statement_restore_state(Compiler_Context* cc, LL_Backend_Spirv* b, Code* stmt) {
    size_t current_function = b->current_function;



    spirv_generate_statement(cc, b, stmt);



    b->current_function = current_function;
}



SpvId spirv_const_to_operand(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type, LL_Eval_Value const_value) {
    SpvId result;
    SpvId typeid = spirv_generate_type(cc, b, type);
    type = ll_get_base_type(type);
    switch (type->kind) {
    case LL_TYPE_FLOAT: {
        if (type->width <= 32) {
            union {
                float f;
                uint32_t i;
            } a = { .f = (float)const_value.as_f64 };
            result = emit_op_dst(SpvOpConstant, typeid, a.i);
        } else if (type->width <= 64) {
            union {
                double f;
                uint32_t i[2];
            } a = { .f = const_value.as_f64 };
            result = emit_op_dst(SpvOpConstant, typeid, a.i[0], a.i[1]);
        } else oc_todo("bigger types");
    } break;
    case LL_TYPE_INT:
    case LL_TYPE_UINT:
        if (type->width <= 32) {
            result = emit_op_dst(SpvOpConstant, typeid, (uint32_t)const_value.as_u64);
        } else {
            result = emit_op_dst(SpvOpConstant, typeid, (uint32_t)(const_value.as_u64 & 0xFFFFFFFF), (uint32_t)(const_value.as_u64 >> 32));
        }
        break;
    case LL_TYPE_BOOL:
        result = emit_op_dst(SpvOpConstant, typeid, (uint32_t)const_value.as_u64);
        break;
    case LL_TYPE_ANYBOOL:
        if (const_value.as_u64) {
            result = emit_op_dst(SpvOpConstantTrue, typeid);
        } else {
            result = emit_op_dst(SpvOpConstantFalse, typeid);
        }
        break;
    // case LL_TYPE_SLICE:
    // case LL_TYPE_STRING:
    // case LL_TYPE_ARRAY:
    // case LL_TYPE_STRUCT: {
    //     oc_assert((b->data_items.count & 0xF0000000u) == 0); // oc_todo: maybe support more
    //     result = LL_IR_OPERAND_DATA_BIT | (uint32_t)b->data_items.count;
    //     LL_Backend_Layout layout = cc->target->get_layout(type);
    //     oc_array_append(&cc->arena, &b->data_items, ((LL_Ir_Data_Item) { .ptr = const_value.as_object, .len = max(layout.size, layout.alignment), .type = type}));
    // } break;
    default: ll_print_type(type); oc_todo("implement type"); break;
    }

    return result;
}

SpvId spirv_generate_cast_if_needed(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* to_type, SpvId from, LL_Type* from_type) {
    (void)cc;
    (void)b;
    if (to_type == from_type) return from;
    SpvId result = 0;
    SpvId to_type_id = spirv_generate_type(cc, b, to_type);
    SpvId from_type_id = spirv_generate_type(cc, b, from_type);
    if (to_type_id == from_type_id) return from; // will this every happen...

    switch (from_type->kind) {
    case LL_TYPE_POINTER:
        switch (to_type->kind) {
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
            case LL_TYPE_BOOL:
                if (from_type->width == to_type->width) {
                    oc_assert(from_type->rows == to_type->rows);
                    oc_assert(from_type->columns == to_type->columns);
                    result = emit_op_dst(SpvOpBitcast, to_type_id, from);
                }
                break;
            default: oc_todo("handle cast types to {}\n", (int)to_type->kind); break;
        }
        break;
    case LL_TYPE_INT:
        switch (to_type->kind) {
            case LL_TYPE_FLOAT:
                result = emit_op_dst(SpvOpConvertSToF, to_type_id, from);
                break;
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
            case LL_TYPE_BOOL:
            case LL_TYPE_POINTER:
                if (from_type->width == to_type->width) {
                    oc_assert(from_type->rows == to_type->rows);
                    oc_assert(from_type->columns == to_type->columns);
                    result = emit_op_dst(SpvOpBitcast, to_type_id, from);
                } else {
                    if (to_type->kind == LL_TYPE_INT) {
                        result = emit_op_dst(SpvOpSConvert, to_type_id, from);
                    } else {
                        result = emit_op_dst(SpvOpUConvert, to_type_id, from);
                    }
                }
                break;
            default: oc_todo("handle cast types to {}\n", (int)to_type->kind); break;
        }
        break;
    case LL_TYPE_UINT:
        switch (to_type->kind) {
            case LL_TYPE_FLOAT:
                result = emit_op_dst(SpvOpConvertUToF, to_type_id, from);
                break;
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
            case LL_TYPE_BOOL:
            case LL_TYPE_POINTER:
                if (from_type->width == to_type->width) {
                    oc_assert(from_type->rows == to_type->rows);
                    oc_assert(from_type->columns == to_type->columns);
                    result = emit_op_dst(SpvOpBitcast, to_type_id, from);
                } else {
                    if (to_type->kind == LL_TYPE_INT) {
                        result = emit_op_dst(SpvOpSConvert, to_type_id, from);
                    } else {
                        result = emit_op_dst(SpvOpUConvert, to_type_id, from);
                    }
                }
                break;
            default: oc_todo("handle cast types to {}\n", (int)to_type->kind); break;
        }
        break;
    case LL_TYPE_FLOAT:
        switch (to_type->kind) {
            case LL_TYPE_FLOAT:
                result = emit_op_dst(SpvOpFConvert, to_type_id, from);
                break;
            case LL_TYPE_UINT:
                result = emit_op_dst(SpvOpConvertFToU, to_type_id, from);
                break;
            case LL_TYPE_INT:
                result = emit_op_dst(SpvOpConvertFToS, to_type_id, from);
                break;
            case LL_TYPE_BOOL:
                result = emit_op_dst(SpvOpConvertFToU, to_type_id, from);
                break;
            default: oc_todo("handle cast types to {}\n", (int)to_type->kind); break;
        }
        break;

    default: oc_todo("handle cast types from\n"); return from;
    }

    return result;
}

SpvId spirv_generate_constant(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type, void* value) {
    SpvId typeid = spirv_generate_type(cc, b, type);
    LL_Type_Kind kind = type->kind;
    size_t width = type->width;
    union {
        void* ptr;
        float* f;
        double* d;
        uint32_t* u32;
        uint64_t* u64;
    } input = { .ptr = value };
    union {
        float f;
        double d;
        uint32_t u32;
        uint64_t u64;
        uint32_t encoded_value[2];
    } output;
    switch (kind) {
    case LL_TYPE_FLOAT:
        if (width <= 32) {
            output.f = *input.f;
        } else {
            output.d = *input.d;
        }
        break;
    case LL_TYPE_UINT:
    case LL_TYPE_INT:
    case LL_TYPE_BOOL:
        if (width <= 32) {
            output.u32 = *input.u32;
        } else {
            output.u64 = *input.u64;
        }
        break;
    }
    SpvId result;
    if (width <= 32) {
        result = emit_type_op_dst_rev(SpvOpConstant, typeid, output.encoded_value[0]);
    } else {
        result = emit_type_op_dst_rev(SpvOpConstant, typeid, output.encoded_value[0], output.encoded_value[1]);
    }
    return result;
}

bool spirv_determine_if_explicit(Compiler_Context* cc, LL_Backend_Spirv* b, Code* expression) {
    LL_Type* base_type = ll_get_base_type(expression->type);
    if (base_type->kind == LL_TYPE_POINTER) {
        if (spirv_storage_class_needs_explicit(((LL_Type_Pointer*)base_type)->spirv_storage_class)) {
            expression->spv.is_explicit = true;
            return true;
        }
    }
    return expression->spv.is_explicit;
}

SpvId spirv_explicit_copy_fix(Compiler_Context* cc, LL_Backend_Spirv* b, Code* from_expression, Code* to_expression, SpvId from_id) {
    if (from_expression->spv.is_explicit == to_expression->spv.is_explicit) return from_id;
    SpvId result = 0;

    bool is_invariant;
    SpvId to_typeid = spirv_generate_type_with_parameters(cc, b, to_expression->type, (Spirv_Type_Parameters) { .needs_explicit_layout = to_expression->spv.is_explicit, .is_invariant = &is_invariant });
    if (to_expression->type->spirv_type != to_expression->type->explicit_spirv_type) {
        result = emit_op_dst(SpvOpCopyLogical, to_typeid, from_id);
    }
    return result;
}

SpvId spirv_generate_vector_constructor(Compiler_Context* cc, LL_Backend_Spirv* b, Code_Invoke* inv) {
    oc_assert(inv->expr->has_const);
    LL_Type* dest_type = inv->expr->const_value.as_type;

    uword arg_count = inv->arguments.count;
    uword components = dest_type->rows * dest_type->columns;
    SpvId args[components];

    oc_assert(inv->arguments.count > 0);
    Code* current_arg = inv->arguments.items[0];
    SpvId current_arg_id;
    bool is_constant = true;
    (void)is_constant;

    for (uword si = 0, src_i = 0, di = 0; di < components; ++di) {
        if (src_i == 0) {
            current_arg = inv->arguments.items[si];
            current_arg_id = spirv_generate_expression(cc, b, current_arg, false);
        }

        uword arg_components = current_arg->type->rows * current_arg->type->columns;
        if (arg_components > 1) {
            SpvId arg_typeid = spirv_generate_type(cc, b, current_arg->type->base_type);
            SpvId extract_id = emit_op_dst(SpvOpCompositeExtract, arg_typeid, current_arg_id, src_i);
            args[di] = extract_id;
        } else {
            args[di] = current_arg_id;
        }

        src_i += 1;
        if (src_i >= arg_components) {
            src_i = 0;
            si += 1;
        }
    }

    SpvId dest_typeid = spirv_generate_type(cc, b, dest_type);
    SpvId result = emit_rev(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, SpvOpCompositeConstruct, dest_typeid, args, components);
    return result;
}

SpvId spirv_load(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type, SpvId ptr, bool dst_is_explicit) {
    oc_assert(type->kind == LL_TYPE_POINTER);
    type = ll_get_base_type(type);
    LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)type;
    SpvId result = 0;
    bool is_invariant;
    bool explicit = spirv_storage_class_needs_explicit(ptr_type->spirv_storage_class);

    SpvId typeid = spirv_generate_type_with_parameters(cc, b, ptr_type->element_type, (Spirv_Type_Parameters) { .needs_explicit_layout = explicit, .is_invariant = &is_invariant });
    if (explicit) {
        result = emit_op_dst(SpvOpLoad, typeid, ptr, SpvMemoryAccessAlignedMask, 16);
    } else {
        result = emit_op_dst(SpvOpLoad, typeid, ptr);
    }

    LL_Type* element_type = ll_get_base_type(ptr_type->element_type);

    if (dst_is_explicit != explicit && element_type->kind == LL_TYPE_STRUCT) {
        SpvId non_explicit_typeid = spirv_generate_type_with_parameters(cc, b, ptr_type->element_type, (Spirv_Type_Parameters) { .needs_explicit_layout = dst_is_explicit, .is_invariant = &is_invariant });
        result = emit_op_dst(SpvOpCopyLogical, non_explicit_typeid, result);
    }
    return result;
}

SpvId spirv_generate_expression(Compiler_Context* cc, LL_Backend_Spirv* b, Code* expr, bool lvalue) {
    SpvId result = 0;
    SpvId typeid = spirv_generate_type(cc, b, expr->type);
    SpvId r1, r2;

    switch (expr->kind) {
    // @Note: dxc generates constants in types section... why?
    case CODE_KIND_LITERAL_INT: {
        Code_Literal* lit = CODE_AS(expr, Code_Literal);
        if (expr->type->kind == LL_TYPE_FLOAT) {
            if (expr->type->width <= 32) {
                union {
                    float f;
                    uint32_t i;
                } a = { .f = (float)lit->u64 };
                result = emit_type_op_dst_rev(SpvOpConstant, typeid, a.i);
            } else if (expr->type->width <= 64) {
                union {
                    double f;
                    uint32_t i[2];
                } a = { .f = (double)lit->u64 };
                result = emit_type_op_dst_rev(SpvOpConstant, typeid, a.i[0], a.i[1]);
            } else oc_todo("bigger types");
        } else {
            if (expr->type->width <= 32) {
                result = emit_type_op_dst_rev(SpvOpConstant, typeid, (uint32_t)lit->u64);
            } else {
                result = emit_type_op_dst_rev(SpvOpConstant, typeid, (uint32_t)(lit->u64 & 0xFFFFFFFF), (uint32_t)(lit->u64 >> 32));
            }
        }
    } break;
    case CODE_KIND_LITERAL_FLOAT: {
        Code_Literal* lit = CODE_AS(expr, Code_Literal);
        if (expr->type->width <= 32) {
            union {
                float f;
                uint32_t i;
            } a = { .f = (float)lit->f64 };
            result = emit_type_op_dst_rev(SpvOpConstant, typeid, a.i);
        } else if (expr->type->width <= 64) {
            union {
                double f;
                uint32_t i[2];
            } a = { .f = lit->f64 };
            result = emit_type_op_dst_rev(SpvOpConstant, typeid, a.i[0], a.i[1]);
        } else {
            oc_assert(false);
        }
    } break;

    case CODE_KIND_BUILTIN: {
        Code_Ident* ident = CODE_AS(expr, Code_Ident);
        if (cc->vertex) {
            if (string_eql(ident->str, lit("vertex_index"))) {
                oc_assert(!lvalue);
                result = b->vertex_index;
                result = emit_op_dst(SpvOpLoad, typeid, result);
                return result;
            } else if (string_eql(ident->str, lit("index_index"))) {
                oc_assert(!lvalue);
                result = b->index_index;
                result = emit_op_dst(SpvOpLoad, typeid, result);
            } else if (string_eql(ident->str, lit("instance_index"))) {
                oc_assert(!lvalue);
                result = b->instance_index;
                result = emit_op_dst(SpvOpLoad, typeid, result);
            } else if (string_eql(ident->str, lit("position"))) {
                SpvId typeid = spirv_get_pointer_type(cc, b, expr->type, SpvStorageClassOutput);
                int32_t index = 0;
                SpvId index_id = spirv_generate_constant(cc, b, cc->typer->ty_int32, &index);
                result = emit_op_dst(SpvOpAccessChain, typeid, b->per_vertex, index_id);

                if (!lvalue) {
                    result = emit_op_dst(SpvOpLoad, typeid, result);
                }
            } else oc_assert(false);
        } else if (cc->fragment) {
            if (string_eql(ident->str, lit("color"))) {
                result = b->frag_color_index;

                if (!lvalue) {
                    result = emit_op_dst(SpvOpLoad, b->frag_color_typeid, result);
                }
            } else if (string_eql(ident->str, lit("fragment_input"))) {
                oc_assert(!lvalue);
                result = b->index_index;
                result = emit_op_dst(SpvOpLoad, typeid, result);
            } else oc_assert(false);
        } else oc_assert(false);
    } break;

    case CODE_KIND_IDENT: {
        Code_Ident* ident = CODE_AS(expr, Code_Ident);

        if (ident->str.ptr == LL_KEYWORD_TRUE.ptr) {
            oc_assert(!lvalue);
            result = emit_type_op_dst_rev(SpvOpConstant, typeid, 1);
            break;
        } else if (ident->str.ptr == LL_KEYWORD_FALSE.ptr) {
            oc_assert(!lvalue);
            result = emit_type_op_dst_rev(SpvOpConstant, typeid, 0);
            break;
        } else if (ident->str.ptr == LL_KEYWORD_NULL.ptr) {
            oc_assert(!lvalue);
            result = emit_type_op_dst_rev(SpvOpConstantNull, typeid);
            break;
        }

        if (ident->base.has_const) {
            result = spirv_const_to_operand(cc, b, ident->base.type, ident->base.const_value);
            break; // break outer switch
        }

        Code* decl = (Code*)ident->resolved_decl;
        switch (decl->kind) {
        case CODE_KIND_VARIABLE_DECLARATION:
            result = CODE_AS(decl, Code_Variable_Declaration)->ir_index;
            expr->spv.is_explicit = decl->spv.is_explicit;
            break;
        case CODE_KIND_FUNCTION_DECLARATION:
            if (CODE_AS(decl, Code_Function_Declaration)->ir_index == 0) {
                spirv_generate_statement_restore_state(cc, b, decl);
            }
            Spirv_Function* spv_function = FUNCTION(CODE_AS(decl, Code_Function_Declaration)->ir_index);
            result = spv_function->spv_id;
            return result;
            break;
        case CODE_KIND_PARAMETER:
            result = CODE_AS(decl, Code_Variable_Declaration)->ir_index;
            oc_assert(!lvalue);
            return result;
            break;
        default: oc_assert(false);
        }

        if (!lvalue) {
            result = emit_op_dst(SpvOpLoad, typeid, result);
        }
        
        break;
    }

    case CODE_KIND_PRE_OP: {
        Code_Operation* op = CODE_AS(expr, Code_Operation);
        switch (op->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '-': {
            result = spirv_generate_expression(cc, b, op->right, false);
            if (op->right->type->kind == LL_TYPE_FLOAT) {
                result = emit_op_dst(SpvOpFNegate, typeid, result);
            } else {
                // @Robustness: probably doens't make sense to have unsigned int here
                oc_assert(op->right->type->kind == LL_TYPE_INT || op->right->type->kind == LL_TYPE_UINT);
                result = emit_op_dst(SpvOpSNegate, typeid, result);
            }
        } break;
        case '*': {
            result = spirv_generate_expression(cc, b, op->right, false);
            expr->spv.is_explicit = spirv_determine_if_explicit(cc, b, op->right);
            if (!lvalue) {
                result = spirv_load(cc, b, op->right->type, result, false);
            }
        } break;
        case '&': {
            result = spirv_generate_expression(cc, b, op->right, true);
            // if (op->right->kind != CODE_KIND_INDEX && op->right->kind != CODE_KIND_BINARY_OP) {
            //     result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA, expr->type, result);
            // }
            break;
        }
#pragma GCC diagnostic push
        default: break;
        }
    } break;


    case CODE_KIND_BINARY_OP: {
        Code_Operation* op = CODE_AS(expr, Code_Operation);
        uint32_t spv_opcode = 0;
        switch (op->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '.': {
            Code_Ident* right_ident = CODE_AS(op->right, Code_Ident);

            Code_Declaration* field_scope = right_ident->resolved_decl ? right_ident->resolved_decl : NULL;
            if (field_scope) {
                typeof(*b->current_access_chain_tmp) chain_access = { 0 };

                if (op->left->kind == CODE_KIND_BUILTIN) {
                    Code_Ident* ident = CODE_AS(op->left, Code_Ident);
                    if (string_eql(ident->str, lit("fragment_input"))) {
                        Code_Variable_Declaration* field_decl = CODE_AS(field_scope, Code_Variable_Declaration);

                        result = field_decl->ir_index;
                        if (!lvalue) {
                            result = emit_op_dst(SpvOpLoad, typeid, field_decl->ir_index);
                        }
                        return result;
                    }
                }

                Oc_Arena_Save save;
                bool had_access_chain = b->current_access_chain_tmp != NULL;
                if (!had_access_chain) {
                    save = oc_arena_save(&cc->tmp_arena);
                    b->current_access_chain_tmp = &chain_access;
                }


                // @TODO: probably should assert that this struct scope is the same as the lhs of the dot
                Code_Declaration* struct_scope = field_scope->within_scope->decl;
                oc_assert(struct_scope->base.kind == CODE_KIND_STRUCT);

                LL_Type_Struct* struct_type = (LL_Type_Struct*)ll_get_base_type(struct_scope->declared_type);
                oc_assert(struct_type->base.kind == LL_TYPE_STRUCT);

                spirv_calculate_struct_offsets(&struct_type->base);
                Code_Variable_Declaration* field_decl = CODE_AS(field_scope, Code_Variable_Declaration);
                oc_assert(field_decl->base.base.kind == CODE_KIND_VARIABLE_DECLARATION);

                typeof(b->current_access_chain_tmp) old_chain_access = b->current_access_chain_tmp;

                if (op->left->type->kind == LL_TYPE_POINTER) {
                    b->current_access_chain_tmp = NULL;
                    result = spirv_generate_expression(cc, b, op->left, false);

                    // return result;
                    b->current_access_chain_tmp = old_chain_access;
                    if (b->current_access_chain_tmp) {
                        oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, ((LL_Type_Pointer*)op->left->type)->spirv_storage_class);
                        oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, result);
                    }
                } else {
                    result = spirv_generate_expression(cc, b, op->left, true);
                    b->current_access_chain_tmp = old_chain_access;

                    // if (op->left->kind != CODE_KIND_INDEX && !(op->left->kind == CODE_KIND_BINARY_OP && CODE_AS(op->left, Code_Operation)->op.kind == '.')) {
                    if (b->current_access_chain_tmp && b->current_access_chain_tmp->count == 0) {
                        // base case. the commented line above was the old base case, but i think the current condition makes more sense and is more robust
                        // @TODO: don't hard code function sc here
                        oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, SpvStorageClassFunction);
                        oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, result);
                    }
                }

                SpvId member_id = spirv_generate_constant(cc, b, cc->typer->ty_uint32, &field_decl->ordered_index);
                oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, member_id);

                SpvStorageClass load_sc = 0;
                if (!had_access_chain) {
                    // @Robustness: wow this is messy, we just assume storage class is in current access chain
                    oc_assert(b->current_access_chain_tmp->count >= 2);
                    load_sc = *b->current_access_chain_tmp->items;

                    SpvId ptr_typeid = spirv_get_pointer_type(cc, b, expr->type, load_sc);
                    result = emit_rev(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, SpvOpAccessChain, ptr_typeid, b->current_access_chain_tmp->items + 1, b->current_access_chain_tmp->count - 1);

                    oc_arena_restore(&cc->tmp_arena, save);
                    b->current_access_chain_tmp = NULL;
                }

                if (!lvalue) {
                    if (load_sc == SpvStorageClassPhysicalStorageBuffer) {
                        result = emit_op_dst(SpvOpLoad, typeid, result, SpvMemoryAccessAlignedMask, 16);
                    } else {
                        result = emit_op_dst(SpvOpLoad, typeid, result);
                    }
                }

                return result;
            } else {
                LL_Type* base_type = ll_get_base_type(op->left->type);
                if (base_type->kind == LL_TYPE_ARRAY && op->base.has_const) {
                    if (lvalue) oc_todo("handle invalid lvalue");

                    result = spirv_generate_constant(cc, b, op->base.type, &op->base.const_value.as_u64);
                    return result;
                }
            }
        } break;
        case '*':
            if (ll_type_is_vector_or_matrix(op->left->type) && ll_type_is_vector_or_matrix(op->right->type)) {
                // vector/matrix mul
                r2 = spirv_generate_expression(cc, b, op->right, false);
                r1 = spirv_generate_expression(cc, b, op->left, false);
                if (ll_type_is_matrix(op->left->type) && ll_type_is_matrix(op->right->type)) {
                    result = emit_op_dst(SpvOpMatrixTimesMatrix, typeid, r1, r2);
                } else if (ll_type_is_vector(op->left->type) && ll_type_is_matrix(op->right->type)) {
                    result = emit_op_dst(SpvOpVectorTimesMatrix, typeid, r1, r2);
                } else if (ll_type_is_matrix(op->left->type) && ll_type_is_vector(op->right->type)) {
                    result = emit_op_dst(SpvOpMatrixTimesVector, typeid, r1, r2);
                } else oc_assert(false);
                return result;
            } else if (ll_type_is_vector_or_matrix(op->left->type) || ll_type_is_vector_or_matrix(op->right->type)) {
                // mul by scalar
                r2 = spirv_generate_expression(cc, b, op->right, false);
                r1 = spirv_generate_expression(cc, b, op->left, false);

                if (ll_type_is_vector(op->left->type)) {
                    result = emit_op_dst(SpvOpVectorTimesScalar, typeid, r1, r2);
                } else if (ll_type_is_matrix(op->left->type)) {
                    result = emit_op_dst(SpvOpMatrixTimesScalar, typeid, r1, r2);
                } else oc_assert(false);
                return result;
            }
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFMul : SpvOpIMul;
            break;
        case '+': spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFAdd : SpvOpIAdd; break;
        case '-': spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFSub : SpvOpISub; break;
        case '/': spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFDiv : (expr->type->kind == LL_TYPE_INT) ? SpvOpSDiv : SpvOpUDiv; break;
        case '%': spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFMod : (expr->type->kind == LL_TYPE_INT) ? SpvOpSMod : SpvOpUMod; break;

        case '<':
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFOrdLessThan : (expr->type->kind == LL_TYPE_INT) ? SpvOpSLessThan : SpvOpULessThan;
            goto DO_BIN_OP_BOOLEAN;
        case LL_TOKEN_KIND_LTE:
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFOrdLessThanEqual : (expr->type->kind == LL_TYPE_INT) ? SpvOpSLessThanEqual : SpvOpULessThanEqual;
            goto DO_BIN_OP_BOOLEAN;
        case '>':
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFOrdGreaterThan : (expr->type->kind == LL_TYPE_INT) ? SpvOpSGreaterThan : SpvOpUGreaterThan;
            goto DO_BIN_OP_BOOLEAN;
        case LL_TOKEN_KIND_GTE:
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFOrdGreaterThanEqual : (expr->type->kind == LL_TYPE_INT) ? SpvOpSGreaterThanEqual : SpvOpUGreaterThanEqual;
            goto DO_BIN_OP_BOOLEAN;
        case LL_TOKEN_KIND_EQUALS:
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFOrdEqual : SpvOpIEqual;
            goto DO_BIN_OP_BOOLEAN;
        case LL_TOKEN_KIND_NEQUALS:
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFOrdNotEqual : SpvOpINotEqual;
            goto DO_BIN_OP_BOOLEAN;
DO_BIN_OP_BOOLEAN:
            r2 = spirv_generate_expression(cc, b, op->right, false);
            r1 = spirv_generate_expression(cc, b, op->left, false);

            result = emit_op_dst(spv_opcode, typeid, r1, r2);
            return result;
        
        // case LL_TOKEN_KIND_OR: {
        //     r1 = spirv_generate_expression(cc, b, op->left, false);
        //     r1 = IR_APPEND_OP_DST(LL_IR_OPCODE_TEST, op->left->type, r1);
        //     r2 = spirv_generate_expression(cc, b, op->right, false);
        //     r2 = IR_APPEND_OP_DST(LL_IR_OPCODE_TEST, op->right->type, r2);
        //     result = IR_APPEND_OP_DST(LL_IR_OPCODE_OR, expr->type, r1, r2);
        //     return result;
        // } break;
        // case LL_TOKEN_KIND_AND: {
        //     r1 = spirv_generate_expression(cc, b, op->left, false);
        //     r1 = IR_APPEND_OP_DST(LL_IR_OPCODE_TEST, op->left->type, r1);
        //     r2 = spirv_generate_expression(cc, b, op->right, false);
        //     r2 = IR_APPEND_OP_DST(LL_IR_OPCODE_TEST, op->right->type, r2);
        //     result = IR_APPEND_OP_DST(LL_IR_OPCODE_AND, expr->type, r1, r2);
        //     return result;
        // } break;


        case LL_TOKEN_KIND_ASSIGN_PERCENT:
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFMod : (expr->type->kind == LL_TYPE_INT) ? SpvOpSMod : SpvOpUMod;
            goto DO_BIN_OP_ASSIGN_OP;
        case LL_TOKEN_KIND_ASSIGN_DIVIDE:
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFDiv : (expr->type->kind == LL_TYPE_INT) ? SpvOpSDiv : SpvOpUDiv;
            goto DO_BIN_OP_ASSIGN_OP;
        case LL_TOKEN_KIND_ASSIGN_TIMES:
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFMul : SpvOpIMul;
            goto DO_BIN_OP_ASSIGN_OP;
        case LL_TOKEN_KIND_ASSIGN_MINUS:
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFSub : SpvOpISub;
            goto DO_BIN_OP_ASSIGN_OP;
        case LL_TOKEN_KIND_ASSIGN_PLUS:
            spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFAdd : SpvOpIAdd;
DO_BIN_OP_ASSIGN_OP:
            r2 = spirv_generate_expression(cc, b, op->right, false);
            r2 = spirv_generate_cast_if_needed(cc, b, expr->type, r2, op->right->type);

            r1 = spirv_generate_expression(cc, b, op->left, false);
            r1 = spirv_generate_cast_if_needed(cc, b, expr->type, r1, op->left->type);

            r1 = emit_op_dst(spv_opcode, typeid, r1, r2);

            result = spirv_generate_expression(cc, b, op->left, true);
            emit_op(SpvOpStore, result, r1);
            return r1;
        case '=':
            result = spirv_generate_expression(cc, b, op->left, true);
            r2 = spirv_generate_expression(cc, b, op->right, false);
            r2 = spirv_generate_cast_if_needed(cc, b, expr->type, r2, op->right->type);
            emit_op(SpvOpStore, result, r2);
            return r2;

#pragma GCC diagnostic pop
        default:
            oc_assert(false);
            result = 0;
            break;
        }

        r1 = spirv_generate_expression(cc, b, op->left, false);
        r1 = spirv_generate_cast_if_needed(cc, b, expr->type, r1, op->left->type);

        r2 = spirv_generate_expression(cc, b, op->right, false);
        r2 = spirv_generate_cast_if_needed(cc, b, expr->type, r2, op->right->type);

        result = emit_op_dst(spv_opcode, typeid, r1, r2);
    } break;

    case CODE_KIND_SWIZZLE: {
        Code_Swizzle* swizzle = CODE_AS(expr, Code_Swizzle);
        r1 = spirv_generate_expression(cc, b, swizzle->vector, false);
        r2 = r1;

        struct {
            SpvId v1;
            SpvId v2;
            SpvId selectors[16];
        } operands;
        SpvId numbers[16];
        uint32_t numbers_count = 0;

        for (uint32_t i = 0; i < swizzle->count; ++i) {
            if (CODE_SWIZZLE_IS_COMPONENT(swizzle->components[i])) {
                operands.selectors[i] = CODE_SWIZZLE_GET_COMPONENT(swizzle->components[i]);
            } else {
                operands.selectors[i] = swizzle->vector->type->columns + numbers_count;

                union {
                    float f;
                    double d;
                    uint32_t encoded_value[0];
                } value;
                if (swizzle->vector->type->kind == LL_TYPE_FLOAT) {
                    if (swizzle->vector->type->width <= 32) {
                        value.f = (float) CODE_SWIZZLE_GET_VALUE(swizzle->components[i]);
                    } else {
                        value.d = (double)CODE_SWIZZLE_GET_VALUE(swizzle->components[i]);
                    }
                } else {
                    value.encoded_value[0] = CODE_SWIZZLE_GET_VALUE(swizzle->components[i]);
                    value.encoded_value[1] = 0;
                }

                SpvId constant_id = spirv_generate_constant(cc, b, swizzle->vector->type->base_type, &value);
                numbers[numbers_count++] = constant_id;
            }
        }

        if (numbers_count == 1) {
            SpvId base_id = spirv_generate_type(cc, b, swizzle->vector->type->base_type);
            // type must be a vector so we just stick a null value here if there's only one element
            numbers[numbers_count++] = emit_type_op_dst_rev(SpvOpConstantNull, base_id);
        }

        if (numbers_count) {
            LL_Type new_type = *swizzle->vector->type;
            new_type.spirv_type = 0;
            new_type.columns = numbers_count;
            oc_assert(ll_type_is_vector(&new_type));


            // even though this could be a scalar, let's keep it a distinct type by keeping base_type with a value
            LL_Type* number_vec = ll_intern_type(cc, cc->typer, &new_type);
            SpvId number_vec_id = spirv_generate_type(cc, b, number_vec);

            r2 = emit_rev(cc, b, (typeof(b->code_header)*)&b->code_types, SpvOpConstantComposite, number_vec_id, numbers, numbers_count);
        }
        operands.v1 = r1;
        operands.v2 = r2;

        result = emit_rev(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, SpvOpVectorShuffle, typeid, &operands.v1, 2 + swizzle->count);
    } break;

    case CODE_KIND_CAST: {
        Code_Cast* cast = CODE_AS(expr, Code_Cast);
        SpvId value = spirv_generate_expression(cc, b, cast->expr, false);
        result = spirv_generate_cast_if_needed(cc, b, cast->base.type, value, cast->expr->type);
    } break;

    case CODE_KIND_INVOKE: {
        oc_assert(!lvalue);
        Code_Invoke* inv = CODE_AS(expr, Code_Invoke);

        if (inv->expr->kind == CODE_KIND_IDENT && CODE_AS(inv->expr, Code_Ident)->str.ptr == LL_KEYWORD_SIZEOF.ptr) {
            result = spirv_const_to_operand(cc, b, expr->type, expr->const_value);
            break;
        }

        if (inv->expr->type->kind == LL_TYPE_TYPE && inv->expr->has_const) {
            result = spirv_generate_vector_constructor(cc, b, inv);
            break;
        }

        LL_Ir_Operand invokee;
        if (inv->resolved_fn_inst) {
            oc_assert(false);
            // if (inv->resolved_fn_inst->ir_index == 0) {

            //     LL_Ir_Function fn = {
            //         .fn_type = inv->resolved_fn_inst->fn_type,
            //         .flags = 0,
            //     };
            //     inv->resolved_fn_inst->ir_index = ir_insert_function(cc, b, fn);
            //     oc_assert(inv->resolved_fn_inst->ir_index != 0);

            //     if (inv->resolved_fn_inst->body) {
            //         int32_t last_function = b->current_function;
            //         LL_Ir_Block_Ref last_block = b->current_block;
            //         b->current_function = inv->resolved_fn_inst->ir_index;
            //         b->current_block = fn.entry;

            //         ir_generate_statement(cc, b, inv->resolved_fn_inst->body, can_continue);
            //         if (!*can_continue) return 0;

            //         if (!b->blocks.items[b->current_block].did_branch) {
            //             IR_APPEND_OP(LL_IR_OPCODE_RET);
            //         }

            //         b->current_block = last_block;
            //         b->current_function = last_function;
            //     }
            // }

            // invokee = inv->resolved_fn_inst->ir_index;
        } else {
            invokee = spirv_generate_expression(cc, b, inv->expr, true);
        }

        LL_Type_Function* fn_type = (LL_Type_Function*)inv->expr->type;
        SpvId return_id = spirv_generate_type(cc, b, fn_type->return_type);

        SpvId opcodes[1 + inv->ordered_arguments.count];
        opcodes[0] = invokee;

        
        SpvId* arguments = opcodes + 1;
        for (size_t i = 0; i < inv->ordered_arguments.count; ++i) {
            LL_Type* parameter_type;
            bool arg_lea = false;
            bool arg_lvalue = false;

            if (i >= fn_type->parameter_count - 1 && fn_type->is_variadic) {
                parameter_type = cc->typer->ty_int32;
            } else {
                parameter_type = inv->ordered_arguments.items[i]->type;
                if (i == 0 && inv->has_this_arg) {
                    if (fn_type->parameters[i]->kind == LL_TYPE_POINTER) {
                        if (parameter_type->kind != LL_TYPE_POINTER) {
                            arg_lvalue = true;
                            parameter_type = ll_typer_get_ptr_type(cc, cc->typer, parameter_type);
                            if (inv->ordered_arguments.items[i]->kind != CODE_KIND_INDEX && inv->ordered_arguments.items[i]->kind != CODE_KIND_BINARY_OP) {
                                arg_lea = true;
                            }
                        }
                    }
                }
            }

            if (inv->fn_decl) {
                if (!inv->fn_decl->parameters.items[i].base.ident && parameter_type->kind == LL_TYPE_TYPE && inv->ordered_arguments.items[i]->has_const) {
                    // we don't generate arguments for const only operands
                    continue;
                }
            }

            LL_Ir_Operand arg_operand = spirv_generate_expression(cc, b, inv->ordered_arguments.items[i], arg_lvalue);
            if (arg_lea) {
                // arg_operand = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA, parameter_type, arg_operand);
                oc_assert(false);
            }

            arguments[i] = arg_operand;
        }

        result = emit_rev(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, SpvOpFunctionCall, return_id, opcodes, 1 + inv->ordered_arguments.count);

        break;
    }

    case CODE_KIND_ARRAY_INITIALIZER: {
        Code_Initializer* lit = CODE_AS(expr, Code_Initializer);

        SpvId value_ids[lit->count];
        memset(value_ids, 0, sizeof(SpvId) * lit->count);

        uint64_t i, k;
        for (i = 0, k = 0; i < lit->count; ++i, ++k) {
            if (lit->items[i]->kind == CODE_KIND_KEY_VALUE) {
                Code_Key_Value* kv = CODE_AS(lit->items[i], Code_Key_Value);
                SpvId value_id = spirv_generate_expression(cc, b, kv->value, false);

                if (kv->key->has_const && kv->value->has_const) {
                    k = kv->key->const_value.as_u64;
                    value_ids[k] = value_id;
                } else {
                    oc_assert(false);
                }
            } else {
                SpvId value_id = spirv_generate_expression(cc, b, lit->items[i], false);
                value_ids[k] = value_id;
            }
        }

        if (lit->base.has_const) {
            result = emit_rev(cc, b, (typeof(b->code_header)*)&b->code_types, SpvOpConstantComposite, typeid, value_ids, lit->count);
        } else {
            result = emit_rev(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, SpvOpCompositeConstruct, typeid, value_ids, lit->count);
        }
    } break;

    case CODE_KIND_INDEX: {
        Code_Slice* op = CODE_AS(expr, Code_Slice);
        typeof(*b->current_access_chain_tmp) chain_access = { 0 };

        Oc_Arena_Save save;
        bool had_access_chain = b->current_access_chain_tmp != NULL;

        SpvId lvalue_id;
        switch (op->ptr->type->kind) {
        case LL_TYPE_POINTER: {
            typeof(b->current_access_chain_tmp) old_chain_access = b->current_access_chain_tmp;
            b->current_access_chain_tmp = NULL;
            lvalue_id = spirv_generate_expression(cc, b, op->ptr, false);
            SpvId rvalue_id = spirv_generate_expression(cc, b, op->start, false);
            b->current_access_chain_tmp = old_chain_access;

            bool is_invariant;
            SpvId typeid = spirv_generate_type_with_parameters(cc, b, op->ptr->type, (Spirv_Type_Parameters) { .needs_explicit_layout = spirv_storage_class_needs_explicit(((LL_Type_Pointer*)op->ptr->type)->spirv_storage_class), .is_invariant = &is_invariant });

            result = emit_op_dst(SpvOpPtrAccessChain, typeid, lvalue_id, rvalue_id);
            if (b->current_access_chain_tmp) {
                oc_assert(lvalue);
                oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, ((LL_Type_Pointer*)op->ptr->type)->spirv_storage_class);
                oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, result);
            } else {
                if (!lvalue) {
                    result = spirv_load(cc, b, op->ptr->type, result, false);
                }
            } 

            return result;
        }
        case LL_TYPE_STRING: {
            oc_assert(false);
        } break;
        case LL_TYPE_SLICE: {
            oc_assert(false);
        } break;
        default: {
            if (!had_access_chain) {
                save = oc_arena_save(&cc->tmp_arena);
                b->current_access_chain_tmp = &chain_access;
            }

            typeof(b->current_access_chain_tmp) old_chain_access = b->current_access_chain_tmp;
            lvalue_id = spirv_generate_expression(cc, b, op->ptr, true);
            b->current_access_chain_tmp = old_chain_access;
        } break;
        }

        // if (op->ptr->kind != CODE_KIND_INDEX && !(op->ptr->kind == CODE_KIND_BINARY_OP && CODE_AS(op->ptr, Code_Operation)->op.kind == '.')) {
        if (b->current_access_chain_tmp && b->current_access_chain_tmp->count == 0) {
            // base case. the commented line above was the old base case, but i think the current condition makes more sense and is more robust

            // @TODO: don't hard code function sc here
            oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, SpvStorageClassFunction);
            oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, lvalue_id);
        }

        // save the chain access, in the case the index is also a chain access
        typeof(b->current_access_chain_tmp) old_chain_access = b->current_access_chain_tmp;
        SpvId rvalue_id = spirv_generate_expression(cc, b, op->start, false);
        oc_array_append(&cc->tmp_arena, b->current_access_chain_tmp, rvalue_id);
        b->current_access_chain_tmp = old_chain_access;

        SpvStorageClass load_sc = 0;
        if (!had_access_chain) {
            // @Robustness: wow this is messy, we just assume storage class is in current access chain
            oc_assert(b->current_access_chain_tmp->count >= 2);
            load_sc = *b->current_access_chain_tmp->items;

            SpvId ptr_typeid = spirv_get_pointer_type(cc, b, expr->type, load_sc);
            result = emit_rev(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, SpvOpAccessChain, ptr_typeid, b->current_access_chain_tmp->items + 1, b->current_access_chain_tmp->count - 1);

            oc_arena_restore(&cc->tmp_arena, save);
            b->current_access_chain_tmp = NULL;
        }

        if (!lvalue) {
            spirv_load(cc, b, op->ptr->type, result, false);
        }
    } break;

    case CODE_KIND_BREAK: {
        Code_Control_Flow* cf = CODE_AS(expr, Code_Control_Flow);
        if (cf->expr) {
            result = spirv_generate_expression(cc, b, cf->expr, false);
            oc_assert(false);
            // IR_APPEND_OP(LL_IR_OPCODE_STORE, cf->referenced_scope->break_value, result);
        }

        emit_op(SpvOpBranch, cf->referenced_scope->break_block_ref);
        emit_op_dst_noarg(SpvOpLabel);
        return 0;
    }

    case CODE_KIND_CONTINUE: {
        Code_Control_Flow* cf = CODE_AS(expr, Code_Control_Flow);
        emit_op(SpvOpBranch, cf->referenced_scope->continue_block_ref);
        emit_op_dst_noarg(SpvOpLabel);
        return 0;
    }

    case CODE_KIND_RETURN: {
        Code_Control_Flow* cf = CODE_AS(expr, Code_Control_Flow);
        if (cf->expr) {


            Code_Scope* ref_scope = cf->referenced_scope;
            Code_Function_Declaration* fn = (Code_Function_Declaration*)ref_scope->decl;
            oc_assert(fn->base.base.kind == CODE_KIND_FUNCTION_DECLARATION);

            bool is_main = string_eql(fn->base.ident->str, lit("main"));
            if (is_main) {
                Code_Scope* scope;
                LL_Type* base_type = ll_get_base_type_and_scope(fn->base.type->type, &scope);

                if (base_type->kind == LL_TYPE_STRUCT) {
                    result = spirv_generate_expression(cc, b, cf->expr, true);

                    Code_Struct* struct_decl = (Code_Struct*)scope->decl;
                    oc_assert(struct_decl->base.base.kind == CODE_KIND_STRUCT);

                    for (uint32_t i = 0; i < struct_decl->block->statements.count; ++i) {
                        Code_Variable_Declaration* var_decl = (Code_Variable_Declaration*)struct_decl->block->statements.items[i];
                        if (var_decl->base.base.kind != CODE_KIND_VARIABLE_DECLARATION) continue;

                        SpvId field_ptr_type_id = spirv_get_pointer_type(cc, b, var_decl->base.ident->base.type, SpvStorageClassFunction);
                        SpvId field_type_id = spirv_generate_type(cc, b, var_decl->base.ident->base.type);
                        SpvId field_index = spirv_generate_constant(cc, b, cc->typer->ty_uint32, &var_decl->ordered_index);
                        SpvId field_id = emit_op_dst(SpvOpAccessChain, field_ptr_type_id, result, field_index);
                            field_id = emit_op_dst(SpvOpLoad, field_type_id, field_id); // @TODO: support other ptr storage classes
                        emit_op(SpvOpStore, var_decl->ir_index, field_id);
                    }
                } else {
                    result = spirv_generate_expression(cc, b, cf->expr, false);
                    oc_assert(b->output_variable_ids.count > 0);
                    emit_op(SpvOpStore, b->output_variable_ids.items[0], result);
                }

                emit_op(SpvOpReturn);
            } else {
                result = spirv_generate_expression(cc, b, cf->expr, false);
                emit_op(SpvOpReturnValue, result);
            }            
        } else {
            emit_op(SpvOpReturn);
        }
        FUNCTION()->function_did_return = true;
        return 0;
    }

    case CODE_KIND_IF: {
        Code_If* iff = CODE_AS(expr, Code_If);

        SpvId cond_id = spirv_generate_expression(cc, b, iff->cond, false);
        uint64_t zero_value = 0;
        if (iff->cond->type->kind != LL_TYPE_ANYBOOL) {
            SpvId zero = spirv_generate_constant(cc, b, iff->cond->type, &zero_value);
            cond_id = emit_op_dst(SpvOpINotEqual, b->bool_id, cond_id, zero);
        }

        SpvId merge_block_id = reserve_id();
        SpvId then_block_id = iff->body ? reserve_id() : merge_block_id;
        SpvId else_block_id = iff->else_clause ? reserve_id() : merge_block_id;

        emit_op(SpvOpSelectionMerge, merge_block_id, SpvSelectionControlMaskNone);
        emit_op(SpvOpBranchConditional, cond_id, then_block_id, else_block_id);

        if (iff->body) {
            emit_debug_name(then_block_id, lit("then_block"));
            emit_op(SpvOpLabel, then_block_id);
            spirv_generate_statement(cc, b, iff->body);
            emit_op(SpvOpBranch, merge_block_id);
        } else {
            // FUNCTION()->code.items[patch_then] = ;

            // emit_op(SpvOpBranch, 0xcccccccc);
            // patch_merge = FUNCTION()->code.count;
        }

        if (iff->else_clause) {
            emit_debug_name(else_block_id, lit("else_block"));
            emit_op(SpvOpLabel, else_block_id);
            spirv_generate_statement(cc, b, iff->else_clause);
            emit_op(SpvOpBranch, merge_block_id);
        }

        emit_debug_name(merge_block_id, lit("if_merge_block"));
        emit_op(SpvOpLabel, merge_block_id);

        return 0;
    } break;

    case CODE_KIND_WHILE:
    case CODE_KIND_FOR: {
        Code_Loop* loop = CODE_AS(expr, Code_Loop);
        if (loop->init) {
            spirv_generate_statement(cc, b, loop->init);
        }

        SpvId loop_merge_block = reserve_id();
        SpvId body_block = reserve_id();
        SpvId cond_block = loop->cond ? reserve_id() : body_block;
        SpvId merge_block_id = reserve_id();
        SpvId update_block_id = loop->update ? reserve_id() : cond_block;

        if (loop->body && loop->body->kind == CODE_KIND_BLOCK) {
            CODE_AS(loop->body, Code_Scope)->block_ref = body_block;
        }

        // if (loop->base.type) {
        //     Code_Ident* block_ident = oc_arena_alloc(&cc->arena, sizeof(Code_Ident));
        //     block_ident->base.type = expr->type;
        //     block_ident->str = oc_sprintf(&cc->arena, "block_result\n");
        //     LL_Ir_Local var = {
        //         .ident = block_ident,
        //     };
        //     uint32_t index = FUNCTION()->locals.count;
        //     oc_array_append(&cc->arena, &FUNCTION()->locals, var);
        //     loop->for_scope->break_value = LL_IR_OPERAND_LOCAL_BIT | index;
        // }
        if (loop->for_scope) {
            loop->for_scope->break_block_ref = merge_block_id;
            loop->for_scope->continue_block_ref = update_block_id;
        }

        emit_op(SpvOpBranch, loop_merge_block);
        emit_op(SpvOpLabel, loop_merge_block);
        emit_op(SpvOpLoopMerge, merge_block_id, update_block_id, SpvLoopControlMaskNone);
        emit_op(SpvOpBranch, cond_block);

        if (loop->cond) {
            emit_debug_name(cond_block, lit("loop_condition_block"));
            emit_op(SpvOpLabel, cond_block);
            result = spirv_generate_expression(cc, b, loop->cond, false);
            emit_op(SpvOpBranchConditional, result, body_block, merge_block_id);
        }

        emit_op(SpvOpLabel, body_block);
        emit_debug_name(body_block, lit("loop_body_block"));
        if (loop->body) {
            spirv_generate_statement(cc, b, loop->body);
        }

        if (loop->update) {
            emit_debug_name(update_block_id, lit("loop_update_block"));
            emit_op(SpvOpBranch, update_block_id);
            emit_op(SpvOpLabel, update_block_id);
            spirv_generate_expression(cc, b, loop->update, false);
        }
        emit_op(SpvOpBranch, loop_merge_block);

        emit_debug_name(merge_block_id, lit("loop_done_block"));
        emit_op(SpvOpLabel, merge_block_id);

        // if (loop->base.type) {
        //     return IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, loop->base.type, loop->for_scope->break_value);
        // } else {
        //     return 0;
        // }
    } break;

    }
    return result;
}

// This is a thing in case we want to intern pointer types in the future.
// Right now we don't for performace (yay saving .00001 ms)
SpvId spirv_get_pointer_type(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type, SpvStorageClass storage_class) {
    LL_Type* ptr_type = ll_typer_get_ptr_type_with_storage_class(cc, cc->typer, type, storage_class);
    SpvId result = spirv_generate_type(cc, b, ptr_type);
    // result = emit_type_op_dst(SpvOpTypePointer, storage_class, result);
    return result;
}

SpvId spirv_generate_type(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type) {
    bool is_invariant;
    Spirv_Type_Parameters parameters = { .is_invariant = &is_invariant };
    SpvId result = spirv_generate_type_with_parameters(cc, b, type, parameters);
    return result;
}

SpvId spirv_generate_type_with_parameters(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type, Spirv_Type_Parameters parameters) {
    if (!type) {
        return 0;
    }
    *parameters.is_invariant = type->explicit_spirv_type == type->spirv_type;
    if (parameters.needs_explicit_layout) {
        if (type->explicit_spirv_type) return type->explicit_spirv_type;
    } else {
        if (type->spirv_type) return type->spirv_type;
    }
    SpvId result = 0;

    // if a type's spirv_type and explicit_spirv_type are the same
    bool is_invariant = false;

    if (type->rows > 1 || type->columns > 1) {
        result = spirv_generate_type_with_parameters(cc, b, type->base_type, parameters);
        oc_assert(*parameters.is_invariant);
        is_invariant = true;

        if (type->rows > 1) {
            oc_assert(type->columns > 1);
            LL_Type* new_type = ll_typer_get_vector_type(cc, cc->typer, type->base_type, 1, type->rows);
            result = spirv_generate_type_with_parameters(cc, b, new_type, parameters);
            result = emit_type_op_dst(SpvOpTypeMatrix, result, type->columns);
        } else if (type->columns > 1) {
            result = emit_type_op_dst(SpvOpTypeVector, result, type->columns);
        }
    } else {
        // leaf types
        switch (type->kind) {
        case LL_TYPE_VOID:
            is_invariant = true;
            result = emit_type_op_dst(SpvOpTypeVoid);
            break;
        case LL_TYPE_INT:
            is_invariant = true;
            result = emit_type_op_dst(SpvOpTypeInt, type->width, 1);
            break;
        case LL_TYPE_ANYINT:
            is_invariant = true;
            result = spirv_generate_type_with_parameters(cc, b, cc->typer->ty_int32, parameters);
            break;
        case LL_TYPE_UINT:
            is_invariant = true;
            result = emit_type_op_dst(SpvOpTypeInt, type->width, 0);
            break;
        case LL_TYPE_FLOAT:
            is_invariant = true;
            result = emit_type_op_dst(SpvOpTypeFloat, type->width);
            break;
        case LL_TYPE_BOOL: {
            is_invariant = true;
            LL_Type new_type = *type;
            new_type.kind = LL_TYPE_UINT;
            LL_Type* gen_type = ll_intern_type(cc, cc->typer, &new_type);
            result = spirv_generate_type_with_parameters(cc, b, gen_type, parameters);
            break;

        }
        case LL_TYPE_ANYBOOL:
            is_invariant = true;
            result = b->bool_id;
            break;
        case LL_TYPE_POINTER: {
            LL_Type_Pointer* ptr = (LL_Type_Pointer*)type;
            LL_Backend_Layout layout = spirv_get_layout(ptr->element_type);
            // bool explicit = parameters.needs_explicit_layout;
            if (spirv_storage_class_needs_explicit(ptr->spirv_storage_class)) {
                parameters.needs_explicit_layout = true;
                if (type->explicit_spirv_type) return type->explicit_spirv_type;
            } else {
                parameters.needs_explicit_layout = false;
                if (type->spirv_type) return type->spirv_type;
            }
            SpvId element_type = spirv_generate_type_with_parameters(cc, b, ptr->element_type, parameters);
            result = emit_type_op_dst(SpvOpTypePointer, ptr->spirv_storage_class, element_type);
            if (parameters.needs_explicit_layout) {
                emit_annotation_op(SpvOpDecorate, result, SpvDecorationArrayStride, max(layout.size, layout.alignment));
            }
        } break;
        case LL_TYPE_FUNCTION: {
            LL_Type_Function* fn_type = (LL_Type_Function*)type;
            b->temp.count = 0;
            SpvId return_type_id = spirv_generate_type_with_parameters(cc, b, fn_type->return_type, parameters);

            SpvId operands[1 + fn_type->parameter_count];
            operands[0] = return_type_id;

            for (size_t i = 0; i < fn_type->parameter_count; ++i) {
                operands[i + 1] = spirv_generate_type_with_parameters(cc, b, fn_type->parameters[i], parameters);
            }

            result = emit(cc, b, (typeof(b->code_header)*)&b->code_types, SpvOpTypeFunction, operands, 1 + fn_type->parameter_count);
            is_invariant = true;
        } break;
        case LL_TYPE_ARRAY: {
            LL_Type_Array* array = (LL_Type_Array*)type;
            SpvId element_typeid = spirv_generate_type_with_parameters(cc, b, array->element_type, parameters);
            is_invariant = *parameters.is_invariant;
            SpvId size_id;
            if (array->base.width > 0xFFFFFFFF) {
                size_id = spirv_generate_constant(cc, b, cc->typer->ty_uint32, &array->base.width);
            } else {
                size_id = spirv_generate_constant(cc, b, cc->typer->ty_uint64, &array->base.width);
            }
            result = emit_type_op_dst(SpvOpTypeArray, element_typeid, size_id);
        } break;
        case LL_TYPE_STRUCT: {
            LL_Type_Struct* struc = (LL_Type_Struct*)type;
            spirv_calculate_struct_offsets(type);
            SpvId member_types[struc->field_count];

            result = reserve_id();
            if (parameters.needs_explicit_layout) {
                type->explicit_spirv_type = result;
            } else {
                type->spirv_type = result;
            }

            for (uint32_t i = 0; i < struc->field_count; ++i) {
                member_types[i] = spirv_generate_type_with_parameters(cc, b, struc->fields[i], parameters);
                if (parameters.needs_explicit_layout) emit_annotation_op(SpvOpMemberDecorate, result, i, SpvDecorationOffset, struc->offsets[i]);
                if (ll_type_is_matrix(struc->fields[i])) {
                    // emit_annotation_op(SpvOpMemberDecorate, result, i, SpvDecorationRowMajor);
                    LL_Backend_Layout layout = spirv_get_layout(struc->fields[i]);
                    emit_annotation_op(SpvOpMemberDecorate, result, i, SpvDecorationMatrixStride, layout.alignment);
                }
            }

            oc_array_append(&cc->arena, &b->code_types, (SpvOpTypeStruct) | ((2 + struc->field_count) << 16));
            oc_array_append(&cc->arena, &b->code_types, result);
            oc_array_append_many(&cc->arena, &b->code_types, member_types, struc->field_count);
        } break;
        case LL_TYPE_NAMED: {
            LL_Type_Named* named = (LL_Type_Named*)type;
            result = spirv_generate_type_with_parameters(cc, b, named->actual_type, parameters);
            is_invariant = *parameters.is_invariant;
            emit_debug_name(result, named->scope->decl->ident->str);
        } break;
        default:
            printf("Unhandled type: %d\n", type->kind);
            break;
        }
    }

    if (result == 0) {
        print("is zero\n");
    }

    *parameters.is_invariant = is_invariant;
    if (is_invariant) {
        type->explicit_spirv_type = result;
        type->spirv_type = result;
    } else {
        if (parameters.needs_explicit_layout) {
            type->explicit_spirv_type = result;
        } else {
            type->spirv_type = result;
        }
    }
    return result;
}


// For now, this assumes std430
LL_Backend_Layout spirv_get_layout(LL_Type* ty) {
    LL_Backend_Layout sub_layout;
    switch (ty->kind) {
    case LL_TYPE_INT:
    case LL_TYPE_UINT:
    case LL_TYPE_CHAR:
    case LL_TYPE_FLOAT:
        sub_layout = (LL_Backend_Layout) { .size = ty->width / 8 * ty->rows * ty->columns, .alignment = ty->width / 8 };
        switch (ty->columns) {
        case 2: sub_layout.alignment *= 2; break;
        case 3:
        case 4: sub_layout.alignment *= 4; break;
        default: break;
        }
        // column-major matrices are treated as an array of columns, so no need to do anything else here
        return sub_layout;
    case LL_TYPE_POINTER: return (LL_Backend_Layout) { .size = 8, .alignment = 8 };
    case LL_TYPE_ARRAY: {
        sub_layout = spirv_get_layout(((LL_Type_Array*)ty)->element_type);
        return (LL_Backend_Layout) { .size = max(sub_layout.size, sub_layout.alignment) * ty->width, .alignment = sub_layout.alignment };
    } break;
    case LL_TYPE_STRING:
    case LL_TYPE_SLICE: {
        return (LL_Backend_Layout) { .size = 16, .alignment = 8 };
    } break;
    case LL_TYPE_STRUCT: {
        LL_Type_Struct* struct_type = (LL_Type_Struct*)ty;
        spirv_calculate_struct_offsets(ty);
        if (struct_type->field_count == 0) return (LL_Backend_Layout) { .size = 0, .alignment = 1 };

        sub_layout = spirv_get_layout(struct_type->fields[struct_type->field_count - 1]);
        size_t size = struct_type->offsets[struct_type->field_count - 1] + max(sub_layout.size, sub_layout.alignment);
        size = oc_align_forward(size, struct_type->base.struct_alignment);

        return (LL_Backend_Layout) { .size = size, .alignment = struct_type->base.struct_alignment };
    } break;
    case LL_TYPE_NAMED: {
        return spirv_get_layout(((LL_Type_Named*)ty)->actual_type);
    } break;
    default: return (LL_Backend_Layout) { .size = 0, .alignment = 1 };
    }
}


void spirv_calculate_struct_offsets(LL_Type* type) {
    if (type->kind != LL_TYPE_STRUCT) return;

    LL_Type_Struct* struct_type = (LL_Type_Struct*)type;
    if (struct_type->has_offsets) return;
    
    uint32_t offset = 0;
    struct_type->base.struct_alignment = 1;
    for (uint32_t i = 0; i < struct_type->field_count; ++i) {
        spirv_calculate_struct_offsets(struct_type->fields[i]);

        LL_Backend_Layout l = spirv_get_layout(struct_type->fields[i]);
        if (l.alignment > struct_type->base.struct_alignment) {
            struct_type->base.struct_alignment = l.alignment;
        }
        offset = oc_align_forward(offset, l.alignment);
        struct_type->offsets[i] = offset;
        offset = oc_align_forward(offset + max(l.size, l.alignment), l.alignment);
    }

    struct_type->has_offsets = true;
}

#undef FUNCTION