
#include <alloca.h>
#include <string.h>

#include "common.h"
#include "typer.h"
#include "lexer.h"

#define create_type(type) ({\
			__typeof__(type) t = type; \
			ll_intern_type(cc, typer, &t); \
		})

LL_Typer ll_typer_create(Compiler_Context* cc) {
	LL_Typer result = { 0 };
	LL_Typer* typer = &result;

	LL_Scope root_scope = {
		.kind = LL_SCOPE_KIND_PACKAGE,
	};
	result.root_scope = result.current_scope = arena_memdup(&cc->arena, &root_scope, sizeof(root_scope));

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

	result.ty_string = create_type(((LL_Type){ .kind = LL_TYPE_STRING }));

	return result;
}

void ll_typer_run(Compiler_Context* cc, LL_Typer* typer, Ast_Base* node) {
	ll_typer_type_statement(cc, typer, node);
}

LL_Type* ll_intern_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* type) {
	LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

	if (t) {
		res = *t;
	} else {
		res = arena_memdup(&cc->arena, type, sizeof(*type));
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
	int i;
	if (a->kind != b->kind) return false;

	switch (a->kind) {
	case LL_TYPE_INT: return a->width == b->width;
	case LL_TYPE_UINT: return a->width == b->width;
	case LL_TYPE_FLOAT: return a->width == b->width;
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
	LL_Type_Pointer ptr_type = {
		.base = { .kind = LL_TYPE_POINTER },
		.element_type = element_type,
	};


	LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&ptr_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

	if (t) {
		res = *t;
	} else {
		res = arena_memdup(&cc->arena, &ptr_type, sizeof(ptr_type));
		MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
	}

	return res;
}

LL_Type* ll_typer_get_fn_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* return_type, LL_Type** parameter_types, size_t parameter_count, bool is_variadic) {
	LL_Type_Function fn_type = {
		.base = { .kind = LL_TYPE_FUNCTION },
		.return_type = return_type,
		.parameter_count = parameter_count,
		.parameters = parameter_types,
		.is_variadic = is_variadic,
	};

	LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&fn_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

	if (t) {
		res = *t;
	} else {
		fn_type.parameters = arena_memdup(&cc->arena, fn_type.parameters, sizeof(*fn_type.parameters) * fn_type.parameter_count);
		res = arena_memdup(&cc->arena, &fn_type, sizeof(fn_type));
		MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
	}

	return res;
}

LL_Type* ll_typer_implicit_cast_tofrom(Compiler_Context* cc, LL_Typer* typer, LL_Type* from, LL_Type* to) {
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
	}

	return NULL;
}


LL_Type* ll_typer_type_statement(Compiler_Context* cc, LL_Typer* typer, Ast_Base* stmt) {
	int i;
	LL_Type** types;

	switch (stmt->kind) {
	case AST_KIND_BLOCK:
		for (i = 0; i < AST_AS(stmt, Ast_Block)->count; ++i) {
			ll_typer_type_statement(cc, typer, AST_AS(stmt, Ast_Block)->items[i]);
		}
		break;
	case AST_KIND_VARIABLE_DECLARATION: {
		Ast_Variable_Declaration* var_decl = AST_AS(stmt, Ast_Variable_Declaration);
		LL_Type* declared_type = ll_typer_get_type_from_typename(cc, typer, var_decl->type);
		var_decl->ident->base.type = declared_type;

		LL_Scope_Simple* var_scope = arena_alloc(&cc->arena, sizeof(LL_Scope_Simple));
		var_scope->kind = LL_SCOPE_KIND_LOCAL;
		var_scope->ident = var_decl->ident;
		var_scope->decl = (Ast_Base*)var_decl;
		ll_typer_scope_put(cc, typer, (LL_Scope*)var_scope);

		if (var_decl->initializer) {

			ll_print_type(declared_type);
			typer->current_scope = (LL_Scope*)var_scope;
			LL_Type* init_type = ll_typer_type_expression(cc, typer, var_decl->initializer, declared_type);
			typer->current_scope = var_scope->parent;

			if (!ll_typer_implicit_cast_tofrom(cc, typer, init_type, declared_type)) {
				assert(init_type != NULL);
				assert(declared_type != NULL);
				fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: variable initializer does not match declared type of variable! Expected ");
				ll_print_type_raw(declared_type, stderr);
				fprintf(stderr, " but got ");
				ll_print_type_raw(init_type, stderr);
				fprintf(stderr, "\n");
			}
		}

		break;
	}
	case AST_KIND_FUNCTION_DECLARATION: {
		Ast_Function_Declaration* fn_decl = AST_AS(stmt, Ast_Function_Declaration);


		LL_Scope* fn_scope = arena_alloc(&cc->arena, sizeof(LL_Scope));
		fn_scope->kind = LL_SCOPE_KIND_FUNCTION;
		fn_scope->ident = fn_decl->ident;
		fn_scope->decl = (Ast_Base*)fn_decl;
		memset(&fn_scope->children, 0, sizeof(fn_scope->children));

		ll_typer_scope_put(cc, typer, fn_scope);
		bool did_variadic = false;

		typer->current_scope = fn_scope;
		if (fn_decl->parameters.count) {
			types = alloca(sizeof(*types) * fn_decl->parameters.count);
			for (i = 0; i < fn_decl->parameters.count; ++i) {
				if (did_variadic) {
					fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: No parameters can be after variadic parameter\n");
					break;
				}
				if (fn_decl->parameters.items[i].flags & LL_PARAMETER_FLAG_VARIADIC) {
					if (fn_decl->parameters.items[i].type) {
						fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: Typed variadic parameter not supported yet\n");
					}
					did_variadic = true;
					types[i] = NULL;
				} else {
					types[i] = ll_typer_get_type_from_typename(cc, typer, fn_decl->parameters.items[i].type);
				}
				fn_decl->parameters.items[i].ident->base.type = types[i];
				fn_decl->parameters.items[i].ir_index = i;
					
				LL_Scope_Simple* param_scope = arena_alloc(&cc->arena, sizeof(LL_Scope_Simple));
				param_scope->kind = LL_SCOPE_KIND_PARAMETER;
				param_scope->ident = fn_decl->parameters.items[i].ident;
				param_scope->decl = (Ast_Base*)&fn_decl->parameters.items[i];

				ll_typer_scope_put(cc, typer, (LL_Scope*)param_scope);
			}
		}

		LL_Type* return_type = ll_typer_get_type_from_typename(cc, typer, fn_decl->return_type);

		LL_Type* fn_type = ll_typer_get_fn_type(cc, typer, return_type, types, fn_decl->parameters.count, did_variadic);
		stmt->type = fn_type;
		fn_decl->ident->base.type = fn_type;

		LL_Type_Function* last_fn = typer->current_fn;
		typer->current_fn = (LL_Type_Function*)fn_type;

		if (fn_decl->body) {
			if (fn_decl->storage_class & LL_STORAGE_CLASS_EXTERN) {
				fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: Extern function shouldn't have a body\n");
			}
			ll_typer_type_statement(cc, typer, fn_decl->body);
		}
		typer->current_scope = fn_scope->parent;
		typer->current_fn = last_fn;

		break;
	}
	default: return ll_typer_type_expression(cc, typer, stmt, NULL);
	}

	return NULL;
}

LL_Type* ll_typer_type_expression(Compiler_Context* cc, LL_Typer* typer, Ast_Base* expr, LL_Type* expected_type) {
	LL_Type* result;
	switch (expr->kind) {
	case AST_KIND_IDENT: {
		LL_Scope* scope = ll_typer_find_symbol_up_scope(cc, typer, AST_AS(expr, Ast_Ident)->str);
		if (!scope) {
			fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: symbol '" FMT_SV_FMT "' not found!\n", FMT_SV_ARG(AST_AS(expr, Ast_Ident)->str));
		}
		AST_AS(expr, Ast_Ident)->resolved_scope = scope;
		printf("typer type %.*s - %p\n", scope->ident->str.len, scope->ident->str.ptr, scope->ident->base.type);
		result = scope->ident->base.type;
		break;
	}
	case AST_KIND_LITERAL_INT:
		if (expected_type) {
			result = expected_type;
		} else {
			result = typer->ty_anyint;
		}
		break;
	case AST_KIND_LITERAL_STRING:
		result = typer->ty_string;
		break;
	case AST_KIND_BINARY_OP: {
		LL_Type* lhs_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->left, NULL);
		LL_Type* rhs_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->right, NULL);

		if (lhs_type->kind == LL_TYPE_ANYINT && rhs_type->kind == LL_TYPE_ANYINT && expected_type) {
			printf("both\n");
			ll_print_type(expected_type);
			lhs_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->left, expected_type);
			rhs_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->right, expected_type);
		} else if (lhs_type->kind == LL_TYPE_ANYINT || lhs_type->kind == LL_TYPE_ANYFLOAT) {
			printf("left\n");
			lhs_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->left, rhs_type);
		} else if (rhs_type->kind == LL_TYPE_ANYINT || rhs_type->kind == LL_TYPE_ANYFLOAT) {
			printf("right\n");
			rhs_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->right, lhs_type);
		}


		result = ll_typer_implicit_cast_leftright(cc, typer, lhs_type, rhs_type);
		if (!result) {
			fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: types of operands to operation do not match! Found left type was ");
			ll_print_type_raw(lhs_type, stderr);
			fprintf(stderr, " but right type was ");
			ll_print_type_raw(rhs_type, stderr);
			fprintf(stderr, "\n");

			fprintf(stderr, "Error: lhs type does not match rhs type\n");
		}
		break;
	}
	case AST_KIND_PRE_OP: {
		LL_Type* expr_type;
		result = NULL;
		switch (AST_AS(expr, Ast_Operation)->op.kind) {
		case '-': {
			expr_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->right, expected_type);
			switch (expr_type->kind) {
			case LL_TYPE_INT:
			case LL_TYPE_UINT:
			case LL_TYPE_FLOAT:
			case LL_TYPE_ANYINT:
			case LL_TYPE_ANYFLOAT:
				result = expr_type;
				break;
			default: break;
			}
		}
		case '*': {

			if (expected_type) {
				expr_type = ll_typer_get_ptr_type(cc, typer, expected_type);
				expr_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->right, expr_type);
			} else {
				expr_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->right, NULL);
			}

			switch (expr_type->kind) {
			case LL_TYPE_POINTER: result = ((LL_Type_Pointer*)expr_type)->element_type;
			default: break;
			}
			break;
		}
		case '&': {
			if (expected_type && expected_type->kind == LL_TYPE_POINTER) {
				LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)expected_type;
				expr_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->right, ptr_type->element_type);
			} else {
				expr_type = ll_typer_type_expression(cc, typer, AST_AS(expr, Ast_Operation)->right, NULL);
			}
			result = ll_typer_get_ptr_type(cc, typer, expr_type);
			break;
		}
		default: break;
		}

		if (!result) {
			fprintf(stderr, "\x1b[31;1mTODO:\x1b[0m operator '");
			lexer_print_token_raw_to_fd(&AST_AS(expr, Ast_Operation)->op, stderr);
			fprintf(stderr, "' cannot be applied to expression of type \n");
			ll_print_type_raw(expr_type, stderr);
			fprintf(stderr, "\n");
		}
		
		break;
	}
	case AST_KIND_INVOKE: {
		int pi, di;
		Ast_Invoke* inv = AST_AS(expr, Ast_Invoke);
		LL_Type_Function* fn_type = (LL_Type_Function*)ll_typer_type_expression(cc, typer, inv->expr, NULL);
		if (fn_type->base.kind != LL_TYPE_FUNCTION) {
			fprintf(stderr, "\x1b[31;1mTODO:\x1b[0m expression is not callable\n");
			return NULL;
		}

		for (pi = 0, di = 0; pi < inv->arguments.count && (fn_type->is_variadic || di < fn_type->parameter_count); ++pi, ++di) {
			LL_Type* declared_type = fn_type->parameters[di];
			LL_Type* provided_type = ll_typer_type_expression(cc, typer, inv->arguments.items[pi], declared_type);
			if (fn_type->is_variadic && di >= fn_type->parameter_count - 1) {
				continue;
			}


			if (!ll_typer_implicit_cast_tofrom(cc, typer, provided_type, declared_type)) {
				fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: provided argument type does not match declared type of function! Expected ");
				ll_print_type_raw(declared_type, stderr);
				fprintf(stderr, " but got ");
				ll_print_type_raw(provided_type, stderr);
				fprintf(stderr, "\n");
			}
		}

		size_t expected_arg_count = fn_type->is_variadic ? (fn_type->parameter_count - 1) : fn_type->parameter_count;

		if (pi < inv->arguments.count) {
			fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: expected only %zu arguments but got %zu\n", expected_arg_count, inv->arguments.count);
		}
		if (di < expected_arg_count) {
			fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: expected %zu arguments but only got %zu\n", expected_arg_count, inv->arguments.count);
		}

		result = fn_type->return_type;
		break;
	}
	case AST_KIND_RETURN: {
		Ast_Control_Flow* cf = AST_AS(expr, Ast_Control_Flow);
		if (cf->expr) ll_typer_type_expression(cc, typer, cf->expr, typer->current_fn->return_type);
		result = NULL;

		break;
	}
	default: fprintf(stderr, "\x1b[31;1mTODO:\x1b[0m type expression %d\n", expr->kind);
	}

	expr->type = result;
	return result;
}

LL_Type* ll_typer_get_type_from_typename(Compiler_Context* cc, LL_Typer* typer, Ast_Base* typename) {
	LL_Type* result;

	switch (typename->kind) {
	case AST_KIND_IDENT:
		if (string_view_starts_with(AST_AS(typename, Ast_Ident)->str, str_lit("int"))) {
			if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT8.ptr) return typer->ty_int8;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT16.ptr) return typer->ty_int16;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT32.ptr) return typer->ty_int32;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT64.ptr) return typer->ty_int64;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_INT.ptr) return typer->ty_int32;
		} else if (string_view_starts_with(AST_AS(typename, Ast_Ident)->str, str_lit("uint"))) {
			if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT8.ptr) return typer->ty_uint8;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT16.ptr) return typer->ty_uint16;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT32.ptr) return typer->ty_uint32;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT64.ptr) return typer->ty_uint64;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_UINT.ptr) return typer->ty_uint32;
		} else if (string_view_starts_with(AST_AS(typename, Ast_Ident)->str, str_lit("float"))) {
			if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_FLOAT16.ptr) return typer->ty_float16;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_FLOAT32.ptr) return typer->ty_float32;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_FLOAT64.ptr) return typer->ty_float64;
			else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_FLOAT.ptr) return typer->ty_float32;
		} else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_STRING.ptr) {
			return typer->ty_string;
		} else if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_VOID.ptr) {
			return typer->ty_void;
		}

		// TODO: search identifier in symbol table
		
		return NULL;
	case AST_KIND_TYPE_POINTER: {
		result = ll_typer_get_ptr_type(cc, typer, ll_typer_get_type_from_typename(cc, typer, AST_AS(typename, Ast_Type_Pointer)->element));
		break;
	}

	default: fprintf(stderr, "\x1b[31;1mTODO:\x1b[0m typename node %d\n", typename->kind);
	}

	return result;
}

void ll_print_type_raw(LL_Type* type, FILE* fd) {
	int i;
	switch (type->kind) {
	case LL_TYPE_VOID: fprintf(stderr, "void"); break;
	case LL_TYPE_INT: fprintf(stderr, "int%zu", type->width); break;
	case LL_TYPE_UINT: fprintf(stderr, "uint%zu", type->width); break;
	case LL_TYPE_ANYINT: fprintf(stderr, "anyint"); break;
	case LL_TYPE_FLOAT: fprintf(stderr, "float%zu", type->width); break;
	case LL_TYPE_ANYFLOAT: fprintf(stderr, "anyfloat"); break;
	case LL_TYPE_STRING: fprintf(stderr, "string"); break;
	case LL_TYPE_POINTER: {
		LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)type;
		ll_print_type_raw(ptr_type->element_type, fd);
		fprintf(stderr, "*");
		break;
	}
	case LL_TYPE_FUNCTION: {
		LL_Type_Function* fn_type = (LL_Type_Function*)type;
		if (fn_type->return_type) ll_print_type_raw(fn_type->return_type, fd);
		fprintf(stderr, " (");
		for (i = 0; i < fn_type->parameter_count; ++i) {
			if (i > 0) fprintf(stderr, ", ");
			if (fn_type->parameters[i]) ll_print_type_raw(fn_type->parameters[i], fd);
		}
		fprintf(stderr, ")");
		break;
	}
	default: assert(false); break;
	}
}

void ll_print_type(LL_Type* type) {
	ll_print_type_raw(type, stdout);
	printf("\n");
}

void ll_typer_scope_put(Compiler_Context* cc, LL_Typer* typer, LL_Scope* scope) {
	size_t hash = stbds_hash_string(scope->ident->str, MAP_DEFAULT_SEED) % LEN(typer->current_scope->children);
	LL_Scope_Map_Entry* current = typer->current_scope->children[hash];

	LL_Scope_Map_Entry* new_entry = arena_alloc(&cc->arena, sizeof(*new_entry));
	new_entry->scope = scope;
	new_entry->next = current;

	typer->current_scope->children[hash] = new_entry;
	scope->parent = typer->current_scope;
}

LL_Scope* ll_scope_get(LL_Scope* scope, String_View symbol_name) {
	size_t hash = stbds_hash_string(symbol_name, MAP_DEFAULT_SEED) % LEN(scope->children);
	LL_Scope_Map_Entry* current = scope->children[hash];

	while (current) {
		if (string_view_eql(current->scope->ident->str, symbol_name)) break;
		current = current->next;
	}

	if (current) return current->scope;
	return NULL;
}

LL_Scope* ll_typer_find_symbol_up_scope(Compiler_Context* cc, LL_Typer* typer, String_View symbol_name) {
	LL_Scope* found_scope;
	LL_Scope* current = typer->current_scope;

	while (current) {
		found_scope = ll_scope_get(current, symbol_name);
		if (found_scope) return found_scope;
		current = current->parent;
	}

	return NULL;
}

void ll_scope_print(LL_Scope* scope, int indent) {
	int i;
	LL_Scope_Map_Entry* current;

	for (i = 0; i < indent; ++i) {
		printf("  ");
	}

	switch (scope->kind) {
	case LL_SCOPE_KIND_LOCAL: printf("Local"); break;
	case LL_SCOPE_KIND_FUNCTION: printf("Function"); break;
	case LL_SCOPE_KIND_PACKAGE: printf("Module"); break;
	}
	if (scope->ident)
		printf(" '" FMT_SV_FMT "'", FMT_SV_ARG(scope->ident->str));
	printf("\n");

	switch (scope->kind) {
	case LL_SCOPE_KIND_LOCAL: return;
	default: break;
	}
	for (int i = 0; i < LEN(scope->children); ++i) {
		current = scope->children[i];
		while (current) {
			ll_scope_print(current->scope, indent + 1);
			current = current->next;
		}
	}
}

