
#include "ir.h"
#include "../common.h"
#include "../ast.h"

static void ir_append_op(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Block* block, LL_Ir_Opcode opcode, LL_Ir_Operand* operands, size_t operands_count) {
	arena_da_append(&cc->arena, &block->ops, (uint32_t)opcode);
	arena_da_append_many(&cc->arena, &block->ops, (uint32_t*)operands, operands_count);
}

size_t ir_get_op_count(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Opcode* opcode_list, size_t i) {
	LL_Ir_Opcode opcode = opcode_list[i];

	switch (opcode) {
	case LL_IR_OPCODE_RET: return 1;
	case LL_IR_OPCODE_RETVALUE: return 2;
	case LL_IR_OPCODE_STORE: return 3;
	case LL_IR_OPCODE_LOAD: return 3;
	case LL_IR_OPCODE_ADD: return 4;
	case LL_IR_OPCODE_LEA: return 3;
    case LL_IR_OPCODE_INVOKEVALUE: {
		uint32_t count = opcode_list[i + 1 + 2];
		return 4 + count;
    }
	case LL_IR_OPCODE_INVOKE: {
		uint32_t count = opcode_list[i + 1 + 1];
		return 3 + count;
	}
	}
}

static void ir_gen_reverse_ops(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Block* block) {
	arena_da_reserve(&cc->arena, &block->rops, block->ops.count);

	size_t i;
	for (i = 0; i < block->ops.count;) {
		size_t count = ir_get_op_count(cc, b, block->ops.items, i);
		size_t dst_i = block->ops.count - i - count;
		memcpy(&block->rops.items[dst_i], &block->ops.items[i], sizeof(block->ops.items[i]) * count);
		i += count;
	}
}

#define IR_APPEND_OP(opcode, ...) ({ \
			LL_Ir_Operand _ops[] = {__VA_ARGS__}; \
			ir_append_op(cc, b, b->current_block, opcode, _ops, LEN(_ops)); \
		})

#define IR_APPEND_OP_DST(opcode, type_, ...) ({ \
			LL_Ir_Operand dst = NEXTREG(type_); \
			LL_Ir_Operand _ops[] = {dst,__VA_ARGS__}; \
			ir_append_op(cc, b, b->current_block, opcode, _ops, LEN(_ops)); \
			dst; \
		})

#define FUNCTION() (&b->fns.items[b->current_function])
#define BLOCK() (b->current_block)
#define NEXTREG(type_) ({ 																				\
			uint32_t reg = FUNCTION()->registers.count; 												\
			arena_da_append(&cc->arena, &FUNCTION()->registers, ((LL_Ir_Register){ .type = type_ })); 	\
			LL_IR_OPERAND_REGISTER_BIT | reg; 															\
		})
#define INDENT "    "

void ir_print_op(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Opcode* opcode_list, size_t i) {
	LL_Ir_Opcode opcode = opcode_list[i];
	LL_Ir_Operand* operands = &opcode_list[i + 1];
	int offset = 0;

	switch (opcode) {
	case LL_IR_OPCODE_RET: printf(INDENT "ret"); break;
	case LL_IR_OPCODE_RETVALUE: printf(INDENT "ret " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0])); break;
	case LL_IR_OPCODE_STORE: printf(INDENT "store " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;
	case LL_IR_OPCODE_LOAD: printf(INDENT OPERAND_FMT " = load " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;
	case LL_IR_OPCODE_LEA: printf(INDENT OPERAND_FMT " = lea " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;
	case LL_IR_OPCODE_ADD: printf(INDENT OPERAND_FMT " = add " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_INVOKEVALUE:
		offset = 1;
	case LL_IR_OPCODE_INVOKE: {
		uint32_t count = operands[1 + offset];
		if (opcode == LL_IR_OPCODE_INVOKEVALUE) {
			printf(INDENT OPERAND_FMT " = call " OPERAND_FMT " ", OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]));
		} else {
			printf(INDENT "call " OPERAND_FMT " ", OPERAND_FMT_VALUE(operands[0]));
		}
		for (uint32_t j = 0; j < count; ++j) {
			if (j > 0) printf(", ");
			printf(OPERAND_FMT, OPERAND_FMT_VALUE(operands[2 + offset + j]));
		}
		break;
	}
	}
}

static void ir_print_block(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Block* block) {
	size_t i;
	for (i = 0; i < block->ops.count; ) {
		ir_print_op(cc, b, block->ops.items, i);
		printf("\n");
		i += ir_get_op_count(cc, b, block->ops.items, i);
	}
}

static void ir_print(Compiler_Context* cc, LL_Backend_Ir* b) {
	int fi;
	for (fi = 0; fi < b->fns.count; ++fi) {
		LL_Ir_Function* fn = &b->fns.items[fi];
		LL_Ir_Block* block = fn->entry;
		printf("function " FMT_SV_FMT ":\n", FMT_SV_ARG(fn->ident->str));

		int bi = 0;
		while (block) {
			printf("b%d:\n", bi);
			ir_print_block(cc, b, block);
			bi++;
			block = block->next;
		}
		printf("\n");
	}
}

void ir_init(Compiler_Context* cc, LL_Backend_Ir* b) {
	memset(b, 0, sizeof(*b));
	b->current_function = -1;
}

bool ir_write_to_file(Compiler_Context* cc, LL_Backend_Ir* b, char* filepath) {
	ir_print(cc, b);
	return false;
}

void ir_generate_statement(Compiler_Context* cc, LL_Backend_Ir* b, Ast_Base* stmt) {
	int i;
	switch (stmt->kind) {
	case AST_KIND_BLOCK:
		for (i = 0; i < AST_AS(stmt, Ast_Block)->count; ++i) {
			ir_generate_statement(cc, b, AST_AS(stmt, Ast_Block)->items[i]);
		}
		break;
	case AST_KIND_VARIABLE_DECLARATION: {
		Ast_Variable_Declaration* var_decl = AST_AS(stmt, Ast_Variable_Declaration);
		if (var_decl->storage_class & LL_STORAGE_CLASS_EXTERN) break;
		assert(b->current_function != -1);

		LL_Ir_Local var = {
			.ident = var_decl->ident,
		};
		var_decl->ir_index = FUNCTION()->locals.count;
		arena_da_append(&cc->arena, &FUNCTION()->locals, var);

		if (var_decl->initializer) {
			LL_Ir_Operand op = ir_generate_expression(cc, b, var_decl->initializer, false);
			IR_APPEND_OP(LL_IR_OPCODE_STORE, LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index, op);
		}

		break;
	}
	case AST_KIND_FUNCTION_DECLARATION: {
		Ast_Function_Declaration* fn_decl = AST_AS(stmt, Ast_Function_Declaration);

		LL_Ir_Function fn = {
			.ident = fn_decl->ident,
			.entry = arena_alloc(&cc->arena, sizeof(LL_Ir_Block)),
			.exit = arena_alloc(&cc->arena, sizeof(LL_Ir_Block)),
			.flags = 0,
		};
		memset(&fn.entry->ops, 0, sizeof(fn.entry->ops));
		memset(&fn.entry->rops, 0, sizeof(fn.entry->rops));
		fn.entry->next = fn.entry->prev = NULL;
		fn.entry->did_branch = false;

		fn_decl->ir_index = b->fns.count;

		if (fn_decl->storage_class & LL_STORAGE_CLASS_EXTERN) {
			fn.flags |= LL_IR_FUNCTION_FLAG_EXTERN;
		}
		arena_da_append(&cc->arena, &b->fns, fn);

		if (fn_decl->body) {
			int32_t last_function = b->current_function;
			LL_Ir_Block* last_block = b->current_block;
			b->current_function = fn_decl->ir_index;
			b->current_block = fn.entry;

			ir_generate_statement(cc, b, fn_decl->body);

			b->current_block->next = fn.exit;
			if (!b->current_block->did_branch) {
				b->current_block = fn.exit;
				IR_APPEND_OP(LL_IR_OPCODE_RET);
			}
			ir_gen_reverse_ops(cc, b, b->current_block);
			b->current_block = last_block;
			b->current_function = last_function;
		}
		break;
	}
	default:
		ir_generate_expression(cc, b, stmt, false);
		break;
	}
}

LL_Ir_Operand ir_generate_expression(Compiler_Context* cc, LL_Backend_Ir* b, Ast_Base* expr, bool lvalue) {
	LL_Ir_Operand result = 696969;
	LL_Ir_Opcode op, r1, r2;
	int i;

	switch (expr->kind) {
	case AST_KIND_LITERAL_INT:
		if (AST_AS(expr, Ast_Literal)->i64 >= 0 && AST_AS(expr, Ast_Literal)->i64 <= 0xFFFFFFF) {
			return AST_AS(expr, Ast_Literal)->i64;
		} else {
			fprintf(stderr, "TODO: implement bigger ints\n");
		}
		assert(!lvalue);
		break;
	case AST_KIND_LITERAL_STRING: {
		Ast_Literal* lit = AST_AS(expr, Ast_Literal);
		assert((b->data_items.count & 0xF0000000u) == 0); // TODO: maybe support more
		result = LL_IR_OPERAND_DATA_BIT | (uint32_t)b->data_items.count;
		arena_da_append(&cc->arena, &b->data_items, ((LL_Ir_Data_Item) { .ptr = lit->str.ptr, .len = lit->str.len }));
		result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA, expr->type, result);
		break;
	}
	case AST_KIND_PRE_OP:
		switch (AST_AS(expr, Ast_Operation)->op.kind) {
		case '-': {
			/* assert(false); */
			break;
		}
		case '*': {
			result = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
			if (!lvalue) {
				result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, expr->type, result);
			}
			break;
		}
		case '&': {
			result = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, true);
			result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA, expr->type, result);
			break;
		}
		default: break;
		}

		break;
	case AST_KIND_BINARY_OP:
		switch (AST_AS(expr, Ast_Operation)->op.kind) {
		case '+': op = LL_IR_OPCODE_ADD; break;
		default: assert(false);
		}
		r1 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, false);
		r2 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
		result = IR_APPEND_OP_DST(op, expr->type, r1, r2);
		break;
	case AST_KIND_INVOKE: {
		assert(!lvalue);
		Ast_Invoke* inv = AST_AS(expr, Ast_Invoke);

		LL_Ir_Operand invokee = ir_generate_expression(cc, b, inv->expr, true);
		LL_Type_Function* fn_type = (LL_Type_Function*)inv->expr->type;

		LL_Ir_Operand ops[3 + inv->arguments.count];

		int offset, opcode;
		if (fn_type->return_type && fn_type->return_type->kind != LL_TYPE_VOID) {
			result = NEXTREG(fn_type->return_type);
			ops[0] = result;
			opcode = LL_IR_OPCODE_INVOKEVALUE;
			offset = 1;
		} else {
			opcode = LL_IR_OPCODE_INVOKE;
			offset = 0;
		}

		ops[0 + offset] = invokee;
		ops[1 + offset] = inv->arguments.count;
		for (i = 0; i < inv->arguments.count; ++i) {
			ops[i + 2 + offset] = ir_generate_expression(cc, b, inv->arguments.items[i], false);
            switch (ops[i + 2 + offset] & LL_IR_OPERAND_TYPE_MASK) {
            case LL_IR_OPERAND_IMMEDIATE_BIT:
				ops[i + 2 + offset] = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, fn_type->parameters[i], (ops[i + 2 + offset] & LL_IR_OPERAND_VALUE_MASK));
                break;
            default: break;
            }
		}

		ir_append_op(cc, b, b->current_block, opcode, ops, 2 + offset + inv->arguments.count);

		break;
	}
	case AST_KIND_IDENT: {
		Ast_Ident* ident = AST_AS(expr, Ast_Ident);

		Ast_Base* decl = ident->resolved_scope->decl;
		switch (decl->kind) {
		case AST_KIND_VARIABLE_DECLARATION: result = LL_IR_OPERAND_LOCAL_BIT | AST_AS(decl, Ast_Variable_Declaration)->ir_index; break;
		case AST_KIND_FUNCTION_DECLARATION: result = LL_IR_OPERAND_FUNCTION_BIT | AST_AS(decl, Ast_Function_Declaration)->ir_index; break;
		case AST_KIND_PARAMETER: result = LL_IR_OPERAND_PARMAETER_BIT | AST_AS(decl, Ast_Parameter)->ir_index; break;
		default: assert(false);
		}
		printf("%.*s - %p -- %u\n", ident->str.len, ident->str.ptr, ident->resolved_scope, (result & LL_IR_OPERAND_VALUE_MASK));

		if (!lvalue) {
			result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, ident->base.type, result);
		}
		
		break;
	}
	case AST_KIND_RETURN: {
		Ast_Control_Flow* cf = AST_AS(expr, Ast_Control_Flow);
		if (cf->expr) {
			result = ir_generate_expression(cc, b, cf->expr, false);
			IR_APPEND_OP(LL_IR_OPCODE_RETVALUE, result);
		} else {
			IR_APPEND_OP(LL_IR_OPCODE_RET);
		}
		BLOCK()->did_branch = true;
		return 0;
	}
	default:
		fprintf(stderr, "TODO: implement generate expr %d\n", expr->kind);
		break;
	}
	return result;
}

LL_Type* ir_get_operand_type(LL_Ir_Function* fn, LL_Ir_Operand operand) {
	switch (operand & LL_IR_OPERAND_TYPE_MASK) {
	case LL_IR_OPERAND_IMMEDIATE_BIT:
		assert(false);
		return NULL;
	case LL_IR_OPERAND_REGISTER_BIT:
		return fn->registers.items[operand & LL_IR_OPERAND_VALUE_MASK].type;
	case LL_IR_OPERAND_LOCAL_BIT:
		return fn->locals.items[operand & LL_IR_OPERAND_VALUE_MASK].ident->base.type;
	case LL_IR_OPERAND_PARMAETER_BIT: {
		LL_Type_Function* fn_type = (LL_Type_Function*)fn->ident->base.type;
		return fn_type->parameters[operand & LL_IR_OPERAND_VALUE_MASK];
	}
	case LL_IR_OPERAND_FUNCTION_BIT:
		return fn->ident->base.type;
	default: assert(false);
	}
}

