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

    size_t current_function;
    bool do_debug;
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



#define emit_type_op_dst(op, ...) emit(cc, b, (typeof(b->code_header)*)&b->code_types, (op), ((uint32_t[]) { __VA_ARGS__ }), get_size_by_array(__VA_ARGS__))
#define emit_type_op_dst_rev(op, typeid, ...) emit_rev(cc, b, (typeof(b->code_header)*)&b->code_types, (op), (typeid), ((uint32_t[]) { __VA_ARGS__ }), get_size_by_array(__VA_ARGS__))
#define emit_op_dst(op, typeid, ...)      emit_rev(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, (op), (typeid), ((uint32_t[]) { __VA_ARGS__ }), get_size_by_array(__VA_ARGS__))

void spirv_init(Compiler_Context* cc, LL_Backend_Spirv* b) {
    memset(b, 0, sizeof(*b));
    b->next_result_id = 1;
    emit_header_raw(SpvMagicNumber);
    emit_header_raw(SpvVersion);
    emit_header_raw(0);
    emit_header_raw(0);
    emit_header_raw(0);
    emit_header_op(SpvOpCapability, SpvCapabilityShader);
    emit_header_op(SpvOpMemoryModel, SpvAddressingModelPhysical64, SpvMemoryModelVulkan);
    oc_array_extend_count_unint(&cc->arena, &b->functions, 1);
    b->do_debug = true;
}

bool spirv_write_to_file(Compiler_Context* cc, LL_Backend_Spirv* b, char* filepath) {
    (void)cc;
    b->code_header.items[3] = b->next_result_id;
    FILE* fptr;
    if (fopen_s(&fptr, filepath, "wb")) {
        eprint("Unable to open output file: %s\n", filepath);
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

SpvId spirv_generate_type(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type);
SpvId spirv_generate_expression(Compiler_Context* cc, LL_Backend_Spirv* b, Code* expr, bool lvalue);

void spirv_generate_statement(Compiler_Context* cc, LL_Backend_Spirv* b, Code* stmt) {
    printf(" srtrtotjlk: %d\n", stmt->kind);

    switch (stmt->kind) {
    case CODE_KIND_BLOCK: {
        Code_Scope* blk = CODE_AS(stmt, Code_Scope);
        for (size_t i = 0; i < blk->declarations.capacity; ++i) {
            if (blk->declarations.entries[i].filled) {
                Code_Declaration* decl = blk->declarations.entries[i]._value;
                if (decl->base.kind == CODE_KIND_VARIABLE_DECLARATION) {
                    Code_Variable_Declaration* var_decl = (Code_Variable_Declaration*)decl;
                    SpvId typeid = spirv_generate_type(cc, b, decl->type->type);
                    // SpvId variable_id = emit_rev(cc, b, (typeof(b->code_header)*)&b->code_types, SpvOpVariable, typeid, NULL, 0);
                    SpvId variable_id = emit_op_dst(SpvOpVariable, typeid, SpvStorageClassFunction);
                    var_decl->ir_index = variable_id;
                    emit_debug_name(variable_id, var_decl->base.ident->str);
                } else {
                    spirv_generate_statement(cc, b, (Code*)blk->declarations.entries[i]._value);
                }
            }
        }

        for (size_t i = 0; i < blk->statements.count; ++i) {
            spirv_generate_statement(cc, b, blk->statements.items[i]);
        }
    } break;
    case CODE_KIND_VARIABLE_DECLARATION: {
        Code_Variable_Declaration* var_decl = CODE_AS(stmt, Code_Variable_Declaration);
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

        LL_Type_Function* fn_type = (LL_Type_Function*)fn_decl->base.ident->base.type;
        SpvId return_type = spirv_generate_type(cc, b, fn_type->return_type);
        SpvId spirv_function_type = spirv_generate_type(cc, b, (LL_Type*)fn_type);

        SpvId function_id = emit_op_dst(SpvOpFunction, return_type, 0, spirv_function_type);
        fn->spv_id = function_id;
        emit_debug_name(function_id, fn_decl->base.ident->str);

        if (fn_decl->body) spirv_generate_statement(cc, b, (Code*)fn_decl->body);
        if (!FUNCTION()->function_did_return) {
            emit_op(SpvOpReturn);
        }
        emit_op(SpvOpFunctionEnd);

        b->current_function = last_function;
    } break;
    default: spirv_generate_expression(cc, b, stmt, false);
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

    switch (from_type->kind) {
    case LL_TYPE_INT:
        switch (to_type->kind) {
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
                if (from_type->width == to_type->width) return from;
                break;
            default: oc_todo("handle cast types to {}\n", (int)to_type->kind); break;
        }
        break;
    case LL_TYPE_UINT:
        switch (to_type->kind) {
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
                if (from_type->width == to_type->width) return from;
                break;
            default: oc_todo("handle cast types to {}\n", (int)to_type->kind); break;
        }
        break;
    default: oc_todo("handle cast types from\n"); return from;
    }

    // return IR_APPEND_OP_DST(LL_IR_OPCODE_CAST, to_type, from);
    // return emit_op_dst(O)
    oc_assert(false);
}



SpvId spirv_generate_expression(Compiler_Context* cc, LL_Backend_Spirv* b, Code* expr, bool lvalue) {
    SpvId result = 0;
    SpvId typeid = spirv_generate_type(cc, b, expr->type);
    SpvId r1, r2;

    switch (expr->kind) {
    case CODE_KIND_LITERAL_INT: {
        Code_Literal* lit = CODE_AS(expr, Code_Literal);
        if (expr->type->kind == LL_TYPE_FLOAT) {
            if (expr->type->width <= 32) {
                union {
                    float f;
                    uint32_t i;
                } a = { .f = (float)lit->u64 };
                result = emit_op_dst(SpvOpConstant, typeid, a.i);
            } else if (expr->type->width <= 64) {
                union {
                    double f;
                    uint32_t i[2];
                } a = { .f = (double)lit->u64 };
                result = emit_op_dst(SpvOpConstant, typeid, a.i[0], a.i[1]);
            } else oc_todo("bigger types");
        } else {
            if (expr->type->width <= 32) {
                result = emit_op_dst(SpvOpConstant, typeid, (uint32_t)lit->u64);
            } else {
                result = emit_op_dst(SpvOpConstant, typeid, (uint32_t)(lit->u64 & 0xFFFFFFFF), (uint32_t)(lit->u64 >> 32));
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
            result = emit_op_dst(SpvOpConstant, typeid, a.i);
        } else if (expr->type->width <= 64) {
            union {
                double f;
                uint32_t i[2];
            } a = { .f = lit->f64 };
            result = emit_op_dst(SpvOpConstant, typeid, a.i[0], a.i[1]);
        } else {
            oc_assert(false);
        }
    } break;

    case CODE_KIND_IDENT: {
        Code_Ident* ident = CODE_AS(expr, Code_Ident);

        if (ident->str.ptr == LL_KEYWORD_TRUE.ptr) {
            oc_assert(!lvalue);
            result = emit_op_dst(SpvOpConstantTrue, typeid);
            break;
        } else if (ident->str.ptr == LL_KEYWORD_FALSE.ptr) {
            oc_assert(!lvalue);
            result = emit_op_dst(SpvOpConstantTrue, typeid);
            break;
        } else if (ident->str.ptr == LL_KEYWORD_NULL.ptr) {
            oc_assert(!lvalue);
            result = emit_op_dst(SpvOpConstantNull, typeid);
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
            if (!lvalue) {
                result = emit_op_dst(SpvOpLoad, typeid, result);
            }
        } break;
        case '&': {
            oc_assert(false);
            // result = spirv_generate_expression(cc, b, op->right, true);
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
            oc_assert(false);
        } break;
        case '+': spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFAdd : SpvOpIAdd; break;
        case '-': spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFSub : SpvOpISub; break;
        case '*': spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFMul : SpvOpIMul; break;
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
            spv_opcode = spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFMod : (expr->type->kind == LL_TYPE_INT) ? SpvOpSMod : SpvOpUMod;
            goto DO_BIN_OP_ASSIGN_OP;
        case LL_TOKEN_KIND_ASSIGN_DIVIDE:
            spv_opcode = spv_opcode = (expr->type->kind == LL_TYPE_FLOAT) ? SpvOpFDiv : (expr->type->kind == LL_TYPE_INT) ? SpvOpSDiv : SpvOpUDiv;
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


    case CODE_KIND_INVOKE: {
        oc_assert(!lvalue);
        Code_Invoke* inv = CODE_AS(expr, Code_Invoke);

        if (inv->expr->kind == CODE_KIND_IDENT && CODE_AS(inv->expr, Code_Ident)->str.ptr == LL_KEYWORD_SIZEOF.ptr) {
            result = spirv_const_to_operand(cc, b, expr->type, expr->const_value);
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

            // ops[offset++] = arg_operand;
            arguments[i] = arg_operand;
        }

        // emit_op_dst(SpvOpFunctionCall, return_id);
        result = emit_rev(cc, b, (typeof(b->code_header)*)&FUNCTION()->code, SpvOpFunctionCall, return_id, opcodes, 1 + inv->ordered_arguments.count);

        break;
    }

    case CODE_KIND_RETURN: {
        Code_Control_Flow* cf = CODE_AS(expr, Code_Control_Flow);
        if (cf->expr) {
            result = spirv_generate_expression(cc, b, cf->expr, false);
            emit_op(SpvOpReturnValue, result);
        } else {
            emit_op(SpvOpReturn);
        }
        FUNCTION()->function_did_return = true;
        return 0;
    }

    }
    return result;
}

SpvId spirv_generate_type(Compiler_Context* cc, LL_Backend_Spirv* b, LL_Type* type) {
    if (!type) return 0;
    if (type->spirv_type) return type->spirv_type;
    SpvId result = 0;
    switch (type->kind) {
    case LL_TYPE_VOID:
        result = emit_type_op_dst(SpvOpTypeVoid);
        break;
    case LL_TYPE_INT:
        result = emit_type_op_dst(SpvOpTypeInt, type->width, 1);
        break;
    case LL_TYPE_UINT:
        result = emit_type_op_dst(SpvOpTypeInt, type->width, 0);
        break;
    case LL_TYPE_FLOAT:
        result = emit_type_op_dst(SpvOpTypeFloat, type->width);
        break;
    case LL_TYPE_FUNCTION: {
        LL_Type_Function* fn_type = (LL_Type_Function*)type;
        b->temp.count = 0;
        SpvId return_type_id = spirv_generate_type(cc, b, fn_type->return_type);
        result = emit_type_op_dst(SpvOpTypeFunction, return_type_id);
    } break;
    default:
        printf("Unhandled type: %d\n", type->kind);
        break;
    }
    if (type->rows > 1) {
        result = emit_type_op_dst(SpvOpTypeVector, result, type->rows);
    }
    if (type->columns > 1) {
        if (type->rows == 1) {
            result = emit_type_op_dst(SpvOpTypeVector, result, 1);
        }
        result = emit_type_op_dst(SpvOpTypeMatrix, result, type->columns);
    }


    type->spirv_type = result;
    return result;
}
#undef FUNCTION