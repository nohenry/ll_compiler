#pragma once

typedef struct {
	void (*append_u8)(Compiler_Context* cc, void* b, uint8_t value);
	void (*append_u16)(Compiler_Context* cc, void* b, uint16_t value);
	void (*append_u32)(Compiler_Context* cc, void* b, uint32_t value);
	void (*append_u64)(Compiler_Context* cc, void* b, uint64_t value);
} X86_64_Machine_Code_Writer;

typedef enum {
	X86_64_OPERAND_REGISTER_rax = 0,
	X86_64_OPERAND_REGISTER_rcx,
	X86_64_OPERAND_REGISTER_rdx,
	X86_64_OPERAND_REGISTER_rbx,
	X86_64_OPERAND_REGISTER_rsp,
	X86_64_OPERAND_REGISTER_rbp,
	X86_64_OPERAND_REGISTER_rsi,
	X86_64_OPERAND_REGISTER_rdi,
	X86_64_OPERAND_REGISTER_r8,
	X86_64_OPERAND_REGISTER_r9,
	X86_64_OPERAND_REGISTER_r10,
	X86_64_OPERAND_REGISTER_r11,
	X86_64_OPERAND_REGISTER_r12,
	X86_64_OPERAND_REGISTER_r13,
	X86_64_OPERAND_REGISTER_r14,
	X86_64_OPERAND_REGISTER_r15,
} X86_64_Operand_Register;

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

#define mod_rm(r) ((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)r) & 0xF)
#define mod_reg(r) ((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)r) & 0xF)
#define add_to_opcode (((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)
#define imm (((uint32_t)OPERANDS_TYPE_imm) << 5)

enum {
	OPCODE_ADD,
	OPCODE_ADC,
	OPCODE_AND,
	OPCODE_CALL,
	OPCODE_CBW,
	OPCODE_CWDE,
	OPCODE_CDQE,
	OPCODE_CWD,
	OPCODE_CDQ,
	OPCODE_CQO,
	OPCODE_CLC,
	OPCODE_CLI,
	OPCODE_CLD,
	OPCODE_CMC,

	OPCODE_CMOVO,
	OPCODE_CMOVNO,
	OPCODE_CMOVB,
	OPCODE_CMOVAE,
	OPCODE_CMOVE,
	OPCODE_CMOVNE,
	OPCODE_CMOVBE,
	OPCODE_CMOVA,
	OPCODE_CMOVS,
	OPCODE_CMOVNS,
	OPCODE_CMOVPE,
	OPCODE_CMOVPO,
	OPCODE_CMOVL,
	OPCODE_CMOVGE,
	OPCODE_CMOVLE,
	OPCODE_CMOVG,

	OPCODE_CMP,
	OPCODE_CMPSB,
	OPCODE_CMPSW,
	OPCODE_CMPSD,
	OPCODE_CMPSQ,
	OPCODE_DEC,
	OPCODE_DIV,
	OPCODE_FWAIT,
	OPCODE_HLT,
	OPCODE_IDIV,
	OPCODE_IN,
	OPCODE_INSB,
	OPCODE_INSW,
	OPCODE_INSD,
	OPCODE_INC,
	OPCODE_INT,
	OPCODE_INT1,
	OPCODE_INT3,
	OPCODE_INTO,
	OPCODE_IMUL,
	OPCODE_IRET,
	OPCODE_IRETD,
	OPCODE_IRETQ,

	OPCODE_JMP,
	OPCODE_JA,
	OPCODE_JAE,
	OPCODE_JB,
	OPCODE_JBE,
	OPCODE_JC,
	OPCODE_JCXZ,
	OPCODE_JECXZ,
	OPCODE_JRCXZ,
	OPCODE_JE,
	OPCODE_JG,
	OPCODE_JGE,
	OPCODE_JL,
	OPCODE_JLE,

	OPCODE_LAHF,
	OPCODE_LEA,
	OPCODE_LODSB,
	OPCODE_LODSW,
	OPCODE_LODSD,
	OPCODE_LODSQ,
	OPCODE_LOOP,
	OPCODE_LOOPE,
	OPCODE_LOOPNE,

	OPCODE_MOV,
	OPCODE_MOVSB,
	OPCODE_MOVSW,
	OPCODE_MOVSD,
	OPCODE_MOVSQ,
	OPCODE_MOVXSD,
	OPCODE_MUL,
	OPCODE_NEG,
	OPCODE_NOT,

	OPCODE_OR,
	OPCODE_OUT,
	OPCODE_OUTSB,
	OPCODE_OUTSW,
	OPCODE_OUTSD,
	OPCODE_POP,
	OPCODE_POPF,
	OPCODE_POPFQ,
	OPCODE_PUSH,
	OPCODE_PUSHF,
	OPCODE_PUSHFQ,
	OPCODE_RET,
	OPCODE_RET_FAR,
	OPCODE_RCL,
	OPCODE_RCR,
	OPCODE_ROL,
	OPCODE_ROR,
	OPCODE_SAHF,
	OPCODE_SAL,
	OPCODE_SAR,
	OPCODE_SBB,
	OPCODE_SCASB,
	OPCODE_SCASW,
	OPCODE_SCASD,
	OPCODE_SCASQ,
	OPCODE_SHL,
	OPCODE_SHR,
	OPCODE_STC,
	OPCODE_STI,
	OPCODE_STD,
	OPCODE_STOSB,
	OPCODE_STOSW,
	OPCODE_STOSD,
	OPCODE_STOSQ,
	OPCODE_SUB,
	OPCODE_SYSCALL,
	OPCODE_TEST,
	OPCODE_XCHG,
	OPCODE_XLAT,
	OPCODE_XOR,
};

typedef enum {
	OPERANDS_TYPE_modrm = 0,
	OPERANDS_TYPE_modreg,
	OPERANDS_TYPE_add_to_opcode,
	OPERANDS_TYPE_imm,
} X86_64_Operands_Type;

typedef struct {
	uint32_t opcode;
	uint32_t operands; // top 3 is Operands_Type, next 1 is don't add rex, bottom 4 is argument
} X86_64_Instruction_Variant;

typedef struct {
	X86_64_Instruction_Variant
		noarg,
		al_i8, ax_i16, eax_i32, rax_i32,
			   ax_i8,  eax_i8,
		       ax_r16, eax_r32, rax_r64,
		al_dx, ax_dx,  eax_dx,
		rm8, rm16,  rm32, rm64,
		rm8_cl, rm16_cl,  rm32_cl, rm64_cl,
		rm8_r8, rm16_r16, rm32_r32, rm64_r64,
		r16, r32, r64,
		r8_rm8, r16_rm16, r32_rm32, r64_rm64,
		r8_i8, r16_i16, r32_i32, r64_i64,
		rm8_i8, rm16_i16, rm32_i32, rm64_i32, rm16_i8, rm32_i8, rm64_i8,
		i8, i16, i32,
		rel8, rel16, rel32,
		r16_rm16_i8, r32_rm32_i8, r64_rm64_i8, 
		r16_rm16_i16, r32_rm32_i32, r64_rm64_i32;
} X86_64_Instruction;

typedef enum {
	X86_64_VARIANT_KIND_noarg,

	X86_64_VARIANT_KIND_al_i8,
   	X86_64_VARIANT_KIND_ax_i16,
   	X86_64_VARIANT_KIND_eax_i32,
   	X86_64_VARIANT_KIND_rax_i32,

   	X86_64_VARIANT_KIND_ax_i8,
   	X86_64_VARIANT_KIND_eax_i8,

   	X86_64_VARIANT_KIND_ax_r16,
   	X86_64_VARIANT_KIND_eax_r32,
   	X86_64_VARIANT_KIND_rax_r32,

   	X86_64_VARIANT_KIND_al_dx,
   	X86_64_VARIANT_KIND_ax_dx,
   	X86_64_VARIANT_KIND_eax_dx,

	X86_64_VARIANT_KIND_rm8,
	X86_64_VARIANT_KIND_rm16,
	X86_64_VARIANT_KIND_rm32,
	X86_64_VARIANT_KIND_rm64,

	X86_64_VARIANT_KIND_rm8_r8,
	X86_64_VARIANT_KIND_rm16_r16,
	X86_64_VARIANT_KIND_rm32_r32,
	X86_64_VARIANT_KIND_rm64_r64,

	X86_64_VARIANT_KIND_rm8_cl,
	X86_64_VARIANT_KIND_rm16_cl,
	X86_64_VARIANT_KIND_rm32_cl,
	X86_64_VARIANT_KIND_rm64_cl,

	X86_64_VARIANT_KIND_r16,
	X86_64_VARIANT_KIND_r32,
	X86_64_VARIANT_KIND_r64,

	X86_64_VARIANT_KIND_r8_rm8,
	X86_64_VARIANT_KIND_r16_rm16,
	X86_64_VARIANT_KIND_r32_rm32,
	X86_64_VARIANT_KIND_r64_rm64,

	X86_64_VARIANT_KIND_r8_i8,
	X86_64_VARIANT_KIND_r16_i16,
	X86_64_VARIANT_KIND_r32_i32,
	X86_64_VARIANT_KIND_r64_i64,

	X86_64_VARIANT_KIND_rm8_i8,
	X86_64_VARIANT_KIND_rm16_i16,
	X86_64_VARIANT_KIND_rm32_i32,
	X86_64_VARIANT_KIND_rm64_i32,
	X86_64_VARIANT_KIND_rm16_i8,
	X86_64_VARIANT_KIND_rm32_i8,
	X86_64_VARIANT_KIND_rm64_i8,

	X86_64_VARIANT_KIND_i8,
	X86_64_VARIANT_KIND_i16,
	X86_64_VARIANT_KIND_i32,

	X86_64_VARIANT_KIND_rel8,
	X86_64_VARIANT_KIND_rel16,
	X86_64_VARIANT_KIND_rel32,

	X86_64_VARIANT_KIND_r16_rm16_i8,
   	X86_64_VARIANT_KIND_r32_rm32_i8,
	X86_64_VARIANT_KIND_r64_rm64_i8,
	X86_64_VARIANT_KIND_r16_rm16_i16,
	X86_64_VARIANT_KIND_r32_rm32_i32,
   	X86_64_VARIANT_KIND_r64_rm64_i32,
} X86_64_Variant_Kind;

#define X86_64_REG_BASE (0x10u)

typedef enum {
	X86_64_PREFIX_REP = 0xF3,
	X86_64_PREFIX_REPE = 0xF3,
	X86_64_PREFIX_REPNE = 0xF2,
} X86_64_Prefixes;

#define X86_64_SIB_INDEX (1u << 1u)
#define X86_64_SIB_SCALE (1u << 2u)

typedef struct {
	uint8_t reg0, reg1, reg2, reg3;
	uint8_t use_sib;
	uint8_t scale, index;
	int64_t displacement, immediate, relative;
	uint8_t rep;
} X86_64_Instruction_Parameters;

void x86_64_write_instruction(Compiler_Context* cc, X86_64_Machine_Code_Writer* b, X86_64_Variant_Kind variant, X86_64_Instruction_Variant instruction, X86_64_Instruction_Parameters parameters);
void x86_64_run_tests(Compiler_Context* cc, X86_64_Machine_Code_Writer* b);
extern const X86_64_Instruction x86_64_instructions_table[];
extern const size_t x86_64_instructions_table_size;

#define X86_64_WRITE_INSTRUCTION(op, variant, parameters) x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_ ## variant, x86_64_instructions_table[op] . variant, parameters)

