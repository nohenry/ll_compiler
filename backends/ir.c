
#include "ir.h"
#include "../common.h"
#include "../ast.h"
#include "../eval.h"

static void ir_append_op(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Block_Ref block, LL_Ir_Opcode opcode, LL_Ir_Operand* operands, size_t operands_count) {
	arena_da_append(&cc->arena, &b->blocks.items[block].ops, (uint32_t)opcode);
	arena_da_append_many(&cc->arena, &b->blocks.items[block].ops, (uint32_t*)operands, operands_count);
}

size_t ir_get_op_count(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Opcode* opcode_list, size_t i) {
	LL_Ir_Opcode opcode = opcode_list[i];

	switch (opcode) {
	case LL_IR_OPCODE_RET: return 1;
	case LL_IR_OPCODE_RETVALUE: return 2;
	case LL_IR_OPCODE_STORE: return 3;
	case LL_IR_OPCODE_MEMCOPY: return 4;
	case LL_IR_OPCODE_LOAD: return 3;
	case LL_IR_OPCODE_CAST: return 3;

	case LL_IR_OPCODE_SUB:
	case LL_IR_OPCODE_MUL:
	case LL_IR_OPCODE_DIV:
	case LL_IR_OPCODE_LT:
	case LL_IR_OPCODE_LTE:
	case LL_IR_OPCODE_GT:
	case LL_IR_OPCODE_GTE:
	case LL_IR_OPCODE_EQ:
	case LL_IR_OPCODE_NEQ:
	case LL_IR_OPCODE_AND:
	case LL_IR_OPCODE_OR:
	case LL_IR_OPCODE_XOR:
	case LL_IR_OPCODE_ADD: return 4;

	case LL_IR_OPCODE_BRANCH: return 2;
	case LL_IR_OPCODE_BRANCH_COND: return 4;

	case LL_IR_OPCODE_LEA: return 3;
	case LL_IR_OPCODE_LEA_INDEX: return 5;
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

static void ir_gen_reverse_ops(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Block_Ref block_ref) {
	LL_Ir_Block* block = &b->blocks.items[block_ref];
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

#define FUNCTION() ((b->current_function & CURRENT_CONST_STACK) ? (&b->const_stack.items[b->current_function & CURRENT_INDEX]) : (&b->fns.items[b->current_function & CURRENT_INDEX]))
#define BLOCK() (&b->blocks.items[b->current_block])
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
	case LL_IR_OPCODE_MEMCOPY: printf(INDENT "memcpy " OPERAND_FMT ", " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_LOAD: printf(INDENT OPERAND_FMT " = load " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;
	case LL_IR_OPCODE_LEA: printf(INDENT OPERAND_FMT " = lea " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;
	case LL_IR_OPCODE_LEA_INDEX: printf(INDENT OPERAND_FMT " = lea " OPERAND_FMT " + " OPERAND_FMT " * " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2]), OPERAND_FMT_VALUE(operands[3])); break;
	case LL_IR_OPCODE_CAST: printf(INDENT OPERAND_FMT " = cast " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;

	case LL_IR_OPCODE_ADD: printf(INDENT OPERAND_FMT " = add " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_SUB: printf(INDENT OPERAND_FMT " = sub " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_MUL: printf(INDENT OPERAND_FMT " = mul " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_DIV: printf(INDENT OPERAND_FMT " = div " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_LT: printf(INDENT OPERAND_FMT " = lt " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_LTE: printf(INDENT OPERAND_FMT " = lte " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_GT: printf(INDENT OPERAND_FMT " = gt " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_GTE: printf(INDENT OPERAND_FMT " = gte " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_EQ: printf(INDENT OPERAND_FMT " = eq " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_NEQ: printf(INDENT OPERAND_FMT " = neq " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_AND: printf(INDENT OPERAND_FMT " = and " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_OR: printf(INDENT OPERAND_FMT " = or " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
	case LL_IR_OPCODE_XOR: printf(INDENT OPERAND_FMT " = xor " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;

	case LL_IR_OPCODE_BRANCH: printf(INDENT "branch babs%u", operands[0] & LL_IR_OPERAND_VALUE_MASK); break;
	case LL_IR_OPCODE_BRANCH_COND: printf(INDENT "branch_cond " OPERAND_FMT ", babs%u, babs%u", OPERAND_FMT_VALUE(operands[0]), operands[1] & LL_IR_OPERAND_VALUE_MASK, operands[2] & LL_IR_OPERAND_VALUE_MASK); break;

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
		LL_Ir_Block_Ref block = fn->entry;

		if (fn->ident) {
			printf("function " FMT_SV_FMT ":\n", FMT_SV_ARG(fn->ident->str));
		} else {
			printf("function:\n");
		}

		int bi = 0;
		while (block) {
			printf("b%d(abs%d):\n", bi, block);
			ir_print_block(cc, b, &b->blocks.items[block]);
			bi++;
			block = b->blocks.items[block].next;
		}
		printf("\n");
	}
}

void ir_init(Compiler_Context* cc, LL_Backend_Ir* b) {
	memset(b, 0, sizeof(*b));
	b->current_function = -1;
	// first function is used for const eval
	arena_da_append(&cc->arena, &b->fns, ((LL_Ir_Function){}));
	arena_da_append(&cc->arena, &b->blocks, ((LL_Ir_Block){}));
}

bool ir_write_to_file(Compiler_Context* cc, LL_Backend_Ir* b, char* filepath) {
	ir_print(cc, b);
	return false;
}

LL_Ir_Block_Ref ir_create_block(Compiler_Context* cc, LL_Backend_Ir* b, bool append) {
	LL_Ir_Block block;
	LL_Ir_Block_Ref result = b->free_block ? b->free_block : b->blocks.count;

	memset(&block.ops, 0, sizeof(block.ops));
	memset(&block.rops, 0, sizeof(block.rops));
	block.did_branch = false;
	block.bi = FUNCTION()->block_count;
	FUNCTION()->block_count++; 
	block.ref1 = block.ref2 = 0;
	block.generated_offset = -1;

	block.next = 0;
	if (append) {
		block.prev = FUNCTION()->exit;
		b->blocks.items[FUNCTION()->exit].next = result;
		FUNCTION()->exit = result;
	} else {
		block.prev = 0;
	}

	if (b->free_block) {
		b->free_block = b->blocks.items[b->free_block].next;
		memcpy(&b->blocks.items[b->free_block], &block, sizeof(block));
	} else {
		arena_da_append(&cc->arena, &b->blocks, block);
	}

	return result;
}

LL_Ir_Operand ir_generate_cast_if_needed(Compiler_Context* cc, LL_Backend_Ir* b, LL_Type* to_type, LL_Ir_Operand from, LL_Type* from_type) {
    if (to_type == from_type) return from;

    switch (from_type->kind) {
    case LL_TYPE_INT:
        switch (to_type->kind) {
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
                if (from_type->width == to_type->width) return from;
                break;
            default: fprintf(stderr, "\x1b[31;1mTODO\x1b[0m: handle cast types to %d\n", to_type->kind); break;
        }
        break;
    case LL_TYPE_UINT:
        switch (to_type->kind) {
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
                if (from_type->width == to_type->width) return from;
                break;
            default: fprintf(stderr, "\x1b[31;1mTODO\x1b[0m: handle cast types to %d\n", to_type->kind); break;
        }
        break;
    default: fprintf(stderr, "\x1b[31;1mTODO\x1b[0m: handle cast types from\n"); break;
    }

    return IR_APPEND_OP_DST(LL_IR_OPCODE_CAST, to_type, from);
}

LL_Ir_Operand ir_generate_lhs_load_if_needed(Compiler_Context* cc, LL_Backend_Ir* b, LL_Type* type, LL_Ir_Operand value) {
	switch (LL_IR_OPERAND_TYPE_MASK & value) {
	case LL_IR_OPERAND_IMMEDIATE_BIT:
		if (b->flags & LL_BACKEND_IR_FLAG_LHS_IMMEDIATE) return value;
		return IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, type, value);
	default: return value;
	}
}

LL_Ir_Operand ir_generate_rhs_load_if_needed(Compiler_Context* cc, LL_Backend_Ir* b, LL_Type* type, LL_Ir_Operand value) {
	switch (LL_IR_OPERAND_TYPE_MASK & value) {
	case LL_IR_OPERAND_IMMEDIATE_BIT:
		if (b->flags & LL_BACKEND_IR_FLAG_RHS_IMMEDIATE) return value;
		return IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, type, value);
	default: return value;
	}
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
			LL_Ir_Operand last_copy_operand = b->copy_operand;
			b->copy_operand = LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index;
			LL_Ir_Operand op = ir_generate_expression(cc, b, var_decl->initializer, false);
			b->copy_operand = last_copy_operand;

			switch (var_decl->ident->base.type->kind) {
			case LL_TYPE_ARRAY:
				if (op != (uint32_t)-1) {
					LL_Backend_Layout layout = cc->target->get_layout(var_decl->ident->base.type);
					IR_APPEND_OP(LL_IR_OPCODE_MEMCOPY, LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index, op, layout.size);
				}
				break;
			default:
				IR_APPEND_OP(LL_IR_OPCODE_STORE, LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index, op);
				break;
			}
		}

		break;
	}
	case AST_KIND_FUNCTION_DECLARATION: {
		Ast_Function_Declaration* fn_decl = AST_AS(stmt, Ast_Function_Declaration);

		LL_Ir_Block_Ref entry_block_ref = b->blocks.count;
		LL_Ir_Block entry_block = { 0 };
		entry_block.generated_offset = -1;
		arena_da_append(&cc->arena, &b->blocks, entry_block);

		LL_Ir_Function fn = {
			.ident = fn_decl->ident,
			.entry = entry_block_ref,
			.exit = entry_block_ref,
			.flags = 0,
			.generated_offset = LL_IR_FUNCTION_OFFSET_INVALID,
			.block_count = 1,
		};

		fn_decl->ir_index = b->fns.count;

		if (fn_decl->storage_class & LL_STORAGE_CLASS_EXTERN) {
			fn.flags |= LL_IR_FUNCTION_FLAG_EXTERN;
		}
		arena_da_append(&cc->arena, &b->fns, fn);

		if (fn_decl->body) {
			int32_t last_function = b->current_function;
			LL_Ir_Block_Ref last_block = b->current_block;
			b->current_function = fn_decl->ir_index;
			b->current_block = fn.entry;

			ir_generate_statement(cc, b, fn_decl->body);

			if (!b->blocks.items[b->current_block].did_branch) {
				b->current_block = FUNCTION()->exit;
				IR_APPEND_OP(LL_IR_OPCODE_RET);
			}

			LL_Ir_Block_Ref next_block = FUNCTION()->exit;
			while (next_block) {
				ir_gen_reverse_ops(cc, b, next_block);
				next_block = b->blocks.items[next_block].prev;
			}

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
	case AST_KIND_BLOCK: {
		LL_Ir_Operand last_block_value = b->block_value;

		Ast_Ident* block_ident = arena_alloc(&cc->arena, sizeof(Ast_Ident));
		block_ident->base.type = expr->type;
		block_ident->str.ptr = arena_sprintf(&cc->arena, "block_result\n");
		block_ident->str.len = strlen(block_ident->str.ptr);
		LL_Ir_Local var = {
			.ident = block_ident,
		};
		uint32_t index = FUNCTION()->locals.count;
		arena_da_append(&cc->arena, &FUNCTION()->locals, var);
		b->block_value = LL_IR_OPERAND_LOCAL_BIT | index;

		for (i = 0; i < AST_AS(expr, Ast_Block)->count; ++i) {
			ir_generate_expression(cc, b, AST_AS(expr, Ast_Block)->items[i], false);
		}

		result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, expr->type, b->block_value);
		b->block_value = last_block_value;
		break;
	}
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
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
			if (AST_AS(expr, Ast_Operation)->right->kind != AST_KIND_INDEX) {
				result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA, expr->type, result);
			}
			break;
		}
#pragma GCC diagnostic push
		default: break;
		}

		break;
	case AST_KIND_BINARY_OP:
		switch (AST_AS(expr, Ast_Operation)->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
		case '+': op = LL_IR_OPCODE_ADD; break;
		case '-': op = LL_IR_OPCODE_SUB; break;
		case '*': op = LL_IR_OPCODE_MUL; break;
		case '/': op = LL_IR_OPCODE_DIV; break;

		case '<':
			op = LL_IR_OPCODE_LT;
			goto DO_BIN_OP_BOOLEAN;
		case LL_TOKEN_KIND_LTE:
			op = LL_IR_OPCODE_LTE;
			goto DO_BIN_OP_BOOLEAN;
		case '>':
			op = LL_IR_OPCODE_GT;
			goto DO_BIN_OP_BOOLEAN;
		case LL_TOKEN_KIND_GTE:
			op = LL_IR_OPCODE_GTE;
			goto DO_BIN_OP_BOOLEAN;
		case LL_TOKEN_KIND_EQUALS:
		   	op = LL_IR_OPCODE_EQ;
			goto DO_BIN_OP_BOOLEAN;
		case LL_TOKEN_KIND_NEQUALS:
			op = LL_IR_OPCODE_NEQ;
			goto DO_BIN_OP_BOOLEAN;
DO_BIN_OP_BOOLEAN:
			r1 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, false);
			r1 = ir_generate_lhs_load_if_needed(cc, b, AST_AS(expr, Ast_Operation)->left->type, r1);
			r2 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
			r2 = ir_generate_lhs_load_if_needed(cc, b, AST_AS(expr, Ast_Operation)->right->type, r2);

			result = IR_APPEND_OP_DST(op, expr->type, r1, r2);
			return result;

		case LL_TOKEN_KIND_ASSIGN_PERCENT:
			op = LL_IR_OPCODE_DIV;
			goto DO_BIN_OP_ASSIGN_OP;
		case LL_TOKEN_KIND_ASSIGN_DIVIDE:
			op = LL_IR_OPCODE_DIV;
			goto DO_BIN_OP_ASSIGN_OP;
		case LL_TOKEN_KIND_ASSIGN_TIMES:
			op = LL_IR_OPCODE_MUL;
			goto DO_BIN_OP_ASSIGN_OP;
		case LL_TOKEN_KIND_ASSIGN_MINUS:
			op = LL_IR_OPCODE_SUB;
			goto DO_BIN_OP_ASSIGN_OP;
		case LL_TOKEN_KIND_ASSIGN_PLUS:
			op = LL_IR_OPCODE_ADD;
DO_BIN_OP_ASSIGN_OP:
			r1 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, false);
			r1 = ir_generate_cast_if_needed(cc, b, expr->type, r1, AST_AS(expr, Ast_Operation)->left->type);
			r1 = ir_generate_lhs_load_if_needed(cc, b, expr->type, r1);

			r2 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
			r2 = ir_generate_cast_if_needed(cc, b, expr->type, r2, AST_AS(expr, Ast_Operation)->right->type);

			r1 = IR_APPEND_OP_DST(op, expr->type, r1, r2);

			result = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, true);
			IR_APPEND_OP(LL_IR_OPCODE_STORE, result, r1);
			return r1;
		case '=':
			printf("append assign %d\n", b->current_block);
			result = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, true);
			r2 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
			r2 = ir_generate_cast_if_needed(cc, b, expr->type, r2, AST_AS(expr, Ast_Operation)->right->type);
			IR_APPEND_OP(LL_IR_OPCODE_STORE, result, r2);
			return r1;

#pragma GCC diagnostic pop
		default: assert(false);
		}

		r1 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, false);
        r1 = ir_generate_cast_if_needed(cc, b, expr->type, r1, AST_AS(expr, Ast_Operation)->left->type);
		r1 = ir_generate_lhs_load_if_needed(cc, b, expr->type, r1);

		r2 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
        r2 = ir_generate_cast_if_needed(cc, b, expr->type, r2, AST_AS(expr, Ast_Operation)->right->type);
		r2 = ir_generate_lhs_load_if_needed(cc, b, expr->type, r2);

		result = IR_APPEND_OP_DST(op, expr->type, r1, r2);
		break;
	case AST_KIND_CAST: {
		Ast_Cast* cast = AST_AS(expr, Ast_Cast);
		result = ir_generate_expression(cc, b, cast->expr, false);
		result = IR_APPEND_OP_DST(LL_IR_OPCODE_CAST, cast->base.type, result);
		break;
	}
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
			LL_Type* parameter_type;
			if (i >= fn_type->parameter_count - 1 && fn_type->is_variadic) {
				parameter_type = cc->typer->ty_int32;
			} else {
				parameter_type = fn_type->parameters[i];
			}

			ops[i + 2 + offset] = ir_generate_expression(cc, b, inv->arguments.items[i], false);
            switch (ops[i + 2 + offset] & LL_IR_OPERAND_TYPE_MASK) {
            case LL_IR_OPERAND_IMMEDIATE_BIT:
				ops[i + 2 + offset] = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, parameter_type, (ops[i + 2 + offset] & LL_IR_OPERAND_VALUE_MASK));
                break;
            default: break;
            }
		}

		ir_append_op(cc, b, b->current_block, opcode, ops, 2 + offset + inv->arguments.count);

		break;
	}
	case AST_KIND_ARRAY_INITIALIZER: {
		Ast_Initializer* lit = AST_AS(expr, Ast_Initializer);
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
		break;
	}
	case AST_KIND_INDEX: {
		Ast_Operation* op = AST_AS(expr, Ast_Operation);

		LL_Ir_Operand lvalue_op;
		if (op->left->type->kind == LL_TYPE_POINTER) {
			printf("doing pointer\n");
			lvalue_op = ir_generate_expression(cc, b, op->left, false);
		} else {
			lvalue_op = ir_generate_expression(cc, b, op->left, true);
		}
		LL_Ir_Operand rvalue_op = ir_generate_expression(cc, b, op->right, false);
		LL_Backend_Layout layout = cc->target->get_layout(expr->type);

		result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA_INDEX, expr->type, lvalue_op, rvalue_op, max(layout.size, layout.alignment));

		if (!lvalue && expr->type->kind != LL_TYPE_ARRAY) {
			result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, expr->type, result);
		}

		break;
	}
	case AST_KIND_IDENT: {
		Ast_Ident* ident = AST_AS(expr, Ast_Ident);

		if (AST_AS(expr, Ast_Ident)->str.ptr == LL_KEYWORD_TRUE.ptr) {
			assert(!lvalue);
			result = LL_IR_OPERAND_IMMEDIATE_BIT | 0u;
			break;
		} else if (AST_AS(expr, Ast_Ident)->str.ptr == LL_KEYWORD_FALSE.ptr) {
			assert(!lvalue);
			result = LL_IR_OPERAND_IMMEDIATE_BIT | 1u;
			break;
		}

		Ast_Base* decl = ident->resolved_scope->decl;
		switch (decl->kind) {
		case AST_KIND_VARIABLE_DECLARATION: result = LL_IR_OPERAND_LOCAL_BIT | AST_AS(decl, Ast_Variable_Declaration)->ir_index; break;
		case AST_KIND_FUNCTION_DECLARATION: result = LL_IR_OPERAND_FUNCTION_BIT | AST_AS(decl, Ast_Function_Declaration)->ir_index; break;
		case AST_KIND_PARAMETER: result = LL_IR_OPERAND_PARMAETER_BIT | AST_AS(decl, Ast_Parameter)->ir_index; break;
		default: assert(false);
		}

		if (!lvalue) {
			result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, ident->base.type, result);
		}
		
		break;
	}
	case AST_KIND_CONST: {
		Ast_Marker* cf = AST_AS(expr, Ast_Marker);
		LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, b, cf->expr);
		result = value.uval;
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
	case AST_KIND_BREAK: {
		Ast_Control_Flow* cf = AST_AS(expr, Ast_Control_Flow);
		if (cf->expr) {
			result = ir_generate_expression(cc, b, cf->expr, false);

			IR_APPEND_OP(LL_IR_OPCODE_STORE, b->block_value, result);
			/* if (result == LL_IR_OPERAND_REGISTER_BIT) { */
			/* 	b->block_value = result; */
			/* } else { */
			/* 	/1* b->block_value = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, cf->expr->type, result); *1/ */
			/* 	b->block_value = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, cf->expr->type, result); */
			/* } */
		}
		return 0;
	}
	case AST_KIND_IF: {
		Ast_If* iff = AST_AS(expr, Ast_If);
		LL_Ir_Block_Ref body_block = ir_create_block(cc, b, true);
		LL_Ir_Block_Ref else_block = ir_create_block(cc, b, true);
		LL_Ir_Block_Ref end_block = iff->else_clause ? ir_create_block(cc, b, true) : else_block;

		b->blocks.items[body_block].ref1 = b->current_block;
		b->blocks.items[end_block].ref1 = body_block;
		if (else_block != end_block) {
			b->blocks.items[else_block].ref1 = b->current_block;
			b->blocks.items[end_block].ref2 = else_block;
		}

		result = ir_generate_expression(cc, b, iff->cond, false);
		if ((result & LL_IR_OPERAND_TYPE_MASK) == LL_IR_OPERAND_IMMEDIATE_BIT) {
			result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, cc->typer->ty_bool, result);
			result = IR_APPEND_OP_DST(LL_IR_OPCODE_NEQ, cc->typer->ty_bool, result, 0);
		} else if (ir_get_operand_type(FUNCTION(), result)->kind != LL_TYPE_BOOL) {
			result = IR_APPEND_OP_DST(LL_IR_OPCODE_NEQ, cc->typer->ty_bool, result, 0);
		}
		IR_APPEND_OP(LL_IR_OPCODE_BRANCH_COND, result, body_block, else_block);

		if (iff->body) {
			b->current_block = body_block;
			ir_generate_statement(cc, b, iff->body);
			IR_APPEND_OP(LL_IR_OPCODE_BRANCH, end_block);
		}

		if (iff->else_clause) {
			b->current_block = else_block;
			ir_generate_statement(cc, b, iff->else_clause);
			IR_APPEND_OP(LL_IR_OPCODE_BRANCH, end_block);
		}

		b->current_block = end_block;

		return 0;
	}
	case AST_KIND_FOR: {
		Ast_Loop* loop = AST_AS(expr, Ast_Loop);
		LL_Ir_Block_Ref cond_block = ir_create_block(cc, b, true);
		LL_Ir_Block_Ref body_block = ir_create_block(cc, b, true);
		LL_Ir_Block_Ref end_block = ir_create_block(cc, b, true);
		b->blocks.items[end_block].ref1 = cond_block;
		/* cond_block->ref1 = body_block; */

		/* LL_Ir_Block* end_block = ir_create_block(cc, b, true); */

		if (loop->init) ir_generate_statement(cc, b, loop->init);

		if (loop->cond) {
			b->current_block = cond_block;
			result = ir_generate_expression(cc, b, loop->cond, false);
			IR_APPEND_OP(LL_IR_OPCODE_BRANCH_COND, result, body_block, end_block);
		}

		b->current_block = body_block;
		if (loop->body) {
			ir_generate_statement(cc, b, loop->body);
		}

		if (loop->update) {
			ir_generate_expression(cc, b, loop->update, false);
			IR_APPEND_OP(LL_IR_OPCODE_BRANCH, cond_block);
		}

		b->current_block = end_block;

		/* if (loop->cond) { */
		/* 	result = ll_typer_type_expression(cc, typer, loop->cond, typer->ty_int32); */
		/* 	switch (result->kind) { */
		/* 	case LL_TYPE_POINTER: */
		/* 	case LL_TYPE_ANYINT: */
		/* 	case LL_TYPE_UINT: */
		/* 	case LL_TYPE_INT: break; */
		/* 	default: */
		/* 		fprintf(stderr, "\x1b[31;1merror:\x1b[0m if statement condition should be boolean, integer or pointer\n"); */
		/* 		break; */
		/* 	} */
		/* } */
		/* if (loop->update) ll_typer_type_expression(cc, typer, loop->update, NULL); */

		/* if (loop->body) { */
		/* 	ll_typer_type_statement(cc, typer, loop->body); */
		/* } */

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

