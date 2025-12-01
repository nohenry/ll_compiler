
// #include <malloc.h>
// #include <string.h>

#include "common.h"
#include "typer.h"
#include "lexer.h"
#include "eval.h"

#define create_type(type) ({\
            __typeof__(type) t = type; \
            ll_intern_type(cc, typer, &t); \
        })

#define create_scope(kind, decl) create_scope_(cc, typer, kind, (Ast_Base*)(decl))

static LL_Scope* create_scope_(Compiler_Context* cc, LL_Typer* typer, LL_Scope_Kind kind, Ast_Base* decl) {
    (void)typer;
    LL_Scope* result;
    result = oc_arena_alloc(&cc->arena, sizeof(LL_Scope));
    result->kind = kind;
    result->ident = NULL;
    result->decl = decl;
    result->next_anon = 0;
    memset(&result->children, 0, sizeof(result->children));
    return result;
}

#define create_scope_simple(kind, decl) create_scope_simple_(cc, typer, kind, (Ast_Base*)(decl))

static LL_Scope_Simple* create_scope_simple_(Compiler_Context* cc, LL_Typer* typer, LL_Scope_Kind kind, Ast_Base* decl) {
    (void)typer;
    LL_Scope_Simple* result;
    result = oc_arena_alloc(&cc->arena, sizeof(LL_Scope_Simple));
    result->kind = kind;
    result->ident = NULL;
    result->decl = decl;
    return result;
}

LL_Typer ll_typer_create(Compiler_Context* cc) {
    LL_Typer result = { 0 };
    LL_Typer* typer = &result;

    LL_Scope root_scope = {
        .kind = LL_SCOPE_KIND_PACKAGE,
    };
    result.root_scope = result.current_scope = oc_arena_dup(&cc->arena, &root_scope, sizeof(root_scope));

    result.ty_void = create_type(((LL_Type){ .kind = LL_TYPE_VOID }));

    result.ty_int8 = create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = 8 }));
    result.ty_int16 = create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = 16 }));
    result.ty_int32 = create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = 32 }));
    result.ty_int64 = create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = 64 }));
    result.ty_uint8 = create_type(((LL_Type){ .kind = LL_TYPE_UINT, .width = 8 }));
    result.ty_uint16 = create_type(((LL_Type){ .kind = LL_TYPE_UINT, .width = 16 }));
    result.ty_uint32 = create_type(((LL_Type){ .kind = LL_TYPE_UINT, .width = 32 }));
    result.ty_uint64 = create_type(((LL_Type){ .kind = LL_TYPE_UINT, .width = 64 }));
    result.ty_anyint = create_type(((LL_Type){ .kind = LL_TYPE_ANYINT }));

    result.ty_float16 = create_type(((LL_Type){ .kind = LL_TYPE_FLOAT, .width = 16 }));
    result.ty_float32 = create_type(((LL_Type){ .kind = LL_TYPE_FLOAT, .width = 32 }));
    result.ty_float64 = create_type(((LL_Type){ .kind = LL_TYPE_FLOAT, .width = 64 }));
    result.ty_anyfloat = create_type(((LL_Type){ .kind = LL_TYPE_ANYFLOAT }));

    result.ty_bool8 = create_type(((LL_Type){ .kind = LL_TYPE_BOOL, .width = 8 }));
    result.ty_bool16 = create_type(((LL_Type){ .kind = LL_TYPE_BOOL, .width = 16 }));
    result.ty_bool32 = create_type(((LL_Type){ .kind = LL_TYPE_BOOL, .width = 32 }));
    result.ty_bool64 = create_type(((LL_Type){ .kind = LL_TYPE_BOOL, .width = 64 }));
    result.ty_bool = create_type(((LL_Type){ .kind = LL_TYPE_BOOL, .width = 1 }));
    result.ty_anybool = create_type(((LL_Type){ .kind = LL_TYPE_ANYBOOL }));

    result.ty_string = create_type(((LL_Type){ .kind = LL_TYPE_STRING }));

    return result;
}

void ll_typer_run(Compiler_Context* cc, LL_Typer* typer, Ast_Base* node) {
    ll_typer_type_statement(cc, typer, &node);
}

LL_Type* ll_intern_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* type) {
    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        res = oc_arena_dup(&cc->arena, type, sizeof(*type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}

size_t ll_type_hash(LL_Type* type, size_t seed) {
    LL_Type** tts;
    switch (type->kind) {
    case LL_TYPE_POINTER: {
        LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)type;
        struct {
            LL_Type_Kind kind;
            uint32_t padding;
            LL_Type* element;
        } type_hash = {
            .kind = type->kind,
            .padding = 0,
            .element = ptr_type->element_type,
        };
        size_t hash = stbds_siphash_bytes(&type_hash, sizeof(type_hash), seed);
        return hash;
    }
    case LL_TYPE_FUNCTION: {
        LL_Type_Function* fn_type = (LL_Type_Function*)type;
        tts = alloca(sizeof(*tts) * (fn_type->parameter_count + 1 /* for return type */));
        tts[0] = fn_type->return_type;
        memcpy(tts + 1, fn_type->parameters, sizeof(*fn_type->parameters) * fn_type->parameter_count);

        return stbds_siphash_bytes(tts, sizeof(*tts) * (fn_type->parameter_count + 1 /* for return type */), seed);
    }
    default: return stbds_siphash_bytes(type, sizeof(*type), seed);
    }
}

bool ll_type_eql(LL_Type* a, LL_Type* b) {
    uint32_t i;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
    case LL_TYPE_INT: return a->width == b->width;
    case LL_TYPE_UINT: return a->width == b->width;
    case LL_TYPE_FLOAT: return a->width == b->width;
    case LL_TYPE_BOOL: return a->width == b->width;
    case LL_TYPE_POINTER: {
        LL_Type_Pointer *fa = (LL_Type_Pointer*)a, *fb = (LL_Type_Pointer*)b;
        return fa->element_type == fb->element_type;
    }
    case LL_TYPE_FUNCTION: {
        LL_Type_Function *fa = (LL_Type_Function*)a, *fb = (LL_Type_Function*)b;
        if (fa->return_type != fb->return_type) return false;
        if (fa->parameter_count != fb->parameter_count) return false;
        for (i = 0; i < fb->parameter_count; ++i) {
            if (fa->parameters[i] != fb->parameters[i]) return false;
        }

        return true;
    }
    /* return stbds_siphash_bytes(type, sizeof(*type), seed); */
    default: return true;
    }
}

LL_Type* ll_typer_get_ptr_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* element_type) {
    LL_Type_Pointer ptr_type = { 0 };
    ptr_type.base.kind = LL_TYPE_POINTER;
    ptr_type.element_type = element_type;


    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&ptr_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        res = oc_arena_dup(&cc->arena, &ptr_type, sizeof(ptr_type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}

LL_Type* ll_typer_get_array_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* element_type, size_t size) {
    LL_Type_Array array_type = { 0 };
    array_type.base.kind = LL_TYPE_ARRAY;
    array_type.base.width = size;
    array_type.element_type = element_type;

    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&array_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        res = oc_arena_dup(&cc->arena, &array_type, sizeof(array_type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}

LL_Type* ll_typer_get_fn_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* return_type, LL_Type** parameter_types, size_t parameter_count, bool is_variadic) {
    LL_Type_Function fn_type = { 0 };
    fn_type.base.kind = LL_TYPE_FUNCTION;
    fn_type.return_type = return_type;
    fn_type.parameter_count = parameter_count;
    fn_type.parameters = parameter_types;
    fn_type.is_variadic = is_variadic;

    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&fn_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        fn_type.parameters = oc_arena_dup(&cc->arena, fn_type.parameters, sizeof(*fn_type.parameters) * fn_type.parameter_count);
        res = oc_arena_dup(&cc->arena, &fn_type, sizeof(fn_type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}

LL_Type* ll_typer_implicit_cast_tofrom(Compiler_Context* cc, LL_Typer* typer, LL_Type* from, LL_Type* to) {
    (void)cc;
    if (from == to) return to;

    if (from == typer->ty_anyint) {
        if (to->kind == LL_TYPE_INT) return to;
        else if (to->kind == LL_TYPE_UINT) return to;
        else if (to->kind == LL_TYPE_FLOAT) return to;
    } else if (from == typer->ty_anyint) {
        if (to->kind == LL_TYPE_FLOAT) return to;
    } else if (to->kind == LL_TYPE_INT) {
        if (from->kind == LL_TYPE_INT) return to;
        else if (from->kind == LL_TYPE_UINT) return to;
        else if (from->kind == LL_TYPE_FLOAT) return to;
    } else if (to->kind == LL_TYPE_UINT) {
        if (from->kind == LL_TYPE_INT) return to;
        else if (from->kind == LL_TYPE_UINT) return to;
        else if (from->kind == LL_TYPE_FLOAT) return to;
    } else if (from->kind == LL_TYPE_INT) {
        if (to->kind == LL_TYPE_INT) return to;
        else if (to->kind == LL_TYPE_UINT) return to;
        else if (to->kind == LL_TYPE_FLOAT) return to;
    } else if (from->kind == LL_TYPE_UINT) {
        if (to->kind == LL_TYPE_INT) return to;
        else if (to->kind == LL_TYPE_UINT) return to;
        else if (to->kind == LL_TYPE_FLOAT) return to;
    } else if (from->kind == LL_TYPE_BOOL) {
        if (to->kind == LL_TYPE_BOOL) return to;
    } else if (to->kind == LL_TYPE_BOOL) {
        if (from->kind == LL_TYPE_BOOL) return to;
        else if (from->kind == LL_TYPE_ANYBOOL) return to;
    }

    return NULL;
}

LL_Type* ll_typer_implicit_cast_leftright(Compiler_Context* cc, LL_Typer* typer, LL_Type* lhs, LL_Type* rhs) {
    if (lhs == rhs) return lhs;

    if (lhs == typer->ty_anyint) {
        if (rhs->kind == LL_TYPE_INT) return rhs;
        else if (rhs->kind == LL_TYPE_UINT) return rhs;
        else if (rhs->kind == LL_TYPE_FLOAT) return rhs;
        else if (rhs->kind == LL_TYPE_ANYINT) return lhs;
    } else if (rhs == typer->ty_anyint) {
        if (lhs->kind == LL_TYPE_INT) return lhs;
        else if (lhs->kind == LL_TYPE_UINT) return lhs;
        else if (lhs->kind == LL_TYPE_FLOAT) return lhs;
        else if (lhs->kind == LL_TYPE_ANYINT) return lhs;
    } else if (lhs->kind == LL_TYPE_INT) {
        if (rhs->kind == LL_TYPE_INT) return create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = max(lhs->width, rhs->width) }));
        else if (rhs->kind == LL_TYPE_UINT) return create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = max(lhs->width, rhs->width) }));
        else if (rhs->kind == LL_TYPE_FLOAT) return rhs;
    } else if (lhs->kind == LL_TYPE_UINT) {
        if (rhs->kind == LL_TYPE_INT) return create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = max(lhs->width, rhs->width) }));
        else if (rhs->kind == LL_TYPE_UINT) return create_type(((LL_Type){ .kind = LL_TYPE_UINT, .width = max(lhs->width, rhs->width) }));
        else if (rhs->kind == LL_TYPE_FLOAT) return rhs;
    } else if (rhs->kind == LL_TYPE_INT) {
        if (rhs->kind == LL_TYPE_FLOAT) return rhs;
    } else if (rhs->kind == LL_TYPE_UINT) {
        if (rhs->kind == LL_TYPE_FLOAT) return rhs;
    } else if (lhs->kind == LL_TYPE_FLOAT && rhs->kind == LL_TYPE_FLOAT) {
        return create_type(((LL_Type){ .kind = LL_TYPE_FLOAT, .width = max(lhs->width, rhs->width) }));
    } else if (lhs == typer->ty_anybool) {
        if (rhs->kind == LL_TYPE_BOOL) return rhs;
    } else if (rhs == typer->ty_anybool) {
        if (lhs->kind == LL_TYPE_BOOL) return lhs;
    } else if (lhs->kind == LL_TYPE_BOOL && rhs->kind == LL_TYPE_BOOL) {
        return create_type(((LL_Type){ .kind = LL_TYPE_BOOL, .width = max(lhs->width, rhs->width) }));
    }

    return NULL;
}


LL_Type* ll_typer_type_statement(Compiler_Context* cc, LL_Typer* typer, Ast_Base** stmt) {
    uint32_t i;
    LL_Type** types;

    switch ((*stmt)->kind) {
    case AST_KIND_BLOCK: {
        LL_Scope* block_scope = NULL;
        if (typer->current_scope->kind != LL_SCOPE_KIND_LOOP && typer->current_scope->kind != LL_SCOPE_KIND_PACKAGE) {
            block_scope = create_scope(LL_SCOPE_KIND_BLOCK_VALUE, *stmt);
            ll_typer_scope_put(cc, typer, block_scope);
            typer->current_scope = block_scope;
        }

        AST_AS((*stmt), Ast_Block)->scope = block_scope;

        for (i = 0; i < AST_AS((*stmt), Ast_Block)->count; ++i) {
            ll_typer_type_statement(cc, typer, &AST_AS((*stmt), Ast_Block)->items[i]);
        }

        if (block_scope) {
            typer->current_scope = block_scope->parent;
        }
    } break;
    case AST_KIND_VARIABLE_DECLARATION: {
        Ast_Variable_Declaration* var_decl = AST_AS((*stmt), Ast_Variable_Declaration);
        LL_Type* declared_type = ll_typer_get_type_from_typename(cc, typer, var_decl->type);
        var_decl->ident->base.type = declared_type;

        LL_Scope_Simple* var_scope = (LL_Scope_Simple*)create_scope(LL_SCOPE_KIND_LOCAL, var_decl);
        var_scope->ident = var_decl->ident;
        ll_typer_scope_put(cc, typer, (LL_Scope*)var_scope);

        if (var_decl->initializer) {

            typer->current_scope = (LL_Scope*)var_scope;
            LL_Type* init_type = ll_typer_type_expression(cc, typer, &var_decl->initializer, declared_type, NULL);
            typer->current_scope = var_scope->parent;

            if (!ll_typer_implicit_cast_tofrom(cc, typer, init_type, declared_type)) {
                oc_assert(init_type != NULL);
                oc_assert(declared_type != NULL);
                eprint("\x1b[31;1merror\x1b[0;1m: variable initializer does not match declared type of variable! Expected ");
                ll_print_type_raw(declared_type, &stderr_writer);
                eprint(" but got ");
                ll_print_type_raw(init_type, &stderr_writer);
                eprint("\n");
            }
        }

        break;
    }
    case AST_KIND_FUNCTION_DECLARATION: {
        Ast_Function_Declaration* fn_decl = AST_AS((*stmt), Ast_Function_Declaration);

        LL_Scope* fn_scope = create_scope(LL_SCOPE_KIND_FUNCTION, fn_decl);
        fn_scope->ident = fn_decl->ident;
        fn_decl->ident->resolved_scope = fn_scope;

        ll_typer_scope_put(cc, typer, fn_scope);
        bool did_variadic = false;
        bool did_default = false;

        typer->current_scope = fn_scope;
        if (fn_decl->parameters.count) {
            types = alloca(sizeof(*types) * fn_decl->parameters.count);
            for (i = 0; i < fn_decl->parameters.count; ++i) {
                if (did_variadic) {
                    eprint("\x1b[31;1merror\x1b[0;1m: No parameters can be after variadic parameter\n");
                    break;
                }
                if (fn_decl->parameters.items[i].flags & LL_PARAMETER_FLAG_VARIADIC) {
                    if (fn_decl->parameters.items[i].type) {
                        eprint("\x1b[31;1merror\x1b[0;1m: Typed variadic parameter not supported yet\n");
                    }
                    did_variadic = true;
                } else {
                    types[i] = ll_typer_get_type_from_typename(cc, typer, fn_decl->parameters.items[i].type);
                }

                if (fn_decl->parameters.items[i].initializer) {
                    LL_Type* init_type = ll_typer_type_expression(cc, typer, &fn_decl->parameters.items[i].initializer, types[i], NULL);

                    if (!ll_typer_implicit_cast_tofrom(cc, typer, init_type, types[i])) {
                        eprint("\x1b[31;1merror\x1b[0;1m: provided default parameter type does not match declared type of parameter! Expected ");
                        ll_print_type_raw(types[i], &stderr_writer);
                        eprint(" but got ");
                        ll_print_type_raw(init_type, &stderr_writer);
                        eprint("\n");
                    }

                    did_default = true;
                } else if (did_default) {
                    eprint("\x1b[31;1merror\x1b[0;1m: default arguments can only come after positional arguments\n");
                }

                fn_decl->parameters.items[i].ident->base.type = types[i];
                fn_decl->parameters.items[i].ir_index = i;
                    
                LL_Scope_Simple* param_scope = create_scope_simple(LL_SCOPE_KIND_PARAMETER, &fn_decl->parameters.items[i]);
                param_scope->ident = fn_decl->parameters.items[i].ident;

                ll_typer_scope_put(cc, typer, (LL_Scope*)param_scope);
            }
        } else types = NULL;

        LL_Type* return_type;
        if (fn_decl->return_type) {
            return_type = ll_typer_get_type_from_typename(cc, typer, fn_decl->return_type);
        } else {
            return_type = typer->ty_void;
        }

        LL_Type* fn_type = ll_typer_get_fn_type(cc, typer, return_type, types, fn_decl->parameters.count, did_variadic);
        (*stmt)->type = fn_type;
        fn_decl->ident->base.type = fn_type;
        fn_decl->ident->resolved_scope = fn_scope;

        LL_Type_Function* last_fn = typer->current_fn;
        typer->current_fn = (LL_Type_Function*)fn_type;

        if (fn_decl->body) {
            if (fn_decl->storage_class & LL_STORAGE_CLASS_EXTERN) {
                eprint("\x1b[31;1merror\x1b[0;1m: Extern function shouldn't have a body\x1b[0m\n");
            }
            ll_typer_type_statement(cc, typer, &fn_decl->body);
        }
        typer->current_scope = fn_scope->parent;
        typer->current_fn = last_fn;

        break;
    }
    default: return ll_typer_type_expression(cc, typer, stmt, NULL, NULL);
    }

    return NULL;
}

static LL_Eval_Value const_value_cast(LL_Eval_Value from, LL_Type* from_type, LL_Type* to_type) {
    LL_Eval_Value result;
    if (from_type == to_type) return from;

    switch (from_type->kind) {
    case LL_TYPE_INT:
        switch (to_type->kind) {
        case LL_TYPE_UINT:
            result.uval = from.ival;
            break;
        case LL_TYPE_FLOAT:
            result.fval = from.ival;
        default: oc_todo("implement const cast"); break;
        }
    case LL_TYPE_UINT:
        switch (to_type->kind) {
        case LL_TYPE_INT:
            result.ival = from.uval;
            break;
        case LL_TYPE_FLOAT:
            result.fval = from.ival;
        default: oc_todo("implement const cast"); break;
        }
    default: oc_todo("implement const cast"); break;
    }

    return result;
}

void ll_typer_add_implicit_cast(Compiler_Context* cc, LL_Typer* typer, Ast_Base** expr, LL_Type* expected_type) {
    (void)typer;
    if ((*expr)->type == expected_type) {
        return;
    }

    Ast_Cast cast = {
        .base.kind = AST_KIND_CAST,
        .base.type = expected_type,
        .expr = *expr,
    };

    if ((*expr)->has_const) {
        (*expr)->const_value = const_value_cast((*expr)->const_value, (*expr)->type, expected_type);
    }

    Ast_Base* new_node = oc_arena_dup(&cc->arena, &cast, sizeof(cast));
    *expr = new_node;
}

LL_Type* ll_typer_type_expression(Compiler_Context* cc, LL_Typer* typer, Ast_Base** expr, LL_Type* expected_type, LL_Typer_Resolve_Result *resolve_result) {
    LL_Type* result;
    size_t i;
    switch ((*expr)->kind) {
    case AST_KIND_BLOCK: {
        LL_Type* last_block_type = typer->block_type;
        typer->block_type = expected_type;

        LL_Scope* block_scope = NULL;
        if (typer->current_scope->kind != LL_SCOPE_KIND_LOOP) {
            // merge block scope into loop
            block_scope = create_scope(LL_SCOPE_KIND_BLOCK_VALUE, *expr);
            ll_typer_scope_put(cc, typer, block_scope);
            typer->current_scope = block_scope;
        }

        AST_AS((*expr), Ast_Block)->scope = block_scope;

        for (i = 0; i < AST_AS((*expr), Ast_Block)->count; ++i) {
            ll_typer_type_statement(cc, typer, &AST_AS((*expr), Ast_Block)->items[i]);
        }

        if (block_scope) {
            typer->current_scope = block_scope->parent;
        }

        result = typer->block_type;
        typer->block_type = last_block_type;
        break;
    }
    case AST_KIND_IDENT: {
        if (AST_AS((*expr), Ast_Ident)->str.ptr == LL_KEYWORD_TRUE.ptr || AST_AS((*expr), Ast_Ident)->str.ptr == LL_KEYWORD_FALSE.ptr) {
            return expected_type ? expected_type : typer->ty_anybool;
        }
        LL_Scope* scope = ll_typer_find_symbol_up_scope(cc, typer, AST_AS((*expr), Ast_Ident)->str);
        if (!scope) {
            eprint("\x1b[31;1merror\x1b[0;1m: symbol '{}' not found!\n", AST_AS((*expr), Ast_Ident)->str);
        }
        AST_AS((*expr), Ast_Ident)->resolved_scope = scope;
        if (resolve_result) {
            resolve_result->scope = scope;
        }
        if (expected_type) {
            result = ll_typer_implicit_cast_tofrom(cc, typer, scope->ident->base.type, expected_type);
            if (!result)
                result = scope->ident->base.type;
        } else {
            result = scope->ident->base.type;
        }
        break;
    }
    case AST_KIND_LITERAL_INT:
        (*expr)->has_const = 1u;
        (*expr)->const_value.ival = AST_AS((*expr), Ast_Literal)->i64;

        if (expected_type) {
            switch (expected_type->kind) {
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
            case LL_TYPE_FLOAT:
                result = expected_type;
                break;
            default:
                result = typer->ty_anyint;
                break;
            }
        } else {
            result = typer->ty_anyint;
        }
        break;
    case AST_KIND_LITERAL_STRING:
        result = typer->ty_string;
        break;
    case AST_KIND_ARRAY_INITIALIZER: {
        Ast_Initializer* init = AST_AS((*expr), Ast_Initializer);
        uint8_t* provided_elements = alloca(init->count * sizeof(*provided_elements));
        memset(provided_elements, 0, init->count * sizeof(*provided_elements));

        if (expected_type) {
            oc_assert(expected_type->kind == LL_TYPE_ARRAY);
            LL_Type_Array* arr_type = (LL_Type_Array*)expected_type;
            uint32_t element_index = 0;
            for (i = 0; i < init->count; ++i, ++element_index) {
                LL_Type* provided_type;
                if (init->items[i]->kind == AST_KIND_KEY_VALUE) {
                    Ast_Key_Value* kv = AST_AS((*expr), Ast_Key_Value);
                    LL_Eval_Value key = ll_eval_node(cc, cc->eval_context, cc->bir, kv->key);
                    element_index = (uint32_t)key.uval;
                    provided_type = ll_typer_type_expression(cc, typer, &kv->value, arr_type->element_type, NULL);
                } else {
                    provided_type = ll_typer_type_expression(cc, typer, &init->items[i], arr_type->element_type, NULL);
                }

                if (provided_elements[element_index]) {
                    eprint("\x1b[31;1merror\x1b[0;1m: a value for the index %u was provided more than once\n", element_index);
                }

                provided_elements[element_index] = 1u;

                if (!ll_typer_implicit_cast_tofrom(cc, typer, provided_type, arr_type->element_type)) {
                    oc_assert(provided_type != NULL);
                    oc_assert(arr_type->element_type != NULL);
                    eprint("\x1b[31;1merror\x1b[0;1m: array initializer element does not match declared type of array! Expected ");
                    ll_print_type_raw(arr_type->element_type, &stderr_writer);
                    eprint(" but got ");
                    ll_print_type_raw(provided_type, &stderr_writer);
                    eprint("\n");
                }
            }
            result = expected_type;
        } else oc_assert(false);
        break;
    }
    case AST_KIND_BINARY_OP: {
        Ast_Operation* opr = AST_AS((*expr), Ast_Operation);

        // handle dot member access
        switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '.': {
            LL_Typer_Resolve_Result result = { 0 };	
            ll_typer_type_expression(cc, typer, &opr->left, NULL, &result);

            if (opr->right->kind != AST_KIND_IDENT) {
                eprint("\x1b[31;1merror\x1b[0;1m: member should be ident\x1b[0m\n");
            }
            Ast_Ident* right_ident = AST_AS(opr->right, Ast_Ident);

            if (result.scope) {
                LL_Scope* member_scope = ll_scope_get(result.scope, right_ident->str);
                
                if (!member_scope) {
                    eprint("\x1b[31;1merror\x1b[0;1m: member {} not found\x1b[0m\n", right_ident->str);
                    return NULL;
                }
                right_ident->resolved_scope = member_scope;
                right_ident->base.type = member_scope->ident->base.type;

                if (resolve_result) {
                    resolve_result->scope = member_scope;
                }

                (*expr)->type = right_ident->base.type;
                return right_ident->base.type;
            } else {
                ll_typer_type_expression(cc, typer, &opr->right, NULL, &result);
                if (result.scope->kind == LL_SCOPE_KIND_FUNCTION) {
                    LL_Type_Function* fn_type = (LL_Type_Function*)result.scope->ident->base.type;
                    oc_assert(fn_type->base.kind == LL_TYPE_FUNCTION);

                    if (fn_type->parameter_count > 0) {
                        
                        if (ll_typer_implicit_cast_tofrom(cc, typer, opr->left->type, fn_type->parameters[0])) {
                            right_ident->resolved_scope = result.scope;
                            right_ident->base.type = (LL_Type*)fn_type;

                            if (resolve_result) {
                                resolve_result->scope = result.scope;
                                resolve_result->this_arg = &opr->left;
                            }

                            (*expr)->type = right_ident->base.type;
                            return right_ident->base.type;
                        }
                    }
                }

                eprint("\x1b[31;1merror\x1b[0;1m: member or function {} not found\x1b[0m\n", right_ident->str);
                return NULL;
            }
        }
        default: break;
#pragma GCC diagnostic pop
        }


        LL_Type* lhs_type = ll_typer_type_expression(cc, typer, &opr->left, NULL, NULL);
        LL_Type* rhs_type = ll_typer_type_expression(cc, typer, &opr->right, NULL, NULL);

        if (lhs_type->kind == LL_TYPE_ANYINT && rhs_type->kind == LL_TYPE_ANYINT && expected_type) {
            lhs_type = ll_typer_type_expression(cc, typer, &opr->left, expected_type, NULL);
            rhs_type = ll_typer_type_expression(cc, typer, &opr->right, expected_type, NULL);
        } else if (lhs_type->kind == LL_TYPE_ANYINT || lhs_type->kind == LL_TYPE_ANYFLOAT) {
            lhs_type = ll_typer_type_expression(cc, typer, &opr->left, rhs_type, NULL);
        } else if (rhs_type->kind == LL_TYPE_ANYINT || rhs_type->kind == LL_TYPE_ANYFLOAT) {
            rhs_type = ll_typer_type_expression(cc, typer, &opr->right, lhs_type, NULL);
        }

        switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '+':
        case '-':
        case '*':
        case '/':
            if (expected_type) {
                result = expected_type;
            } else {
                result = ll_typer_implicit_cast_leftright(cc, typer, lhs_type, rhs_type);
            }
            ll_typer_add_implicit_cast(cc, typer, &opr->left, result);
            ll_typer_add_implicit_cast(cc, typer, &opr->right, result);
            break;
        case LL_TOKEN_KIND_ASSIGN_PERCENT:
        case LL_TOKEN_KIND_ASSIGN_DIVIDE:
        case LL_TOKEN_KIND_ASSIGN_TIMES:
        case LL_TOKEN_KIND_ASSIGN_MINUS:
        case LL_TOKEN_KIND_ASSIGN_PLUS:
        case '=':
            result = ll_typer_implicit_cast_leftright(cc, typer, lhs_type, rhs_type);
            // @oc_todo: look at casting lhs
            ll_typer_add_implicit_cast(cc, typer, &opr->right, result);
            break;

        case '>':
        case '<':
        case LL_TOKEN_KIND_GTE:
        case LL_TOKEN_KIND_LTE:
        case LL_TOKEN_KIND_EQUALS:
        case LL_TOKEN_KIND_NEQUALS:
            result = ll_typer_implicit_cast_leftright(cc, typer, lhs_type, rhs_type);
            ll_typer_add_implicit_cast(cc, typer, &opr->left, result);
            ll_typer_add_implicit_cast(cc, typer, &opr->right, result);
            result = typer->ty_bool;
            break;
#pragma GCC diagnostic pop
        default: 
            eprint("Error: Invalid binary operation '");
            lexer_print_token_raw_to_writer(&opr->op, &stderr_writer);
            eprint("'\n");
            break;
        }

        // at this point, we have added casts, both sides should have same types

        if (opr->left->has_const && opr->right->has_const) {
            // @const
            (*expr)->has_const = 1u;
            switch (result->kind) {
            case LL_TYPE_INT:
                switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
                case '+':                   (*expr)->const_value.ival = opr->left->const_value.ival + opr->right->const_value.ival; break;
                case '-':                   (*expr)->const_value.ival = opr->left->const_value.ival = opr->right->const_value.ival; break;
                case '*':                   (*expr)->const_value.ival = opr->left->const_value.ival * opr->right->const_value.ival; break;
                case '/':                   (*expr)->const_value.ival = opr->left->const_value.ival / opr->right->const_value.ival; break;
                case '>': 					(*expr)->const_value.uval = opr->left->const_value.ival > opr->right->const_value.ival; break;
                case '<': 					(*expr)->const_value.uval = opr->left->const_value.ival < opr->right->const_value.ival; break;
#pragma GCC diagnostic pop
                case LL_TOKEN_KIND_GTE: 	(*expr)->const_value.uval = opr->left->const_value.ival >= opr->right->const_value.ival; break;
                case LL_TOKEN_KIND_LTE: 	(*expr)->const_value.uval = opr->left->const_value.ival <= opr->right->const_value.ival; break;
                case LL_TOKEN_KIND_EQUALS:  (*expr)->const_value.uval = opr->left->const_value.ival == opr->right->const_value.ival; break;
                case LL_TOKEN_KIND_NEQUALS: (*expr)->const_value.uval = opr->left->const_value.ival != opr->right->const_value.ival; break;
                default: oc_assert(false); break;
                }
                break;
            case LL_TYPE_UINT:
                switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
                case '+':                   (*expr)->const_value.uval = opr->left->const_value.uval + opr->right->const_value.uval; break;
                case '-':                   (*expr)->const_value.uval = opr->left->const_value.uval = opr->right->const_value.uval; break;
                case '*':                   (*expr)->const_value.uval = opr->left->const_value.uval * opr->right->const_value.uval; break;
                case '/':                   (*expr)->const_value.uval = opr->left->const_value.uval / opr->right->const_value.uval; break;
                case '>': 					(*expr)->const_value.uval = opr->left->const_value.uval > opr->right->const_value.uval; break;
                case '<': 					(*expr)->const_value.uval = opr->left->const_value.uval < opr->right->const_value.uval; break;
#pragma GCC diagnostic pop
                case LL_TOKEN_KIND_GTE: 	(*expr)->const_value.uval = opr->left->const_value.uval >= opr->right->const_value.uval; break;
                case LL_TOKEN_KIND_LTE: 	(*expr)->const_value.uval = opr->left->const_value.uval <= opr->right->const_value.uval; break;
                case LL_TOKEN_KIND_EQUALS:  (*expr)->const_value.uval = opr->left->const_value.uval == opr->right->const_value.uval; break;
                case LL_TOKEN_KIND_NEQUALS: (*expr)->const_value.uval = opr->left->const_value.uval != opr->right->const_value.uval; break;
                default: oc_assert(false); break;
                }
                break;
            case LL_TYPE_FLOAT:
                switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
                case '+':                   (*expr)->const_value.fval = opr->left->const_value.fval + opr->right->const_value.fval; break;
                case '-':                   (*expr)->const_value.fval = opr->left->const_value.fval = opr->right->const_value.fval; break;
                case '*':                   (*expr)->const_value.fval = opr->left->const_value.fval * opr->right->const_value.fval; break;
                case '/':                   (*expr)->const_value.fval = opr->left->const_value.fval / opr->right->const_value.fval; break;
                case '>': 					(*expr)->const_value.uval = opr->left->const_value.fval > opr->right->const_value.fval; break;
                case '<': 					(*expr)->const_value.uval = opr->left->const_value.fval < opr->right->const_value.fval; break;
#pragma GCC diagnostic pop
                case LL_TOKEN_KIND_GTE: 	(*expr)->const_value.uval = opr->left->const_value.fval >= opr->right->const_value.fval; break;
                case LL_TOKEN_KIND_LTE: 	(*expr)->const_value.uval = opr->left->const_value.fval <= opr->right->const_value.fval; break;
                case LL_TOKEN_KIND_EQUALS:  (*expr)->const_value.uval = opr->left->const_value.fval == opr->right->const_value.fval; break;
                case LL_TOKEN_KIND_NEQUALS: (*expr)->const_value.uval = opr->left->const_value.fval != opr->right->const_value.fval; break;
                default: oc_assert(false); break;
                }
            default: oc_todo("implement bvinary op const fold types or error"); break;
            }
        }

        if (!result) {
            eprint("\x1b[31;1merror\x1b[0;1m: types of operands to operation do not match! Found left type was ");
            ll_print_type_raw(lhs_type, &stderr_writer);
            eprint(" but right type was ");
            ll_print_type_raw(rhs_type, &stderr_writer);
            eprint("\n");

            eprint("Error: lhs type does not match rhs type\n");
        }
        break;
    }
    case AST_KIND_PRE_OP: {
        LL_Type* expr_type;
        result = NULL;
        switch (AST_AS((*expr), Ast_Operation)->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '-': {
            expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, expected_type, NULL);

            if (expected_type) {
                result = expected_type;
            } else {
                result = expr_type;
            }

            ll_typer_add_implicit_cast(cc, typer, &AST_AS((*expr), Ast_Operation)->right, result);
        } break;
        case '*': {

            if (expected_type) {
                expr_type = ll_typer_get_ptr_type(cc, typer, expected_type);
                expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, expr_type, NULL);
            } else {
                expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, NULL, NULL);
            }

            switch (expr_type->kind) {
            case LL_TYPE_POINTER: result = ((LL_Type_Pointer*)expr_type)->element_type; break;
            default: break;
            }
        } break;
        case '&': {
            if (expected_type && expected_type->kind == LL_TYPE_POINTER) {
                LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)expected_type;
                expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, ptr_type->element_type, NULL);
            } else {
                expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, NULL, NULL);
            }
            result = ll_typer_get_ptr_type(cc, typer, expr_type);
        } break;
#pragma GCC diagnostic pop
        default: break;
        }

        if (!result) {
            eprint("\x1b[31;1mTODO:\x1b[0m operator '");
            lexer_print_token_raw_to_writer(&AST_AS((*expr), Ast_Operation)->op, &stderr_writer);
            eprint("' cannot be applied to expression of type \n");
            ll_print_type_raw(expr_type, &stderr_writer);
            eprint("\n");
        }
    } break;
    case AST_KIND_INVOKE: {
        uword pi, di;
        Ast_Invoke* inv = AST_AS((*expr), Ast_Invoke);
        Ast_Function_Declaration* fn_decl = NULL;
        LL_Scope* fn_scope = NULL;
        LL_Typer_Resolve_Result resolve = { 0 };

        LL_Type_Function* fn_type = (LL_Type_Function*)ll_typer_type_expression(cc, typer, &inv->expr, NULL, &resolve);
        if (fn_type->base.kind != LL_TYPE_FUNCTION) {
            eprint("\x1b[31;1mTODO:\x1b[0m expression is not callable\n");
            return NULL;
        }

        // if we're directly calling a function:
        if (inv->expr->kind == AST_KIND_IDENT) {
            Ast_Ident* ident = AST_AS(inv->expr, Ast_Ident);
            if (ident->resolved_scope->kind == LL_SCOPE_KIND_FUNCTION) {
                fn_decl = (Ast_Function_Declaration*)ident->resolved_scope->decl;
                fn_scope = ident->resolved_scope;
            }
        }

        Ast_Base** ordered_args = oc_arena_alloc(&cc->arena, sizeof(ordered_args[0]) * (fn_type->parameter_count + inv->arguments.count));
        int ordered_arg_count = 0;
        int variadic_arg_count = 0;

        for (pi = 1, di = 0; pi < inv->arguments.count + 1; ++pi, ++di) {
            LL_Type *declared_type, *provided_type;
            Ast_Base** value;

            Ast_Base** current_arg;
            if (resolve.this_arg) {
                current_arg = resolve.this_arg;
                pi = 0;
                resolve.this_arg = NULL;
            } else {
                current_arg = &inv->arguments.items[pi - 1];
            }
            
            if ((*current_arg)->kind == AST_KIND_KEY_VALUE) {
                if (!fn_decl || !fn_scope) {
                    eprint("\x1b[31;1merror\x1b[0;1m: keyed arguments are only allowed when calling a function directly\n");
                    break;
                }

                // can't have keyed arguments after varaible
                if (fn_type->is_variadic && di >= fn_type->parameter_count - 1) {
                    eprint("\x1b[31;1merror\x1b[0;1m: keyed arguments are only allowed before variadic arguments\n");
                    break;
                }

                Ast_Key_Value* kv = AST_AS((*current_arg), Ast_Key_Value);
                if (kv->key->kind != AST_KIND_IDENT) {
                    eprint("\x1b[31;1merror\x1b[0;1m: expected argument key to be an identifier\n");
                    break;
                }

                LL_Scope* parameter_scope = ll_scope_get(fn_scope, AST_AS(kv->key, Ast_Ident)->str);
                if (parameter_scope->kind != LL_SCOPE_KIND_PARAMETER) {
                    eprint("\x1b[31;1merror\x1b[0;1m: parameter not found\n");
                    break;
                }

                di = AST_AS(parameter_scope->decl, Ast_Parameter)->ir_index;
                declared_type = fn_type->parameters[di];
                provided_type = ll_typer_type_expression(cc, typer, &kv->value, declared_type, NULL);

                value = &kv->value;
            } else {
                if (fn_type->is_variadic) {
                    if (di >= fn_type->parameter_count - 1) {
                        ll_typer_type_expression(cc, typer, &(*current_arg), NULL, NULL);
                        ordered_args[di] = (*current_arg);
                        ordered_arg_count++;
                        variadic_arg_count++;
                        continue;
                    }
                } else if (di >= fn_type->parameter_count) {
                    break;
                }

                declared_type = fn_type->parameters[di];
                provided_type = ll_typer_type_expression(cc, typer, current_arg, declared_type, NULL);
                value = current_arg;
            }

            if (!ll_typer_implicit_cast_tofrom(cc, typer, provided_type, declared_type)) {
                eprint("\x1b[31;1merror\x1b[0;1m: provided argument type does not match declared type of function! Expected ");
                ll_print_type_raw(declared_type, &stderr_writer);
                eprint(" but got ");
                ll_print_type_raw(provided_type, &stderr_writer);
                eprint("\n");
            }

            ll_typer_add_implicit_cast(cc, typer, value, declared_type);
            ordered_arg_count++;
            ordered_args[di] = *value;
            if (!ordered_args[di]->type) {
                ordered_args[di]->type = declared_type;
            }
        }

        size_t missing_count = 0;
        for (di = 0; di < fn_type->parameter_count; ++di) {
            if (di == fn_type->parameter_count - 1 && fn_type->is_variadic) break;
            
            if (!ordered_args[di]) {
                if (fn_decl && fn_decl->parameters.items[di].initializer) {
                    ordered_args[di] = fn_decl->parameters.items[di].initializer;

                    ll_typer_add_implicit_cast(cc, typer, &ordered_args[di], fn_type->parameters[di]);
                    if (!ordered_args[di]->type) {
                        ordered_args[di]->type = fn_type->parameters[di];
                    }

                    ordered_arg_count++;
                } else {
                    missing_count++;
                }
            }
        }

        size_t expected_arg_count = fn_type->is_variadic ? (fn_type->parameter_count - 1) : fn_type->parameter_count;

        if (!fn_type->is_variadic && pi < inv->arguments.count) {
            eprint("\x1b[31;1merror\x1b[0;1m: expected only {} arguments but got {}u\x1b[0m\n", expected_arg_count, inv->arguments.count);
        }
        if (missing_count) {
            if (expected_arg_count == missing_count) {
                eprint("\x1b[31;1merror\x1b[0;1m: expected {} arguments but got none\x1b[0m\n", expected_arg_count);
            } else {
                eprint("\x1b[31;1merror\x1b[0;1m: expected {} arguments but only got {}\x1b[0m\n", expected_arg_count, expected_arg_count - missing_count);
            }
        }

        inv->ordered_arguments.count = ordered_arg_count;
        inv->ordered_arguments.items = ordered_args;

        result = fn_type->return_type;
    } break;
    case AST_KIND_INDEX: {
        Ast_Operation* cf = AST_AS((*expr), Ast_Operation);
        result = ll_typer_type_expression(cc, typer, &cf->left, NULL, NULL);
        ll_typer_type_expression(cc, typer, &cf->right, typer->ty_int32, NULL);

        switch (result->kind) {
        case LL_TYPE_ARRAY:
            result = ((LL_Type_Array*)result)->element_type;
            break;
        case LL_TYPE_POINTER:
            result = ((LL_Type_Pointer*)result)->element_type;
            break;
        default:
            eprint("\x1b[31;1merror\x1b[0;1m: index expression requires an array or pointer type on the left, but found type ");
            ll_print_type_raw(result, &stderr_writer);
            eprint("\n");
            break;
        }

    } break;
    case AST_KIND_CONST: {
        Ast_Marker* cf = AST_AS((*expr), Ast_Marker);
        result = ll_typer_type_expression(cc, typer, &cf->expr, expected_type, NULL);
    } break;
    case AST_KIND_RETURN: {
        Ast_Control_Flow* cf = AST_AS((*expr), Ast_Control_Flow);
        LL_Scope* current_scope = typer->current_scope;
        cf->referenced_scope = NULL;

        while (current_scope) {
            switch (current_scope->kind) {
            case LL_SCOPE_KIND_FUNCTION:
                cf->referenced_scope = current_scope;
                goto AST_RETURN_EXIT_SCOPE;
            case LL_SCOPE_KIND_PACKAGE:
                oc_todo("add error: return outside a funtion");
                goto AST_BREAK_EXIT_SCOPE;
            case LL_SCOPE_KIND_LOCAL:
            case LL_SCOPE_KIND_BLOCK_VALUE:
            case LL_SCOPE_KIND_BLOCK:
            case LL_SCOPE_KIND_PARAMETER:
            case LL_SCOPE_KIND_LOOP:
                break;
            }
            current_scope = current_scope->parent;
        }

AST_RETURN_EXIT_SCOPE:
        if (cf->expr) ll_typer_type_expression(cc, typer, &cf->expr, typer->current_fn->return_type, NULL);
        result = NULL;
    } break;
    case AST_KIND_BREAK: {
        Ast_Control_Flow* cf = AST_AS((*expr), Ast_Control_Flow);

        LL_Scope* current_scope = typer->current_scope;
        cf->referenced_scope = NULL;

        while (current_scope) {
            switch (current_scope->kind) {
            case LL_SCOPE_KIND_LOOP:
                cf->referenced_scope = current_scope;
                goto AST_BREAK_EXIT_SCOPE;
            case LL_SCOPE_KIND_BLOCK:
                cf->referenced_scope = current_scope;
                goto AST_BREAK_EXIT_SCOPE;
            case LL_SCOPE_KIND_BLOCK_VALUE:
                cf->referenced_scope = current_scope;
                goto AST_BREAK_EXIT_SCOPE;
            case LL_SCOPE_KIND_FUNCTION:
                oc_todo("add error: break without a block");
                goto AST_BREAK_EXIT_SCOPE;
            case LL_SCOPE_KIND_PACKAGE:
                oc_todo("add error: break outside a funtion");
                goto AST_BREAK_EXIT_SCOPE;
            case LL_SCOPE_KIND_LOCAL:
                break;
            case LL_SCOPE_KIND_PARAMETER:
                break;
            }
            current_scope = current_scope->parent;
        }

AST_BREAK_EXIT_SCOPE:
        if (cf->expr) ll_typer_type_expression(cc, typer, &cf->expr, typer->block_type, NULL);

        result = NULL;
    } break;
    case AST_KIND_IF: {
        Ast_If* iff = AST_AS((*expr), Ast_If);
        result = ll_typer_type_expression(cc, typer, &iff->cond, typer->ty_bool, NULL);
        switch (result->kind) {
        case LL_TYPE_ANYBOOL:
        case LL_TYPE_BOOL:
        case LL_TYPE_POINTER:
        case LL_TYPE_ANYINT:
        case LL_TYPE_UINT:
        case LL_TYPE_INT:
            break;
        default:
            eprint("\x1b[31;1merror:\x1b[0m if statement condition should be boolean, integer or pointer\n");
            break;
        }

        if (iff->body) {
            ll_typer_type_statement(cc, typer, &iff->body);
        }

        if (iff->else_clause) {
            ll_typer_type_statement(cc, typer, &iff->else_clause);
        }

        result = NULL;
    } break;
    case AST_KIND_FOR: {
        Ast_Loop* loop = AST_AS((*expr), Ast_Loop);

        LL_Scope* loop_scope = create_scope(LL_SCOPE_KIND_LOOP, *expr);
        ll_typer_scope_put(cc, typer, loop_scope);
        typer->current_scope = loop_scope;
        if (loop->init) ll_typer_type_statement(cc, typer, &loop->init);

        if (loop->cond) {
            result = ll_typer_type_expression(cc, typer, &loop->cond, typer->ty_int32, NULL);
            switch (result->kind) {
            case LL_TYPE_BOOL:
            case LL_TYPE_ANYBOOL:
            case LL_TYPE_POINTER:
            case LL_TYPE_ANYINT:
            case LL_TYPE_UINT:
            case LL_TYPE_INT: break;
            default:
                eprint("\x1b[31;1merror:\x1b[0m if statement condition should be boolean, integer or pointer\n");
                break;
            }
        }
        if (loop->update) ll_typer_type_expression(cc, typer, &loop->update, NULL, NULL);

        if (loop->body) {
            ll_typer_type_statement(cc, typer, &loop->body);
        }

        typer->current_scope = loop_scope->parent;

        result = NULL;
    } break;
    default:
        eprint("\x1b[31;1mTODO:\x1b[0m type expression %d\n", (*expr)->kind);
        result = NULL;
        break;
    }

    (*expr)->type = result;
    return result;
}

LL_Type* ll_typer_get_type_from_typename(Compiler_Context* cc, LL_Typer* typer, Ast_Base* typename) {
    LL_Type* result = NULL;

    switch (typename->kind) {
    case AST_KIND_IDENT:
        if (string_starts_with(AST_AS(typename, Ast_Ident)->str, lit("int"))) {
            if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT.ptr) result = typer->ty_int32;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT8.ptr) result = typer->ty_int8;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT16.ptr) result = typer->ty_int16;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT32.ptr) result = typer->ty_int32;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT64.ptr) result = typer->ty_int64;
        } else if (string_starts_with(AST_AS(typename, Ast_Ident)->str, lit("bool"))) {
            if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_BOOL.ptr) result = typer->ty_bool;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_BOOL8.ptr) result = typer->ty_bool8;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_BOOL16.ptr) result = typer->ty_bool16;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_BOOL32.ptr) result = typer->ty_bool32;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_BOOL64.ptr) result = typer->ty_bool64;
        } else if (string_starts_with(AST_AS(typename, Ast_Ident)->str, lit("uint"))) {
            if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT.ptr) result = typer->ty_uint32;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT8.ptr) result = typer->ty_uint8;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT16.ptr) result = typer->ty_uint16;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT32.ptr) result = typer->ty_uint32;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT64.ptr) result = typer->ty_uint64;
        } else if (string_starts_with(AST_AS(typename, Ast_Ident)->str, lit("float"))) {
            if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_FLOAT.ptr) result = typer->ty_float32;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_FLOAT16.ptr) result = typer->ty_float16;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_FLOAT32.ptr) result = typer->ty_float32;
            else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_FLOAT64.ptr) result = typer->ty_float64;
        } else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_STRING.ptr) {
            result = typer->ty_string;
        } else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_VOID.ptr) {
            result = typer->ty_void;
        }

        // oc_todo: search identifier in symbol table
        break;
    case AST_KIND_TYPE_POINTER: {
        result = ll_typer_get_ptr_type(cc, typer, ll_typer_get_type_from_typename(cc, typer, AST_AS(typename, Ast_Type_Pointer)->element));
        break;
    }
    case AST_KIND_INDEX: {
        LL_Type* element_type = ll_typer_get_type_from_typename(cc, typer, AST_AS(typename, Ast_Operation)->left);
        LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, cc->bir, AST_AS(typename, Ast_Operation)->right);
        result = ll_typer_get_array_type(cc, typer, element_type, value.uval);
        break;
    }

    default: eprint("\x1b[31;1mTODO:\x1b[0m typename node {}\n", typename->kind);
    }

    typename->type = result;
    return result;
}

void ll_print_type_raw(LL_Type* type, Oc_Writer* w) {
    uint32_t i;
    switch (type->kind) {
    case LL_TYPE_VOID:     wprint(w, "void"); break;
    case LL_TYPE_INT:      wprint(w, "int{}", type->width); break;
    case LL_TYPE_UINT:     wprint(w, "uint{}", type->width); break;
    case LL_TYPE_ANYINT:   wprint(w, "anyint"); break;
    case LL_TYPE_FLOAT:    wprint(w, "float{}", type->width); break;
    case LL_TYPE_ANYFLOAT: wprint(w, "anyfloat"); break;
    case LL_TYPE_STRING:   wprint(w, "string"); break;
    case LL_TYPE_BOOL:     wprint(w, "bool{}", type->width); break;
    case LL_TYPE_ANYBOOL:  wprint(w, "anybool"); break;
    case LL_TYPE_POINTER: {
        LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)type;
        ll_print_type_raw(ptr_type->element_type, w);
        wprint(w, "*");
        break;
    }
    case LL_TYPE_ARRAY: {
        LL_Type_Array* array_type = (LL_Type_Array*)type;
        ll_print_type_raw(array_type->element_type, w);
        wprint(w, "[{}]", array_type->base.width);
        break;
    }
    case LL_TYPE_FUNCTION: {
        LL_Type_Function* fn_type = (LL_Type_Function*)type;
        if (fn_type->return_type) ll_print_type_raw(fn_type->return_type, w);
        wprint(w, " (");
        for (i = 0; i < fn_type->parameter_count; ++i) {
            if (i > 0) wprint(w, ", ");
            if (fn_type->parameters[i]) ll_print_type_raw(fn_type->parameters[i], w);
        }
        wprint(w, ")");
        break;
    }
    default: oc_assert(false); break;
    }
}

void ll_print_type(LL_Type* type) {
    ll_print_type_raw(type, &stdout_writer);
    print("\n");
}

void ll_typer_scope_put(Compiler_Context* cc, LL_Typer* typer, LL_Scope* scope) {
    size_t hash;
    int kind;
    if (scope->ident) {
        kind = 0;
        hash = stbds_siphash_bytes(&kind, sizeof(kind), MAP_DEFAULT_SEED);
        hash = hash_combine(hash, stbds_hash_string(scope->ident->str, MAP_DEFAULT_SEED));
    } else {
        kind = 1;
        size_t anon = typer->current_scope->next_anon++;
        hash = stbds_siphash_bytes(&kind, sizeof(kind), MAP_DEFAULT_SEED);
        hash = hash_combine(hash, stbds_siphash_bytes(&anon, sizeof(anon), MAP_DEFAULT_SEED));
    }
    hash = hash % oc_len(typer->current_scope->children);
    LL_Scope_Map_Entry* current = typer->current_scope->children[hash];

    LL_Scope_Map_Entry* new_entry = oc_arena_alloc(&cc->arena, sizeof(*new_entry));
    new_entry->scope = scope;
    new_entry->next = current;

    typer->current_scope->children[hash] = new_entry;
    scope->parent = typer->current_scope;
}

LL_Scope* ll_scope_get(LL_Scope* scope, string symbol_name) {
    int kind = 0; // scope is named
    size_t hash = stbds_siphash_bytes(&kind, sizeof(kind), MAP_DEFAULT_SEED);
    hash = hash_combine(hash, stbds_hash_string(symbol_name, MAP_DEFAULT_SEED));
    hash = hash % oc_len(scope->children);
    LL_Scope_Map_Entry* current = scope->children[hash];

    while (current) {
        if (string_eql(current->scope->ident->str, symbol_name)) break;
        current = current->next;
    }

    if (current) return current->scope;
    return NULL;
}

LL_Scope* ll_typer_find_symbol_up_scope(Compiler_Context* cc, LL_Typer* typer, string symbol_name) {
    (void)cc;
    LL_Scope* found_scope;
    LL_Scope* current = typer->current_scope;

    while (current) {
        switch (current->kind) {
        case LL_SCOPE_KIND_PACKAGE:
        case LL_SCOPE_KIND_FUNCTION:
        case LL_SCOPE_KIND_BLOCK:
        case LL_SCOPE_KIND_BLOCK_VALUE:
        case LL_SCOPE_KIND_LOOP:
            found_scope = ll_scope_get(current, symbol_name);
            break;
        default: found_scope = NULL; break;
        }
        if (found_scope) return found_scope;
        current = current->parent;
    }

    return NULL;
}

void ll_scope_print(LL_Scope* scope, int indent, Oc_Writer* w) {
    int i;
    LL_Scope_Map_Entry* current;

    for (i = 0; i < indent; ++i) {
        wprint(w, "  ");
    }

    switch (scope->kind) {
    case LL_SCOPE_KIND_LOCAL:       wprint(w, "Local"); break;
    case LL_SCOPE_KIND_FUNCTION:    wprint(w, "Function"); break;
    case LL_SCOPE_KIND_PACKAGE:     wprint(w, "Module"); break;
    case LL_SCOPE_KIND_PARAMETER:   wprint(w, "Parmeter"); break;
    case LL_SCOPE_KIND_BLOCK:       wprint(w, "Block"); break;
    case LL_SCOPE_KIND_BLOCK_VALUE: wprint(w, "Block_Value"); break;
    case LL_SCOPE_KIND_LOOP:        wprint(w, "Loop"); break;
    }
    if (scope->ident)
        wprint(w, " '{}'", scope->ident->str);
    wprint(w, "\n");

    switch (scope->kind) {
    case LL_SCOPE_KIND_LOCAL: return;
    default: break;
    }
    for (uword i = 0; i < oc_len(scope->children); ++i) {
        current = scope->children[i];
        while (current) {
            ll_scope_print(current->scope, indent + 1, w);
            current = current->next;
        }
    }
}

