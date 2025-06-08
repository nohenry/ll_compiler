
#include "x86_64_common.h"

#define X86_64_APPEND_OP_SEGMENT(segment) _Generic(segment, \
			uint8_t : b->append_u8, \
			uint16_t : b->append_u16, \
			uint32_t : b->append_u32, \
			uint64_t : b->append_u64 \
		)(cc, b, segment)

enum {
	OPCODE_PREFIX_none = 0,

	OPCODE_PREFIX_1B = 0,
	OPCODE_PREFIX_2B = 1,
	OPCODE_PREFIX_3B = 2,

	OPCODE_PREFIX_66 = 1,
	OPCODE_PREFIX_F3 = 2,
	OPCODE_PREFIX_F2 = 3,

	OPCODE_PREFIX_vex = 1,
};

#define BUILD_OPCODE(vex, pre, bc, opcode) (((OPCODE_PREFIX_ ## vex) << 12) | ((OPCODE_PREFIX_ ## pre) << 10) | ((OPCODE_PREFIX_ ## bc) << 8) | (opcode))

#define OPCODE_GET_BYTECOUNT(op) (((op) >> 8) & 0x3)
#define OPCODE_GET_PREFIX(op) (((op) >> 10) & 0x3)
#define OPCODE_GET_VEX(op) (((op) >> 12) & 0x3)

/* 00 00 */

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


#define X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC(op, _opcode) \
	[OPCODE_ ## op ## PS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_ ## op ## PD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_ ## op ## SS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, F3, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_ ## op ## SD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, F2, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_V ## op ## PS] = { 																													\
		.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
		.r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
	}, 																																			\
	[OPCODE_V ## op ## PD] = { 																													\
		.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
		.r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
	}, 																																			\
	[OPCODE_V ## op ## SS] = { 																													\
		.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, F3, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
	}, 																																			\
	[OPCODE_V ## op ## SD] = { 																													\
		.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, F2, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
	}

#define X86_64_OPCODE_MAKE_VECTOR_LOGIC(op, _opcode) \
	[OPCODE_ ## op ## PS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_ ## op ## PD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_V ## op ## PS] = { 																													\
		.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
		.r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
	}, 																																			\
	[OPCODE_V ## op ## PD] = { 																													\
		.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
		.r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
	}	

#define X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC_UNARY(op, _opcode) \
	[OPCODE_ ## op ## PS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_ ## op ## PD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_ ## op ## SS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, F3, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_ ## op ## SD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, F2, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
	[OPCODE_V ## op ## PS] = { 																													\
		.r128_rm128 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
		.r256_rm256 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
	}, 																																			\
	[OPCODE_V ## op ## PD] = { 																													\
		.r128_rm128 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
		.r256_rm256 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
	}, 																																			\
	[OPCODE_V ## op ## SS] = { 																													\
		.r128_rm128 = { .opcode = BUILD_OPCODE(vex, F3, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
	}, 																																			\
	[OPCODE_V ## op ## SD] = { 																													\
		.r128_rm128 = { .opcode = BUILD_OPCODE(vex, F2, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
	}

#define X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(op, vop, _opcode) \
	[op] = { 																													\
		.r64_rm64	= { .opcode = BUILD_OPCODE(none, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },	\
		.r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },		\
	}, 																															\
	[vop] = { 																													\
		.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },	\
		.r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0),vex_vvvv, mod_rm(0)) },	\
	}

#define X86_64_OPCODE_MAKE_Jcc(op, code, code_long) \
	[op] = { 							\
		.rel8  	= { .opcode = code, .operands = MAKE_OPERANDS(imm) }, \
		/* .rel16 	= { .opcode = code, .operands = MAKE_OPERANDS(imm) }, \ */ \
		.rel32 	= { .opcode = BUILD_OPCODE(none, none, 2B, code_long), .operands = MAKE_OPERANDS(imm) }, \
	}

#define X86_64_OPCODE_MAKE_SETcc(op, code) \
	[op] = { 							\
		.rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, code), .operands = MAKE_OPERANDS(mod_rm(0)) }, \
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

	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVO, BUILD_OPCODE(none, none, 2B, 0x40)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVNO, BUILD_OPCODE(none, none, 2B, 0x41)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVB, BUILD_OPCODE(none, none, 2B, 0x42)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVAE, BUILD_OPCODE(none, none, 2B, 0x43)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVE, BUILD_OPCODE(none, none, 2B, 0x44)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVNE, BUILD_OPCODE(none, none, 2B, 0x45)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVBE, BUILD_OPCODE(none, none, 2B, 0x46)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVA, BUILD_OPCODE(none, none, 2B, 0x47)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVS, BUILD_OPCODE(none, none, 2B, 0x48)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVNS, BUILD_OPCODE(none, none, 2B, 0x49)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVPE, BUILD_OPCODE(none, none, 2B, 0x4A)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVPO, BUILD_OPCODE(none, none, 2B, 0x4B)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVL, BUILD_OPCODE(none, none, 2B, 0x4C)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVGE, BUILD_OPCODE(none, none, 2B, 0x4D)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVLE, BUILD_OPCODE(none, none, 2B, 0x4E)),
	X86_64_OPCODE_MAKE_CMOVcc(OPCODE_CMOVG, BUILD_OPCODE(none, none, 2B, 0x4F)),

	[OPCODE_CMPSB] 	= { .noarg = { .opcode = 0xA6u, .operands = 0x00u }, },
	[OPCODE_CMPSW] 	= { .noarg = { .opcode = 0xA7u, .operands = 0x20u }, },
	[OPCODE_CMPSD] 	= { .noarg = { .opcode = 0xA7u, .operands = 0x00u }, },
	[OPCODE_CMPSQ] 	= { .noarg = { .opcode = 0xA7u, .operands = 0x10u }, },
	[OPCODE_CPUID] 	= { .noarg = { .opcode = BUILD_OPCODE(none, none, 2B, 0xA2u) } },
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

		.r16_rm16  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xAFu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r32_rm32  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xAFu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm64  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xAFu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },

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
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JO, 0x70, 0x80u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JNO, 0x71, 0x81u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JB, 0x72, 0x82u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JC, 0x72, 0x82u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JAE, 0x73, 0x83u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JE, 0x74, 0x84u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JNE, 0x75, 0x85u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JBE, 0x76, 0x86u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JA, 0x77, 0x87u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JS, 0x78, 0x88u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JNS, 0x79, 0x89u),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JPE, 0x7a, 0x8au),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JPO, 0x7b, 0x8bu),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JL, 0x7C, 0x8Cu),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JGE, 0x7D, 0x8Du),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JLE, 0x7E, 0x8Eu),
	X86_64_OPCODE_MAKE_Jcc(OPCODE_JG, 0x7F, 0x8Fu),

	[OPCODE_JCXZ] = {
		.rel8  	= { .opcode = 0xE3, .operands = MAKE_OPERANDS(imm) },
	},
	[OPCODE_JECXZ] = {
		.rel8  	= { .opcode = 0x67E3, .operands = MAKE_OPERANDS(imm) },
	},
	[OPCODE_JRCXZ] = {
		.rel8  	= { .opcode = 0xE3, .operands = MAKE_OPERANDS(imm) },
	},

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
	[OPCODE_MOVSD] 	= {
		.noarg = { .opcode = 0xA5u, .operands = 0x00u },
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
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
	[OPCODE_SYSCALL] = { .noarg = { .opcode = BUILD_OPCODE(none, none, 2B, 0x05) } },

	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETO, 0x90),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETNO, 0x91),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETB, 0x92),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETAE, 0x93),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETE, 0x94),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETNE, 0x95),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETBE, 0x96),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETA, 0x97),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETS, 0x98),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETNS, 0x99),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETPE, 0x9A),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETPO, 0x9B),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETL, 0x9C),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETGE, 0x9D),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETLE, 0x9E),
	X86_64_OPCODE_MAKE_SETcc(OPCODE_SETG, 0x9F),

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


	/* [OPCODE_ADDPS] = { */
	/* 	.r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, */
	/* }, */
	/* [OPCODE_ADDPD] = { */
	/* 	.r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, */
	/* }, */
	/* [OPCODE_ADDSS] = { */
	/* 	.r128_rm128	= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, */
	/* }, */
	/* [OPCODE_ADDSD] = { */
	/* 	.r128_rm128	= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, */
	/* }, */

	[OPCODE_COMISS] = {
		.r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_COMISD] = {
		.r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},

	[OPCODE_CVTDQ2PD] = {
	},
	[OPCODE_CVTDQ2PS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTPD2DQ] = {
	},
	[OPCODE_CVTPD2PI] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTPD2PS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTPI2PD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTPI2PS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTPS2DQ] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTPS2PD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTPS2PI] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTSD2SI] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTSD2SS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTSI2SD] = {
		.r128_rm32 = { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r128_rm64 = { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTSI2SS] = {
		.r128_rm32 = { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r128_rm64 = { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTSS2SD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTSS2SI] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTTPD2DQ] = {
	},
	[OPCODE_CVTTPD2PI] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTTPS2DQ] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTTPS2PI] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTTSD2SI] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_CVTTSS2SI] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},

	
	[OPCODE_EMMS] = {
		.noarg			= { .opcode = BUILD_OPCODE(none, none, 2B, 0x77u) },
	},

	[OPCODE_HADDPD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_HADDPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},

	[OPCODE_HSUBPD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_HSUBPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},

	[OPCODE_MOVAPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_MOVAPD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_MOVLPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x13u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},
	[OPCODE_MOVHPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x17u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},
	[OPCODE_MOVHPD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x17u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},
	[OPCODE_MOVHLPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
	},
	[OPCODE_MOVLHPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
	},
	[OPCODE_MOVLPD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x13u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},

	[OPCODE_MOVMSKPS] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
	},
	[OPCODE_MOVMSKPD] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
	},
	[OPCODE_MOVNTPS] = {
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
	},
	[OPCODE_MOVNTPD] = {
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
	},
	[OPCODE_MOVUPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_MOVUPD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_MOVSS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	/* [OPCODE_MOVSD] = { */
	/* 	.r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), }, */
	/* 	.rm128_r128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, */
	/* }, */
	[OPCODE_MOVSLDUP] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},
	[OPCODE_MOVSHDUP] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},
	[OPCODE_MOVDDUP] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},

	[OPCODE_MOVD] = {
		.r64_rm32		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm32_r64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)), },
		.r128_rm32		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm32_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)), },
	},
	[OPCODE_MOVQ] = {
		/* .r64_rm64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), }, */
		/* .rm64_r64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)), }, */
		.r64_rm64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) | 0x10u, },
		.rm64_r64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) | 0x10u, },

		.r128_rm64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm64_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)), },
	},

	[OPCODE_MOVDQA] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x7Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_MOVDQU] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x7Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_UCOMISS] = {
		.r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_UCOMISD] = {
		.r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_UNPCKLPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x14u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},
	[OPCODE_UNPCKHPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x15u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},


	X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC(ADD, 0x58u),
	X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC(MUL, 0x59u),
	X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC(SUB, 0x5Cu),
	X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC(MIN, 0x5Du),
	X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC(DIV, 0x5Eu),
	X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC(MAX, 0x5Fu),

	X86_64_OPCODE_MAKE_VECTOR_LOGIC(AND, 0x54u),
	X86_64_OPCODE_MAKE_VECTOR_LOGIC(ANDN, 0x55u),
	X86_64_OPCODE_MAKE_VECTOR_LOGIC(OR, 0x56u),
	X86_64_OPCODE_MAKE_VECTOR_LOGIC(XOR, 0x57u),

	X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC_UNARY(SQRT, 0x51u),

	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PUNPCKLBW, OPCODE_VPUNPCKLBW,	 0x60u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PUNPCKLWD, OPCODE_VPUNPCKLWD,	 0x61u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PUNPCKLDQ, OPCODE_VPUNPCKLDQ,	 0x62u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PACKSSWB, 	OPCODE_VPACKSSWB,	 0x63u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PCMPGTB, 	OPCODE_VPCMPGTB,	 0x64u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PCMPGTW, 	OPCODE_VPCMPGTW,	 0x65u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PCMPGTD, 	OPCODE_VPCMPGTD,	 0x66u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PACKUSWB, 	OPCODE_VPACKUSWB,	 0x67u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PUNPCKHBW, OPCODE_VPUNPCKHBW,	 0x68u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PUNPCKHWD, OPCODE_VPUNPCKHWD,	 0x69u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PUNPCKHDQ, OPCODE_VPUNPCKHDQ,	 0x6Au),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PACKSSDW, 	OPCODE_VPACKSSDW,	 0x6Bu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PUNPCKLQDQ, OPCODE_VPUNPCKLQDQ, 0x6Cu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PUNPCKHQDQ, OPCODE_VPUNPCKHQDQ, 0x6Du),

	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_CMPEQB, 	OPCODE_VCMPEQB,		 0x74u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_CMPEQW, 	OPCODE_VCMPEQW,		 0x75u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_CMPEQD, 	OPCODE_VCMPEQD,		 0x76u),

	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRLW,		OPCODE_PSRLW,		0xD1u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRLD,		OPCODE_PSRLD,		0xD2u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRLQ,		OPCODE_PSRLQ,		0xD3u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDQ,		OPCODE_PADDQ,		0xD4u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMULLW,	OPCODE_PMULLW,		0xD5u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBUSB,	OPCODE_PSUBUSB,		0xD8u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBUSW,	OPCODE_PSUBUSW,		0xD9u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMINUB,	OPCODE_PMINUB,		0xDAu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PAND,		OPCODE_PAND,		0xDBu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDUSB,	OPCODE_PADDUSB,		0xDCu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDUSW,	OPCODE_PADDUSW,		0xDDu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMAXUB,	OPCODE_PMAXUB,		0xDEu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PANDN,		OPCODE_PANDN,		0xDFu),

	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PAVGB,		OPCODE_PAVGB,		0xE0u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRAW,		OPCODE_PSRAW,		0xE1u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRAD,		OPCODE_PSRAD,		0xE2u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PAVGW,		OPCODE_PAVGW,		0xE3u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMULHUW,	OPCODE_PMULHUW,		0xE4u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMULHW,	OPCODE_PMULHW,		0xE5u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBSB,	OPCODE_PSUBSB,		0xE8u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBSW,	OPCODE_PSUBSW,		0xE9u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMINSW,	OPCODE_PMINSW,		0xEAu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_POR,		OPCODE_POR,			0xEBu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDSB,	OPCODE_PADDSB,		0xECu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDSW,	OPCODE_PADDSW,		0xEDu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMAXSW,	OPCODE_PMAXSW,		0xEEu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PXOR,		OPCODE_PXOR,		0xEFu),

	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSLLW,		OPCODE_PSLLW,		0xF1u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSLLD,		OPCODE_PSLLD,		0xF2u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSLLQ,		OPCODE_PSLLQ,		0xF3u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMULUDQ,	OPCODE_PMULUDQ,		0xF4u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMADDWD,	OPCODE_PMADDWD,		0xF5u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSADBW,	OPCODE_PSADBW,		0xF6u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBB,		OPCODE_PSUBB,		0xF8u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBW,		OPCODE_PSUBW,		0xF9u),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBD,		OPCODE_PSUBD,		0xFAu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBQ,		OPCODE_PSUBQ,		0xFBu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDB,		OPCODE_PADDB,		0xFCu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDW,		OPCODE_PADDW,		0xFDu),
	X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDD,		OPCODE_PADDD,		0xFEu),


	[OPCODE_PSHUFW] = {
		.r64_rm64_i8	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) | 0x10u },
	},
	[OPCODE_PSHUFD] = {
		.r64_rm64_i8	= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) | 0x10u },
	},
	[OPCODE_PSHUFHW] = {
		.r64_rm64_i8	= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) | 0x10u },
	},
	[OPCODE_PSHUFLW] = {
		.r64_rm64_i8	= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) | 0x10u },
	},

	[OPCODE_RSQRTPS] = {
		.r128_rm128 	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},
	[OPCODE_RSQRTSS] = {
		.r128_rm128 	= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},
	[OPCODE_RCPPS] = {
		.r128_rm128 	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},
	[OPCODE_RCPSS] = {
		.r128_rm128 	= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},

	[OPCODE_VRSQRTPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VRSQRTSS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VRCPPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VRCPSS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},

	/* [OPCODE_VADDPS] = { */
	/* 	.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, none, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
	/* 	.r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, none, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
	/* }, */
	/* [OPCODE_VADDPD] = { */
	/* 	.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
	/* 	.r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
	/* }, */
	/* [OPCODE_VADDSS] = { */
	/* 	.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
	/* }, */
	/* [OPCODE_VADDSD] = { */
	/* 	.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
	/* }, */
	[OPCODE_VCOMISS] = {
		.r128_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x2Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VCOMISD] = {
		.r128_rm128	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x2Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VCVTSI2SD] = {
		.r128_vvvv_rm32 = { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
		.r128_vvvv_rm64 = { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
	},
	[OPCODE_VCVTSI2SS] = {
		.r128_vvvv_rm32 = { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
		.r128_vvvv_rm64 = { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
	},

	[OPCODE_VCVTTSD2SI] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VCVTTSS2SI] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},

	[OPCODE_VCVTSD2SI] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VCVTSS2SI] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r64_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},

	[OPCODE_VCVTPS2PD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VCVTPD2PS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VCVTSS2SD] = {
		.r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
	},
	[OPCODE_VCVTSD2SS] = {
		.r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
	},
	[OPCODE_VCVTDQ2PS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VCVTPS2DQ] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VCVTTPS2DQ] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},


	[OPCODE_VHADDPD] = {
		.r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
		.r256_vvvv_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
	},
	[OPCODE_VHADDPS] = {
		.r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
		.r256_vvvv_rm256		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
	},
	[OPCODE_VHSUBPD] = {
		.r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
		.r256_vvvv_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
	},
	[OPCODE_VHSUBPS] = {
		.r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
		.r256_vvvv_rm256		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
	},

	[OPCODE_VMOVAPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm256_r256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_VMOVAPD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm256_r256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_VMOVLPS] = {
		.r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_m(0)), },
		.rm128_r128			= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x13u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},
	[OPCODE_VMOVHPS] = {
		.r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_m(0)), },
		.rm128_r128			= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x17u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},
	[OPCODE_VMOVHPD] = {
		.r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_m(0)), },
		.rm128_r128			= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x17u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},
	[OPCODE_VMOVHLPS] = {
		.r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_r(0)), },
	},
	[OPCODE_VMOVLHPS] = {
		.r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_r(0)), },
	},
	[OPCODE_VMOVLPD] = {
		.r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_m(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x13u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},
	[OPCODE_VMOVMSKPS] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
		.r32_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
	},
	[OPCODE_VMOVMSKPD] = {
		.r32_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
		.r32_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
	},
	[OPCODE_VMOVNTPS] = {
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
		.rm256_r256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
	},
	[OPCODE_VMOVNTPD] = {
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
		.rm256_r256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
	},
	[OPCODE_VMOVUPS] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm256_r256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_VMOVUPD] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.rm256_r256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_VMOVSS] = {
		.r128_vvvv_rm128 	= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_r(0)), },
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},
	[OPCODE_VMOVSD] = {
		.r128_vvvv_rm128 	= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_r(0)), },
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)), },
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
	},
	[OPCODE_VMOVSLDUP] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},
	[OPCODE_VMOVSHDUP] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},
	[OPCODE_VMOVDDUP] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
	},

	[OPCODE_VMOVD] = {
		.r128_rm32		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm32_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_VMOVQ] = {
		.r128_rm64		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm64_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},

	[OPCODE_VMOVDQA] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm256_r256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},
	[OPCODE_VMOVDQU] = {
		.r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm128_r128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
		.r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
		.rm256_r256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
	},

	[OPCODE_VPSHUFD] = {
		.r128_rm128_i8	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
		.r256_rm256_i8	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
	},
	[OPCODE_VPSHUFHW] = {
		.r128_rm128_i8	= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
		.r256_rm256_i8	= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
	},
	[OPCODE_VPSHUFLW] = {
		.r128_rm128_i8	= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
		.r256_rm256_i8	= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
	},

	[OPCODE_VUCOMISS] = {
		.r128_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x2Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VUCOMISD] = {
		.r128_rm128	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x2Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
	},
	[OPCODE_VUNPCKLPS] = {
		.r128_vvvv_rm128 	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x14u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)), },
		.r256_vvvv_rm256 	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x14u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)), },
	},
	[OPCODE_VUNPCKHPS] = {
		.r128_vvvv_rm128 	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x15u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)), },
		.r256_vvvv_rm256 	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x15u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)), },
	},

	[OPCODE_VZEROALL] = {
		.noarg				= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x77u), .operands = 0x10u },
	},
	[OPCODE_VZEROUPPER] = {
		.noarg				= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x77u) },
	},
};

/*
 		   |- (prefix_{none, 66, f3, f2}, prefix_{1b, 2b, 3b})
		   | op
	00 00 00 00

*/

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

	case X86_64_VARIANT_KIND_r128_vvvv_rm128:
	case X86_64_VARIANT_KIND_r256_vvvv_rm256:
	case X86_64_VARIANT_KIND_r128_vvvv_rm32:
	case X86_64_VARIANT_KIND_r128_vvvv_rm64:
	case X86_64_VARIANT_KIND_r128_rm32:
	case X86_64_VARIANT_KIND_r128_rm64:
	case X86_64_VARIANT_KIND_rm32_r128:
	case X86_64_VARIANT_KIND_rm64_r128:
	case X86_64_VARIANT_KIND_r32_rm128:
	case X86_64_VARIANT_KIND_r64_rm128:
	case X86_64_VARIANT_KIND_r32_rm256:
	case X86_64_VARIANT_KIND_r64_rm32:
	case X86_64_VARIANT_KIND_rm32_r64:
		return true;

	default: return false;
	}
}

void x86_64_write_instruction(Compiler_Context* cc, X86_64_Machine_Code_Writer* b, X86_64_Variant_Kind variant, X86_64_Instruction_Variant instruction, X86_64_Instruction_Parameters parameters) {
	uint8_t mod, sib, rex = 0, prefix[4], prefixi = 0;
	uint16_t vex;
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
	case X86_64_VARIANT_KIND_r128_vvvv_rm64:
	case X86_64_VARIANT_KIND_r128_rm64:
	case X86_64_VARIANT_KIND_rm64_r128:
	case X86_64_VARIANT_KIND_r64_rm128:
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
		SWITCH_OP(2)
#undef SWITCH_OP

	bool has_vex_vvvv = false;

	if (OPCODE_GET_VEX(instruction.opcode) == OPCODE_PREFIX_vex) {
		/* ---- VEX ---- */
		vex = OPCODE_GET_PREFIX(instruction.opcode);

		switch (variant) {
		case X86_64_VARIANT_KIND_noarg:
			if (rex & 0x8u) {
				/* REX.L */
				vex |= 0x04u;
			}
			break;
		case X86_64_VARIANT_KIND_r128_vvvv_rm32:
		case X86_64_VARIANT_KIND_r128_vvvv_rm64:
		case X86_64_VARIANT_KIND_r128_vvvv_rm128:
		case X86_64_VARIANT_KIND_r128_rm32:
		case X86_64_VARIANT_KIND_rm32_r128:
		case X86_64_VARIANT_KIND_rm128_r128:
		case X86_64_VARIANT_KIND_r32_rm128:
		case X86_64_VARIANT_KIND_r64_rm128:
		case X86_64_VARIANT_KIND_r64_rm64:
		case X86_64_VARIANT_KIND_r128_rm128_i8:
		case X86_64_VARIANT_KIND_rm32_r64:
		case X86_64_VARIANT_KIND_r64_rm32:
		case X86_64_VARIANT_KIND_r128_rm128: break;

		case X86_64_VARIANT_KIND_r128_rm64:
		case X86_64_VARIANT_KIND_rm64_r128:
			if (rex & 0x8u) {
				/* REX.W */
				vex |= 0x80u;
			}
			break;

		case X86_64_VARIANT_KIND_r256_vvvv_rm256:
		case X86_64_VARIANT_KIND_rm256_r256:
		case X86_64_VARIANT_KIND_r256_rm256:
		case X86_64_VARIANT_KIND_r32_rm256:
		case X86_64_VARIANT_KIND_r256_rm256_i8:
			if (rex & 0x8u) {
				/* REX.W */
				vex |= 0x80u;
			}
			vex |= 0x4u;
			break;
		default: assert(false); break;
		}

#define SWITCH_OP(n)										\
		uint32_t op##n = GET_OPERAND##n(instruction.operands);	\
		switch (op##n >> 5u) {								\
		case OPERANDS_TYPE_vex_vvvv:						\
			printf("%x %x\n", vex, ((uint16_t)(~parameters.reg ## n & 0xFu) << 3u)); \
			vex = vex & ~(uint16_t)(0xFu << 3u) | ((uint16_t)(~parameters.reg ## n & 0xFu) << 3u);	\
			has_vex_vvvv = true; 							\
			break;											\
		default: break;										\
		}

		SWITCH_OP(0)
		SWITCH_OP(1)
		SWITCH_OP(2)
#undef SWITCH_OP

		if (!has_vex_vvvv) vex |= 0xFu << 3u;

		vex |= 0xE100u;

		if (!(vex & 0x4000u) || !(vex & 0x2000u) || (vex & 0x80u)) {
			X86_64_APPEND_OP_SEGMENT((uint8_t)0xC4u);
			X86_64_APPEND_OP_SEGMENT((uint8_t)(vex >> 8u));
			X86_64_APPEND_OP_SEGMENT((uint8_t)(vex & 0xFFu));
		} else {
			X86_64_APPEND_OP_SEGMENT((uint8_t)0xC5u);
			X86_64_APPEND_OP_SEGMENT((uint8_t)(((vex & 0x8000u) >> 8u) | (vex & 0x7Fu)));
		}

		X86_64_APPEND_OP_SEGMENT((uint8_t)(instruction.opcode & 0xFF));
	} else {
		/* if ((instruction.opcode >> 16) & 0xFF) { */
		/* 	X86_64_APPEND_OP_SEGMENT((uint8_t)((instruction.opcode >> 16) & 0xFF)); */
		/* } */
		/* if ((instruction.opcode >> 8) & 0xFF) { */
		/* 	X86_64_APPEND_OP_SEGMENT((uint8_t)((instruction.opcode >> 8) & 0xFF)); */
		/* } */

		if (parameters.rep) {
			// TODO: add check for valid instructions
			X86_64_APPEND_OP_SEGMENT(parameters.rep);
		}

		switch (OPCODE_GET_PREFIX(instruction.opcode)) {
		case OPCODE_PREFIX_none: 
			break;
		case OPCODE_PREFIX_66: 
			X86_64_APPEND_OP_SEGMENT((uint8_t)0x66u);
			break;
		case OPCODE_PREFIX_F3: 
			X86_64_APPEND_OP_SEGMENT((uint8_t)0xF3u);
			break;
		case OPCODE_PREFIX_F2: 
			X86_64_APPEND_OP_SEGMENT((uint8_t)0xF2u);
			break;
		default: assert(false); break;
		}

		if (rex) X86_64_APPEND_OP_SEGMENT(rex);
		for (i = 0; i < prefixi; ++i) {
			X86_64_APPEND_OP_SEGMENT(prefix[i]);
		}

		switch (OPCODE_GET_BYTECOUNT(instruction.opcode)) {
		case OPCODE_PREFIX_1B: 
			break;
		case OPCODE_PREFIX_2B: 
			X86_64_APPEND_OP_SEGMENT((uint8_t)0x0Fu);
			break;
		default: assert(false);
		}

		X86_64_APPEND_OP_SEGMENT((uint8_t)(instruction.opcode & 0xFF));
	}

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
		case OPERANDS_TYPE_modr:							\
			if (parameters.reg ## n & X86_64_REG_BASE) assert(false); \
			goto DO_MODRM_##n;								\
		case OPERANDS_TYPE_modm:							\
			if (!(parameters.reg ## n & X86_64_REG_BASE)) assert(false); \
			goto DO_MODRM_##n;								\
		case OPERANDS_TYPE_modrm:							\
DO_MODRM_ ##n : 											\
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
		SWITCH_OP(2)
#undef SWITCH_OP

		 //mod: 0b00 -> no offset, 0b01 -> 8 bit offset, 0b10 -> 32 bit offset, 0b11 -> no offset

		uint8_t base_reg = (parameters.reg0 & X86_64_REG_BASE) ? (parameters.reg0 & 0x7u) :
			(parameters.reg1 & X86_64_REG_BASE) ? (parameters.reg1 & 0x7u) : 
			(parameters.reg2 & X86_64_REG_BASE) ? (parameters.reg2 & 0x7u) : 0;


		if ((parameters.reg0 & X86_64_REG_BASE) || (parameters.reg1 & X86_64_REG_BASE) || (parameters.reg2 & X86_64_REG_BASE)) {

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
			/* if (!has_vex_vvvv) { */
				// TODO: do we need this?
				mod |= 0xc0;
			/* } */
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

#define xmm(n) X86_64_OPERAND_REGISTER_xmm(n)
#define reg(n) X86_64_OPERAND_REGISTER_ ## n

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
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CMOVA, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CMOVA, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVSS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVSS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVSS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVSS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVUPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVUPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVUPS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVUPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVUPS, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVUPS, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVUPS, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */

/* 	X86_64_WRITE_INSTRUCTION(OPCODE_MOVUPD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
/* 	X86_64_WRITE_INSTRUCTION(OPCODE_VMOVUPD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
/* 	X86_64_WRITE_INSTRUCTION(OPCODE_VMOVUPD, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVLPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(6), .reg2 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVLPS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg1 = X86_64_OPERAND_REGISTER_xmm(4), .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVLPS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg1 = X86_64_OPERAND_REGISTER_xmm(4), .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVHLPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(5) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVHLPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(4), .reg2 = X86_64_OPERAND_REGISTER_xmm(7) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVSLDUP, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg2 = X86_64_OPERAND_REGISTER_xmm(7) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVSLDUP, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg2 = X86_64_OPERAND_REGISTER_xmm(7) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVSLDUP, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg2 = X86_64_OPERAND_REGISTER_xmm(7) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVSD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVSD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVSD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVSD, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_UNPCKLPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2)  })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_UNPCKHPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2)  })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VUNPCKLPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VUNPCKHPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VUNPCKLPS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VUNPCKHPS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVAPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2)  })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVAPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2)  })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVAPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVAPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVAPS, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVAPS, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */


	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVNTPS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1) | X86_64_REG_BASE, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVNTPS, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1) | X86_64_REG_BASE, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVNTPS, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1) | X86_64_REG_BASE, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTPI2PS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTPI2PD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTPS2PI, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTPD2PI, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTTPS2PI, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTTPD2PI, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */


	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTSI2SS, r128_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTSI2SS, r128_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTSI2SD, r128_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTSI2SD, r128_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_VCVTSI2SS, r128_vvvv_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VCVTSI2SS, r128_vvvv_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VCVTSI2SD, r128_vvvv_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VCVTSI2SD, r128_vvvv_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTTSS2SI, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTTSD2SI, r64_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VCVTTSS2SI, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VCVTTSD2SI, r64_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTSS2SI, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_CVTSD2SI, r64_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VCVTSS2SI, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VCVTSD2SI, r64_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */

#define DO_VECTOR_ARITH(op) \
	X86_64_WRITE_INSTRUCTION(OPCODE_ ## op ## PS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); \
	X86_64_WRITE_INSTRUCTION(OPCODE_ ## op ## PD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); \
	X86_64_WRITE_INSTRUCTION(OPCODE_ ## op ## SS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); \
	X86_64_WRITE_INSTRUCTION(OPCODE_ ## op ## SD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); \
	X86_64_WRITE_INSTRUCTION(OPCODE_V ## op ## PS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
	X86_64_WRITE_INSTRUCTION(OPCODE_V ## op ## PS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
	X86_64_WRITE_INSTRUCTION(OPCODE_V ## op ## PD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
	X86_64_WRITE_INSTRUCTION(OPCODE_V ## op ## PD, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
	X86_64_WRITE_INSTRUCTION(OPCODE_V ## op ## SS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
	X86_64_WRITE_INSTRUCTION(OPCODE_V ## op ## SD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) }))

	/* DO_VECTOR_ARITH(ADD); */
	/* DO_VECTOR_ARITH(MUL); */
	/* DO_VECTOR_ARITH(SUB); */
	/* DO_VECTOR_ARITH(MIN); */
	/* DO_VECTOR_ARITH(DIV); */
	/* DO_VECTOR_ARITH(MAX); */

#undef DO_VECTOR_ARITH

	/* X86_64_WRITE_INSTRUCTION(OPCODE_PUNPCKLBW, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VPUNPCKLBW, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(6) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVMSKPS, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVMSKPD, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVMSKPS, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVMSKPD, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVMSKPS, r32_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVMSKPD, r32_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_EMMS, noarg, ((X86_64_Instruction_Parameters) { })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VZEROUPPER, noarg, ((X86_64_Instruction_Parameters) { })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VZEROALL, noarg, ((X86_64_Instruction_Parameters) { })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_PSHUFW, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_PSHUFD, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_PSHUFHW, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_PSHUFLW, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_VPSHUFD, r128_rm128_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VPSHUFHW, r128_rm128_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VPSHUFLW, r128_rm128_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_VPSHUFD, r256_rm256_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VPSHUFHW, r256_rm256_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VPSHUFLW, r256_rm256_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVD, r64_rm32, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVD, rm32_r64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVD, r128_rm32, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVD, rm32_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVQ, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVQ, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVQ, r128_rm64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVQ, rm64_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVD, r128_rm32, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVD, rm32_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVQ, r128_rm64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVQ, rm64_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVDQA, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVDQA, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVDQU, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_MOVDQU, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVDQA, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVDQA, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVDQA, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVDQA, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVDQU, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVDQU, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVDQU, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VMOVDQU, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_HADDPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_HADDPD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VHADDPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VHADDPS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VHADDPD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VHADDPD, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */

	/* X86_64_WRITE_INSTRUCTION(OPCODE_HSUBPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_HSUBPD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VHSUBPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VHSUBPS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VHSUBPD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_VHSUBPD, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
	/* X86_64_WRITE_INSTRUCTION(OPCODE_JA, rel32, ((X86_64_Instruction_Parameters) { .relative = 0x10u })); */

	X86_64_WRITE_INSTRUCTION(OPCODE_CPUID, noarg, ((X86_64_Instruction_Parameters) {}));

#undef xmm
#undef reg
}
