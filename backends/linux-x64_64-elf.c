
#include <stdio.h>

#include "../arena.h"
#include "../backend.h"
#include "../common.h"
#include "../arena.h"
#include "../ast.h"
#include "../typer.h"

#include "x86_64_common.h"

typedef struct {
	size_t count, capacity;
	uint8_t* items;
} Linux_x86_64_Elf_Ops_List;

typedef struct {
	size_t count, capacity;
	uint64_t* items;
} Linux_x86_64_Elf_Local_List;

typedef struct {
	X86_64_Machine_Code_Writer w;
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
	b->w.append_u8 = (__typeof__(b->w.append_u8))linux_x86_64_elf_append_op_segment_u8;
	b->w.append_u16 = (__typeof__(b->w.append_u16))linux_x86_64_elf_append_op_segment_u16;
	b->w.append_u32 = (__typeof__(b->w.append_u32))linux_x86_64_elf_append_op_segment_u32;
	b->w.append_u64 = (__typeof__(b->w.append_u64))linux_x86_64_elf_append_op_segment_u64;
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

void test_movs(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r64_i64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsp }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsi }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_r12 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_r13 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsp, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsi, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rax, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rdx, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rbx, .scale = 2 }) );
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rsp, .scale = 2 }) ); */
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_r12, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rbp, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rsi, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rdi, .scale = 2 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsp, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsi, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi, .displacement = 0x07 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi, .displacement = 0 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rax, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rdx, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rbx, .scale = 2 }) );
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rsp, .scale = 2 }) ); */
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rbp, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rsi, .scale = 2 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rdi, .scale = 2 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rax, .scale = 0, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 0, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rdx, .scale = 0, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rbx, .scale = 0, .displacement = 0x07 }) );
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rsp, .scale = 0, .displacement = 0x07 }) ); */
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rbp, .scale = 0, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rsi, .scale = 0, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rdi, .scale = 0, .displacement = 0x07 }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsi, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
}

void test_shifts(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
#define DO_TESTS(op) \
	X86_64_WRITE_INSTRUCTION(op, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
\
	X86_64_WRITE_INSTRUCTION(op, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) );

	DO_TESTS(OPCODE_SAL)
	DO_TESTS(OPCODE_SAR)
	DO_TESTS(OPCODE_SHL)
	DO_TESTS(OPCODE_SHR)
#undef DO_TESTS
}

void test_arith(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, int opcode) {
	X86_64_WRITE_INSTRUCTION(opcode, al_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }) );
	X86_64_WRITE_INSTRUCTION(opcode, ax_i16, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }) );
	X86_64_WRITE_INSTRUCTION(opcode, eax_i32, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rax_i32, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }) );

	X86_64_WRITE_INSTRUCTION(opcode, rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );

	X86_64_WRITE_INSTRUCTION(opcode, rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );

	X86_64_WRITE_INSTRUCTION(opcode, r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );

	X86_64_WRITE_INSTRUCTION(opcode, r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(opcode, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(opcode, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
	X86_64_WRITE_INSTRUCTION(opcode, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );

	X86_64_WRITE_INSTRUCTION(opcode, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );

	X86_64_WRITE_INSTRUCTION(opcode, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );

	X86_64_WRITE_INSTRUCTION(opcode, rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .immediate = 0x78 }) );

	X86_64_WRITE_INSTRUCTION(opcode, rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_64_WRITE_INSTRUCTION(opcode, rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
}

void test_imul(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
	X86_64_WRITE_INSTRUCTION(OPCODE_IMUL, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_IMUL, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_rax }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_IMUL, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_rax }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_IMUL, r16_rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_IMUL, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_IMUL, r16_rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_IMUL, r32_rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }) );

}

void test_incdec(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
#define DO_TESTS(op) \
	X86_64_WRITE_INSTRUCTION(op, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) ); \
\
	X86_64_WRITE_INSTRUCTION(op, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 }) );  \
	X86_64_WRITE_INSTRUCTION(op, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 }) ); \
	X86_64_WRITE_INSTRUCTION(op, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 }) ); \
\
	X86_64_WRITE_INSTRUCTION(op, r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) ); \
	X86_64_WRITE_INSTRUCTION(op, r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) );

	DO_TESTS(OPCODE_INC)
	DO_TESTS(OPCODE_DEC)

#undef DO_TESTS
}

void test_push(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
	X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx }) );

	X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, i8, ((X86_64_Instruction_Parameters) { .immediate = 0x69 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, i16, ((X86_64_Instruction_Parameters) { .immediate = 0x69 }) );
	X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, i32, ((X86_64_Instruction_Parameters) { .immediate = 0x69 }) );
}

static void linux_x86_64_elf_generate_block(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
	size_t i;
	for (i = 0; i < block->ops.count; ++i) {
		LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
		LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];
		printf("%zu\n", x86_64_instructions_table_size);

		switch (opcode) {
		case LL_IR_OPCODE_RET: break;
		case LL_IR_OPCODE_STORE:
			/* X86_64_OPERAND_MI_MEM(X86_64_OPCODE_MOV, qword, rbp, -0x10, 100); */

			/* test_movs(cc, b); */
 
 			/* test_shifts(cc, b); */
 			/* test_arith(cc, b, OPCODE_ADD); */
 			/* test_arith(cc, b, OPCODE_ADC); */
 			/* test_incdec(cc, b); */
 			/* test_push(cc, b); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_JECXZ, rel8, ((X86_64_Instruction_Parameters) { .relative = 0x60 }) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_JRCXZ, rel8, ((X86_64_Instruction_Parameters) { .relative = 0x60 }) ); */
			/* test_imul(cc, b); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_CMPSB, noarg, ((X86_64_Instruction_Parameters) {}) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_CMPSW, noarg, ((X86_64_Instruction_Parameters) {}) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_CMPSD, noarg, ((X86_64_Instruction_Parameters) {}) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_CMPSQ, noarg, ((X86_64_Instruction_Parameters) {}) ); */

			/* X86_64_WRITE_INSTRUCTION(OPCODE_IN, al_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x01u }) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_IN, ax_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x02u }) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_IN, eax_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x02u }) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_IN, al_dx, ((X86_64_Instruction_Parameters) {}) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_IN, ax_dx, ((X86_64_Instruction_Parameters) {}) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_IN, eax_dx, ((X86_64_Instruction_Parameters) {}) ); */

			/* X86_64_WRITE_INSTRUCTION(OPCODE_OUT, al_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x01u }) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_OUT, ax_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x02u }) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_OUT, eax_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x02u }) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_OUT, al_dx, ((X86_64_Instruction_Parameters) {}) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_OUT, ax_dx, ((X86_64_Instruction_Parameters) {}) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_OUT, eax_dx, ((X86_64_Instruction_Parameters) { .rep = X86_64_PREFIX_REP }) ); */

			/* X86_64_WRITE_INSTRUCTION(OPCODE_RET, noarg, ((X86_64_Instruction_Parameters) {}) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_RET, i16, ((X86_64_Instruction_Parameters) { .immediate = 0x10 }) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_RET_FAR, noarg, ((X86_64_Instruction_Parameters) {}) ); */
			/* X86_64_WRITE_INSTRUCTION(OPCODE_RET_FAR, i16, ((X86_64_Instruction_Parameters) { .immediate = 0x10 }) ); */

			X86_64_WRITE_INSTRUCTION(OPCODE_CALL, rel32, ((X86_64_Instruction_Parameters) { .immediate = 0x10 }) );
			X86_64_WRITE_INSTRUCTION(OPCODE_CALL, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx }) );
			X86_64_WRITE_INSTRUCTION(OPCODE_CALL, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx }) );

			// 0x55 -> 0b01 010 101
			

			/* X86_64_APPEND_OP_SEGMENT((uint8_t)0x48u); */
			/* X86_64_APPEND_OP_SEGMENT((uint8_t)0xb8u); */
			/* X86_64_APPEND_OP_SEGMENT(PUN((int64_t)0x04u, uint64_t)); */

			/* X86_64_APPEND_OP_SEGMENT((uint8_t)0xb8u); */
			/* X86_64_APPEND_OP_SEGMENT(PUN((int32_t)0x04u, uint32_t)); */

			/* X86_64_APPEND_OP_SEGMENT((uint8_t)0xc7u); */
			/* X86_64_APPEND_OP_SEGMENT((uint8_t)0xc3u); */
			/* X86_64_APPEND_OP_SEGMENT(PUN((int32_t)0x04u, uint32_t)); */

			/* c7 c0 04 00 00 00 */
			/* 48 c7 c0 04 00 00 00 */

			/* X86_64_OPERAND_MI_MEM(X86_64_OPCODE_MOV, word, rbp, 0xA0, 100); */
			/* X86_64_OPERAND_MI_MEM(X86_64_OPCODE_MOV, dword, rbp, -0x90, 100); */
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

