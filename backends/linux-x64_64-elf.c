
#include <stdio.h>

#include "../arena.h"
#include "../backend.h"
#include "../common.h"
#include "../arena.h"
#include "../ast.h"
#include "../typer.h"

typedef struct {
	size_t count, capacity;
	uint8_t* items;
} Linux_x86_64_Elf_Ops_List;

typedef struct {
	size_t count, capacity;
	uint64_t* items;
} Linux_x86_64_Elf_Local_List;

typedef struct {
	Linux_x86_64_Elf_Ops_List ops;

	LL_Ir_Function* fn;
	Linux_x86_64_Elf_Local_List locals;
} Linux_x86_64_Elf_Backend;

typedef struct {
	size_t size, alignment;
} Linux_x86_64_Elf_Layout;

void linux_x86_64_elf_append_op_segment_u8(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint8_t segment) {
	segment = AS_LITTLE_ENDIAN_U8(segment);
	arena_da_append(&cc->arena, &b->ops, segment);
}

void linux_x86_64_elf_append_op_segment_u16(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint16_t segment) {
	segment = AS_LITTLE_ENDIAN_U16(segment);
	arena_da_append_many(&cc->arena, &b->ops, (uint8_t*)&segment, 2);
}

void linux_x86_64_elf_append_op_segment_u32(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint32_t segment) {
	segment = AS_LITTLE_ENDIAN_U32(segment);
	arena_da_append_many(&cc->arena, &b->ops, (uint8_t*)&segment, 4);
}

void linux_x86_64_elf_append_op_segment_u64(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint64_t segment) {
	segment = AS_LITTLE_ENDIAN_U64(segment);
	arena_da_append_many(&cc->arena, &b->ops, (uint8_t*)&segment, 8);
}

#define X86_64_APPEND_OP_SEGMENT(segment) _Generic(segment, \
			uint8_t : linux_x86_64_elf_append_op_segment_u8, \
			uint16_t : linux_x86_64_elf_append_op_segment_u16, \
			uint32_t : linux_x86_64_elf_append_op_segment_u32, \
			uint64_t : linux_x86_64_elf_append_op_segment_u64 \
		)(cc, b, segment)


#define X86_64_OPCODE(count, bytes) ((uint32_t)((count << 24u) | bytes & 0xFFFFFFu))
#define X86_64_OPCODE_MOV X86_64_OPCODE(1, 0xC7)

#define X86_64_OPERAND_MI(opcode, size, base, offset, immediate) linux_x86_64_elf_operand_mi(cc, b, opcode, X86_MEMORY_WIDTH_ ## size, X86_OPERAND_REGISTER_ ## base, offset, immediate)
#define X86_64_OPERAND_ZO(opcode)

typedef enum {
	X86_OPERAND_REGISTER_rax = 0,
	X86_OPERAND_REGISTER_rcx,
	X86_OPERAND_REGISTER_rdx,
	X86_OPERAND_REGISTER_rbx,
	X86_OPERAND_REGISTER_rsp,
	X86_OPERAND_REGISTER_rbp,
	X86_OPERAND_REGISTER_rsi,
	X86_OPERAND_REGISTER_rdi,
	X86_OPERAND_REGISTER_r8,
	X86_OPERAND_REGISTER_r9,
	X86_OPERAND_REGISTER_r10,
	X86_OPERAND_REGISTER_r11,
	X86_OPERAND_REGISTER_r12,
	X86_OPERAND_REGISTER_r13,
	X86_OPERAND_REGISTER_r14,
	X86_OPERAND_REGISTER_r15,
} X86_Operand_Register;

typedef enum {
	X86_MEMORY_WIDTH_byte,
	X86_MEMORY_WIDTH_word,
	X86_MEMORY_WIDTH_dword,
	X86_MEMORY_WIDTH_qword,
} X86_Memory_Width;

void linux_x86_64_elf_operand_mi(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint32_t opcode, X86_Memory_Width width, X86_Operand_Register base, int64_t offset, int64_t immediate) {
	uint8_t mod;
	if (offset >= INT8_MIN && offset <= INT8_MAX) mod = 1;
	else if (offset >= INT32_MIN && offset <= INT32_MAX) mod = 2;
	else assert(false);
	uint8_t rm, sib = 0, rex = 0;

	if (base <= X86_OPERAND_REGISTER_rdi) {
		rm = base;
	} else {
		rm = base - X86_OPERAND_REGISTER_r8; rex |= 1;
	}

	if (rm == 5 && mod == 0) {
		/* Case where [RBP] turns into [RIP+disp32] */
		rm = 0;
		/* TODO: maybe? */
	} else if (rm == 4) {
		/* Case where RSP uses SIB */
		rm = 0;
		sib = /* TODO */ 0;
	}
	
	if (width == X86_MEMORY_WIDTH_qword) rex |= 0x8;
	else if (width == X86_MEMORY_WIDTH_word)
		X86_64_APPEND_OP_SEGMENT((uint8_t)0x66);
	if (rex)
		X86_64_APPEND_OP_SEGMENT((uint8_t)(0x40u | rex));


	switch (opcode >> 24u) {
		case 1:
			X86_64_APPEND_OP_SEGMENT((uint8_t)(opcode & 0xFF));
			break;
		case 2:
			X86_64_APPEND_OP_SEGMENT((uint8_t)0xFF);
			X86_64_APPEND_OP_SEGMENT((uint8_t)(opcode & 0xFF));
			break;
		default: assert(false);
	}

	X86_64_APPEND_OP_SEGMENT((uint8_t)((mod << 6u) | rm));

	if (offset >= INT8_MIN && offset <= INT8_MAX) X86_64_APPEND_OP_SEGMENT(PUN((int8_t)offset, uint8_t));
	else if (offset >= INT32_MIN && offset <= INT32_MAX) X86_64_APPEND_OP_SEGMENT(PUN((int32_t)offset, uint32_t));
	else assert(false);

	switch (width) {
	case X86_MEMORY_WIDTH_byte: X86_64_APPEND_OP_SEGMENT(PUN((int8_t)immediate, uint8_t)); break;
	case X86_MEMORY_WIDTH_word: X86_64_APPEND_OP_SEGMENT(PUN((int16_t)immediate, uint16_t)); break;
	case X86_MEMORY_WIDTH_dword: X86_64_APPEND_OP_SEGMENT(PUN((int32_t)immediate, uint32_t)); break;
	case X86_MEMORY_WIDTH_qword: X86_64_APPEND_OP_SEGMENT(PUN((int32_t)immediate, uint32_t)); break;
	}

	/* switch (base) { */
	/* 	case X86_OPERAND_REGISTER_rax: rm = 0; b = 0; break; */
	/* 	case X86_OPERAND_REGISTER_rbx: rm = 3; b = 0; break; */
	/* 	case X86_OPERAND_REGISTER_rcx: rm = 1; b = 0; break; */
	/* 	case X86_OPERAND_REGISTER_rdx: rm = 2; b = 0; break; */
	/* 	case X86_OPERAND_REGISTER_rsp: rm = 4; b = 0; break; */
	/* 	case X86_OPERAND_REGISTER_rbp: rm = 5; b = 0; break; */
	/* 	case X86_OPERAND_REGISTER_rsi: rm = 6; b = 0; break; */
	/* 	case X86_OPERAND_REGISTER_rdi: rm = 7; b = 0; break; */
	/* 	case X86_OPERAND_REGISTER_r8:  rm = 0; b = 1; break; */
	/* 	case X86_OPERAND_REGISTER_r9:  rm = 3; b = 1; break; */
	/* 	case X86_OPERAND_REGISTER_r10: rm = 1; b = 1; break; */
	/* 	case X86_OPERAND_REGISTER_r11: rm = 2; b = 1; break; */
	/* 	case X86_OPERAND_REGISTER_r12: rm = 4; b = 1; break; */
	/* 	case X86_OPERAND_REGISTER_r13: rm = 5; b = 1; break; */
	/* 	case X86_OPERAND_REGISTER_r14: rm = 6; b = 1; break; */
	/* 	case X86_OPERAND_REGISTER_r15: rm = 7; b = 1; break; */
	/* } */
}

static Linux_x86_64_Elf_Layout linux_x86_64_elf_get_layout(LL_Type* ty) {
	switch (ty->kind) {
	case LL_TYPE_INT: return (Linux_x86_64_Elf_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_UINT: return (Linux_x86_64_Elf_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_FLOAT: return (Linux_x86_64_Elf_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_POINTER: return (Linux_x86_64_Elf_Layout) { .size = 8, .alignment = 8 };
	default: return (Linux_x86_64_Elf_Layout) { .size = 0, .alignment = 0 };
	}
}

static void linux_x86_64_elf_write_operand(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand operand) {
	switch (operand & LL_IR_OPERAND_TYPE_MASK) {
	case LL_IR_OPERAND_IMMEDIATE_BIT:
		/* arena_sb_sprintf(&cc->arena, &b->output, "%d", operand & LL_IR_OPERAND_VALUE_MASK); */
	   	break;
	case LL_IR_OPERAND_REGISTER_BIT: {
		/* LL_Type* type = b->fn->registers.items[operand & LL_IR_OPERAND_VALUE_MASK].type; */
		/* switch (type->kind) { */
		/* case LL_TYPE_INT: */
		/* case LL_TYPE_UINT: */
		/* 	switch (type->width) { */
		/* 	case 8: arena_sb_append_cstr(&cc->arena, &b->output, "al"); break; */
		/* 	case 16: arena_sb_append_cstr(&cc->arena, &b->output, "ax"); break; */
		/* 	case 32: arena_sb_append_cstr(&cc->arena, &b->output, "eax"); break; */
		/* 	case 64: arena_sb_append_cstr(&cc->arena, &b->output, "rax"); break; */
		/* 	default: assert(false); */
		/* 	} */
		/* 	break; */
		/* default: assert(false); */
		/* } */
		break;
	}
	case LL_IR_OPERAND_LOCAL_BIT: {
		LL_Type* type = b->fn->locals.items[operand & LL_IR_OPERAND_VALUE_MASK].ident->base.type;
		uint64_t offset = b->locals.items[operand & LL_IR_OPERAND_VALUE_MASK];
		switch (type->kind) {
		case LL_TYPE_INT:
		case LL_TYPE_UINT:
			/* switch (type->width) { */
			/* case 8: arena_sb_append_cstr(&cc->arena, &b->output, "byte"); break; */
			/* case 16: arena_sb_append_cstr(&cc->arena, &b->output, "word"); break; */
			/* case 32: arena_sb_append_cstr(&cc->arena, &b->output, "dword"); break; */
			/* case 64: arena_sb_append_cstr(&cc->arena, &b->output, "qword"); break; */
			/* default: assert(false); */
			/* } */
			/* arena_sb_sprintf(&cc->arena, &b->output, " ptr [rbp - %lu]", offset); */
			break;
		case LL_TYPE_POINTER:
			/* arena_sb_sprintf(&cc->arena, &b->output, "qword ptr [rbp - %lu]", offset); */
			break;
		default: assert(false);
		}
		break;
	}
	}
}

void linux_x86_64_elf_init(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
	memset(&b->ops, 0, sizeof(b->ops));
	/* arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT ".text\n"); */
	/* arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT ".intel_syntax noprefix\n"); */
}

bool linux_x86_64_elf_write_to_file(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, char* filepath) {
	FILE* fptr;
	if (!(fptr = fopen(filepath, "w"))) {
		fprintf(stderr, "Unable to open output file: %s\n", filepath);
		return false;
	}

	return fwrite(b->ops.items, 1, b->ops.count, fptr) == b->ops.count;
}

static void linux_x86_64_elf_generate_block(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
	size_t i;
	for (i = 0; i < block->ops.count; ++i) {
		LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
		LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];

		switch (opcode) {
		case LL_IR_OPCODE_RET: break;
		case LL_IR_OPCODE_STORE:
							   printf("here\n");
			X86_64_OPERAND_MI(X86_64_OPCODE_MOV, qword, rbp, -0x10, 100);
			X86_64_OPERAND_MI(X86_64_OPCODE_MOV, word, rbp, 0xA0, 100);
			X86_64_OPERAND_MI(X86_64_OPCODE_MOV, dword, rbp, -0x90, 100);
			/* X86_64_OPERAND_MI(X86_64_OPCODE_MOV, byte, rbp, 0x10, 100); */
			/* arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "mov "); */
			/* write_operand(cc, b, bir, operands[0]); */
			/* arena_sb_append_cstr(&cc->arena, &b->output, ", "); */
			/* write_operand(cc, b, bir, operands[1]); */
			/* arena_sb_append_cstr(&cc->arena, &b->output, "\n"); */
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
		}
	}
}

void linux_x86_64_elf_generate(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir) {

	int fi;
	for (fi = 0; fi < bir->fns.count; ++fi) {
		LL_Ir_Function* fn = &bir->fns.items[fi];
		LL_Ir_Block* block = fn->entry;
		b->fn = fn;

		arena_da_reserve(&cc->tmp_arena, &b->locals, fn->locals.count);

		uint64_t offset = 0;
		for (int li = 0; li < fn->locals.count; ++li) {
			Linux_x86_64_Elf_Layout l = linux_x86_64_elf_get_layout(fn->locals.items[li].ident->base.type);
			offset = align_forward(offset + l.size, l.alignment);
			b->locals.items[li] = offset;
		}

		/* printf("function " FMT_SV_FMT ":\n", FMT_SV_ARG(fn->ident->str)); */

		/* arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT ".globl "); */
		/* arena_sb_append_strview(&cc->arena, &b->output, fn->ident->str); */
		/* arena_sb_append_cstr(&cc->arena, &b->output, "\n"); */

		/* arena_sb_append_strview(&cc->arena, &b->output, fn->ident->str); */
		/* arena_sb_append_cstr(&cc->arena, &b->output, ":\n"); */

		/* arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "push rbp\n"); */
		/* arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "mov rbp, rsp\n"); */

		int bi = 0;
		while (block) {
			linux_x86_64_elf_generate_block(cc, b, bir, block);
			bi++;
			block = block->next;
		}

		/* arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "pop rbp\n"); */
		/* arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "ret\n\n"); */
	}
}

/* void linux_x86_64_elf_generate_statement(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, Ast_Base* stmt) { */
/* 	int i; */
/* 	switch (stmt->kind) { */
/* 	case AST_KIND_BLOCK: */
/* 		for (i = 0; i < AST_AS(stmt, Ast_Block)->count; ++i) { */
/* 			linux_x86_64_elf_generate_statement(cc, b, AST_AS(stmt, Ast_Block)->items[i]); */
/* 		} */
/* 		break; */
/* 	case AST_KIND_FUNCTION_DECLARATION: { */
/* 		Ast_Function_Declaration* fn_decl = AST_AS(stmt, Ast_Function_Declaration); */
/* 		if (fn_decl->storage_class & LL_STORAGE_CLASS_EXTERN) break; */

/* 		arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT ".globl "); */
/* 		arena_sb_append_strview(&cc->arena, &b->output, fn_decl->ident->str); */
/* 		arena_sb_append_cstr(&cc->arena, &b->output, "\n"); */

/* 		arena_sb_append_strview(&cc->arena, &b->output, fn_decl->ident->str); */
/* 		arena_sb_append_cstr(&cc->arena, &b->output, ":\n"); */

/* 		arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "push rbp\n"); */
/* 		arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "mov rbp, rsp\n"); */

/* 		if (fn_decl->body) { */
/* 			linux_x86_64_elf_generate_statement(cc, b, fn_decl->body); */
/* 		} */

/* 		arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "pop rbp\n"); */
/* 		arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "ret\n\n"); */
/* 		break; */
/* 	} */
/* 	} */
/* } */

/* void linux_x86_64_elf_generate_expression(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, Ast_Base* expr) { */
/* } */

