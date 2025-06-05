
#include "x86_64_common.h"

#define X86_64_APPEND_OP_SEGMENT(segment) _Generic(segment, \
			uint8_t : b->append_u8, \
			uint16_t : b->append_u16, \
			uint32_t : b->append_u32, \
			uint64_t : b->append_u64 \
		)(cc, b, segment)

#define X86_64_OPCODE_MAKE_ARITHMETIC(op, offset, modreg) \
	[op] = { \
		.al_i8  	= { .opcode = 0x04u + offset, .operands = MAKE_OPERANDS(imm) }, \
		.ax_i16  	= { .opcode = 0x05u + offset, .operands = MAKE_OPERANDS(imm) }, \
		.eax_i32  	= { .opcode = 0x05u + offset, .operands = MAKE_OPERANDS(imm) }, \
		.rax_i32  	= { .opcode = 0x05u + offset, .operands = MAKE_OPERANDS(imm) }, \
 \
		.rm8_i8 	= { .opcode = 0x80u, .operands = MAKE_OPERANDS(mod_rm(modreg), imm) }, \
		.rm16_i16	= { .opcode = 0x81u, .operands = MAKE_OPERANDS(mod_rm(modreg), imm) }, \
		.rm32_i32 	= { .opcode = 0x81u, .operands = MAKE_OPERANDS(mod_rm(modreg), imm) }, \
		.rm64_i32 	= { .opcode = 0x81u, .operands = MAKE_OPERANDS(mod_rm(modreg), imm) }, \
 \
		.rm16_i8 	= { .opcode = 0x83u, .operands = MAKE_OPERANDS(mod_rm(modreg), imm) }, \
		.rm32_i8 	= { .opcode = 0x83u, .operands = MAKE_OPERANDS(mod_rm(modreg), imm) }, \
		.rm64_i8 	= { .opcode = 0x83u, .operands = MAKE_OPERANDS(mod_rm(modreg), imm) }, \
 \
		.rm8_r8  	= { .opcode = 0x00u + offset, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, \
		.rm16_r16  	= { .opcode = 0x01u + offset, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, \
		.rm32_r32  	= { .opcode = 0x01u + offset, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, \
		.rm64_r64  	= { .opcode = 0x01u + offset, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, \
 \
		.r8_rm8  	= { .opcode = 0x02u + offset, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, \
		.r16_rm16  	= { .opcode = 0x03u + offset, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, \
		.r32_rm32  	= { .opcode = 0x03u + offset, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, \
		.r64_rm64  	= { .opcode = 0x03u + offset, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, \
	}

#define X86_64_OPCODE_MAKE_Jcc(op, code) \
	[op] = { 							\
		.rel8  	= { .opcode = code, .operands = MAKE_OPERANDS(imm) }, \
		.rel16 	= { .opcode = code, .operands = MAKE_OPERANDS(imm) }, \
		.rel32 	= { .opcode = code, .operands = MAKE_OPERANDS(imm) }, \
	}

#define X86_64_OPCODE_MAKE_CMOVcc(op, code) \
	[op] = { 							\
		.r16_rm16  	= { .opcode = code, .operands = MAKE_OPERANDS(mod_rm(0)) }, \
		.r32_rm32 	= { .opcode = code, .operands = MAKE_OPERANDS(mod_rm(0)) }, \
		.r64_rm64 	= { .opcode = code, .operands = MAKE_OPERANDS(mod_rm(0)) }, \
	}

#define X86_64_OPCODE_MAKE_SHIFT_ROTATE(op, code) \
	[op] = { 																			\
		.rm8	 	= { .opcode = 0xD0u, .operands = MAKE_OPERANDS(mod_rm(code)) }, 	\
		.rm16	 	= { .opcode = 0xD1u, .operands = MAKE_OPERANDS(mod_rm(code)) }, 	\
		.rm32	 	= { .opcode = 0xD1u, .operands = MAKE_OPERANDS(mod_rm(code)) }, 	\
		.rm64	 	= { .opcode = 0xD1u, .operands = MAKE_OPERANDS(mod_rm(code)) }, 	\
		.rm8_i8 	= { .opcode = 0xC0u, .operands = MAKE_OPERANDS(mod_rm(code), imm) }, \
		.rm16_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(code), imm) }, \
		.rm32_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(code), imm) }, \
		.rm64_i8 	= { .opcode = 0xC1u, .operands = MAKE_OPERANDS(mod_rm(code), imm) }, \
		.rm8_cl	 	= { .opcode = 0xD2u, .operands = MAKE_OPERANDS(mod_rm(code)) }, 	\
		.rm16_cl 	= { .opcode = 0xD3u, .operands = MAKE_OPERANDS(mod_rm(code)) }, 	\
		.rm32_cl 	= { .opcode = 0xD3u, .operands = MAKE_OPERANDS(mod_rm(code)) }, 	\
		.rm64_cl 	= { .opcode = 0xD3u, .operands = MAKE_OPERANDS(mod_rm(code)) }, 	\
	}

const X86_64_Instruction x86_64_instructions_table[] = {
	X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_ADD, 0x00u, 0u),
	X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_OR,  0x04u, 1u),
	X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_ADC, 0x10u, 2u),
	X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_SBB, 0x14u, 3u),
	X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_AND, 0x20u, 4u),
	X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_SUB, 0x24u, 5u),
	X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_XOR, 0x30u, 6u),
	X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_CMP, 0x34u, 7u),
	[OPCODE_CALL] = {
		.rel32		= { .opcode = 0xE8, .operands = MAKE_OPERANDS(imm) },
		.rm64		= { .opcode = 0xFF, .operands = MAKE_OPERANDS(mod_rm(2)) | 0x10u },
	},
	[OPCODE_CBW] 	= { .noarg = { .opcode = 0x98, .operands = 0x20, }, },
	[OPCODE_CWDE] 	= { .noarg = { .opcode = 0x98 }, },
	[OPCODE_CDQE] 	= { .noarg = { .opcode = 0x98, .operands = 0x10 }, },
	[OPCODE_CWD] 	= { .noarg = { .opcode = 0x99, .operands = 0x20, }, },
	[OPCODE_CDQ] 	= { .noarg = { .opcode = 0x99 }, },
	[OPCODE_CQO] 	= { .noarg = { .opcode = 0x99, .operands = 0x10 }, },

	[OPCODE_CLC]	= { .noarg = { .opcode = 0xF8 } },
	[OPCODE_CLI]	= { .noarg = { .opcode = 0xFA } },
	[OPCODE_CLD]	= { .noarg = { .opcode = 0xFC } },
	[OPCODE_CMC]   	= { .noarg = { .opcode = 0xF5u }, },

	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVO, 0x0f40),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVNO, 0x0f41),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVB, 0x0f42),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVAE, 0x0f43),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVE, 0x0f44),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVNE, 0x0f45),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVBE, 0x0f46),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVA, 0x0f47),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVS, 0x0f48),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVNS, 0x0f49),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVPE, 0x0f4A),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVPO, 0x0f4B),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVL, 0x0f4C),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVGE, 0x0f4D),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVLE, 0x0f4E),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVG, 0x0f4F),

	[OPCODE_CMPSB] 	= { .noarg = { .opcode = 0xA6u, .operands = 0x00u }, },
	[OPCODE_CMPSW] 	= { .noarg = { .opcode = 0xA7u, .operands = 0x20u }, },
	[OPCODE_CMPSD] 	= { .noarg = { .opcode = 0xA7u, .operands = 0x00u }, },
	[OPCODE_CMPSQ] 	= { .noarg = { .opcode = 0xA7u, .operands = 0x10u }, },
	[OPCODE_DEC] = {
		.rm8  		= { .opcode = 0xFEu, .operands = MAKE_OPERANDS(mod_rm(1)) },
		.rm16  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(1)) },
		.rm32  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(1)) },
		.rm64  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(1)) },
	},
	[OPCODE_DIV] = {
		.rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(6)) }, 
		.rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(6)) }, 
		.rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(6)) }, 
		.rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(6)) }, 
	},
	[OPCODE_FWAIT] 	= { .noarg = { .opcode = 0x9bu } },
	[OPCODE_HLT] = {
		.noarg		= { .opcode = 0xF4u },
	},
	[OPCODE_IDIV] = {
		.rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(7)) }, 
		.rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(7)) }, 
		.rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(7)) }, 
		.rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(7)) }, 
	},
	[OPCODE_IMUL] = {
		.rm8  		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(5)) },
		.rm16  		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(5)) },
		.rm32  		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(5)) },
		.rm64  		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(5)) },

		.r16_rm16  	= { .opcode = 0x0FAFu, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r32_rm32  	= { .opcode = 0x0FAFu, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm64  	= { .opcode = 0x0FAFu, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },

		.r16_rm16_i8  	= { .opcode = 0x6Bu, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r32_rm32_i8  	= { .opcode = 0x6Bu, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm64_i8  	= { .opcode = 0x6Bu, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },

		.r16_rm16_i16  	= { .opcode = 0x69u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r32_rm32_i32  	= { .opcode = 0x69u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm64_i32  	= { .opcode = 0x69u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_IN] = {
		.al_i8 		= { .opcode = 0xE4u, .operands = MAKE_OPERANDS(imm) },
		.ax_i8 		= { .opcode = 0xE5u, .operands = MAKE_OPERANDS(imm) },
		.eax_i8		= { .opcode = 0xE5u, .operands = MAKE_OPERANDS(imm) },
		.al_dx 		= { .opcode = 0xECu, .operands = 0x00u },
		.ax_dx 		= { .opcode = 0xEDu, .operands = 0x00u },
		.eax_dx		= { .opcode = 0xEDu, .operands = 0x00u },
	},
	[OPCODE_INSB] 	= { .noarg	= { .opcode = 0x6Cu, } },
	[OPCODE_INSW] 	= { .noarg	= { .opcode = 0x6Du, .operands = 0x20u } },
	[OPCODE_INSD] 	= { .noarg	= { .opcode = 0x6Du, } },
	[OPCODE_INC] = {
		.rm8  		= { .opcode = 0xFEu, .operands = MAKE_OPERANDS(mod_rm(0)) },
		.rm16  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(0)) },
		.rm32  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(0)) },
		.rm64  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(0)) },
	},

	[OPCODE_INT] = {
		.i8 		= { .opcode = 0xCDu, .operands = MAKE_OPERANDS(imm) },
	},
	[OPCODE_INT1] = {
		.noarg 		= { .opcode = 0xF1u, .operands = 0x00u },
	},
	[OPCODE_INT3] = {
		.noarg 		= { .opcode = 0xCCu, .operands = 0x00u },
	},
	[OPCODE_INTO] = {
		.noarg 		= { .opcode = 0xF1u, .operands = 0x00u },
	},
	[OPCODE_IRET]	= { .noarg	= { .opcode = 0xCF, .operands = 0x20 } },
	[OPCODE_IRETD]	= { .noarg 	= { .opcode = 0xCF } },
	[OPCODE_IRETQ]	= { .noarg 	= { .opcode = 0xCF, .operands = 0x10 } },
	[OPCODE_JMP] = {
		.rel8  		= { .opcode = 0xEBu, .operands = MAKE_OPERANDS(imm) },
		.rel32 		= { .opcode = 0xE9u, .operands = MAKE_OPERANDS(imm) },
		.rm64  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(4)) },
		// TODO: add far jumps
	},
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JA, 0x77),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JAE, 0x73),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JB, 0x72),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JBE, 0x76),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JC, 0x72),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JCXZ, 0xE3),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JECXZ, 0x67E3),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JRCXZ, 0xE3),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JE, 0x74),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JG, 0x7F),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JGE, 0x7D),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JL, 0x7C),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JLE, 0x7E),
	[OPCODE_LAHF] 	= { .noarg	= { .opcode = 0x9F } },
	[OPCODE_LEA] = {
		.r16_rm16	= { .opcode = 0x8D, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r32_rm32	= { .opcode = 0x8D, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm64	= { .opcode = 0x8D, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_LODSB] 	= { .noarg	= { .opcode = 0xACu } },
	[OPCODE_LODSW] 	= { .noarg	= { .opcode = 0xADu, .operands = 0x20u, } },
	[OPCODE_LODSD] 	= { .noarg	= { .opcode = 0xADu } },
	[OPCODE_LODSQ] 	= { .noarg	= { .opcode = 0xADu, .operands = 0x10u, } },
	[OPCODE_LOOP] 	= { .rel8 = { .opcode = 0xE2u, .operands = MAKE_OPERANDS(imm) } },
	[OPCODE_LOOPE] 	= { .rel8 = { .opcode = 0xE1u, .operands = MAKE_OPERANDS(imm) } },
	[OPCODE_LOOPNE] = { .rel8 = { .opcode = 0xE0u, .operands = MAKE_OPERANDS(imm) } },
	[OPCODE_MOV] = {
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
	[OPCODE_MOVSB] 	= { .noarg = { .opcode = 0xA4u, .operands = 0x00u }, },
	[OPCODE_MOVSW] 	= { .noarg = { .opcode = 0xA5u, .operands = 0x20u }, },
	[OPCODE_MOVSD] 	= { .noarg = { .opcode = 0xA5u, .operands = 0x00u }, },
	[OPCODE_MOVSQ] 	= { .noarg = { .opcode = 0xA5u, .operands = 0x10u }, },
	[OPCODE_MOVXSD] = {
		.r16_rm16  	= { .opcode = 0x63u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r32_rm32  	= { .opcode = 0x63u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm64  	= { .opcode = 0x63u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_MUL] = {
		.rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(4)) }, 
		.rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(4)) }, 
		.rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(4)) }, 
		.rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(4)) }, 
	},
	[OPCODE_NEG] = {
		.rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(3)) }, 
		.rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(3)) }, 
		.rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(3)) }, 
		.rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(3)) }, 
	},
	[OPCODE_NOT] = {
		.rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(2)) }, 
		.rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(2)) }, 
		.rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(2)) }, 
		.rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(2)) }, 
	},
	[OPCODE_OUT] = {
		.al_i8 		= { .opcode = 0xE6u, .operands = MAKE_OPERANDS(imm) },
		.ax_i8 		= { .opcode = 0xE7u, .operands = MAKE_OPERANDS(imm) },
		.eax_i8		= { .opcode = 0xE7u, .operands = MAKE_OPERANDS(imm) },
		.al_dx 		= { .opcode = 0xEEu, .operands = 0x00u },
		.ax_dx 		= { .opcode = 0xEFu, .operands = 0x00u },
		.eax_dx		= { .opcode = 0xEFu, .operands = 0x00u },
	},
	[OPCODE_OUTSB] 	= { .noarg	= { .opcode = 0x6Eu, } },
	[OPCODE_OUTSW] 	= { .noarg	= { .opcode = 0x6Fu, .operands = 0x20u } },
	[OPCODE_OUTSD] 	= { .noarg	= { .opcode = 0x6Fu, } },
	[OPCODE_POP] = {
		.rm16 		= { .opcode = 0x8Fu, .operands = MAKE_OPERANDS(mod_rm(0)) },
		.rm64 		= { .opcode = 0x8Fu, .operands = MAKE_OPERANDS(mod_rm(0)) | 0x10 },
		.r16 		= { .opcode = 0x58u, .operands = MAKE_OPERANDS(add_to_opcode) },
		.r64 		= { .opcode = 0x58u, .operands = MAKE_OPERANDS(add_to_opcode) | 0x10 },
	},
	[OPCODE_POPF] 	= { .noarg = { .opcode = 0x9D, .operands = 0x20u } },
	[OPCODE_POPFQ] 	= { .noarg = { .opcode = 0x9D, .operands = 0x00u } },
	[OPCODE_PUSH] = {
		.rm16 		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(6)) },
		.rm64 		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(6)) | 0x10 },
		.r16 		= { .opcode = 0x50u, .operands = MAKE_OPERANDS(add_to_opcode) },
		.r64 		= { .opcode = 0x50u, .operands = MAKE_OPERANDS(add_to_opcode) | 0x10 },

		.i8 		= { .opcode = 0x6Au, .operands = MAKE_OPERANDS(add_to_opcode) },
		.i16 		= { .opcode = 0x68u, .operands = MAKE_OPERANDS(add_to_opcode) },
		.i32 		= { .opcode = 0x68u, .operands = MAKE_OPERANDS(add_to_opcode) },
	},
	[OPCODE_PUSHF] 	= { .noarg = { .opcode = 0x9C, .operands = 0x20u } },
	[OPCODE_PUSHFQ] = { .noarg = { .opcode = 0x9C, .operands = 0x00u } },
	[OPCODE_RET] = {
		.noarg		= { .opcode = 0xC3, .operands = 0x00u },
		.i16		= { .opcode = 0xC2, .operands = MAKE_OPERANDS(imm) },
	},
	[OPCODE_RET_FAR] = {
		.noarg		= { .opcode = 0xCB, .operands = 0x00u },
		.i16		= { .opcode = 0xCA, .operands = MAKE_OPERANDS(imm) },
	},
	[OPCODE_SAHF] 	= { .noarg	= { .opcode = 0x9E } },
	X86_64_OPCODE_MAKE_SHIFT_ROTATE(OPCODE_RCL, 2),
	X86_64_OPCODE_MAKE_SHIFT_ROTATE(OPCODE_RCR, 3),
	X86_64_OPCODE_MAKE_SHIFT_ROTATE(OPCODE_ROL, 0),
	X86_64_OPCODE_MAKE_SHIFT_ROTATE(OPCODE_ROR, 1),
	X86_64_OPCODE_MAKE_SHIFT_ROTATE(OPCODE_SAL, 4),
	X86_64_OPCODE_MAKE_SHIFT_ROTATE(OPCODE_SAR, 7),
	X86_64_OPCODE_MAKE_SHIFT_ROTATE(OPCODE_SHL, 4),
	X86_64_OPCODE_MAKE_SHIFT_ROTATE(OPCODE_SHR, 5),
	[OPCODE_SCASB] 	= { .noarg	= { .opcode = 0xAEu } },
	[OPCODE_SCASW] 	= { .noarg	= { .opcode = 0xAFu, .operands = 0x20u } },
	[OPCODE_SCASD] 	= { .noarg	= { .opcode = 0xAFu } },
	[OPCODE_SCASQ] 	= { .noarg	= { .opcode = 0xAFu, .operands = 0x10u } },
	[OPCODE_STC]	= { .noarg = { .opcode = 0xF9 } },
	[OPCODE_STI]	= { .noarg = { .opcode = 0xFB } },
	[OPCODE_STD]	= { .noarg = { .opcode = 0xFD } },
	[OPCODE_STOSB] 	= { .noarg	= { .opcode = 0xAAu } },
	[OPCODE_STOSW] 	= { .noarg	= { .opcode = 0xABu, .operands = 0x20u, } },
	[OPCODE_STOSD] 	= { .noarg	= { .opcode = 0xABu } },
	[OPCODE_STOSQ] 	= { .noarg	= { .opcode = 0xABu, .operands = 0x10u, } },
	[OPCODE_SYSCALL] = { .noarg = { .opcode = 0x0F05 } },
	[OPCODE_TEST] = { 
		.al_i8  	= { .opcode = 0xA8u, .operands = MAKE_OPERANDS(imm) }, 
		.ax_i16  	= { .opcode = 0xA9u, .operands = MAKE_OPERANDS(imm) }, 
		.eax_i32  	= { .opcode = 0xA9u, .operands = MAKE_OPERANDS(imm) }, 
		.rax_i32  	= { .opcode = 0xA9u, .operands = MAKE_OPERANDS(imm) }, 
 
		.rm8_i8 	= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(0), imm) }, 
		.rm16_i16	= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(0), imm) }, 
		.rm32_i32 	= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(0), imm) }, 
		.rm64_i32 	= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(0), imm) }, 
 
		.rm8_r8  	= { .opcode = 0x84u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, 
		.rm16_r16  	= { .opcode = 0x85u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, 
		.rm32_r32  	= { .opcode = 0x85u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, 
		.rm64_r64  	= { .opcode = 0x85u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, 
	},
	[OPCODE_XCHG] = {
		.ax_r16 = { .opcode = 0x90u, .operands = MAKE_OPERANDS(add_to_opcode) },
		.eax_r32 = { .opcode = 0x90u, .operands = MAKE_OPERANDS(add_to_opcode) },
		.rax_r64 = { .opcode = 0x90u, .operands = MAKE_OPERANDS(add_to_opcode) },

		.rm8_r8 = { .opcode = 0x87u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.rm16_r16 = { .opcode = 0x87u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.rm32_r32 = { .opcode = 0x87u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.rm64_r64 = { .opcode = 0x87u, .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },

		.r8_rm8 = { .opcode = 0x86u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r16_rm16 = { .opcode = 0x87u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r32_rm32 = { .opcode = 0x87u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm64 = { .opcode = 0x87u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_XLAT] 	= { .noarg = { .opcode = 0xD7u, .operands = 0x00u }, },
};

const size_t x86_64_instructions_table_size = sizeof(x86_64_instructions_table);

static bool x86_64_uses_modrm(X86_64_Variant_Kind kind) {
	switch (kind) {
	case X86_64_VARIANT_KIND_rm8:
	case X86_64_VARIANT_KIND_rm16:
	case X86_64_VARIANT_KIND_rm32:
	case X86_64_VARIANT_KIND_rm64:

	case X86_64_VARIANT_KIND_rm8_r8:
	case X86_64_VARIANT_KIND_rm16_r16:
	case X86_64_VARIANT_KIND_rm32_r32:
	case X86_64_VARIANT_KIND_rm64_r64:

	case X86_64_VARIANT_KIND_rm8_cl:
	case X86_64_VARIANT_KIND_rm16_cl:
	case X86_64_VARIANT_KIND_rm32_cl:
	case X86_64_VARIANT_KIND_rm64_cl:

	case X86_64_VARIANT_KIND_r8_rm8:
	case X86_64_VARIANT_KIND_r16_rm16:
	case X86_64_VARIANT_KIND_r32_rm32:
	case X86_64_VARIANT_KIND_r64_rm64:

	case X86_64_VARIANT_KIND_rm8_i8:
	case X86_64_VARIANT_KIND_rm16_i16:
	case X86_64_VARIANT_KIND_rm32_i32:
	case X86_64_VARIANT_KIND_rm64_i32:
	case X86_64_VARIANT_KIND_rm16_i8:
	case X86_64_VARIANT_KIND_rm32_i8:
	case X86_64_VARIANT_KIND_rm64_i8:

	case X86_64_VARIANT_KIND_r16_rm16_i8:
   	case X86_64_VARIANT_KIND_r32_rm32_i8:
	case X86_64_VARIANT_KIND_r64_rm64_i8:
	case X86_64_VARIANT_KIND_r16_rm16_i16:
	case X86_64_VARIANT_KIND_r32_rm32_i32:
   	case X86_64_VARIANT_KIND_r64_rm64_i32:
		return true;
	default: return false;
	}
}

void x86_64_write_instruction(Compiler_Context* cc, X86_64_Machine_Code_Writer* b, X86_64_Variant_Kind variant, X86_64_Instruction_Variant instruction, X86_64_Instruction_Parameters parameters) {
	uint8_t mod, sib, rex = 0, prefix[4], prefixi = 0;
	int i;

	if (instruction.opcode == 0) {
		fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: invalid opcode\n");
		return;
	}

	switch (variant) {
	case X86_64_VARIANT_KIND_noarg:
		if (instruction.operands & 0x10)
			rex |= 0x48;
		else if (instruction.operands & 0x20)
			prefix[prefixi++] = 0x66;
		break;
	case X86_64_VARIANT_KIND_rm64:
	case X86_64_VARIANT_KIND_rm64_r64:
	case X86_64_VARIANT_KIND_rm64_cl:
	case X86_64_VARIANT_KIND_r64_rm64:
	case X86_64_VARIANT_KIND_r64_i64:
	case X86_64_VARIANT_KIND_rm64_i32:
	case X86_64_VARIANT_KIND_rm64_i8:
	case X86_64_VARIANT_KIND_rax_i32:
	case X86_64_VARIANT_KIND_r64:
	case X86_64_VARIANT_KIND_r64_rm64_i8:
   	case X86_64_VARIANT_KIND_r64_rm64_i32:
		if (!(instruction.operands & 0x10))
			rex |= 0x48;
		break;
	case X86_64_VARIANT_KIND_rm16:
	case X86_64_VARIANT_KIND_rm16_r16:
	case X86_64_VARIANT_KIND_rm16_cl:
	case X86_64_VARIANT_KIND_r16_rm16:
	case X86_64_VARIANT_KIND_r16_i16:
	case X86_64_VARIANT_KIND_rm16_i16:
	case X86_64_VARIANT_KIND_rm16_i8:
	case X86_64_VARIANT_KIND_ax_i16:
	case X86_64_VARIANT_KIND_r16:
	case X86_64_VARIANT_KIND_i16:
	case X86_64_VARIANT_KIND_r16_rm16_i8:
	case X86_64_VARIANT_KIND_r16_rm16_i16:
	case X86_64_VARIANT_KIND_ax_i8:
	case X86_64_VARIANT_KIND_ax_dx:
		prefix[prefixi++] = 0x66;
		break;
	default: break;
	}

	if (parameters.rep) {
		// TODO: add check for valid instructions
		X86_64_APPEND_OP_SEGMENT(parameters.rep);
	}

#define SWITCH_OP(n)												\
		uint32_t op##n = GET_OPERAND##n(instruction.operands);		\
		switch (op##n >> 5u) {										\
		case OPERANDS_TYPE_modrm:									\
			if ((parameters.reg ## n & 0xFu) >= X86_64_OPERAND_REGISTER_r8)	{ 	\
				rex |= 0x41u; 										\
			} 														\
			break;													\
		default: break;												\
		}

		SWITCH_OP(0)
		SWITCH_OP(1)
#undef SWITCH_OP

	if (rex) X86_64_APPEND_OP_SEGMENT(rex);
	for (i = 0; i < prefixi; ++i) {
		X86_64_APPEND_OP_SEGMENT(prefix[i]);
	}

	if ((instruction.opcode >> 8) & 0xFF) {
		X86_64_APPEND_OP_SEGMENT((uint8_t)((instruction.opcode >> 8) & 0xFF));
	}
	X86_64_APPEND_OP_SEGMENT((uint8_t)(instruction.opcode & 0xFF));

	if (x86_64_uses_modrm(variant)) {
		mod = 0;
		sib = 0;

#define SWITCH_OP(n)										\
		uint32_t op##n = GET_OPERAND##n(instruction.operands);	\
		switch (op##n >> 5u) {								\
		case OPERANDS_TYPE_modreg:							\
			mod |= ((op ##n & 0xFu) << 3u);					\
			mod |= (parameters.reg ##n & 0x7u) << 3u;		\
			break;											\
		case OPERANDS_TYPE_modrm:							\
			mod |= ((op ## n & 0xFu) << 3u);				\
			if (parameters.use_sib) { 						\
				sib |= parameters.reg ## n & 0x7u;			\
			} else { 										\
				mod |= parameters.reg ## n & 0x7u;			\
			} 												\
			break;											\
		default: break;										\
		}

		SWITCH_OP(0)
		SWITCH_OP(1)
#undef SWITCH_OP

		 //mod: 0b00 -> no offset, 0b01 -> 8 bit offset, 0b10 -> 32 bit offset, 0b11 -> no offset

		uint8_t base_reg = (parameters.reg0 & X86_64_REG_BASE) ? (parameters.reg0 & 0x7u) :
			(parameters.reg1 & X86_64_REG_BASE) ? (parameters.reg1 & 0x7u) : 0;


		if ((parameters.reg0 & X86_64_REG_BASE) || (parameters.reg1 & X86_64_REG_BASE)) {

			int64_t offset = parameters.displacement;

			if (!parameters.use_sib && base_reg == X86_64_OPERAND_REGISTER_rsp) {
				parameters.use_sib = 1;
				sib = (X86_64_OPERAND_REGISTER_rsp << 3u) | X86_64_OPERAND_REGISTER_rsp;
			}

			if (parameters.use_sib) {
				mod |= 0x4u;
				if (parameters.use_sib & X86_64_SIB_SCALE) {
					if (!(parameters.use_sib & X86_64_SIB_INDEX)) {
						fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: scale requires index\n");
						abort();
					}
					sib |= parameters.scale << 6u;
				}
				if (parameters.use_sib & X86_64_SIB_INDEX) {
					if (parameters.index == X86_64_OPERAND_REGISTER_rsp)  {
						fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: invalid use of rsp as index with rbp as base\x1b[0m\n");
						abort();
					}
					sib |= (parameters.index & 7u) << 3u;
				} else {
					sib |= X86_64_OPERAND_REGISTER_rsp << 3u; // rsp as index means no index
				}

				if (offset == 0)  {
					// rbp as base means no base, only if mod is 0
					if (base_reg == X86_64_OPERAND_REGISTER_rbp) {
						// TODO: i feel like this might be wrong, below too
						assert((mod >> 6) == 0);
						if (parameters.use_sib & X86_64_SIB_SCALE) {
						} else {
							mod |= 0x40u;
						}
					}
					X86_64_APPEND_OP_SEGMENT(mod);
					X86_64_APPEND_OP_SEGMENT(sib);
					if (base_reg == X86_64_OPERAND_REGISTER_rbp) {
						// rbp needs dsp32, so we just add 0
						if (parameters.use_sib & X86_64_SIB_SCALE) {
							X86_64_APPEND_OP_SEGMENT(PUN((int32_t)0u, uint32_t));
						} else {
							X86_64_APPEND_OP_SEGMENT(PUN((int8_t)0u, uint8_t));
						}
					}
				} else if (offset >= INT8_MIN && offset <= INT8_MAX) {
					mod |= 0x40;
					X86_64_APPEND_OP_SEGMENT(mod);
					X86_64_APPEND_OP_SEGMENT(sib);
					X86_64_APPEND_OP_SEGMENT(PUN((int8_t)offset, uint8_t));
				} else if (offset >= INT32_MIN && offset <= INT32_MAX) {
					mod |= 0x80;
					X86_64_APPEND_OP_SEGMENT(mod);
					X86_64_APPEND_OP_SEGMENT(sib);
					X86_64_APPEND_OP_SEGMENT(PUN((int32_t)offset, uint32_t));
				}
			} else if (offset == 0 && base_reg != X86_64_OPERAND_REGISTER_rbp) {
				X86_64_APPEND_OP_SEGMENT(mod);
			} else if (offset >= INT8_MIN && offset <= INT8_MAX) {
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
			if (parameters.use_sib) {
				X86_64_APPEND_OP_SEGMENT(sib);
			}
		}

	}

	switch (variant) {
	case X86_64_VARIANT_KIND_al_i8:
	case X86_64_VARIANT_KIND_rm8_i8:
	case X86_64_VARIANT_KIND_r8_i8:
	case X86_64_VARIANT_KIND_rm16_i8:
	case X86_64_VARIANT_KIND_rm32_i8:
	case X86_64_VARIANT_KIND_rm64_i8:
	case X86_64_VARIANT_KIND_i8:
	case X86_64_VARIANT_KIND_r16_rm16_i8:
   	case X86_64_VARIANT_KIND_r32_rm32_i8:
	case X86_64_VARIANT_KIND_r64_rm64_i8:
   	case X86_64_VARIANT_KIND_ax_i8:
   	case X86_64_VARIANT_KIND_eax_i8:
		X86_64_APPEND_OP_SEGMENT(PUN((int8_t)parameters.immediate, uint8_t));
	   	break;

	case X86_64_VARIANT_KIND_ax_i16:
	case X86_64_VARIANT_KIND_rm16_i16:
	case X86_64_VARIANT_KIND_r16_i16:
	case X86_64_VARIANT_KIND_i16:
	case X86_64_VARIANT_KIND_r16_rm16_i16:
		X86_64_APPEND_OP_SEGMENT(PUN((int16_t)parameters.immediate, uint16_t));
		break;

	case X86_64_VARIANT_KIND_rax_i32:
	case X86_64_VARIANT_KIND_eax_i32:
	case X86_64_VARIANT_KIND_rm32_i32:
	case X86_64_VARIANT_KIND_r32_i32:
	case X86_64_VARIANT_KIND_rm64_i32:
	case X86_64_VARIANT_KIND_i32:
	case X86_64_VARIANT_KIND_r32_rm32_i32:
   	case X86_64_VARIANT_KIND_r64_rm64_i32:
		X86_64_APPEND_OP_SEGMENT(PUN((int32_t)parameters.immediate, uint32_t));
		break;

	case X86_64_VARIANT_KIND_r64_i64:
		X86_64_APPEND_OP_SEGMENT(PUN((int64_t)parameters.immediate, uint64_t));
		break;


	/* Relative addresses */
	case X86_64_VARIANT_KIND_rel8:
		X86_64_APPEND_OP_SEGMENT(PUN((int8_t)parameters.relative, uint8_t));
	   	break;

	case X86_64_VARIANT_KIND_rel16:
		X86_64_APPEND_OP_SEGMENT(PUN((int16_t)parameters.relative, uint16_t));
	   	break;

	case X86_64_VARIANT_KIND_rel32:
		X86_64_APPEND_OP_SEGMENT(PUN((int32_t)parameters.relative, uint32_t));
	   	break;

	default: break;
	}

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

void x86_64_run_tests(Compiler_Context* cc, X86_64_Machine_Code_Writer* b) {
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

	/* X86_64_WRITE_INSTRUCTION(OPCODE_CALL, rel32, ((X86_64_Instruction_Parameters) { .immediate = 0x10 }) ); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CALL, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx }) ); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CALL, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx }) ); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CMOVA, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx })); */
	X86_64_WRITE_INSTRUCTION(OPCODE_CMOVA, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx }));
	X86_64_WRITE_INSTRUCTION(OPCODE_CMOVA, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx }));

	// 0x55 -> 0b01 010 101
	

	/* X86_64_APPEND_OP_SEGMENT((uint8_t)0x48u); */
	/* X86_64_APPEND_OP_SEGMENT((uint8_t)0xb8u); */
	/* X86_64_APPEND_OP_SEGMENT(PUN((int64_t)0x04u, uint64_t)); */

	/* X86_64_APPEND_OP_SEGMENT((uint8_t)0xb8u); */
	/* X86_64_APPEND_OP_SEGMENT(PUN((int32_t)0x04u, uint32_t)); */

	/* X86_64_APPEND_OP_SEGMENT((uint8_t)0xc7u); */
	/* X86_64_APPEND_OP_SEGMENT((uint8_t)0xc3u); */
	/* X86_64_APPEND_OP_SEGMENT(PUN((int32_t)0x04u, uint32_t)); */
}
