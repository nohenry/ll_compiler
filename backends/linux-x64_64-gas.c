
#include <inttypes.h>
#include <stdio.h>

#include "../core/core1.h"
#include "../src/backend.h"
#include "../src/common.h"
#include "../src/ast.h"
#include "../src/typer.h"

typedef struct {
	size_t count, capacity;
	uint64_t* items;
} Local_List;

typedef struct {
	Oc_String_Builder output;

	LL_Ir_Function* fn;
	Local_List locals;
} Linux_x86_64_Gas_Backend;

typedef struct {
	size_t size, alignment;
} Layout;

static Layout get_layout(LL_Type* ty) {
	switch (ty->kind) {
	case LL_TYPE_INT: return (Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_UINT: return (Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_FLOAT: return (Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_POINTER: return (Layout) { .size = 8, .alignment = 8 };
	default: return (Layout) { .size = 0, .alignment = 0 };
	}
}

static void write_operand(Compiler_Context* cc, Linux_x86_64_Gas_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand operand) {
	switch (operand & LL_IR_OPERAND_TYPE_MASK) {
	case LL_IR_OPERAND_IMMEDIATE_BIT: arena_sb_sprintf(&cc->arena, &b->output, "%d", operand & LL_IR_OPERAND_VALUE_MASK); break;
	case LL_IR_OPERAND_REGISTER_BIT: {
		LL_Type* type = b->fn->registers.items[operand & LL_IR_OPERAND_VALUE_MASK].type;
		switch (type->kind) {
		case LL_TYPE_INT:
		case LL_TYPE_UINT:
			switch (type->width) {
			case 8: oc_sb_append_char_str(&b->output, "al"); break;
			case 16: oc_sb_append_char_str(&b->output, "ax"); break;
			case 32: oc_sb_append_char_str(&b->output, "eax"); break;
			case 64: oc_sb_append_char_str(&b->output, "rax"); break;
			default: oc_assert(false);
			}
			break;
		case LL_TYPE_POINTER: oc_sb_append_char_str(&b->output, "rax"); break;
		default: oc_assert(false);
		}
		break;
	}
	case LL_IR_OPERAND_LOCAL_BIT: {
		LL_Type* type = b->fn->locals.items[operand & LL_IR_OPERAND_VALUE_MASK].ident->base.type;
		uint64_t offset = b->locals.items[operand & LL_IR_OPERAND_VALUE_MASK];
		switch (type->kind) {
		case LL_TYPE_INT:
		case LL_TYPE_UINT:
			switch (type->width) {
			case 8: oc_sb_append_char_str(&b->output, "byte"); break;
			case 16: oc_sb_append_char_str(&b->output, "word"); break;
			case 32: oc_sb_append_char_str(&b->output, "dword"); break;
			case 64: oc_sb_append_char_str(&b->output, "qword"); break;
			default: oc_assert(false);
			}
			arena_sb_sprintf(&cc->arena, &b->output, " ptr [rbp - %" PRIu64 "]", offset);
			break;
		case LL_TYPE_POINTER:
			arena_sb_sprintf(&cc->arena, &b->output, "qword ptr [rbp - %" PRIu64 "]", offset);
			break;
		default: oc_assert(false);
		}
		break;
	}
	}
}

void linux_x86_64_gas_init(Compiler_Context* cc, Linux_x86_64_Gas_Backend* b) {
	memset(&b->output, 0, sizeof(b->output));
	oc_sb_append_char_str(&b->output, BACKEND_INDENT ".text\n");
	oc_sb_append_char_str(&b->output, BACKEND_INDENT ".intel_syntax noprefix\n");
}

bool linux_x86_64_gas_write_to_file(Compiler_Context* cc, Linux_x86_64_Gas_Backend* b, char* filepath) {
	FILE* fptr;
	if (!(fptr = fopen(filepath, "w"))) {
		eprint("Unable to open output file: %s\n", filepath);
		return false;
	}

	return fwrite(b->output.ptr, 1, b->output.len, fptr) == b->output.len;
}

static void generate_block(Compiler_Context* cc, Linux_x86_64_Gas_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
	size_t i;
	for (i = 0; i < block->ops.count; ++i) {
		LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
		LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];

		switch (opcode) {
		case LL_IR_OPCODE_RET: break;
		case LL_IR_OPCODE_STORE:
			oc_sb_append_char_str(&b->output, BACKEND_INDENT "mov ");
			write_operand(cc, b, bir, operands[0]);
			oc_sb_append_char_str(&b->output, ", ");
			write_operand(cc, b, bir, operands[1]);
			oc_sb_append_char_str(&b->output, "\n");
		   	i += 2;
		   	break;
		case LL_IR_OPCODE_LOAD: i += 2; break;
		case LL_IR_OPCODE_INVOKE: {
			uint32_t count = operands[2];
			for (uint32_t j = 0; j < count; ++j) {
			}

			i += 3 + count;
		   	break;
		}
		case LL_IR_OPCODE_LEA:
			oc_sb_append_char_str(&b->output, BACKEND_INDENT "lea rax, ");
			write_operand(cc, b, bir, operands[1]);
			oc_sb_append_char_str(&b->output, "\n");
			break;
		}
	}
}

void linux_x86_64_gas_generate(Compiler_Context* cc, Linux_x86_64_Gas_Backend* b, LL_Backend_Ir* bir) {

	int fi;
	for (fi = 0; fi < bir->fns.count; ++fi) {
		LL_Ir_Function* fn = &bir->fns.items[fi];
		LL_Ir_Block* block = fn->entry;
		b->fn = fn;

		oc_array_reserve(&cc->tmp_arena, &b->locals, fn->locals.count);

		uint64_t offset = 0;
		for (int li = 0; li < fn->locals.count; ++li) {
			Layout l = get_layout(fn->locals.items[li].ident->base.type);
			offset = oc_align_forward(offset + l.size, l.alignment);
			b->locals.items[li] = offset;
		}

		/* printf("function " FMT_SV_FMT ":\n", FMT_SV_ARG(fn->ident->str)); */

		oc_sb_append_char_str(&b->output, BACKEND_INDENT ".globl ");
		oc_sb_append_string(&cc->arena, &b->output, fn->ident->str);
		oc_sb_append_char_str(&b->output, "\n");

		oc_sb_append_string(&cc->arena, &b->output, fn->ident->str);
		oc_sb_append_char_str(&b->output, ":\n");

		oc_sb_append_char_str(&b->output, BACKEND_INDENT "push rbp\n");
		oc_sb_append_char_str(&b->output, BACKEND_INDENT "mov rbp, rsp\n");

		int bi = 0;
		while (block) {
			generate_block(cc, b, bir, block);
			bi++;
			block = block->next;
		}

		oc_sb_append_char_str(&b->output, BACKEND_INDENT "pop rbp\n");
		oc_sb_append_char_str(&b->output, BACKEND_INDENT "ret\n\n");
	}
}

/* void linux_x86_64_gas_generate_statement(Compiler_Context* cc, Linux_x86_64_Gas_Backend* b, Ast_Base* stmt) { */
/* 	int i; */
/* 	switch (stmt->kind) { */
/* 	case AST_KIND_BLOCK: */
/* 		for (i = 0; i < AST_AS(stmt, Ast_Block)->count; ++i) { */
/* 			linux_x86_64_gas_generate_statement(cc, b, AST_AS(stmt, Ast_Block)->items[i]); */
/* 		} */
/* 		break; */
/* 	case AST_KIND_FUNCTION_DECLARATION: { */
/* 		Ast_Function_Declaration* fn_decl = AST_AS(stmt, Ast_Function_Declaration); */
/* 		if (fn_decl->storage_class & LL_STORAGE_CLASS_EXTERN) break; */

/* 		oc_sb_append_char_str(&b->output, BACKEND_INDENT ".globl "); */
/* 		oc_sb_append_string(&cc->arena, &b->output, fn_decl->ident->str); */
/* 		oc_sb_append_char_str(&b->output, "\n"); */

/* 		oc_sb_append_string(&cc->arena, &b->output, fn_decl->ident->str); */
/* 		oc_sb_append_char_str(&b->output, ":\n"); */

/* 		oc_sb_append_char_str(&b->output, BACKEND_INDENT "push rbp\n"); */
/* 		oc_sb_append_char_str(&b->output, BACKEND_INDENT "mov rbp, rsp\n"); */

/* 		if (fn_decl->body) { */
/* 			linux_x86_64_gas_generate_statement(cc, b, fn_decl->body); */
/* 		} */

/* 		oc_sb_append_char_str(&b->output, BACKEND_INDENT "pop rbp\n"); */
/* 		oc_sb_append_char_str(&b->output, BACKEND_INDENT "ret\n\n"); */
/* 		break; */
/* 	} */
/* 	} */
/* } */

/* void linux_x86_64_gas_generate_expression(Compiler_Context* cc, Linux_x86_64_Gas_Backend* b, Ast_Base* expr) { */
/* } */

