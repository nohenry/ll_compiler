
#include <stdlib.h>
#include <inttypes.h>

#include "c.h"
#include "../common.h"
#include "../typer.h"

typedef enum {
	LL_BACKEND_C_VALUE_KIND_NONE,
} LL_Backend_C_Value_Kind;

typedef struct {
	LL_Backend_C_Value_Kind kind;
} LL_Backend_C_Value;

typedef struct {
	uint32_t locals_count;	
} LL_Backend_C_Frame;

typedef struct {
	uint32_t count, capacity;
	LL_Backend_C_Frame* items;
} LL_Backend_C_Frame_List;

typedef struct ll_backend_c_anon_type_entry {
	LL_Type* value;
	int64_t name_id;
	struct ll_backend_c_anon_type_entry* next;
} LL_Backend_C_Anon_Type_Entry;

typedef struct {
	String_Builder o;
	String_Builder typedef_o;
	LL_Backend_C_Frame_List frames;

	LL_Backend_C_Anon_Type_Entry* anon_entries[LL_DEFAULT_MAP_ENTRY_COUNT];

	uint32_t block_value;
	int indent;
	int64_t next_anon_id;
} LL_Backend_C;

#ifdef FUNCTION
#undef FUNCTION
#endif
#define FUNCTION() (&b->frames.items[b->frames.count - 1])

#define output_str(str) arena_sb_append_cstr(&cc->arena, o, str)
#define output_fmt(fmt, ...) arena_sb_sprintf(&cc->arena, o, fmt, ## __VA_ARGS__)

#define BACKEND_C_ANON_TYPEDEF_PREFIX "___anon_"

void backend_c_init(Compiler_Context* cc, LL_Backend_C* b) {
	memset(b, 0, sizeof(*b));
}

LL_Backend_C_Value backend_c_generate_expression(Compiler_Context* cc, LL_Backend_C* b, String_Builder* o, Ast_Base* expr, bool lvalue);
void backend_c_generate_typename(Compiler_Context* cc, LL_Backend_C* b, String_Builder* o, Ast_Base* type, String_View* var_name);
void backend_c_generate_typename_impl(Compiler_Context* cc, LL_Backend_C* b, String_Builder* o, LL_Type* type);

typedef struct {
	int64_t name_id;
	LL_Type* type;
} LL_Backend_C_Typedef_Queue_Entry;

typedef struct {
	uint32_t count, capacity;
	LL_Backend_C_Typedef_Queue_Entry* items;
} LL_Backend_C_Typedef_Queue;

int queue_item_compare(const void* a, const void* b) {
	LL_Backend_C_Typedef_Queue_Entry* aa = (LL_Backend_C_Typedef_Queue_Entry*)a;
	LL_Backend_C_Typedef_Queue_Entry* bb = (LL_Backend_C_Typedef_Queue_Entry*)b;

	return aa->name_id - bb->name_id;
}

bool backend_c_write_to_file(Compiler_Context* cc, LL_Backend_C* b, char* filepath) {
	FILE* out_file = fopen(filepath, "w");
	fwrite("#include <stdint.h>\n", sizeof(*b->o.items), sizeof("#include <stdint.h>\n")-1, out_file);
	String_Builder* o = &b->typedef_o;

	LL_Backend_C_Typedef_Queue queue = { 0 };
	int64_t max_value = 0;

	for (int i = 0; i < LEN(b->anon_entries); ++i) {
		LL_Backend_C_Anon_Type_Entry* current = b->anon_entries[i];

		while (current) {
			arena_da_append(&cc->arena, &queue, ((LL_Backend_C_Typedef_Queue_Entry){ current->name_id, current->value }));
			current = current->next;
		}
	}
	qsort(queue.items, queue.count, sizeof(queue.items[0]), queue_item_compare);

	/* for (;;) { */
		for (int i = 0; i < queue.count; ++i) {
			if (queue.items[i].name_id < 0) continue;

			output_str("typedef ");
			switch (queue.items[i].type->kind) {
			case LL_TYPE_ARRAY:
				arena_sb_append_cstr(&cc->arena, &b->typedef_o, "struct {");
				backend_c_generate_typename_impl(cc, b, &b->typedef_o, ((LL_Type_Array*)queue.items[i].type)->element_type);
				arena_sb_sprintf(&cc->arena, &b->typedef_o, " val[%zu];} ", queue.items[i].type->width);
				break;
			default: TODO("implement anon type"); break;
			}
			output_fmt(BACKEND_C_ANON_TYPEDEF_PREFIX "%" PRId64 ";\n", queue.items[i].name_id);

			queue.items[i].name_id = -1;
		}
	/* } */

	fwrite(b->typedef_o.items, sizeof(*b->typedef_o.items), b->typedef_o.count, out_file);

	fwrite(b->o.items, sizeof(*b->o.items), b->o.count, out_file);
	fclose(out_file);
	return true;
}

int64_t backend_c_get_anon_struct_for_type(Compiler_Context* cc, LL_Backend_C* b, LL_Type* type) {
	LL_Backend_C_Anon_Type_Entry* result = NULL;
	String_Builder* o = &b->typedef_o;

	size_t hash = HASH_VALUE_FN(type, MAP_DEFAULT_SEED) % LEN(b->anon_entries);
	LL_Backend_C_Anon_Type_Entry* current = b->anon_entries[hash];
	while (current) {
		if (current->value == type) {
			result = current;
			break;
		}
		current = current->next;
	}

	if (!result) {
		result = arena_alloc(&cc->arena, sizeof(LL_Backend_C_Anon_Type_Entry));
		size_t hash = stbds_siphash_bytes(&type, sizeof(type), MAP_DEFAULT_SEED) % LEN(b->anon_entries);
		result->next = b->anon_entries[hash];
		b->anon_entries[hash] = result;

		result->value = type;

		/* output_str("typdef "); */
		switch (type->kind) {
		case LL_TYPE_ARRAY:
			arena_sb_append_cstr(&cc->arena, &b->typedef_o, "#if 0\n");
			backend_c_generate_typename_impl(cc, b, &b->typedef_o, ((LL_Type_Array*)type)->element_type);
			arena_sb_append_cstr(&cc->arena, &b->typedef_o, "\n#endif\n");
			/* arena_sb_sprintf(&cc->arena, &b->typedef_o, " val[%zu];}", type->width); */
			break;
		default: TODO("implement anon type"); break;
		}
		result->name_id = b->next_anon_id++;
		/* arena_sb_sprintf(&cc->arena, &b->typedef_o, BACKEND_C_ANON_TYPEDEF_PREFIX "%" PRId64 ";\n", result->name_id); */
	}

	return result->name_id;
}

void backend_c_generate_statement(Compiler_Context* cc, LL_Backend_C* b, String_Builder* o, Ast_Base* stmt) {
	int i;
	for (i = 0; i < b->indent; ++i) {
		output_str("    ");
	}
	switch (stmt->kind) {
	case AST_KIND_BLOCK:
		output_str("{\n");
		b->indent++;
		for (i = 0; i < AST_AS(stmt, Ast_Block)->count; ++i) {
			backend_c_generate_statement(cc, b, o, AST_AS(stmt, Ast_Block)->items[i]);
			output_str("\n");
		}
		b->indent--;
		for (i = 0; i < b->indent; ++i) {
			output_str("    ");
		}
		output_str("}");
		break;
	case AST_KIND_VARIABLE_DECLARATION: {
		Ast_Variable_Declaration* var_decl = AST_AS(stmt, Ast_Variable_Declaration);
		if (var_decl->storage_class & LL_STORAGE_CLASS_EXTERN) break;
		assert(b->frames.count != 0);

		LL_Ir_Local var = {
			.ident = var_decl->ident,
		};
		var_decl->ir_index = FUNCTION()->locals_count++;
		/* arena_da_append(&cc->arena, &FUNCTION()->locals, var); */

		backend_c_generate_typename(cc, b, o, var_decl->type, &var_decl->ident->str);

		if (var_decl->initializer) {
			/* LL_Ir_Operand last_copy_operand = b->copy_operand; */
			/* b->copy_operand = LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index; */
			/* b->copy_operand = last_copy_operand; */

			switch (var_decl->ident->base.type->kind) {
			case LL_TYPE_ARRAY:
				/* if (value.kind != LL_BACKEND_C_VALUE_KIND_NONE) { */
					/* LL_Backend_Layout layout = cc->target->get_layout(var_decl->ident->base.type); */
					/* IR_APPEND_OP(LL_IR_OPCODE_MEMCOPY, LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index, op, layout.size); */
				/* } */
				output_str(" = ");
				backend_c_generate_expression(cc, b, o, var_decl->initializer, false);
				break;
			default:
				output_str(" = ");
				backend_c_generate_expression(cc, b, o, var_decl->initializer, false);
				/* IR_APPEND_OP(LL_IR_OPCODE_STORE, LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index, op); */
				break;
			}
		}
		output_str(";");

		break;
	}
	case AST_KIND_FUNCTION_DECLARATION: {
		Ast_Function_Declaration* fn_decl = AST_AS(stmt, Ast_Function_Declaration);

		LL_Backend_C_Frame frame = {
			.locals_count = 0,
		};
		arena_da_append(&cc->arena, &b->frames, frame);

		backend_c_generate_typename(cc, b, o, fn_decl->return_type, &fn_decl->ident->str);
		output_str("(");
		output_str(")");

		if (fn_decl->storage_class & LL_STORAGE_CLASS_EXTERN) {
			/* fn.flags |= LL_IR_FUNCTION_FLAG_EXTERN; */
		}

		if (fn_decl->body) {
			output_str(" ");
			backend_c_generate_statement(cc, b, o, fn_decl->body);
		} else {
			output_str(";");
		}
			output_str("\n");
		/* output_str("\n"); */
		b->frames.count -= 1;

		break;
	}
	default:
		backend_c_generate_expression(cc, b, o, stmt, false);
		output_str(";");
		break;
	}
}

void backend_c_generate_root(Compiler_Context* cc, LL_Backend_C* b, Ast_Base* stmt) {
	backend_c_generate_statement(cc, b, &b->o, stmt);
}

LL_Backend_C_Value backend_c_generate_expression(Compiler_Context* cc, LL_Backend_C* b, String_Builder* o, Ast_Base* expr, bool lvalue) {
	LL_Backend_C_Value result;
	LL_Ir_Opcode op, r1, r2;
	int i;

	switch (expr->kind) {
	case AST_KIND_BLOCK: {
		uint32_t last_block_value = b->block_value;
		b->block_value = FUNCTION()->locals_count++;

		output_str("({");
		for (i = 0; i < AST_AS(expr, Ast_Block)->count; ++i) {
			backend_c_generate_statement(cc, b, o, AST_AS(expr, Ast_Block)->items[i]);
			output_str(";\n");
		}
		output_str("})");
		
		/* result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, expr->type, b->block_value); */
		b->block_value = last_block_value;
		break;
	}
	case AST_KIND_LITERAL_INT:
		output_fmt("%" PRId64, AST_AS(expr, Ast_Literal)->i64);
		assert(!lvalue);
		break;
	case AST_KIND_LITERAL_STRING: {
		output_str("\"");

		size_t index = b->o.count;
		arena_da_reserve(&cc->arena, &b->o, b->o.count + AST_AS(expr, Ast_Literal)->str.len);
		for (int i = 0; index < b->o.count; ++index, ++i) {
			char c = AST_AS(expr, Ast_Literal)->str.ptr[i];
			switch (c) {
			case '\x00' ... '\x1f':
				arena_da_reserve(&cc->arena, &b->o, b->o.count + 3);
				b->o.items[index++] = '\\';
				b->o.items[index++] = 'x';

				if ((c - '\x00') / 16 >= 9) {
					b->o.items[index++] = ((c - '\x00') / 16 - 10 + 'a');
				} else {
					b->o.items[index++] = ((c - '\x00') / 16 + '0');
				}

				if ((c - '\x00') % 16 >= 9) {
					b->o.items[index] = ((c - '\x00') % 16 - 10 + 'a');
				} else {
					b->o.items[index] = ((c - '\x00') % 16 + '0');
				}
				break;
			default: b->o.items[index] = c; break;
			}
		}
		output_str("\"");
		break;
	}
	case AST_KIND_PRE_OP:
		switch (AST_AS(expr, Ast_Operation)->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
		case '-': {
			backend_c_generate_expression(cc, b, o, AST_AS(expr, Ast_Operation)->right, false);
			/* assert(false); */
			break;
		}
		case '*': {
			backend_c_generate_expression(cc, b, o, AST_AS(expr, Ast_Operation)->right, false);
			break;
		}
		case '&': {
			backend_c_generate_expression(cc, b, o, AST_AS(expr, Ast_Operation)->right, false);
			break;
		}
#pragma GCC diagnostic push
		default: break;
		}

		break;
	case AST_KIND_BINARY_OP:
		output_str("(");
		backend_c_generate_expression(cc, b, o, AST_AS(expr, Ast_Operation)->right, false);

		switch (AST_AS(expr, Ast_Operation)->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
		case '+': output_str("+"); break;
		case '-': output_str("-"); break;
		case '*': output_str("*"); break;
		case '/': output_str("/"); break;

		case '<': output_str("<"); break;
		case LL_TOKEN_KIND_LTE: output_str("<="); break;
		case '>': output_str(">"); break;
		case LL_TOKEN_KIND_GTE: output_str(">"); break;
		case LL_TOKEN_KIND_EQUALS: output_str("=="); break;
		case LL_TOKEN_KIND_NEQUALS: output_str("!="); break;

		case LL_TOKEN_KIND_ASSIGN_PERCENT: output_str("%="); break;
		case LL_TOKEN_KIND_ASSIGN_DIVIDE: output_str("/="); break;
		case LL_TOKEN_KIND_ASSIGN_TIMES: output_str("*="); break;
		case LL_TOKEN_KIND_ASSIGN_MINUS: output_str("-="); break;
		case LL_TOKEN_KIND_ASSIGN_PLUS: output_str("+="); break;

		case '=': output_str("="); break;
#pragma GCC diagnostic pop
		default: assert(false);
		}

		backend_c_generate_expression(cc, b, o, AST_AS(expr, Ast_Operation)->left, false);
		output_str(")");

		break;
	case AST_KIND_CAST: {
		Ast_Cast* cast = AST_AS(expr, Ast_Cast);

		output_str("(");
		output_str("(");
		backend_c_generate_typename(cc, b, o, &cast->base, NULL);
		output_str(")");
		backend_c_generate_expression(cc, b, o, cast->expr, false);
		output_str(")");
		break;
	}
	case AST_KIND_INVOKE: {
		assert(!lvalue);
		Ast_Invoke* inv = AST_AS(expr, Ast_Invoke);

		backend_c_generate_expression(cc, b, o, inv->expr, false);
		LL_Type_Function* fn_type = (LL_Type_Function*)inv->expr->type;

		output_str("(");
		for (i = 0; i < inv->arguments.count; ++i) {
			LL_Type* parameter_type;
			if (i >= fn_type->parameter_count - 1 && fn_type->is_variadic) {
				parameter_type = cc->typer->ty_int32;
			} else {
				parameter_type = fn_type->parameters[i];
			}

			if (i) {
				output_str(", ");
			}
			backend_c_generate_expression(cc, b, o, inv->arguments.items[i], false);
		}
		output_str(")");

		break;
	}
	case AST_KIND_ARRAY_INITIALIZER: {
		Ast_Initializer* lit = AST_AS(expr, Ast_Initializer);

		output_str("{{");

		uint64_t k;
		for (i = 0, k = 0; i < lit->count; ++i, ++k) {
			if (i) output_str(", ");
			if (lit->items[i]->kind == AST_KIND_KEY_VALUE) {
				Ast_Key_Value* kv = AST_AS(lit->items[i], Ast_Key_Value);
				output_str("[");
				backend_c_generate_expression(cc, b, o, kv->key, false);
				output_str("] = ");
				backend_c_generate_expression(cc, b, o, kv->value, false);
			} else {
				backend_c_generate_expression(cc, b, o, lit->items[i], false);
			}
		}

		output_str("}}");
#if 0
		/* assert((b->data_items.count & 0xF0000000u) == 0); // TODO: maybe support more */
		bool const_literal = true;
		LL_Type* element_type = ((LL_Type_Array*)lit->base.type)->element_type;
		LL_Backend_Layout layout = cc->target->get_layout(element_type);
		size_t size = max(layout.size, layout.alignment);

		uint8_t* last_initializer_ptr = b->initializer_ptr;
		uint8_t* initializer_ptr;
		if (!last_initializer_ptr) {
			initializer_ptr = arena_alloc(&cc->arena, lit->base.type->width * size);
			b->initializer_ptr = initializer_ptr;

			result = LL_IR_OPERAND_DATA_BIT | (uint32_t)b->data_items.count;
			IR_APPEND_OP(LL_IR_OPCODE_MEMCOPY, b->copy_operand, result, (uint32_t)(lit->base.type->width * size));
		}

		uint64_t k;
		for (i = 0, k = 0; i < lit->count; ++i, ++k) {
			if (lit->items[i]->kind == AST_KIND_KEY_VALUE) {
				printf("here %d\n", i);
				Ast_Key_Value* kv = AST_AS(lit->items[i], Ast_Key_Value);
				LL_Ir_Operand vvalue = ir_generate_expression(cc, b, kv->value, false);
				if (kv->key->has_const && kv->value->has_const) {

					if (layout.size <= 8) {
						b->initializer_ptr[kv->key->const_value.uval] = (uint8_t)kv->value->const_value.uval;
					} else if (layout.size <= 16) {
						((uint16_t*)b->initializer_ptr)[kv->key->const_value.uval] = (uint16_t)kv->value->const_value.uval;
					} else if (layout.size <= 32) {
						((uint32_t*)b->initializer_ptr)[kv->key->const_value.uval] = (uint32_t)kv->value->const_value.uval;
					} else if (layout.size <= 64) {
						((uint64_t*)b->initializer_ptr)[kv->key->const_value.uval] = (uint64_t)kv->value->const_value.uval;
					} else {
						TODO("implement other const size");
						/* memcpy(&b->initializer_ptr[kv->key->const_value.uval * layout.size], &kv->value->const_value.uval, sizeof(kv->value->const_value.uval)); */
					}
					k = kv->key->const_value.uval;
				} else {
					LL_Ir_Operand kvalue = ir_generate_expression(cc, b, kv->key, false);
					result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA_INDEX, element_type, b->copy_operand, kvalue, layout.size);
					IR_APPEND_OP(LL_IR_OPCODE_STORE, result, vvalue);
				}
			} else {
				printf("buiklsjdlfk %d\n", lit->items[i]->has_const);

				LL_Ir_Operand vvalue = ir_generate_expression(cc, b, lit->items[i], false);
				switch (lit->items[i]->type->kind) {
				case LL_TYPE_ARRAY:
					b->initializer_ptr += size;
					break;
				default:
					if (lit->items[i]->has_const) {
						if (layout.size <= 1) {
							b->initializer_ptr[k] = (uint8_t)lit->items[i]->const_value.uval;
						} else if (layout.size <= 2) {
							((uint16_t*)b->initializer_ptr)[k] = (uint16_t)lit->items[i]->const_value.uval;
						} else if (layout.size <= 4) {
							((uint32_t*)b->initializer_ptr)[k] = (uint32_t)lit->items[i]->const_value.uval;
							printf("write to pointer %p\n", b->initializer_ptr);
						} else if (layout.size <= 8) {
							((uint64_t*)b->initializer_ptr)[k] = (uint64_t)lit->items[i]->const_value.uval;
						} else {
							TODO("implement other const size");
							/* memcpy(&b->initializer_ptr[kv->key->const_value.uval * layout.size], &kv->value->const_value.uval, sizeof(kv->value->const_value.uval)); */
						}
					} else {
						result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA_INDEX, element_type, b->copy_operand, (uint32_t)k, layout.size);
						IR_APPEND_OP(LL_IR_OPCODE_STORE, result, vvalue);
					}
					break;
				}
			}
		}

		/* void* data_ptr = arena_alloc(&cc->arena, layout.size); */

		if (!last_initializer_ptr) {
			arena_da_append(&cc->arena, &b->data_items, ((LL_Ir_Data_Item) { .ptr = initializer_ptr, .len = lit->base.type->width * size }));
			b->initializer_ptr = last_initializer_ptr;
		}

		result = (uint32_t)-1;
		/* result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA, expr->type, result); */
#endif
		break;
	}
	case AST_KIND_INDEX: {
		Ast_Operation* op = AST_AS(expr, Ast_Operation);

		LL_Backend_Layout layout = cc->target->get_layout(expr->type);

		backend_c_generate_expression(cc, b, o, op->left, false);
		switch (op->left->type->kind) {
		case LL_TYPE_POINTER:
			output_str("[");
			backend_c_generate_expression(cc, b, o, op->right, false);
			output_str("]");
			break;
		case LL_TYPE_ARRAY:
			output_str(".val[");
			backend_c_generate_expression(cc, b, o, op->right, false);
			output_str("]");
			break;
		default: TODO("implement index type"); break;
		}

		break;
	}
	case AST_KIND_IDENT: {
		Ast_Ident* ident = AST_AS(expr, Ast_Ident);

		if (AST_AS(expr, Ast_Ident)->str.ptr == LL_KEYWORD_TRUE.ptr) {
			assert(!lvalue);
			output_str("true");
			break;
		} else if (AST_AS(expr, Ast_Ident)->str.ptr == LL_KEYWORD_FALSE.ptr) {
			assert(!lvalue);
			output_str("false");
			break;
		}

		arena_sb_append_buf(&cc->arena, &b->o, ident->str.ptr, ident->str.len);
		
		break;
	}
	case AST_KIND_CONST: {
		Ast_Marker* cf = AST_AS(expr, Ast_Marker);
		/* LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, b, cf->expr); */
		/* output_fmt("%" PRIu64, value.uval); */
		break;
	}
	case AST_KIND_RETURN: {
		Ast_Control_Flow* cf = AST_AS(expr, Ast_Control_Flow);
		if (cf->expr) {
			output_str("({ return ");
			backend_c_generate_expression(cc, b, o, cf->expr, false);
			output_str("; })");
		} else {
			output_str("({ return; })");
		}
		break;
	}
	case AST_KIND_BREAK: {
		Ast_Control_Flow* cf = AST_AS(expr, Ast_Control_Flow);
		if (cf->expr) {
			output_fmt("___l%u = ", b->block_value);
			backend_c_generate_expression(cc, b, o, cf->expr, false);
			output_str("; ");
		}
		output_str("({ break; })");
		break;
	}
	case AST_KIND_IF: {
		Ast_If* iff = AST_AS(expr, Ast_If);

		output_str("if (");
		backend_c_generate_expression(cc, b, o, iff->cond, false);
		output_str(")");

		if (iff->body) {
			if (iff->body->kind != AST_KIND_BLOCK) {
				output_str("{\n");
				backend_c_generate_expression(cc, b, o, iff->body, false);
				output_str("}");
			} else {
				backend_c_generate_expression(cc, b, o, iff->body, false);
			}
		}

		if (iff->else_clause) {
			output_str(" else ");
			if (iff->body->kind != AST_KIND_BLOCK) {
				output_str("{\n");
				backend_c_generate_expression(cc, b, o, iff->body, false);
				output_str("}");
			} else {
				backend_c_generate_expression(cc, b, o, iff->body, false);
			}
		}

		break;
	}
	case AST_KIND_FOR: {
		Ast_Loop* loop = AST_AS(expr, Ast_Loop);
		/* cond_block->ref1 = body_block; */

		/* LL_Ir_Block* end_block = ir_create_block(cc, b, true); */

		output_str("for (");

		if (loop->init) backend_c_generate_statement(cc, b, o, loop->init);
		output_str(";");

		if (loop->cond) {
			backend_c_generate_expression(cc, b, o, loop->cond, false);
		}
		output_str(";");

		if (loop->update) {
			backend_c_generate_expression(cc, b, o, loop->update, false);
		}
		output_str(")");

		if (loop->body) {
			if (loop->body->kind != AST_KIND_BLOCK) {
				output_str("{\n");
				backend_c_generate_statement(cc, b, o, loop->body);
				output_str("}");
			} else {
				backend_c_generate_statement(cc, b, o, loop->body);
			}
		} else {
			output_str(";");
		}

		break;
	}
	default:
		fprintf(stderr, "TODO: implement generate expr %d\n", expr->kind);
		break;
	}
	return result;
}

void backend_c_generate_typename_impl(Compiler_Context* cc, LL_Backend_C* b, String_Builder* o, LL_Type* type) {
	switch (type->kind) {
	case LL_TYPE_VOID: output_str("void"); break;
	case LL_TYPE_INT:
		if (type->width == 0)
			output_str("void");
		else
			output_fmt("int%zu_t", ((type->width + 1) / 8) * 8);
		break;
	case LL_TYPE_UINT:
		if (type->width == 0)
			output_str("void");
		else
			output_fmt("uint%zu_t", ((type->width + 1) / 8) * 8);
		break;
	case LL_TYPE_BOOL:
		if (type->width == 0)
			output_str("void");
		else if (type->width == 1)
			output_str("bool");
		else
			output_fmt("uint%zu_t", ((type->width + 1) / 8) * 8);
		break;
	case LL_TYPE_POINTER:
		// int ( * p )
		// need to do this first in case of
		// int ( * p ) []
		/* arena_sb_append_cstr(&cc->arena, right, ")"); */
		backend_c_generate_typename_impl(cc, b, o, ((LL_Type_Pointer*)type)->element_type);
		output_str("*");
	   	break;
	case LL_TYPE_ARRAY: {
		int64_t name_id = backend_c_get_anon_struct_for_type(cc, b, type);

		output_fmt(BACKEND_C_ANON_TYPEDEF_PREFIX "%" PRId64 "", name_id);

		// struct { int val[]; }* p
		// struct { struct { int val[]; } val[]; }* p
		/* output_str("struct{"); */
		/* backend_c_generate_typename_impl(cc, b, right, ((LL_Type_Array*)type)->element_type); */
		/* output_fmt(" val[%zu];}", type->width); */
		/* output_fmt("", type->type->width); */
	   	break;
	}
	default:
		TODO("implement other types");
		break;
	}
}

void backend_c_generate_typename(Compiler_Context* cc, LL_Backend_C* b, String_Builder* o, Ast_Base* type, String_View* var_name) {
	backend_c_generate_typename_impl(cc, b, o, type->type);
	if (var_name) {
		output_str(" ");
		arena_sb_append_buf(&cc->arena, o, var_name->ptr, var_name->len);
	}
	/* arena_sb_append_buf(&cc->arena, &b->o, right.items, right.count); */
}

