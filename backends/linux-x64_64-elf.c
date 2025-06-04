
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

#define X86_64_OPERAND_MI_MEM(opcode, size, base, offset, immediate) linux_x86_64_elf_operand_mi(cc, b, opcode, X86_MEMORY_WIDTH_ ## size, X86_OPERAND_REGISTER_ ## base, offset, immediate, 0)
#define X86_64_OPERAND_MI_MEM_EXT(opcode, ext, size, base, offset, immediate) linux_x86_64_elf_operand_mi(cc, b, opcode, X86_MEMORY_WIDTH_ ## size, X86_OPERAND_REGISTER_ ## base, offset, immediate, ext)
#define X86_64_OPERAND_MI_REG(opcode, size, base, offset, immediate) linux_x86_64_elf_operand_mi(cc, b, opcode, X86_MEMORY_WIDTH_ ## size, X86_OPERAND_REGISTER_ ## base, offset, immediate, 0)
#define X86_64_OPERAND_MI_REG_EXT(opcode, ext, size, base, offset, immediate) linux_x86_64_elf_operand_mi(cc, b, opcode, X86_MEMORY_WIDTH_ ## size, X86_OPERAND_REGISTER_ ## base, offset, immediate, ext)

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

#define INVALID_OP 0xD6

#define MAKE_OPERANDS0() (0u)
#define MAKE_OPERANDS1(op) ((op) << 24u)
#define MAKE_OPERANDS2(op1, op2) (MAKE_OPERANDS1(op1) | ((op2) << 16u))
#define MAKE_OPERANDS3(op1, op2, op3) (MAKE_OPERANDS2(op1, op2) | ((op3) << 8u))
#define MAKE_OPERANDS4(op1, op2, op3, op4) (MAKE_OPERANDS3(op1, op2, op3) | (op4))

#define GET_MACRO(_0, _1, _2, _3, _4, NAME, ...) NAME
#define MAKE_OPERANDS(...) GET_MACRO(_0, ##__VA_ARGS__, MAKE_OPERANDS4, MAKE_OPERANDS3, MAKE_OPERANDS2, MAKE_OPERANDS1, MAKE_OPERANDS0)(__VA_ARGS__)

#define GET_OPERAND0(operands) ((operands >> 24u) & 0xFF)
#define GET_OPERAND1(operands) ((operands >> 16u) & 0xFF)
#define GET_OPERAND2(operands) ((operands >> 8u) & 0xFF)
#define GET_OPERAND3(operands) (operands & 0xFF)

/* 31    24 23    16 15     8 7      0 */
/* 00000000 00000000 00000000 00000000 */

#define mod_rm(r) ((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)r) & 0x1F)
#define mod_reg(r) ((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)r) & 0x1F)
#define add_to_opcode (((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)
#define imm (((uint32_t)OPERANDS_TYPE_imm) << 5)

enum {
	OPCODE_MOVE,
	OPCODE_SAL,
	OPCODE_SAR,
	OPCODE_SHL,
	OPCODE_SHR,
};

enum {
	OPERANDS_TYPE_modrm = 0,
	OPERANDS_TYPE_modreg,
	OPERANDS_TYPE_add_to_opcode,
	OPERANDS_TYPE_imm,
} X86_Operands_Type;

typedef struct {
	uint32_t opcode;
	uint32_t operands; // top 3 is Operands_Type, bottom 5 is argument
} X86_Instruction_Variant;

typedef struct {
	X86_Instruction_Variant
		rm8_r8, rm16_r16, rm32_r32, rm64_r64,
		r8_rm8, r16_rm16, r32_rm32, r64_rm64,
		r8_i8, r16_i16, r32_i32, r64_i64,
		rm8_i8, rm16_i16, rm32_i32, rm64_i32, rm16_i8, rm32_i8, rm64_i8;
} X86_Instruction;

typedef enum {
	X86_VARIANT_KIND_rm8_r8,
	X86_VARIANT_KIND_rm16_r16,
	X86_VARIANT_KIND_rm32_r32,
	X86_VARIANT_KIND_rm64_r64,

	X86_VARIANT_KIND_r8_rm8,
	X86_VARIANT_KIND_r16_rm16,
	X86_VARIANT_KIND_r32_rm32,
	X86_VARIANT_KIND_r64_rm64,

	X86_VARIANT_KIND_r8_i8,
	X86_VARIANT_KIND_r16_i16,
	X86_VARIANT_KIND_r32_i32,
	X86_VARIANT_KIND_r64_i64,

	X86_VARIANT_KIND_rm8_i8,
	X86_VARIANT_KIND_rm16_i16,
	X86_VARIANT_KIND_rm32_i32,
	X86_VARIANT_KIND_rm64_i32,
	X86_VARIANT_KIND_rm16_i8,
	X86_VARIANT_KIND_rm32_i8,
	X86_VARIANT_KIND_rm64_i8,
} X86_Variant_Kind;

const X86_Instruction instructions[] = {
	[OPCODE_MOVE] = {
		.rm8_r8  	= { .opcode = 0x88u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.rm16_r16  	= { .opcode = 0x89u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.rm32_r32  	= { .opcode = 0x89u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.rm64_r64  	= { .opcode = 0x89u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },

		.r8_rm8  	= { .opcode = 0x8Au, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r16_rm16  	= { .opcode = 0x8Bu, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r32_rm32  	= { .opcode = 0x8Bu, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm64  	= { .opcode = 0x8Bu, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },

		.r8_i8  	= { .opcode = 0xB0u, .operands = MAKE_OPERANDS(add_to_opcode, imm) },
		.r16_i16  	= { .opcode = 0xB8u, .operands = MAKE_OPERANDS(add_to_opcode, imm) },
		.r32_i32  	= { .opcode = 0xB8u, .operands = MAKE_OPERANDS(add_to_opcode, imm) },
		.r64_i64  	= { .opcode = 0xB8u, .operands = MAKE_OPERANDS(add_to_opcode, imm) },

		.rm8_i8 	= { .opcode = 0xC6u, .operands = MAKE_OPERANDS(mod_rm(0), imm) },
		.rm16_i16	= { .opcode = 0xC7u, .operands = MAKE_OPERANDS(mod_rm(0), imm) },
		.rm32_i32 	= { .opcode = 0xC7u, .operands = MAKE_OPERANDS(mod_rm(0), imm) },
		.rm64_i32 	= { .opcode = 0xC7u, .operands = MAKE_OPERANDS(mod_rm(0), imm) },
	},
	[OPCODE_SAL] = {
		.rm8_i8 	= { .opcode = 0xC0u, .operands = MAKE_OPERANDS(mod_rm(4), imm) },
		.rm16_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(4), imm) },
		.rm32_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(4), imm) },
		.rm64_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(4), imm) },
	},
	[OPCODE_SAR] = {
		.rm8_i8 	= { .opcode = 0xC0u, .operands = MAKE_OPERANDS(mod_rm(7), imm) },
		.rm16_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(7), imm) },
		.rm32_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(7), imm) },
		.rm64_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(7), imm) },
	},
	[OPCODE_SHL] = {
		.rm8_i8 	= { .opcode = 0xC0u, .operands = MAKE_OPERANDS(mod_rm(4), imm) },
		.rm16_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(4), imm) },
		.rm32_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(4), imm) },
		.rm64_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(4), imm) },
	},
	[OPCODE_SHR] = {
		.rm8_i8 	= { .opcode = 0xC0u, .operands = MAKE_OPERANDS(mod_rm(5), imm) },
		.rm16_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(5), imm) },
		.rm32_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(5), imm) },
		.rm64_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(5), imm) },
	},
};

#define X86_REG_BASE (0x08u)

typedef struct {
	uint8_t reg0, reg1, reg2, reg3;
	int64_t displacement, immediate;
} X86_Instruction_Parameters;

bool x86_uses_modrm(X86_Variant_Kind kind) {
	switch (kind) {
	case X86_VARIANT_KIND_rm8_r8:
	case X86_VARIANT_KIND_rm16_r16:
	case X86_VARIANT_KIND_rm32_r32:
	case X86_VARIANT_KIND_rm64_r64:

	case X86_VARIANT_KIND_r8_rm8:
	case X86_VARIANT_KIND_r16_rm16:
	case X86_VARIANT_KIND_r32_rm32:
	case X86_VARIANT_KIND_r64_rm64:

	case X86_VARIANT_KIND_rm8_i8:
	case X86_VARIANT_KIND_rm16_i16:
	case X86_VARIANT_KIND_rm32_i32:
	case X86_VARIANT_KIND_rm64_i32:
	case X86_VARIANT_KIND_rm16_i8:
	case X86_VARIANT_KIND_rm32_i8:
	case X86_VARIANT_KIND_rm64_i8:
		return true;
	default: return false;
	}
}

void linux_x86_64_elf_write_instruction(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, X86_Variant_Kind variant, X86_Instruction_Variant instruction, X86_Instruction_Parameters parameters) {
	uint8_t mod, rex = 0, prefix[4], prefixi = 0;
	int i;

	switch (variant) {
	case X86_VARIANT_KIND_rm64_r64:
	case X86_VARIANT_KIND_r64_rm64:
	case X86_VARIANT_KIND_r64_i64:
	case X86_VARIANT_KIND_rm64_i32:
	case X86_VARIANT_KIND_rm64_i8:
		rex |= 0x48;
		break;
	case X86_VARIANT_KIND_rm16_r16:
	case X86_VARIANT_KIND_r16_rm16:
	case X86_VARIANT_KIND_r16_i16:
	case X86_VARIANT_KIND_rm16_i16:
	case X86_VARIANT_KIND_rm16_i8:
		prefix[prefixi++] = 0x66;
		break;
	default: break;
	}

	if (rex) X86_64_APPEND_OP_SEGMENT(rex);
	for (i = 0; i < prefixi; ++i) {
		X86_64_APPEND_OP_SEGMENT(prefix[i]);
	}

	X86_64_APPEND_OP_SEGMENT((uint8_t)(instruction.opcode & 0xFF));

	if (x86_uses_modrm(variant)) {
		mod = 0;

#define SWITCH_OP(n)										\
		uint32_t op##n = GET_OPERAND##n(instruction.operands);	\
		switch (op##n >> 5u) {								\
		case OPERANDS_TYPE_modreg:							\
			mod |= ((op ##n & 0x1Fu) << 3u);					\
			mod |= (parameters.reg ##n & 0x7u) << 3u;			\
			break;											\
		case OPERANDS_TYPE_modrm:							\
			mod |= ((op ## n & 0x1Fu) << 3u);					\
			mod |= parameters.reg ## n & 0x7u;					\
			break;											\
		default: break;										\
		}
		
		SWITCH_OP(0)
		SWITCH_OP(1)
#undef SWITCH_OP
		 //mod: 0b00 -> no offset, 0b01 -> 8 bit offset, 0b10 -> 32 bit offset, 0b11 -> no offset

		if ((parameters.reg0 & X86_REG_BASE) || (parameters.reg1 & X86_REG_BASE)) {
			int64_t offset = parameters.displacement;
			if (offset >= INT8_MIN && offset <= INT8_MAX) {
				mod |= 0x40;
				X86_64_APPEND_OP_SEGMENT(mod);
				X86_64_APPEND_OP_SEGMENT(PUN((int8_t)offset, uint8_t));
			} else if (offset >= INT32_MIN && offset <= INT32_MAX) {
				mod |= 0x80;
				X86_64_APPEND_OP_SEGMENT(mod);
				X86_64_APPEND_OP_SEGMENT(PUN((int32_t)offset, uint32_t));
			} else assert(false);
		} else {
			mod |= 0xc0;
			X86_64_APPEND_OP_SEGMENT(mod);
		}
	}

	switch (variant) {
	case X86_VARIANT_KIND_rm8_i8:
	case X86_VARIANT_KIND_r8_i8:
	case X86_VARIANT_KIND_rm16_i8:
	case X86_VARIANT_KIND_rm32_i8:
	case X86_VARIANT_KIND_rm64_i8:
		X86_64_APPEND_OP_SEGMENT(PUN((int8_t)parameters.immediate, uint8_t));
	   	break;

	case X86_VARIANT_KIND_rm16_i16:
	case X86_VARIANT_KIND_r16_i16:
		X86_64_APPEND_OP_SEGMENT(PUN((int16_t)parameters.immediate, uint16_t));
		break;

	case X86_VARIANT_KIND_rm32_i32:
	case X86_VARIANT_KIND_r32_i32:
	case X86_VARIANT_KIND_rm64_i32:
		X86_64_APPEND_OP_SEGMENT(PUN((int32_t)parameters.immediate, uint32_t));
		break;

	case X86_VARIANT_KIND_r64_i64:
		X86_64_APPEND_OP_SEGMENT(PUN((int64_t)parameters.immediate, uint64_t));
		break;
	default: break;
	}
}

#define X86_WRITE_INSTRUCTION(op, variant, parameters) linux_x86_64_elf_write_instruction(cc, b, X86_VARIANT_KIND_ ## variant, instructions[op] . variant, parameters)

void linux_x86_64_elf_operand_mi(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint32_t opcode, X86_Memory_Width width, X86_Operand_Register base, int64_t offset, int64_t immediate, uint8_t ext) {
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

void test_movs(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm8_r8, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm16_r16, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm32_r32, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm64_r64, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, .displacement = 0x10 }) );

	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm8_r8, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm16_r16, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm32_r32, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm64_r64, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, }) );

	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r8_rm8, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rcx, .reg1 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r16_rm16, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rcx, .reg1 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r32_rm32, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rcx, .reg1 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r64_rm64, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rcx, .reg1 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );

	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r8_rm8, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r16_rm16, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r32_rm32, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r64_rm64, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rbp, .reg1 = X86_OPERAND_REGISTER_rax, }) );

	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r8_i8, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r16_i16, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r32_i32, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, r64_i64, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );

	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm8_i8, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm16_i16, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm32_i32, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm64_i32, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );

	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm8_i8, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm16_i16, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm32_i32, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
	X86_WRITE_INSTRUCTION(OPCODE_MOVE, rm64_i32, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
}

void test_shifts(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
#define DO_TESTS(op) \
	X86_WRITE_INSTRUCTION(op, rm8_i8, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rax, .immediate = 0x4 }) );  \
	X86_WRITE_INSTRUCTION(op, rm16_i8, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
	X86_WRITE_INSTRUCTION(op, rm32_i8, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
	X86_WRITE_INSTRUCTION(op, rm64_i8, ((X86_Instruction_Parameters) { .reg0 = X86_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
\
	X86_WRITE_INSTRUCTION(op, rm8_i8, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) );  \
	X86_WRITE_INSTRUCTION(op, rm16_i8, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
	X86_WRITE_INSTRUCTION(op, rm32_i8, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
	X86_WRITE_INSTRUCTION(op, rm64_i8, ((X86_Instruction_Parameters) { .reg0 = X86_REG_BASE | X86_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) );

	DO_TESTS(OPCODE_SAL)
	DO_TESTS(OPCODE_SAR)
	DO_TESTS(OPCODE_SHL)
	DO_TESTS(OPCODE_SHR)
#undef DO_TESTS
}

static void linux_x86_64_elf_generate_block(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
	size_t i;
	for (i = 0; i < block->ops.count; ++i) {
		LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
		LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];

		switch (opcode) {
		case LL_IR_OPCODE_RET: break;
		case LL_IR_OPCODE_STORE:
			/* X86_64_OPERAND_MI_MEM(X86_64_OPCODE_MOV, qword, rbp, -0x10, 100); */

			/* test_movs(cc, b); */
			test_shifts(cc, b);
			// 0x55 -> 0b01 010 101
			
			/* X86_64_APPEND_OP_SEGMENT((uint8_t)0xFFu); */

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

