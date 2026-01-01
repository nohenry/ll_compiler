#pragma once
/** 
 * MIT License
 *
 * Copyright (c) 2025 Oliver Clarke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
    #define OC_MACHINE_CODE_IMPLEMENTATION 1
    #define OC_MACHINE_CODE_TEST_ENABLE 1

Examples compared to intel asm:

    mov rbp, rax
        OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_rbp,
            .reg1 = X86_64_OPERAND_REGISTER_rax,
        }));

    mov rdx, 0x78
        OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_MOV, r16_i16, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_rdx,
            .immediate = 0x78
        }));

    mov BYTE [rbp + 0x10], al
        OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_MOV, rm8_r8, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp,
            .reg1 = X86_64_OPERAND_REGISTER_rax,
            .displacement = 0x10
        }));

    mov ecx, DWORD [RBP + 0x10]
        OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_rcx,
            .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp,
            .displacement = 0x10, .immediate = 0x78
        }));


    mov eax, DWORD [rdi + rcx*2 + 0x07]
                    ^ reg1 |  |     |
                    base ^ index  |
                            ^ scale
                                    ^ displacement
        OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_rax,
            .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi,
            .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE,
            .index = X86_64_OPERAND_REGISTER_rcx,
            .scale = 2,
            .displacement = 0x07
        }));


    add rdx, 0x78
        OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm64_i8, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_rdx,
            .immediate = 0x78
        }));



Vector Examples:


    movss xmm0, xmm1
        OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_MOVSS, r128_rm128, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_xmm(0),
            .reg1 = X86_64_OPERAND_REGISTER_xmm(1)
        }));


    vcvtsi2sd xmm1, xmm2, rdx
        OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_VCVTSI2SD, r128_vvvv_rm64, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_xmm(1),
            .reg1 = X86_64_OPERAND_REGISTER_xmm(2),
            .reg2 = X86_64_OPERAND_REGISTER_rdx
        }));

No arguments:

    OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_CPUID, noarg, ((X86_64_Instruction_Parameters) {}));
    OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_CLI  , noarg, ((X86_64_Instruction_Parameters) {}));
    OC_X86_64_WRITE_INSTRUCTION(w, X86_64_OPCODE_INT3 , noarg, ((X86_64_Instruction_Parameters) {}));
*/

// #include "core1.h"
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <stdint.h>

#ifndef oc_len
#define oc_len(arr) (sizeof(arr)/sizeof((arr)[0]))
#endif
#ifndef oc_pun
#define oc_pun(value, type) ({ __typeof__(value) _v = (value); *(type*)&_v; })
#endif

typedef struct {
    void (*append_u8)(void* w, unsigned char value);
    void (*append_u16)(void* w, unsigned short value);
    void (*append_u32)(void* w, unsigned int value);
    void (*append_u64)(void* w, unsigned long long int value);

    void (*append_many)(void* w, unsigned char* bytes, unsigned long long int count);
    void (*end_instruction)(void* w);

    void (*log_error)(void* w, const char* fmt, ...);
    void (*assert_abort)(void* w, const char* fmt, ...);
} OC_Machine_Code_Writer;

typedef struct {
    OC_Machine_Code_Writer base;
    unsigned char* buffer;
    int count, capacity;

    unsigned int* strides;
    int strides_count, strides_capacity;
} OC_Default_Machine_Code_Writer;
void oc_default_machine_code_writer_append_u8(void* w, unsigned char value);
void oc_default_machine_code_writer_append_u16(void* w, unsigned short value);
void oc_default_machine_code_writer_append_u32(void* w, unsigned int value);
void oc_default_machine_code_writer_append_u64(void* w, unsigned long long int value);
void oc_default_machine_code_writer_append_many(void* w, unsigned char* bytes, unsigned long long int count);
void oc_default_machine_code_writer_end_instruction(void* w);
void oc_default_machine_code_writer_log_error(void* w, const char* fmt, ...);
void oc_default_machine_code_writer_assert_abort(void* w, const char* fmt, ...);
extern const OC_Default_Machine_Code_Writer oc_default_machine_code_writer;

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

    X86_64_OPERAND_REGISTER_invalid,
} X86_64_Operand_Register;

#define X86_64_OPERAND_REGISTER_xmm(n) ((n))
#define X86_64_OPERAND_REGISTER_ymm(n) ((n))
#define X86_64_OPERAND_REGISTER_zmm(n) ((n))

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

#define mod_rm(r) ((((unsigned int)OPERANDS_TYPE_modrm) << 5) | ((unsigned int)r) & 0xF)
#define mod_m(r) ((((unsigned int)OPERANDS_TYPE_modm) << 5) | ((unsigned int)r) & 0xF)
#define mod_r(r) ((((unsigned int)OPERANDS_TYPE_modr) << 5) | ((unsigned int)r) & 0xF)
#define mod_reg(r) ((((unsigned int)OPERANDS_TYPE_modreg) << 5) | ((unsigned int)r) & 0xF)
#define add_to_opcode (((unsigned int)OPERANDS_TYPE_add_to_opcode) << 5)
#define imm (((unsigned int)OPERANDS_TYPE_imm) << 5)
#define vex_vvvv (((unsigned int)OPERANDS_TYPE_vex_vvvv) << 5)

enum {
    X86_64_OPCODE_ADD,
    X86_64_OPCODE_ADC,
    X86_64_OPCODE_AND,
    X86_64_OPCODE_CALL,
    X86_64_OPCODE_CBW,
    X86_64_OPCODE_CWDE,
    X86_64_OPCODE_CDQE,
    X86_64_OPCODE_CWD,
    X86_64_OPCODE_CDQ,
    X86_64_OPCODE_CQO,
    X86_64_OPCODE_CLC,
    X86_64_OPCODE_CLI,
    X86_64_OPCODE_CLD,
    X86_64_OPCODE_CMC,

    X86_64_OPCODE_CMOVO,
    X86_64_OPCODE_CMOVNO,
    X86_64_OPCODE_CMOVB,
    X86_64_OPCODE_CMOVAE,
    X86_64_OPCODE_CMOVE,
    X86_64_OPCODE_CMOVNE,
    X86_64_OPCODE_CMOVBE,
    X86_64_OPCODE_CMOVA,
    X86_64_OPCODE_CMOVS,
    X86_64_OPCODE_CMOVNS,
    X86_64_OPCODE_CMOVPE,
    X86_64_OPCODE_CMOVPO,
    X86_64_OPCODE_CMOVL,
    X86_64_OPCODE_CMOVGE,
    X86_64_OPCODE_CMOVLE,
    X86_64_OPCODE_CMOVG,

    X86_64_OPCODE_CMP,
    X86_64_OPCODE_CMPSB,
    X86_64_OPCODE_CMPSW,
    X86_64_OPCODE_CMPSD,
    X86_64_OPCODE_CMPSQ,
    X86_64_OPCODE_CPUID,
    X86_64_OPCODE_DEC,
    X86_64_OPCODE_DIV,
    X86_64_OPCODE_FWAIT,
    X86_64_OPCODE_HLT,
    X86_64_OPCODE_IDIV,
    X86_64_OPCODE_IN,
    X86_64_OPCODE_INSB,
    X86_64_OPCODE_INSW,
    X86_64_OPCODE_INSD,
    X86_64_OPCODE_INC,
    X86_64_OPCODE_INT,
    X86_64_OPCODE_INT1,
    X86_64_OPCODE_INT3,
    X86_64_OPCODE_INTO,
    X86_64_OPCODE_IMUL,
    X86_64_OPCODE_IRET,
    X86_64_OPCODE_IRETD,
    X86_64_OPCODE_IRETQ,

    X86_64_OPCODE_JMP,
    X86_64_OPCODE_JO,
    X86_64_OPCODE_JNO,
    X86_64_OPCODE_JA,
    X86_64_OPCODE_JAE,
    X86_64_OPCODE_JB,
    X86_64_OPCODE_JBE,
    X86_64_OPCODE_JC,
    X86_64_OPCODE_JCXZ,
    X86_64_OPCODE_JECXZ,
    X86_64_OPCODE_JRCXZ,
    X86_64_OPCODE_JE,
    X86_64_OPCODE_JNE,
    X86_64_OPCODE_JS,
    X86_64_OPCODE_JNS,
    X86_64_OPCODE_JPE,
    X86_64_OPCODE_JPO,
    X86_64_OPCODE_JG,
    X86_64_OPCODE_JGE,
    X86_64_OPCODE_JL,
    X86_64_OPCODE_JLE,

    X86_64_OPCODE_LAHF,
    X86_64_OPCODE_LEA,
    X86_64_OPCODE_LODSB,
    X86_64_OPCODE_LODSW,
    X86_64_OPCODE_LODSD,
    X86_64_OPCODE_LODSQ,
    X86_64_OPCODE_LOOP,
    X86_64_OPCODE_LOOPE,
    X86_64_OPCODE_LOOPNE,

    X86_64_OPCODE_MOV,
    X86_64_OPCODE_MOVSB,
    X86_64_OPCODE_MOVSW,
    X86_64_OPCODE_MOVSD,
    X86_64_OPCODE_MOVSQ,
    X86_64_OPCODE_MOVSX,
    X86_64_OPCODE_MOVSXD,
    X86_64_OPCODE_MOVZX,

    X86_64_OPCODE_MUL,
    X86_64_OPCODE_NEG,
    X86_64_OPCODE_NOT,

    X86_64_OPCODE_OR,
    X86_64_OPCODE_OUT,
    X86_64_OPCODE_OUTSB,
    X86_64_OPCODE_OUTSW,
    X86_64_OPCODE_OUTSD,
    X86_64_OPCODE_POP,
    X86_64_OPCODE_POPF,
    X86_64_OPCODE_POPFQ,
    X86_64_OPCODE_PUSH,
    X86_64_OPCODE_PUSHF,
    X86_64_OPCODE_PUSHFQ,
    X86_64_OPCODE_RET,
    X86_64_OPCODE_RET_FAR,
    X86_64_OPCODE_RCL,
    X86_64_OPCODE_RCR,
    X86_64_OPCODE_ROL,
    X86_64_OPCODE_ROR,
    X86_64_OPCODE_SAHF,
    X86_64_OPCODE_SAL,
    X86_64_OPCODE_SAR,
    X86_64_OPCODE_SBB,
    X86_64_OPCODE_SCASB,
    X86_64_OPCODE_SCASW,
    X86_64_OPCODE_SCASD,
    X86_64_OPCODE_SCASQ,
    X86_64_OPCODE_SHL,
    X86_64_OPCODE_SHR,
    X86_64_OPCODE_STC,
    X86_64_OPCODE_STI,
    X86_64_OPCODE_STD,
    X86_64_OPCODE_STOSB,
    X86_64_OPCODE_STOSW,
    X86_64_OPCODE_STOSD,
    X86_64_OPCODE_STOSQ,
    X86_64_OPCODE_SUB,
    X86_64_OPCODE_SYSCALL,

    X86_64_OPCODE_SETO,
    X86_64_OPCODE_SETNO,
    X86_64_OPCODE_SETB,
    X86_64_OPCODE_SETAE,
    X86_64_OPCODE_SETE,
    X86_64_OPCODE_SETNE,
    X86_64_OPCODE_SETBE,
    X86_64_OPCODE_SETA,
    X86_64_OPCODE_SETS,
    X86_64_OPCODE_SETNS,
    X86_64_OPCODE_SETPE,
    X86_64_OPCODE_SETPO,
    X86_64_OPCODE_SETL,
    X86_64_OPCODE_SETGE,
    X86_64_OPCODE_SETLE,
    X86_64_OPCODE_SETG,

    X86_64_OPCODE_TEST,
    X86_64_OPCODE_XCHG,
    X86_64_OPCODE_XLAT,
    X86_64_OPCODE_XOR,


    X86_64_OPCODE_LDMXCSR,
    X86_64_OPCODE_STMXCSR,

    X86_64_OPCODE_ADDPS,
    X86_64_OPCODE_ADDPD,
    X86_64_OPCODE_ADDSS,
    X86_64_OPCODE_ADDSD,
    X86_64_OPCODE_MULPS,
    X86_64_OPCODE_MULPD,
    X86_64_OPCODE_MULSS,
    X86_64_OPCODE_MULSD,
    X86_64_OPCODE_SUBPS,
    X86_64_OPCODE_SUBPD,
    X86_64_OPCODE_SUBSS,
    X86_64_OPCODE_SUBSD,
    X86_64_OPCODE_MINPS,
    X86_64_OPCODE_MINPD,
    X86_64_OPCODE_MINSS,
    X86_64_OPCODE_MINSD,
    X86_64_OPCODE_DIVPS,
    X86_64_OPCODE_DIVPD,
    X86_64_OPCODE_DIVSS,
    X86_64_OPCODE_DIVSD,
    X86_64_OPCODE_MAXPS,
    X86_64_OPCODE_MAXPD,
    X86_64_OPCODE_MAXSS,
    X86_64_OPCODE_MAXSD,
    X86_64_OPCODE_SQRTPS,
    X86_64_OPCODE_SQRTPD,
    X86_64_OPCODE_SQRTSS,
    X86_64_OPCODE_SQRTSD,
    X86_64_OPCODE_RSQRTPS,
    X86_64_OPCODE_RSQRTSS,
    X86_64_OPCODE_RCPPS,
    X86_64_OPCODE_RCPSS,

    X86_64_OPCODE_ANDPS,
    X86_64_OPCODE_ANDPD,
    X86_64_OPCODE_ANDNPS,
    X86_64_OPCODE_ANDNPD,
    X86_64_OPCODE_ORPS,
    X86_64_OPCODE_ORPD,
    X86_64_OPCODE_XORPS,
    X86_64_OPCODE_XORPD,

    X86_64_OPCODE_PUNPCKLBW,
    X86_64_OPCODE_PUNPCKLWD,
    X86_64_OPCODE_PUNPCKLDQ,
    X86_64_OPCODE_PUNPCKLQDQ,
    X86_64_OPCODE_PACKSSWB,
    X86_64_OPCODE_PCMPGTB,
    X86_64_OPCODE_PCMPGTW,
    X86_64_OPCODE_PCMPGTD,
    X86_64_OPCODE_PACKUSWB,
    X86_64_OPCODE_PUNPCKHBW,
    X86_64_OPCODE_PUNPCKHWD,
    X86_64_OPCODE_PUNPCKHDQ,
    X86_64_OPCODE_PUNPCKHQDQ,
    X86_64_OPCODE_PACKSSDW,

    X86_64_OPCODE_PSHUFB,
    X86_64_OPCODE_PSHUFW,
    X86_64_OPCODE_PSHUFD,
    X86_64_OPCODE_PSHUFHW,
    X86_64_OPCODE_PSHUFLW,

    X86_64_OPCODE_PSRLW,
    X86_64_OPCODE_PSRLD,
    X86_64_OPCODE_PSRLQ,
    X86_64_OPCODE_PADDQ,
    X86_64_OPCODE_PMULLW,
    X86_64_OPCODE_PAVGB,
    X86_64_OPCODE_PSRAW,
    X86_64_OPCODE_PSRAD,
    X86_64_OPCODE_PAVGW,
    X86_64_OPCODE_PMULHUW,
    X86_64_OPCODE_PMULHW,
    X86_64_OPCODE_PSLLW,
    X86_64_OPCODE_PSLLD,
    X86_64_OPCODE_PSLLQ,
    X86_64_OPCODE_PMULUDQ,
    X86_64_OPCODE_PMADDWD,
    X86_64_OPCODE_PSADBW,
    X86_64_OPCODE_PSUBUSB,
    X86_64_OPCODE_PSUBUSW,
    X86_64_OPCODE_PMINUB,
    X86_64_OPCODE_PAND,
    X86_64_OPCODE_PADDUSB,
    X86_64_OPCODE_PADDUSW,
    X86_64_OPCODE_PMAXUB,
    X86_64_OPCODE_PANDN,
    X86_64_OPCODE_PSUBSB,
    X86_64_OPCODE_PSUBSW,
    X86_64_OPCODE_PMINSW,
    X86_64_OPCODE_POR,
    X86_64_OPCODE_PADDSB,
    X86_64_OPCODE_PADDSW,
    X86_64_OPCODE_PMAXSW,
    X86_64_OPCODE_PXOR,
    X86_64_OPCODE_PSUBB,
    X86_64_OPCODE_PSUBW,
    X86_64_OPCODE_PSUBD,
    X86_64_OPCODE_PSUBQ,
    X86_64_OPCODE_PADDB,
    X86_64_OPCODE_PADDW,
    X86_64_OPCODE_PADDD,

    X86_64_OPCODE_CMPEQB,
    X86_64_OPCODE_CMPEQW,
    X86_64_OPCODE_CMPEQD,
    X86_64_OPCODE_COMISS,
    X86_64_OPCODE_COMISD,

    X86_64_OPCODE_CVTDQ2PD,
    X86_64_OPCODE_CVTDQ2PS,
    X86_64_OPCODE_CVTPD2DQ,
    X86_64_OPCODE_CVTPD2PI,
    X86_64_OPCODE_CVTPD2PS,
    X86_64_OPCODE_CVTPI2PD,
    X86_64_OPCODE_CVTPI2PS,
    X86_64_OPCODE_CVTPS2DQ,
    X86_64_OPCODE_CVTPS2PD,
    X86_64_OPCODE_CVTPS2PI,
    X86_64_OPCODE_CVTSD2SI,
    X86_64_OPCODE_CVTSD2SS,
    X86_64_OPCODE_CVTSI2SD,
    X86_64_OPCODE_CVTSI2SS,
    X86_64_OPCODE_CVTSS2SD,
    X86_64_OPCODE_CVTSS2SI,
    X86_64_OPCODE_CVTTPD2DQ,
    X86_64_OPCODE_CVTTPD2PI,
    X86_64_OPCODE_CVTTPS2DQ,
    X86_64_OPCODE_CVTTPS2PI,
    X86_64_OPCODE_CVTTSD2SI,
    X86_64_OPCODE_CVTTSS2SI,

    X86_64_OPCODE_EMMS,
    X86_64_OPCODE_HADDPD,
    X86_64_OPCODE_HADDPS,
    X86_64_OPCODE_HSUBPD,
    X86_64_OPCODE_HSUBPS,

    X86_64_OPCODE_MOVAPS,
    X86_64_OPCODE_MOVAPD,
    X86_64_OPCODE_MOVLPS,
    X86_64_OPCODE_MOVHPS,
    X86_64_OPCODE_MOVHPD,
    X86_64_OPCODE_MOVHLPS,
    X86_64_OPCODE_MOVLHPS,
    X86_64_OPCODE_MOVLPD,
    X86_64_OPCODE_MOVMSKPS,
    X86_64_OPCODE_MOVMSKPD,
    X86_64_OPCODE_MOVNTPS,
    X86_64_OPCODE_MOVNTPD,
    X86_64_OPCODE_MOVUPS,
    X86_64_OPCODE_MOVUPD,
    X86_64_OPCODE_MOVSS,
    /* X86_64_OPCODE_MOVSD, */
    X86_64_OPCODE_MOVSLDUP,
    X86_64_OPCODE_MOVSHDUP,
    X86_64_OPCODE_MOVDDUP,
    X86_64_OPCODE_MOVD,
    X86_64_OPCODE_MOVQ,
    X86_64_OPCODE_MOVDQA,
    X86_64_OPCODE_MOVDQU,
    X86_64_OPCODE_UCOMISS,
    X86_64_OPCODE_UCOMISD,
    X86_64_OPCODE_UNPCKLPS,
    X86_64_OPCODE_UNPCKHPS,

    X86_64_OPCODE_VADDPS,
    X86_64_OPCODE_VADDPD,
    X86_64_OPCODE_VADDSS,
    X86_64_OPCODE_VADDSD,
    X86_64_OPCODE_VMULPS,
    X86_64_OPCODE_VMULPD,
    X86_64_OPCODE_VMULSS,
    X86_64_OPCODE_VMULSD,
    X86_64_OPCODE_VSUBPS,
    X86_64_OPCODE_VSUBPD,
    X86_64_OPCODE_VSUBSS,
    X86_64_OPCODE_VSUBSD,
    X86_64_OPCODE_VMINPS,
    X86_64_OPCODE_VMINPD,
    X86_64_OPCODE_VMINSS,
    X86_64_OPCODE_VMINSD,
    X86_64_OPCODE_VDIVPS,
    X86_64_OPCODE_VDIVPD,
    X86_64_OPCODE_VDIVSS,
    X86_64_OPCODE_VDIVSD,
    X86_64_OPCODE_VMAXPS,
    X86_64_OPCODE_VMAXPD,
    X86_64_OPCODE_VMAXSS,
    X86_64_OPCODE_VMAXSD,
    X86_64_OPCODE_VSQRTPS,
    X86_64_OPCODE_VSQRTPD,
    X86_64_OPCODE_VSQRTSS,
    X86_64_OPCODE_VSQRTSD,
    X86_64_OPCODE_VRSQRTPS,
    X86_64_OPCODE_VRSQRTSS,
    X86_64_OPCODE_VRCPPS,
    X86_64_OPCODE_VRCPSS,

    X86_64_OPCODE_VANDPS,
    X86_64_OPCODE_VANDPD,
    X86_64_OPCODE_VANDNPS,
    X86_64_OPCODE_VANDNPD,
    X86_64_OPCODE_VORPS,
    X86_64_OPCODE_VORPD,
    X86_64_OPCODE_VXORPS,
    X86_64_OPCODE_VXORPD,

    X86_64_OPCODE_VHADDPD,
    X86_64_OPCODE_VHADDPS,
    X86_64_OPCODE_VHSUBPD,
    X86_64_OPCODE_VHSUBPS,

    X86_64_OPCODE_VPUNPCKLBW,
    X86_64_OPCODE_VPUNPCKLWD,
    X86_64_OPCODE_VPUNPCKLDQ,
    X86_64_OPCODE_VPUNPCKLQDQ,
    X86_64_OPCODE_VPACKSSWB,
    X86_64_OPCODE_VPCMPGTB,
    X86_64_OPCODE_VPCMPGTW,
    X86_64_OPCODE_VPCMPGTD,
    X86_64_OPCODE_VPACKUSWB,
    X86_64_OPCODE_VPUNPCKHBW,
    X86_64_OPCODE_VPUNPCKHWD,
    X86_64_OPCODE_VPUNPCKHDQ,
    X86_64_OPCODE_VPUNPCKHQDQ,
    X86_64_OPCODE_VPACKSSDW,
    X86_64_OPCODE_VPSHUFD,
    X86_64_OPCODE_VPSHUFHW,
    X86_64_OPCODE_VPSHUFLW,

    X86_64_OPCODE_VPSRLW,
    X86_64_OPCODE_VPSRLD,
    X86_64_OPCODE_VPSRLQ,
    X86_64_OPCODE_VPADDQ,
    X86_64_OPCODE_VPMULLW,
    X86_64_OPCODE_VPAVGB,
    X86_64_OPCODE_VPSRAW,
    X86_64_OPCODE_VPSRAD,
    X86_64_OPCODE_VPAVGW,
    X86_64_OPCODE_VPMULHUW,
    X86_64_OPCODE_VPMULHW,
    X86_64_OPCODE_VPSLLW,
    X86_64_OPCODE_VPSLLD,
    X86_64_OPCODE_VPSLLQ,
    X86_64_OPCODE_VPMULUDQ,
    X86_64_OPCODE_VPMADDWD,
    X86_64_OPCODE_VPSADBW,
    X86_64_OPCODE_VPSUBUSB,
    X86_64_OPCODE_VPSUBUSW,
    X86_64_OPCODE_VPMINUB,
    X86_64_OPCODE_VPAND,
    X86_64_OPCODE_VPADDUSB,
    X86_64_OPCODE_VPADDUSW,
    X86_64_OPCODE_VPMAXUB,
    X86_64_OPCODE_VPANDN,
    X86_64_OPCODE_VPSUBSB,
    X86_64_OPCODE_VPSUBSW,
    X86_64_OPCODE_VPMINSW,
    X86_64_OPCODE_VPOR,
    X86_64_OPCODE_VPADDSB,
    X86_64_OPCODE_VPADDSW,
    X86_64_OPCODE_VPMAXSW,
    X86_64_OPCODE_VPXOR,
    X86_64_OPCODE_VPSUBB,
    X86_64_OPCODE_VPSUBW,
    X86_64_OPCODE_VPSUBD,
    X86_64_OPCODE_VPSUBQ,
    X86_64_OPCODE_VPADDB,
    X86_64_OPCODE_VPADDW,
    X86_64_OPCODE_VPADDD,

    X86_64_OPCODE_VCMPEQB,
    X86_64_OPCODE_VCMPEQW,
    X86_64_OPCODE_VCMPEQD,
    X86_64_OPCODE_VCOMISS,
    X86_64_OPCODE_VCOMISD,
    X86_64_OPCODE_VCVTSI2SD,
    X86_64_OPCODE_VCVTSI2SS,
    X86_64_OPCODE_VCVTSS2SI,
    X86_64_OPCODE_VCVTTSS2SI,
    X86_64_OPCODE_VCVTSD2SI,
    X86_64_OPCODE_VCVTTSD2SI,
    X86_64_OPCODE_VCVTPS2PD,
    X86_64_OPCODE_VCVTPD2PS,
    X86_64_OPCODE_VCVTSS2SD,
    X86_64_OPCODE_VCVTSD2SS,
    X86_64_OPCODE_VCVTDQ2PS,
    X86_64_OPCODE_VCVTPS2DQ,
    X86_64_OPCODE_VCVTTPS2DQ,
    X86_64_OPCODE_VMOVAPS,
    X86_64_OPCODE_VMOVAPD,
    X86_64_OPCODE_VMOVLPS,
    X86_64_OPCODE_VMOVHPS,
    X86_64_OPCODE_VMOVHPD,
    X86_64_OPCODE_VMOVHLPS,
    X86_64_OPCODE_VMOVLHPS,
    X86_64_OPCODE_VMOVLPD,
    X86_64_OPCODE_VMOVMSKPS,
    X86_64_OPCODE_VMOVMSKPD,
    X86_64_OPCODE_VMOVNTPS,
    X86_64_OPCODE_VMOVNTPD,
    X86_64_OPCODE_VMOVUPS,
    X86_64_OPCODE_VMOVUPD,
    X86_64_OPCODE_VMOVSS,
    X86_64_OPCODE_VMOVSD,
    X86_64_OPCODE_VMOVSLDUP,
    X86_64_OPCODE_VMOVSHDUP,
    X86_64_OPCODE_VMOVDDUP,
    X86_64_OPCODE_VMOVD,
    X86_64_OPCODE_VMOVQ,
    X86_64_OPCODE_VMOVDQA,
    X86_64_OPCODE_VMOVDQU,
    X86_64_OPCODE_VUCOMISS,
    X86_64_OPCODE_VUCOMISD,
    X86_64_OPCODE_VUNPCKLPS,
    X86_64_OPCODE_VUNPCKHPS,
    X86_64_OPCODE_VZEROALL,
    X86_64_OPCODE_VZEROUPPER,
};

typedef enum {
    OPERANDS_TYPE_modrm = 1,
    OPERANDS_TYPE_modreg,
    OPERANDS_TYPE_add_to_opcode,
    OPERANDS_TYPE_imm,
    OPERANDS_TYPE_vex_vvvv,
    OPERANDS_TYPE_modm,
    OPERANDS_TYPE_modr,
} X86_64_Operands_Type;

// oc_todo: maybe add __attribute__((packed))
typedef struct {
    unsigned int operands; // top 3 is Operands_Type, next 1 is don't add rex, bottom 4 is argument
    unsigned short opcode;
} X86_64_Instruction_Variant;

// clang -E backends/x86_64_common.c | grep -Po
// noarg, al_i8, ax_i16, eax_i32, rax_i32, ax_i8,  eax_i8, ax_r16, eax_r32, rax_r64, al_dx, ax_dx,  eax_dx, rm8, rm16,  rm32, rm64, rm8_cl, rm16_cl,  rm32_cl, rm64_cl, rm16_r16, rm64_r64, r16, r32, r64, r16_rm16, r64_rm64, r8_i8, r16_i16, r32_i32, r64_i64, rm8_i8, rm16_i16, rm32_i32, rm64_i32, rm16_i8, rm32_i8, rm64_i8, i8, i16, i32, rel8, rel16, rel32, r16_rm16_i8, r16_rm16_i16, r32_rm32_i32, r64_rm64_i32, r8_rm8, r32_rm32, rm32_r32, rm8_r8, r128_rm128, r256_rm256, rm128_r128, rm256_r256, r64_rm32, rm32_r64, r32_rm32_i8, r64_rm64_i8, r128_rm128_i8, r256_rm256_i8, r128_vvvv_rm128, r256_vvvv_rm256, r128_vvvv_rm32, r128_vvvv_rm64, r128_rm32, r128_rm64, rm32_rm128, rm64_rm128, rm32_r128, rm64_r128, r32_rm128, r64_rm128, r32_rm256
// \.noarg\s*\= |\.al_i8\s*\= |\.ax_i16\s*\= |\.eax_i32\s*\= |\.rax_i32\s*\= |\.ax_i8\s*\= |\. eax_i8\s*\= |\.ax_r16\s*\= |\.eax_r32\s*\= |\.rax_r64\s*\= |\.al_dx\s*\= |\.ax_dx\s*\= |\. eax_dx\s*\= |\.rm8\s*\= |\.rm16\s*\= |\. rm32\s*\= |\.rm64\s*\= |\.rm8_cl\s*\= |\.rm16_cl\s*\= |\. rm32_cl\s*\= |\.rm64_cl\s*\= |\.rm16_r16\s*\= |\.rm64_r64\s*\= |\.r16\s*\= |\.r32\s*\= |\.r64\s*\= |\.r16_rm16\s*\= |\.r64_rm64\s*\= |\.r8_i8\s*\= |\.r16_i16\s*\= |\.r32_i32\s*\= |\.r64_i64\s*\= |\.rm8_i8\s*\= |\.rm16_i16\s*\= |\.rm32_i32\s*\= |\.rm64_i32\s*\= |\.rm16_i8\s*\= |\.rm32_i8\s*\= |\.rm64_i8\s*\= |\.i8\s*\= |\.i16\s*\= |\.i32\s*\= |\.rel8\s*\= |\.rel16\s*\= |\.rel32\s*\= |\.r16_rm16_i8\s*\= |\.r16_rm16_i16\s*\= |\.r32_rm32_i32\s*\= |\.r64_rm64_i32\s*\= |\.r8_rm8\s*\= |\.r32_rm32\s*\= |\.rm32_r32\s*\= |\.rm8_r8\s*\= |\.r128_rm128\s*\= |\.r256_rm256\s*\= |\.rm128_r128\s*\= |\.rm256_r256\s*\= |\.r64_rm32\s*\= |\.rm32_r64\s*\= |\.r32_rm32_i8\s*\= |\.r64_rm64_i8\s*\= |\.r128_rm128_i8\s*\= |\.r256_rm256_i8\s*\= |\.r128_vvvv_rm128\s*\= |\.r256_vvvv_rm256\s*\= |\.r128_vvvv_rm32\s*\= |\.r128_vvvv_rm64\s*\= |\.r128_rm32\s*\= |\.r128_rm64\s*\= |\.rm32_rm128\s*\= |\.rm64_rm128\s*\= |\.rm32_r128\s*\= |\.rm64_r128\s*\= |\.r32_rm128\s*\= |\.r64_rm128\s*\= |\.r32_rm256

typedef struct {
    X86_64_Instruction_Variant
        noarg,
        al_i8, ax_i16, eax_i32, rax_i32,
               ax_i8,  eax_i8,
               ax_r16, eax_r32, rax_r64,
        al_dx, ax_dx,  eax_dx,
        rm8, rm16,  rm32, rm64,
        rm8_cl, rm16_cl,  rm32_cl, rm64_cl,
        /* rm8_r8, */ rm16_r16, /* rm32_r32, */ rm64_r64,
        r16, r32, r64,
        /* r8_rm8, */ r16_rm16, /* r32_rm32, */ r64_rm64,
        r16_rm8, r32_rm8, r64_rm8,
        r32_rm16, r64_rm16,
        r8_i8, r16_i16, r32_i32, r64_i64,
        rm8_i8, rm16_i16, rm32_i32, rm64_i32, rm16_i8, rm32_i8, rm64_i8,
        i8, i16, i32,
        rel8, rel16, rel32,
        r16_rm16_i8, 
        r16_rm16_i16, r32_rm32_i32, r64_rm64_i32;

    union {
        struct { X86_64_Instruction_Variant r8_rm8, r32_rm32, rm32_r32, rm8_r8; };
        struct { X86_64_Instruction_Variant r128_rm128, rm128_r128, r256_rm256, rm256_r256, r64_rm32, rm32_r64; };
    };
    union {
        struct { X86_64_Instruction_Variant r32_rm32_i8, r64_rm64_i8; };
        struct { X86_64_Instruction_Variant r128_rm128_i8, r256_rm256_i8; };
    };
        
    X86_64_Instruction_Variant r128_vvvv_rm128, r256_vvvv_rm256,
                               r128_vvvv_rm32,	r128_vvvv_rm64,
                               r128_rm32, r128_rm64,
                               rm32_rm128, rm64_rm128,
                               rm32_r128, rm64_r128,
                               r32_rm128, r64_rm128,
                               r32_rm256;
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
       /* X86_64_VARIANT_KIND_rax_r32, */

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

    X86_64_VARIANT_KIND_r16_rm8,
    X86_64_VARIANT_KIND_r32_rm8,
    X86_64_VARIANT_KIND_r64_rm8,
    X86_64_VARIANT_KIND_r32_rm16,
    X86_64_VARIANT_KIND_r64_rm16,

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

    X86_64_VARIANT_KIND_r128_vvvv_rm128,
    X86_64_VARIANT_KIND_r256_vvvv_rm256,

    X86_64_VARIANT_KIND_r128_vvvv_rm32,
    X86_64_VARIANT_KIND_r128_vvvv_rm64,
    X86_64_VARIANT_KIND_r128_rm32,
    X86_64_VARIANT_KIND_r128_rm64,
    X86_64_VARIANT_KIND_rm32_r128,
    X86_64_VARIANT_KIND_rm64_r128,
    X86_64_VARIANT_KIND_r32_rm128,
    X86_64_VARIANT_KIND_r64_rm128,
    X86_64_VARIANT_KIND_r32_rm256,
    X86_64_VARIANT_KIND_rm32_r64,
    X86_64_VARIANT_KIND_r64_rm32,
} X86_64_Variant_Kind;

#define X86_64_VARIANT_KIND_r128_rm128 X86_64_VARIANT_KIND_r8_rm8
#define X86_64_VARIANT_KIND_rm128_r128 X86_64_VARIANT_KIND_r32_rm32
#define X86_64_VARIANT_KIND_r256_rm256 X86_64_VARIANT_KIND_rm32_r32
#define X86_64_VARIANT_KIND_rm256_r256 X86_64_VARIANT_KIND_rm8_r8

#define X86_64_VARIANT_KIND_r128_rm128_i8 X86_64_VARIANT_KIND_r32_rm32_i8
#define X86_64_VARIANT_KIND_r256_rm256_i8 X86_64_VARIANT_KIND_r64_rm64_i8

#define X86_64_REG_BASE (0x10u)

typedef enum {
    X86_64_PREFIX_REP = 0xF3,
    X86_64_PREFIX_REPE = 0xF3,
    X86_64_PREFIX_REPNE = 0xF2,
} X86_64_Prefixes;

#define X86_64_SIB_INDEX (1u << 1u)
#define X86_64_SIB_SCALE (1u << 2u)

typedef struct {
    unsigned char reg0, reg1, reg2, reg3;
    unsigned char scale, index;
    long long int displacement, immediate, relative;
    unsigned char rep;
    unsigned char use_sib, rbp_is_rip;
} X86_64_Instruction_Parameters;

X86_64_Variant_Kind oc_x86_64_get_inverse_compare(unsigned int /* OPCODE */ opcode);
void oc_x86_64_write_nop(OC_Machine_Code_Writer* b, unsigned char byte_count);
void oc_x86_64_write_instruction(OC_Machine_Code_Writer* b, X86_64_Variant_Kind variant, X86_64_Instruction_Variant instruction, X86_64_Instruction_Parameters parameters);
void oc_x86_64_run_tests(OC_Machine_Code_Writer* b);
extern const X86_64_Instruction x86_64_instructions_table[];
extern const long long int x86_64_instructions_table_size;
extern const X86_64_Operand_Register x86_64_usable_gp_registers[];
extern const long long int x86_64_usable_gp_registers_count;

X86_64_Instruction_Variant oc_x86_64_get_variant(const X86_64_Instruction* inst, X86_64_Variant_Kind kind);

#define OC_X86_64_WRITE_INSTRUCTION(w, op, variant, parameters) oc_x86_64_write_instruction((OC_Machine_Code_Writer*)(w), X86_64_VARIANT_KIND_ ## variant, x86_64_instructions_table[op] . variant, parameters)
#define OC_X86_64_WRITE_INSTRUCTION_DYN(w, op, variant, parameters) oc_x86_64_write_instruction((OC_Machine_Code_Writer*)(w), variant, oc_x86_64_get_variant(&x86_64_instructions_table[op], variant), parameters)








enum {
    AARCH64_OPCODE_CBZ,
    AARCH64_OPCODE_CBNZ,
    AARCH64_OPCODE_TBZ,
    AARCH64_OPCODE_TBNZ,
    AARCH64_OPCODE_B_cond,
    AARCH64_OPCODE_SVC,
    AARCH64_OPCODE_HVC,
    AARCH64_OPCODE_SMC,
    AARCH64_OPCODE_BRK,
    AARCH64_OPCODE_HLT,
    AARCH64_OPCODE_DCPS1,
    AARCH64_OPCODE_DCPS2,
    AARCH64_OPCODE_DCPS3,
    AARCH64_OPCODE_MSR,
    AARCH64_OPCODE_HINT,
    AARCH64_OPCODE_CLREX,
    AARCH64_OPCODE_DSB,
    AARCH64_OPCODE_DMB,
    AARCH64_OPCODE_ISB,
    AARCH64_OPCODE_SYS,
    AARCH64_OPCODE_SYSL,
    AARCH64_OPCODE_MRS,
    AARCH64_OPCODE_BR,
    AARCH64_OPCODE_BLR,
    AARCH64_OPCODE_RET,
    AARCH64_OPCODE_ERET,
    AARCH64_OPCODE_DRPS,
    AARCH64_OPCODE_B,
    AARCH64_OPCODE_BL,
    AARCH64_OPCODE_STXRB,
    AARCH64_OPCODE_STLXRB,
    AARCH64_OPCODE_LDXRB,
    AARCH64_OPCODE_LDAXRB,
    AARCH64_OPCODE_STLRB,
    AARCH64_OPCODE_LDARB,
    AARCH64_OPCODE_STXRH,
    AARCH64_OPCODE_STLXRH,
    AARCH64_OPCODE_LDXRH,
    AARCH64_OPCODE_LDAXRH,
    AARCH64_OPCODE_STLRH,
    AARCH64_OPCODE_LDARH,
    AARCH64_OPCODE_STXR,
    AARCH64_OPCODE_STLXR,
    AARCH64_OPCODE_STXP,
    AARCH64_OPCODE_STLXP,
    AARCH64_OPCODE_LDXR,
    AARCH64_OPCODE_LDAXR,
    AARCH64_OPCODE_LDXP,
    AARCH64_OPCODE_LDAXP,
    AARCH64_OPCODE_STLR,
    AARCH64_OPCODE_LDAR,
    AARCH64_OPCODE_LDR,
    AARCH64_OPCODE_LDRSW,
    AARCH64_OPCODE_PRFM,
    AARCH64_OPCODE_STNP,
    AARCH64_OPCODE_LDNP,
    AARCH64_OPCODE_STP_post,
    AARCH64_OPCODE_LDP_post,
    AARCH64_OPCODE_LDPSW_post,
    AARCH64_OPCODE_STP_off,
    AARCH64_OPCODE_LDP_off,
    AARCH64_OPCODE_LDPSW_off,
    AARCH64_OPCODE_STP_pre,
    AARCH64_OPCODE_LDP_pre,
    AARCH64_OPCODE_LDPSW_pre,
    AARCH64_OPCODE_STURB,
    AARCH64_OPCODE_LDURB,
    AARCH64_OPCODE_LDURSB,
    AARCH64_OPCODE_STUR,
    AARCH64_OPCODE_LDUR,
    AARCH64_OPCODE_STURH,
    AARCH64_OPCODE_LDURH,
    AARCH64_OPCODE_LDURSH,
    AARCH64_OPCODE_LDURSW,
    AARCH64_OPCODE_PRFUM,
    AARCH64_OPCODE_STRB_post,
    AARCH64_OPCODE_LDRB_post,
    AARCH64_OPCODE_LDRSB_post,
    AARCH64_OPCODE_STR_post,
    AARCH64_OPCODE_LDR_post,
    AARCH64_OPCODE_STRH_post,
    AARCH64_OPCODE_LDRH_post,
    AARCH64_OPCODE_LDRSH_post,
    AARCH64_OPCODE_LDRSW_post,
    AARCH64_OPCODE_STTRB,
    AARCH64_OPCODE_LDTRB,
    AARCH64_OPCODE_LDTRSB,
    AARCH64_OPCODE_STTRH,
    AARCH64_OPCODE_LDTRH,
    AARCH64_OPCODE_LDTRSH,
    AARCH64_OPCODE_STTR,
    AARCH64_OPCODE_LDTR,
    AARCH64_OPCODE_LDTRSW,
    AARCH64_OPCODE_STRB_pre,
    AARCH64_OPCODE_LDRB_pre,
    AARCH64_OPCODE_LDRSB_pre,
    AARCH64_OPCODE_STR_pre,
    AARCH64_OPCODE_LDR_pre,
    AARCH64_OPCODE_STRH_pre,
    AARCH64_OPCODE_LDRH_pre,
    AARCH64_OPCODE_LDRSH_pre,
    AARCH64_OPCODE_LDRSW_pre,
    AARCH64_OPCODE_STRB_off,
    AARCH64_OPCODE_LDRB_off,
    AARCH64_OPCODE_LDRSB_off,
    AARCH64_OPCODE_STR_off,
    AARCH64_OPCODE_LDR_off,
    AARCH64_OPCODE_STRH_off,
    AARCH64_OPCODE_LDRH_off,
    AARCH64_OPCODE_LDRSH_off,
    AARCH64_OPCODE_LDRSW_off,
    AARCH64_OPCODE_PRFM_off,
    AARCH64_OPCODE_STRB,
    AARCH64_OPCODE_LDRB,
    AARCH64_OPCODE_LDRSB,
    AARCH64_OPCODE_STR,
    AARCH64_OPCODE_STRH,
    AARCH64_OPCODE_LDRH,
    AARCH64_OPCODE_LDRSH,
    AARCH64_OPCODE_ADR,
    AARCH64_OPCODE_ADRP,
    AARCH64_OPCODE_ADD,
    AARCH64_OPCODE_ADDS,
    AARCH64_OPCODE_SUB,
    AARCH64_OPCODE_SUBS,
    AARCH64_OPCODE_AND,
    AARCH64_OPCODE_ORR,
    AARCH64_OPCODE_MOV,
    AARCH64_OPCODE_EOR,
    AARCH64_OPCODE_ANDS,
    AARCH64_OPCODE_MOVN,
    AARCH64_OPCODE_MOVZ,
    AARCH64_OPCODE_MOVK,
    AARCH64_OPCODE_SBFM,
    AARCH64_OPCODE_BFM,
    AARCH64_OPCODE_UBFM,
    AARCH64_OPCODE_EXTR,
    AARCH64_OPCODE_BIC,
    AARCH64_OPCODE_ORN,
    AARCH64_OPCODE_EON,
    AARCH64_OPCODE_BICS,
    AARCH64_OPCODE_ADC,
    AARCH64_OPCODE_ADCS,
    AARCH64_OPCODE_SBC,
    AARCH64_OPCODE_SBCS,
    AARCH64_OPCODE_CCMN,
    AARCH64_OPCODE_CCMP,
    AARCH64_OPCODE_CSEL,
    AARCH64_OPCODE_CSINC,
    AARCH64_OPCODE_CSINV,
    AARCH64_OPCODE_CSNEG,
    AARCH64_OPCODE_MADD,
    AARCH64_OPCODE_SMADDL,
    AARCH64_OPCODE_UMADDL,
    AARCH64_OPCODE_MSUB,
    AARCH64_OPCODE_SMSUBL,
    AARCH64_OPCODE_UMSUBL,
    AARCH64_OPCODE_SMULH,
    AARCH64_OPCODE_UMULH,
    AARCH64_OPCODE_CRC32X,
    AARCH64_OPCODE_CRC32CX,
    AARCH64_OPCODE_CRC32B,
    AARCH64_OPCODE_CRC32CB,
    AARCH64_OPCODE_CRC32H,
    AARCH64_OPCODE_CRC32CH,
    AARCH64_OPCODE_CRC32W,
    AARCH64_OPCODE_CRC32CW,
    AARCH64_OPCODE_UDIV,
    AARCH64_OPCODE_SDIV,
    AARCH64_OPCODE_LSLV,
    AARCH64_OPCODE_LSRV,
    AARCH64_OPCODE_ASRV,
    AARCH64_OPCODE_RORV,
    AARCH64_OPCODE_RBIT,
    AARCH64_OPCODE_CLZ,
    AARCH64_OPCODE_CLS,
    AARCH64_OPCODE_REV,
    AARCH64_OPCODE_REV16,
    AARCH64_OPCODE_REV32,
};

typedef struct {
    union {
        unsigned int word;
        struct {
            unsigned char : 5;
            unsigned char op2 : 3;
            unsigned int crm : 6;
            unsigned int crn : 6;
            unsigned char op1 : 3;
        };
        struct {
            unsigned char rd : 5;
            unsigned char rn : 5;
            union { unsigned char rd2 : 5; unsigned char ra : 5; };
            unsigned char : 1;
            union { unsigned char rm : 5; unsigned char rs : 5; };
        };
    };
    int immediate;
    unsigned char shift;
    unsigned char cond;
    unsigned char option;
} AArch64_Instruction_Parameters;

#define AARCH64_INSTRUCTION_MODIFIER_post (1u << 0u)
#define AARCH64_INSTRUCTION_MODIFIER_offset (1u << 1u)
#define AARCH64_INSTRUCTION_MODIFIER_pre (1u << 2u)


#define AARCH64_DEFINE_VARIANTS_PARAM_EMPTY(x)
#define AARCH64_DEFINE_VARIANTS_(X, Y, reg_prefix, _has_rd, _has_rn, _has_rm, _has_rs, _has_ra, _has_rd2) \
    X(reg_prefix ## d_imm16_hw)   Y \
    X(reg_prefix ## n_nzcv_imm5_cond)   Y \
    X(reg_prefix ## d_imm14_b40)   _has_rd(1)     Y \
    X(reg_prefix ## d_imm16)       _has_rd(1)     Y \
    X(reg_prefix ## d_imm19)       _has_rd(1)     Y \
    X(reg_prefix ## d_immhi_immlo) _has_rd(1)     Y \
    X(reg_prefix ## d_ ## reg_prefix ## d2_ ## reg_prefix ## n_imm7) _has_rd2(1) _has_rn(1) _has_rd(1)      Y \
    X(reg_prefix ## d_ ## reg_prefix ## d2_ ## reg_prefix ## n_ ## reg_prefix ## s) _has_rs(1) _has_rd2(1) _has_rn(1) _has_rd(1)        Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_imm12_shift) _has_rn(1) _has_rd(1)   Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_imm12) _has_rn(1) _has_rd(1)         Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_imm9) _has_rn(1) _has_rd(1)          Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_imms_immr) _has_rn(1) _has_rd(1)     Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_imms_immr_n) _has_rn(1) _has_rd(1)     Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_ ## reg_prefix ## m_cond) _has_rm(1) _has_rn(1) _has_rd(1)       Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_ ## reg_prefix ## m_imm6_shift) _has_rm(1) _has_rn(1) _has_rd(1) Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_ ## reg_prefix ## m_shift) _has_rm(1) _has_rn(1) _has_rd(1) Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_ ## reg_prefix ## m_option) _has_rm(1) _has_rn(1) _has_rd(1) Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_ ## reg_prefix ## m_option_s) _has_rm(1) _has_rn(1) _has_rd(1) Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_ ## reg_prefix ## m_imm3_option) _has_rm(1) _has_rn(1) _has_rd(1) Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_ ## reg_prefix ## m_ ## reg_prefix ## a) _has_rm(1) _has_rn(1) _has_rd(1)         Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_ ## reg_prefix ## m_s) _has_rm(1) _has_rn(1) _has_rd(1)          Y \
    X(reg_prefix ## d_ ## reg_prefix ## n_ ## reg_prefix ## m) _has_rm(1) _has_rn(1) _has_rd(1)            Y \
    X(reg_prefix ## d_ ## reg_prefix ## n) _has_rn(1) _has_rd(1)               Y \
    X(reg_prefix ## n) _has_rn(1)              Y \
    X(reg_prefix ## d_crn_crm_op1_op2) _has_rd(1)
#define AARCH64_DEFINE_VARIANTS(X, reg_prefix) AARCH64_DEFINE_VARIANTS_(X,, reg_prefix, AARCH64_DEFINE_VARIANTS_PARAM_EMPTY, AARCH64_DEFINE_VARIANTS_PARAM_EMPTY, AARCH64_DEFINE_VARIANTS_PARAM_EMPTY, AARCH64_DEFINE_VARIANTS_PARAM_EMPTY, AARCH64_DEFINE_VARIANTS_PARAM_EMPTY, AARCH64_DEFINE_VARIANTS_PARAM_EMPTY)

#define AARCH64_DEFINE_VARIANT_KIND(X) AARCH64_VARIANT_KIND_ ## X,
#define AARCH64_DEFINE_VARIANT_INSTR(X) X,

typedef struct {
    uint32_t value;
} AArch64_Instruction_Variant;

typedef struct {
    AArch64_Instruction_Variant
        imm16,
        imm26,
        crm_op1_op2,
        crm_op2,
        crm,
        imm19_cond,
        xd_imm14_b40_b5,
        wd_wn_wm_imms,
        xd_xn_xm_imms,
        AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_INSTR, w)
        AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_INSTR, x)
        AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_INSTR, b)
        AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_INSTR, h)
        AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_INSTR, d)
        AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_INSTR, q)
        AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_INSTR, s)
        noarg;
} AArch64_Instruction;

typedef enum {
    AARCH64_VARIANT_KIND_imm16,
    AARCH64_VARIANT_KIND_imm26,
    AARCH64_VARIANT_KIND_imm19_cond,
    AARCH64_VARIANT_KIND_noarg,
    AARCH64_VARIANT_KIND_xd_imm14_b40_b5,

    AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_KIND, w)
    AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_KIND, x)
    AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_KIND, b)
    AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_KIND, h)
    AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_KIND, d)
    AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_KIND, q)
    AARCH64_DEFINE_VARIANTS(AARCH64_DEFINE_VARIANT_KIND, s)

    AARCH64_VARIANT_KIND_crm_op1_op2,
    AARCH64_VARIANT_KIND_crm_op2,
    AARCH64_VARIANT_KIND_crm,
} AArch64_Variant_Kind;

AArch64_Variant_Kind oc_aarch64_get_inverse_compare(unsigned int /* OPCODE */ opcode);
void oc_aarch64_write_nop(OC_Machine_Code_Writer* b, unsigned char byte_count);
void oc_aarch64_write_instruction(OC_Machine_Code_Writer* b, AArch64_Variant_Kind variant, AArch64_Instruction_Variant instruction, AArch64_Instruction_Parameters parameters);
void oc_aarch64_run_tests(OC_Machine_Code_Writer* b);
extern const AArch64_Instruction aarch64_instructions_table[];
extern const long long int aarch64_instructions_table_size;
extern const uint8_t aarch64_usable_gp_registers[];
extern const long long int aarch64_usable_gp_registers_count;

AArch64_Instruction_Variant oc_aarch64_get_variant(const AArch64_Instruction* inst, AArch64_Variant_Kind kind);

#define OC_AARCH64_WRITE_INSTRUCTION(w, op, variant, parameters) oc_aarch64_write_instruction((OC_Machine_Code_Writer*)(w), AARCH64_VARIANT_KIND_ ## variant, aarch64_instructions_table[op] . variant, parameters)
#define OC_AARCH64_WRITE_INSTRUCTION_DYN(w, op, variant, parameters) oc_aarch64_write_instruction((OC_Machine_Code_Writer*)(w), variant, oc_aarch64_get_variant(&aarch64_instructions_table[op], variant), parameters)

#if OC_MACHINE_CODE_IMPLEMENTATION

// #include <stdlib.h>
// #include <stdio.h>
// #include <stdarg.h>

#define default_code_writer_resize(w, byte_count)                                                    \
    do {                                                                                             \
        if ((w)->count + (byte_count) > (w)->capacity) {                                             \
            int   new_cap = (w)->capacity ? ((byte_count) + (w)->capacity * 2) : ((byte_count) * 8); \
            void* new_ptr = realloc((w)->buffer, new_cap);                                           \
            (w)->buffer = new_ptr;                                                                   \
            (w)->capacity = new_cap;                                                                 \
        }                                                                                            \
    } while (0)

#define default_code_writer_append_stride(w, value)           \
    do {                                                      \
        if ((w)->strides_count + 1 > (w)->strides_capacity) { \
            int   new_cap = (1 + (w)->strides_capacity * 2);  \
            void* new_ptr = realloc((w)->strides, new_cap);   \
            (w)->strides = new_ptr;                           \
            (w)->strides_capacity = new_cap;                  \
        }                                                     \
        (w)->strides[(w)->strides_count++] = (value);         \
    } while (0)

#define ENDIAN_TEST_VALUE 0x01020304u
#define ENDIAN_TEST_BIG_ENDIAN 1u
#define ENDIAN_TEST_IS_BIG_ENDIAN (endian_test.c[0] == ENDIAN_TEST_BIG_ENDIAN)
static union {
    uint32_t i;
    char c[4];
} endian_test = {ENDIAN_TEST_VALUE};

// void oc_default_machine_code_writer_append_u8(void* w, unsigned char value) {
//     OC_Default_Machine_Code_Writer *ww = (OC_Default_Machine_Code_Writer*)w;
//     default_code_writer_resize(ww, 1);
//     ww->buffer[ww->count] = value;
//     ww->count += 1;
// }

// void oc_default_machine_code_writer_append_u16(void* w, unsigned short value) {
//     OC_Default_Machine_Code_Writer *ww = (OC_Default_Machine_Code_Writer*)w;
//     default_code_writer_resize(ww, 2);
//     ww->buffer[ww->count + 0] = (value >> 0) & 0xFFu;
//     ww->buffer[ww->count + 1] = (value >> 8) & 0xFFu;
//     ww->count += 2;
// }

// void oc_default_machine_code_writer_append_u32(void* w, unsigned int value) {
//     OC_Default_Machine_Code_Writer *ww = (OC_Default_Machine_Code_Writer*)w;
//     default_code_writer_resize(ww, 4);
//     ww->buffer[ww->count + 0] = (value >> 0);
//     ww->buffer[ww->count + 1] = (value >> 8);
//     ww->buffer[ww->count + 2] = (value >> 16);
//     ww->buffer[ww->count + 3] = (value >> 24);
//     ww->count += 4;
// }

// void oc_default_machine_code_writer_append_u64(void* w, unsigned long long int value) {
//     OC_Default_Machine_Code_Writer *ww = (OC_Default_Machine_Code_Writer*)w;
//     default_code_writer_resize(ww, 8);
//     ww->buffer[ww->count + 0] = (value >> 0);
//     ww->buffer[ww->count + 1] = (value >> 8);
//     ww->buffer[ww->count + 2] = (value >> 16);
//     ww->buffer[ww->count + 3] = (value >> 24);
//     ww->buffer[ww->count + 4] = (value >> 32);
//     ww->buffer[ww->count + 5] = (value >> 40);
//     ww->buffer[ww->count + 6] = (value >> 48);
//     ww->buffer[ww->count + 7] = (value >> 56);
//     ww->count += 8;
// }

// void oc_default_machine_code_writer_append_many(void* w, unsigned char* bytes, unsigned long long int count) {
//     OC_Default_Machine_Code_Writer *ww = (OC_Default_Machine_Code_Writer*)w;
//     default_code_writer_resize(ww, count);
//     for (unsigned long long int i = 0; i < count; i++) {
//         ww->buffer[ww->count + i] = bytes[i];
//     }
//     ww->count += count;
// }

// void oc_default_machine_code_writer_end_instruction(void* w) {
//     OC_Default_Machine_Code_Writer *ww = (OC_Default_Machine_Code_Writer*)w;
//     default_code_writer_append_stride(ww, ww->count);
// }

// #include <stdio.h>
// void oc_default_machine_code_writer_log_error(void* w, const char* fmt, ...) {
//     va_list f;
//     va_start(f, fmt);
//     vfprintf(stderr, fmt, f);
//     va_end(f);
// }

// void oc_default_machine_code_writer_assert_abort(void* w, const char* fmt, ...) {
//     va_list f;
//     va_start(f, fmt);
//     vfprintf(stderr, fmt, f);
//     va_end(f);
//     abort();
// }

// const OC_Default_Machine_Code_Writer oc_default_machine_code_writer = {
//     .base = {
//         .append_u8 = oc_default_machine_code_writer_append_u8,
//         .append_u16 = oc_default_machine_code_writer_append_u16,
//         .append_u32 = oc_default_machine_code_writer_append_u32,
//         .append_u64 = oc_default_machine_code_writer_append_u64,
//         .append_many = oc_default_machine_code_writer_append_many,
//         .end_instruction = oc_default_machine_code_writer_end_instruction,
//         .log_error = oc_default_machine_code_writer_log_error,
//         .assert_abort = oc_default_machine_code_writer_assert_abort,
//     }
// };

const X86_64_Operand_Register x86_64_usable_gp_registers[] = {
    X86_64_OPERAND_REGISTER_rax,
    X86_64_OPERAND_REGISTER_rcx,
    X86_64_OPERAND_REGISTER_rdx,
    X86_64_OPERAND_REGISTER_rbx,
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
};
const long long int x86_64_usable_gp_registers_count = sizeof(x86_64_usable_gp_registers)/sizeof(x86_64_usable_gp_registers[0]);

#define X86_64_APPEND_OP_SEGMENT(segment) _Generic((segment), \
            unsigned char : w->append_u8, \
            unsigned short : w->append_u16, \
            unsigned int : w->append_u32, \
            unsigned long long int : w->append_u64 \
        )(w, (segment))

enum {
    X86_64_OPCODE_PREFIX_none = 0,

    X86_64_OPCODE_PREFIX_1B = 0,
    X86_64_OPCODE_PREFIX_2B = 1,
    X86_64_OPCODE_PREFIX_3B = 2,

    X86_64_OPCODE_PREFIX_66 = 1,
    X86_64_OPCODE_PREFIX_F3 = 2,
    X86_64_OPCODE_PREFIX_F2 = 3,

    X86_64_OPCODE_PREFIX_vex = 1,
};

#define BUILD_OPCODE(vex, pre, bc, opcode) (((X86_64_OPCODE_PREFIX_ ## vex) << 12) | ((X86_64_OPCODE_PREFIX_ ## pre) << 10) | ((X86_64_OPCODE_PREFIX_ ## bc) << 8) | (opcode))

#define X86_64_OPCODE_GET_BYTECOUNT(op) (((op) >> 8) & 0x3)
#define X86_64_OPCODE_GET_PREFIX(op) (((op) >> 10) & 0x3)
#define X86_64_OPCODE_GET_VEX(op) (((op) >> 12) & 0x3)

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
    [X86_64_OPCODE_ ## op ## PS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_ ## op ## PD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_ ## op ## SS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, F3, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_ ## op ## SD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, F2, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_V ## op ## PS] = { 																													\
        .r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
        .r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
    }, 																																			\
    [X86_64_OPCODE_V ## op ## PD] = { 																													\
        .r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
        .r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
    }, 																																			\
    [X86_64_OPCODE_V ## op ## SS] = { 																													\
        .r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, F3, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
    }, 																																			\
    [X86_64_OPCODE_V ## op ## SD] = { 																													\
        .r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, F2, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
    }

#define X86_64_OPCODE_MAKE_VECTOR_LOGIC(op, _opcode) \
    [X86_64_OPCODE_ ## op ## PS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_ ## op ## PD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_V ## op ## PS] = { 																													\
        .r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
        .r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
    }, 																																			\
    [X86_64_OPCODE_V ## op ## PD] = { 																													\
        .r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
        .r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, 		\
    }	

#define X86_64_OPCODE_MAKE_VECTOR_ARITHMETIC_UNARY(op, _opcode) \
    [X86_64_OPCODE_ ## op ## PS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_ ## op ## PD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_ ## op ## SS] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, F3, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_ ## op ## SD] = { .r128_rm128	= { .opcode = BUILD_OPCODE(none, F2, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, }, 	\
    [X86_64_OPCODE_V ## op ## PS] = { 																													\
        .r128_rm128 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
        .r256_rm256 = { .opcode = BUILD_OPCODE(vex, none, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
    }, 																																			\
    [X86_64_OPCODE_V ## op ## PD] = { 																													\
        .r128_rm128 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
        .r256_rm256 = { .opcode = BUILD_OPCODE(vex, 66, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
    }, 																																			\
    [X86_64_OPCODE_V ## op ## SS] = { 																													\
        .r128_rm128 = { .opcode = BUILD_OPCODE(vex, F3, 2B, _opcode), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, 		\
    }, 																																			\
    [X86_64_OPCODE_V ## op ## SD] = { 																													\
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
    X86_64_OPCODE_MAKE_ARITHMETIC(X86_64_OPCODE_ADD, 0x00u, 0u),
    X86_64_OPCODE_MAKE_ARITHMETIC(X86_64_OPCODE_OR,  0x08u, 1u),
    X86_64_OPCODE_MAKE_ARITHMETIC(X86_64_OPCODE_ADC, 0x10u, 2u),
    X86_64_OPCODE_MAKE_ARITHMETIC(X86_64_OPCODE_SBB, 0x14u, 3u),
    X86_64_OPCODE_MAKE_ARITHMETIC(X86_64_OPCODE_AND, 0x20u, 4u),
    X86_64_OPCODE_MAKE_ARITHMETIC(X86_64_OPCODE_SUB, 0x28u, 5u),
    X86_64_OPCODE_MAKE_ARITHMETIC(X86_64_OPCODE_XOR, 0x30u, 6u),
    X86_64_OPCODE_MAKE_ARITHMETIC(X86_64_OPCODE_CMP, 0x38u, 7u),
    [X86_64_OPCODE_CALL] = {
        .rel32		= { .opcode = 0xE8, .operands = MAKE_OPERANDS(imm) },
        .rm64		= { .opcode = 0xFF, .operands = MAKE_OPERANDS(mod_rm(2)) | 0x10u },
    },
    [X86_64_OPCODE_CBW] 	= { .noarg = { .opcode = 0x98, .operands = 0x20, }, },
    [X86_64_OPCODE_CWDE] 	= { .noarg = { .opcode = 0x98 }, },
    [X86_64_OPCODE_CDQE] 	= { .noarg = { .opcode = 0x98, .operands = 0x10 }, },
    [X86_64_OPCODE_CWD] 	= { .noarg = { .opcode = 0x99, .operands = 0x20, }, },
    [X86_64_OPCODE_CDQ] 	= { .noarg = { .opcode = 0x99 }, },
    [X86_64_OPCODE_CQO] 	= { .noarg = { .opcode = 0x99, .operands = 0x10 }, },

    [X86_64_OPCODE_CLC]	= { .noarg = { .opcode = 0xF8 } },
    [X86_64_OPCODE_CLI]	= { .noarg = { .opcode = 0xFA } },
    [X86_64_OPCODE_CLD]	= { .noarg = { .opcode = 0xFC } },
    [X86_64_OPCODE_CMC]   	= { .noarg = { .opcode = 0xF5u }, },

    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVO, BUILD_OPCODE(none, none, 2B, 0x40)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVNO, BUILD_OPCODE(none, none, 2B, 0x41)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVB, BUILD_OPCODE(none, none, 2B, 0x42)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVAE, BUILD_OPCODE(none, none, 2B, 0x43)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVE, BUILD_OPCODE(none, none, 2B, 0x44)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVNE, BUILD_OPCODE(none, none, 2B, 0x45)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVBE, BUILD_OPCODE(none, none, 2B, 0x46)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVA, BUILD_OPCODE(none, none, 2B, 0x47)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVS, BUILD_OPCODE(none, none, 2B, 0x48)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVNS, BUILD_OPCODE(none, none, 2B, 0x49)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVPE, BUILD_OPCODE(none, none, 2B, 0x4A)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVPO, BUILD_OPCODE(none, none, 2B, 0x4B)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVL, BUILD_OPCODE(none, none, 2B, 0x4C)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVGE, BUILD_OPCODE(none, none, 2B, 0x4D)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVLE, BUILD_OPCODE(none, none, 2B, 0x4E)),
    X86_64_OPCODE_MAKE_CMOVcc(X86_64_OPCODE_CMOVG, BUILD_OPCODE(none, none, 2B, 0x4F)),

    [X86_64_OPCODE_CMPSB] 	= { .noarg = { .opcode = 0xA6u, .operands = 0x00u }, },
    [X86_64_OPCODE_CMPSW] 	= { .noarg = { .opcode = 0xA7u, .operands = 0x20u }, },
    [X86_64_OPCODE_CMPSD] 	= { .noarg = { .opcode = 0xA7u, .operands = 0x00u }, },
    [X86_64_OPCODE_CMPSQ] 	= { .noarg = { .opcode = 0xA7u, .operands = 0x10u }, },
    [X86_64_OPCODE_CPUID] 	= { .noarg = { .opcode = BUILD_OPCODE(none, none, 2B, 0xA2u) } },
    [X86_64_OPCODE_DEC] = {
        .rm8  		= { .opcode = 0xFEu, .operands = MAKE_OPERANDS(mod_rm(1)) },
        .rm16  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(1)) },
        .rm32  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(1)) },
        .rm64  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(1)) },
    },
    [X86_64_OPCODE_DIV] = {
        .rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(6)) }, 
        .rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(6)) }, 
        .rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(6)) }, 
        .rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(6)) }, 
    },
    [X86_64_OPCODE_FWAIT] 	= { .noarg = { .opcode = 0x9bu } },
    [X86_64_OPCODE_HLT] = {
        .noarg		= { .opcode = 0xF4u },
    },
    [X86_64_OPCODE_IDIV] = {
        .rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(7)) }, 
        .rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(7)) }, 
        .rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(7)) }, 
        .rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(7)) }, 
    },
    [X86_64_OPCODE_IMUL] = {
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
    [X86_64_OPCODE_IN] = {
        .al_i8 		= { .opcode = 0xE4u, .operands = MAKE_OPERANDS(imm) },
        .ax_i8 		= { .opcode = 0xE5u, .operands = MAKE_OPERANDS(imm) },
        .eax_i8		= { .opcode = 0xE5u, .operands = MAKE_OPERANDS(imm) },
        .al_dx 		= { .opcode = 0xECu, .operands = 0x00u },
        .ax_dx 		= { .opcode = 0xEDu, .operands = 0x00u },
        .eax_dx		= { .opcode = 0xEDu, .operands = 0x00u },
    },
    [X86_64_OPCODE_INSB] 	= { .noarg	= { .opcode = 0x6Cu, } },
    [X86_64_OPCODE_INSW] 	= { .noarg	= { .opcode = 0x6Du, .operands = 0x20u } },
    [X86_64_OPCODE_INSD] 	= { .noarg	= { .opcode = 0x6Du, } },
    [X86_64_OPCODE_INC] = {
        .rm8  		= { .opcode = 0xFEu, .operands = MAKE_OPERANDS(mod_rm(0)) },
        .rm16  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(0)) },
        .rm32  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(0)) },
        .rm64  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(0)) },
    },

    [X86_64_OPCODE_INT] = {
        .i8 		= { .opcode = 0xCDu, .operands = MAKE_OPERANDS(imm) },
    },
    [X86_64_OPCODE_INT1] = {
        .noarg 		= { .opcode = 0xF1u, .operands = 0x00u },
    },
    [X86_64_OPCODE_INT3] = {
        .noarg 		= { .opcode = 0xCCu, .operands = 0x00u },
    },
    [X86_64_OPCODE_INTO] = {
        .noarg 		= { .opcode = 0xF1u, .operands = 0x00u },
    },
    [X86_64_OPCODE_IRET]	= { .noarg	= { .opcode = 0xCF, .operands = 0x20 } },
    [X86_64_OPCODE_IRETD]	= { .noarg 	= { .opcode = 0xCF } },
    [X86_64_OPCODE_IRETQ]	= { .noarg 	= { .opcode = 0xCF, .operands = 0x10 } },
    [X86_64_OPCODE_JMP] = {
        .rel8  		= { .opcode = 0xEBu, .operands = MAKE_OPERANDS(imm) },
        .rel32 		= { .opcode = 0xE9u, .operands = MAKE_OPERANDS(imm) },
        .rm64  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(4)) | 0x10 },
        // oc_todo: add far jumps
    },
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JO, 0x70, 0x80u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JNO, 0x71, 0x81u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JB, 0x72, 0x82u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JC, 0x72, 0x82u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JAE, 0x73, 0x83u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JE, 0x74, 0x84u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JNE, 0x75, 0x85u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JBE, 0x76, 0x86u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JA, 0x77, 0x87u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JS, 0x78, 0x88u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JNS, 0x79, 0x89u),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JPE, 0x7a, 0x8au),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JPO, 0x7b, 0x8bu),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JL, 0x7C, 0x8Cu),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JGE, 0x7D, 0x8Du),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JLE, 0x7E, 0x8Eu),
    X86_64_OPCODE_MAKE_Jcc(X86_64_OPCODE_JG, 0x7F, 0x8Fu),

    [X86_64_OPCODE_JCXZ] = {
        .rel8  	= { .opcode = 0xE3, .operands = MAKE_OPERANDS(imm) },
    },
    [X86_64_OPCODE_JECXZ] = {
        .rel8  	= { .opcode = 0x67E3, .operands = MAKE_OPERANDS(imm) },
    },
    [X86_64_OPCODE_JRCXZ] = {
        .rel8  	= { .opcode = 0xE3, .operands = MAKE_OPERANDS(imm) },
    },

    [X86_64_OPCODE_LAHF] 	= { .noarg	= { .opcode = 0x9F } },
    [X86_64_OPCODE_LEA] = {
        .r16_rm16	= { .opcode = 0x8D, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r32_rm32	= { .opcode = 0x8D, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm64	= { .opcode = 0x8D, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_LODSB] 	= { .noarg	= { .opcode = 0xACu } },
    [X86_64_OPCODE_LODSW] 	= { .noarg	= { .opcode = 0xADu, .operands = 0x20u, } },
    [X86_64_OPCODE_LODSD] 	= { .noarg	= { .opcode = 0xADu } },
    [X86_64_OPCODE_LODSQ] 	= { .noarg	= { .opcode = 0xADu, .operands = 0x10u, } },
    [X86_64_OPCODE_LOOP] 	= { .rel8 = { .opcode = 0xE2u, .operands = MAKE_OPERANDS(imm) } },
    [X86_64_OPCODE_LOOPE] 	= { .rel8 = { .opcode = 0xE1u, .operands = MAKE_OPERANDS(imm) } },
    [X86_64_OPCODE_LOOPNE] = { .rel8 = { .opcode = 0xE0u, .operands = MAKE_OPERANDS(imm) } },
    [X86_64_OPCODE_MOV] = {
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
    [X86_64_OPCODE_MOVSB] 	= { .noarg = { .opcode = 0xA4u, .operands = 0x00u }, },
    [X86_64_OPCODE_MOVSW] 	= { .noarg = { .opcode = 0xA5u, .operands = 0x20u }, },
    [X86_64_OPCODE_MOVSD] 	= {
        .noarg = { .opcode = 0xA5u, .operands = 0x00u },
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_MOVSQ] 	= { .noarg = { .opcode = 0xA5u, .operands = 0x10u }, },
    [X86_64_OPCODE_MOVSX] = {
        .r16_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBEu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r32_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBEu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBEu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r32_rm16  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBFu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm16  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBFu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_MOVSXD] = {
        .r16_rm16  	= { .opcode = 0x63u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm32  	= { .opcode = 0x63u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_MOVZX] = {
        .r16_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB6u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r32_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB6u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB6u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r32_rm16  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB7u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm16  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB7u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_MUL] = {
        .rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(4)) }, 
        .rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(4)) }, 
        .rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(4)) }, 
        .rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(4)) }, 
    },
    [X86_64_OPCODE_NEG] = {
        .rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(3)) }, 
        .rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(3)) }, 
        .rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(3)) }, 
        .rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(3)) }, 
    },
    [X86_64_OPCODE_NOT] = {
        .rm8 		= { .opcode = 0xF6u, .operands = MAKE_OPERANDS(mod_rm(2)) }, 
        .rm16		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(2)) }, 
        .rm32 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(2)) }, 
        .rm64 		= { .opcode = 0xF7u, .operands = MAKE_OPERANDS(mod_rm(2)) }, 
    },
    [X86_64_OPCODE_OUT] = {
        .al_i8 		= { .opcode = 0xE6u, .operands = MAKE_OPERANDS(imm) },
        .ax_i8 		= { .opcode = 0xE7u, .operands = MAKE_OPERANDS(imm) },
        .eax_i8		= { .opcode = 0xE7u, .operands = MAKE_OPERANDS(imm) },
        .al_dx 		= { .opcode = 0xEEu, .operands = 0x00u },
        .ax_dx 		= { .opcode = 0xEFu, .operands = 0x00u },
        .eax_dx		= { .opcode = 0xEFu, .operands = 0x00u },
    },
    [X86_64_OPCODE_OUTSB] 	= { .noarg	= { .opcode = 0x6Eu, } },
    [X86_64_OPCODE_OUTSW] 	= { .noarg	= { .opcode = 0x6Fu, .operands = 0x20u } },
    [X86_64_OPCODE_OUTSD] 	= { .noarg	= { .opcode = 0x6Fu, } },
    [X86_64_OPCODE_POP] = {
        .rm16 		= { .opcode = 0x8Fu, .operands = MAKE_OPERANDS(mod_rm(0)) },
        .rm64 		= { .opcode = 0x8Fu, .operands = MAKE_OPERANDS(mod_rm(0)) | 0x10 },
        .r16 		= { .opcode = 0x58u, .operands = MAKE_OPERANDS(add_to_opcode) },
        .r64 		= { .opcode = 0x58u, .operands = MAKE_OPERANDS(add_to_opcode) | 0x10 },
    },
    [X86_64_OPCODE_POPF] 	= { .noarg = { .opcode = 0x9D, .operands = 0x20u } },
    [X86_64_OPCODE_POPFQ] 	= { .noarg = { .opcode = 0x9D, .operands = 0x00u } },
    [X86_64_OPCODE_PUSH] = {
        .rm16 		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(6)) },
        .rm64 		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(6)) | 0x10 },
        .r16 		= { .opcode = 0x50u, .operands = MAKE_OPERANDS(add_to_opcode) },
        .r64 		= { .opcode = 0x50u, .operands = MAKE_OPERANDS(add_to_opcode) | 0x10 },

        .i8 		= { .opcode = 0x6Au, .operands = MAKE_OPERANDS(add_to_opcode) },
        .i16 		= { .opcode = 0x68u, .operands = MAKE_OPERANDS(add_to_opcode) },
        .i32 		= { .opcode = 0x68u, .operands = MAKE_OPERANDS(add_to_opcode) },
    },
    [X86_64_OPCODE_PUSHF] 	= { .noarg = { .opcode = 0x9C, .operands = 0x20u } },
    [X86_64_OPCODE_PUSHFQ] = { .noarg = { .opcode = 0x9C, .operands = 0x00u } },
    [X86_64_OPCODE_RET] = {
        .noarg		= { .opcode = 0xC3, .operands = 0x00u },
        .i16		= { .opcode = 0xC2, .operands = MAKE_OPERANDS(imm) },
    },
    [X86_64_OPCODE_RET_FAR] = {
        .noarg		= { .opcode = 0xCB, .operands = 0x00u },
        .i16		= { .opcode = 0xCA, .operands = MAKE_OPERANDS(imm) },
    },
    [X86_64_OPCODE_SAHF] 	= { .noarg	= { .opcode = 0x9E } },
    X86_64_OPCODE_MAKE_SHIFT_ROTATE(X86_64_OPCODE_RCL, 2),
    X86_64_OPCODE_MAKE_SHIFT_ROTATE(X86_64_OPCODE_RCR, 3),
    X86_64_OPCODE_MAKE_SHIFT_ROTATE(X86_64_OPCODE_ROL, 0),
    X86_64_OPCODE_MAKE_SHIFT_ROTATE(X86_64_OPCODE_ROR, 1),
    X86_64_OPCODE_MAKE_SHIFT_ROTATE(X86_64_OPCODE_SAL, 4),
    X86_64_OPCODE_MAKE_SHIFT_ROTATE(X86_64_OPCODE_SAR, 7),
    X86_64_OPCODE_MAKE_SHIFT_ROTATE(X86_64_OPCODE_SHL, 4),
    X86_64_OPCODE_MAKE_SHIFT_ROTATE(X86_64_OPCODE_SHR, 5),
    [X86_64_OPCODE_SCASB] 	= { .noarg	= { .opcode = 0xAEu } },
    [X86_64_OPCODE_SCASW] 	= { .noarg	= { .opcode = 0xAFu, .operands = 0x20u } },
    [X86_64_OPCODE_SCASD] 	= { .noarg	= { .opcode = 0xAFu } },
    [X86_64_OPCODE_SCASQ] 	= { .noarg	= { .opcode = 0xAFu, .operands = 0x10u } },
    [X86_64_OPCODE_STC]	= { .noarg = { .opcode = 0xF9 } },
    [X86_64_OPCODE_STI]	= { .noarg = { .opcode = 0xFB } },
    [X86_64_OPCODE_STD]	= { .noarg = { .opcode = 0xFD } },
    [X86_64_OPCODE_STOSB] 	= { .noarg	= { .opcode = 0xAAu } },
    [X86_64_OPCODE_STOSW] 	= { .noarg	= { .opcode = 0xABu, .operands = 0x20u, } },
    [X86_64_OPCODE_STOSD] 	= { .noarg	= { .opcode = 0xABu } },
    [X86_64_OPCODE_STOSQ] 	= { .noarg	= { .opcode = 0xABu, .operands = 0x10u, } },
    [X86_64_OPCODE_SYSCALL] = { .noarg = { .opcode = BUILD_OPCODE(none, none, 2B, 0x05) } },

    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETO, 0x90),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETNO, 0x91),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETB, 0x92),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETAE, 0x93),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETE, 0x94),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETNE, 0x95),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETBE, 0x96),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETA, 0x97),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETS, 0x98),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETNS, 0x99),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETPE, 0x9A),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETPO, 0x9B),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETL, 0x9C),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETGE, 0x9D),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETLE, 0x9E),
    X86_64_OPCODE_MAKE_SETcc(X86_64_OPCODE_SETG, 0x9F),

    [X86_64_OPCODE_TEST] = { 
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
    [X86_64_OPCODE_XCHG] = {
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
    [X86_64_OPCODE_XLAT] 	= { .noarg = { .opcode = 0xD7u, .operands = 0x00u }, },


    /* [X86_64_OPCODE_ADDPS] = { */
    /* 	.r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, */
    /* }, */
    /* [X86_64_OPCODE_ADDPD] = { */
    /* 	.r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, */
    /* }, */
    /* [X86_64_OPCODE_ADDSS] = { */
    /* 	.r128_rm128	= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, */
    /* }, */
    /* [X86_64_OPCODE_ADDSD] = { */
    /* 	.r128_rm128	= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) }, */
    /* }, */

    [X86_64_OPCODE_COMISS] = {
        .r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_COMISD] = {
        .r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },

    [X86_64_OPCODE_CVTDQ2PD] = {
    },
    [X86_64_OPCODE_CVTDQ2PS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTPD2DQ] = {
    },
    [X86_64_OPCODE_CVTPD2PI] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTPD2PS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTPI2PD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTPI2PS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTPS2DQ] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTPS2PD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTPS2PI] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTSD2SI] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTSD2SS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTSI2SD] = {
        .r128_rm32 = { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r128_rm64 = { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTSI2SS] = {
        .r128_rm32 = { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r128_rm64 = { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTSS2SD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTSS2SI] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTTPD2DQ] = {
    },
    [X86_64_OPCODE_CVTTPD2PI] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTTPS2DQ] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTTPS2PI] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTTSD2SI] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_CVTTSS2SI] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },

    
    [X86_64_OPCODE_EMMS] = {
        .noarg			= { .opcode = BUILD_OPCODE(none, none, 2B, 0x77u) },
    },

    [X86_64_OPCODE_HADDPD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_HADDPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },

    [X86_64_OPCODE_HSUBPD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_HSUBPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },

    [X86_64_OPCODE_MOVAPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_MOVAPD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_MOVLPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x13u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_MOVHPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x17u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_MOVHPD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x17u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_MOVHLPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
    },
    [X86_64_OPCODE_MOVLHPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
    },
    [X86_64_OPCODE_MOVLPD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x13u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },

    [X86_64_OPCODE_MOVMSKPS] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
    },
    [X86_64_OPCODE_MOVMSKPD] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
    },
    [X86_64_OPCODE_MOVNTPS] = {
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
    },
    [X86_64_OPCODE_MOVNTPD] = {
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
    },
    [X86_64_OPCODE_MOVUPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_MOVUPD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_MOVSS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    /* [X86_64_OPCODE_MOVSD] = { */
    /* 	.r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), }, */
    /* 	.rm128_r128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) }, */
    /* }, */
    [X86_64_OPCODE_MOVSLDUP] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },
    [X86_64_OPCODE_MOVSHDUP] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },
    [X86_64_OPCODE_MOVDDUP] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },

    [X86_64_OPCODE_MOVD] = {
        .r64_rm32		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm32_r64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)), },
        .r128_rm32		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm32_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)), },
    },
    [X86_64_OPCODE_MOVQ] = {
        /* .r64_rm64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), }, */
        /* .rm64_r64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)), }, */
        .r64_rm64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) | 0x10u, },
        .rm64_r64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) | 0x10u, },

        .r128_rm64		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm64_r128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)), },
    },

    [X86_64_OPCODE_MOVDQA] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x7Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_MOVDQU] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x7Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_UCOMISS] = {
        .r128_rm128	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x2Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_UCOMISD] = {
        .r128_rm128	= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x2Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_UNPCKLPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x14u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },
    [X86_64_OPCODE_UNPCKHPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(none, none, 2B, 0x15u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },

    [X86_64_OPCODE_LDMXCSR] = {
        .rm32		= { .opcode = BUILD_OPCODE(none, none, 2B, 0xAEu), .operands = MAKE_OPERANDS(mod_m(2)), },
    },
    [X86_64_OPCODE_STMXCSR] = {
        .rm32		= { .opcode = BUILD_OPCODE(none, none, 2B, 0xAEu), .operands = MAKE_OPERANDS(mod_m(3)), },
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

    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PUNPCKLBW, X86_64_OPCODE_VPUNPCKLBW,	 0x60u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PUNPCKLWD, X86_64_OPCODE_VPUNPCKLWD,	 0x61u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PUNPCKLDQ, X86_64_OPCODE_VPUNPCKLDQ,	 0x62u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PACKSSWB, 	X86_64_OPCODE_VPACKSSWB,	 0x63u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PCMPGTB, 	X86_64_OPCODE_VPCMPGTB,	 0x64u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PCMPGTW, 	X86_64_OPCODE_VPCMPGTW,	 0x65u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PCMPGTD, 	X86_64_OPCODE_VPCMPGTD,	 0x66u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PACKUSWB, 	X86_64_OPCODE_VPACKUSWB,	 0x67u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PUNPCKHBW, X86_64_OPCODE_VPUNPCKHBW,	 0x68u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PUNPCKHWD, X86_64_OPCODE_VPUNPCKHWD,	 0x69u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PUNPCKHDQ, X86_64_OPCODE_VPUNPCKHDQ,	 0x6Au),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PACKSSDW, 	X86_64_OPCODE_VPACKSSDW,	 0x6Bu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PUNPCKLQDQ, X86_64_OPCODE_VPUNPCKLQDQ, 0x6Cu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PUNPCKHQDQ, X86_64_OPCODE_VPUNPCKHQDQ, 0x6Du),

    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_CMPEQB, 	X86_64_OPCODE_VCMPEQB,		 0x74u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_CMPEQW, 	X86_64_OPCODE_VCMPEQW,		 0x75u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_CMPEQD, 	X86_64_OPCODE_VCMPEQD,		 0x76u),

    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSRLW,		X86_64_OPCODE_VPSRLW,		0xD1u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSRLD,		X86_64_OPCODE_VPSRLD,		0xD2u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSRLQ,		X86_64_OPCODE_VPSRLQ,		0xD3u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PADDQ,		X86_64_OPCODE_VPADDQ,		0xD4u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PMULLW,	X86_64_OPCODE_VPMULLW,		0xD5u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSUBUSB,	X86_64_OPCODE_VPSUBUSB,	0xD8u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSUBUSW,	X86_64_OPCODE_VPSUBUSW,	0xD9u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PMINUB,	X86_64_OPCODE_VPMINUB,		0xDAu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PAND,		X86_64_OPCODE_VPAND,		0xDBu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PADDUSB,	X86_64_OPCODE_VPADDUSB,	0xDCu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PADDUSW,	X86_64_OPCODE_VPADDUSW,	0xDDu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PMAXUB,	X86_64_OPCODE_VPMAXUB,		0xDEu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PANDN,		X86_64_OPCODE_VPANDN,		0xDFu),

    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PAVGB,		X86_64_OPCODE_VPAVGB,		0xE0u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSRAW,		X86_64_OPCODE_VPSRAW,		0xE1u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSRAD,		X86_64_OPCODE_VPSRAD,		0xE2u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PAVGW,		X86_64_OPCODE_VPAVGW,		0xE3u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PMULHUW,	X86_64_OPCODE_VPMULHUW,	0xE4u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PMULHW,	X86_64_OPCODE_VPMULHW,		0xE5u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSUBSB,	X86_64_OPCODE_VPSUBSB,		0xE8u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSUBSW,	X86_64_OPCODE_VPSUBSW,		0xE9u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PMINSW,	X86_64_OPCODE_VPMINSW,		0xEAu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_POR,		X86_64_OPCODE_VPOR,		0xEBu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PADDSB,	X86_64_OPCODE_VPADDSB,		0xECu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PADDSW,	X86_64_OPCODE_VPADDSW,		0xEDu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PMAXSW,	X86_64_OPCODE_VPMAXSW,		0xEEu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PXOR,		X86_64_OPCODE_VPXOR,		0xEFu),

    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSLLW,		X86_64_OPCODE_VPSLLW,		0xF1u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSLLD,		X86_64_OPCODE_VPSLLD,		0xF2u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSLLQ,		X86_64_OPCODE_VPSLLQ,		0xF3u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PMULUDQ,	X86_64_OPCODE_VPMULUDQ,	0xF4u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PMADDWD,	X86_64_OPCODE_VPMADDWD,	0xF5u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSADBW,	X86_64_OPCODE_VPSADBW,		0xF6u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSUBB,		X86_64_OPCODE_VPSUBB,		0xF8u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSUBW,		X86_64_OPCODE_VPSUBW,		0xF9u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSUBD,		X86_64_OPCODE_VPSUBD,		0xFAu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PSUBQ,		X86_64_OPCODE_VPSUBQ,		0xFBu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PADDB,		X86_64_OPCODE_VPADDB,		0xFCu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PADDW,		X86_64_OPCODE_VPADDW,		0xFDu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(X86_64_OPCODE_PADDD,		X86_64_OPCODE_VPADDD,		0xFEu),


    [X86_64_OPCODE_PSHUFW] = {
        .r64_rm64_i8	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) | 0x10u },
    },
    [X86_64_OPCODE_PSHUFD] = {
        .r64_rm64_i8	= { .opcode = BUILD_OPCODE(none, 66, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) | 0x10u },
    },
    [X86_64_OPCODE_PSHUFHW] = {
        .r64_rm64_i8	= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) | 0x10u },
    },
    [X86_64_OPCODE_PSHUFLW] = {
        .r64_rm64_i8	= { .opcode = BUILD_OPCODE(none, F2, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) | 0x10u },
    },

    [X86_64_OPCODE_RSQRTPS] = {
        .r128_rm128 	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },
    [X86_64_OPCODE_RSQRTSS] = {
        .r128_rm128 	= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },
    [X86_64_OPCODE_RCPPS] = {
        .r128_rm128 	= { .opcode = BUILD_OPCODE(none, none, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },
    [X86_64_OPCODE_RCPSS] = {
        .r128_rm128 	= { .opcode = BUILD_OPCODE(none, F3, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },

    [X86_64_OPCODE_VRSQRTPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VRSQRTSS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x52u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VRCPPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VRCPSS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x53u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },

    /* [X86_64_OPCODE_VADDPS] = { */
    /* 	.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, none, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
    /* 	.r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, none, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
    /* }, */
    /* [X86_64_OPCODE_VADDPD] = { */
    /* 	.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
    /* 	.r256_vvvv_rm256 = { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
    /* }, */
    /* [X86_64_OPCODE_VADDSS] = { */
    /* 	.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
    /* }, */
    /* [X86_64_OPCODE_VADDSD] = { */
    /* 	.r128_vvvv_rm128 = { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x58u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) }, */
    /* }, */
    [X86_64_OPCODE_VCOMISS] = {
        .r128_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x2Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VCOMISD] = {
        .r128_rm128	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x2Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTSI2SD] = {
        .r128_vvvv_rm32 = { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
        .r128_vvvv_rm64 = { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTSI2SS] = {
        .r128_vvvv_rm32 = { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
        .r128_vvvv_rm64 = { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
    },

    [X86_64_OPCODE_VCVTTSD2SI] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTTSS2SI] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Cu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },

    [X86_64_OPCODE_VCVTSD2SI] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTSS2SI] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x2Du), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },

    [X86_64_OPCODE_VCVTPS2PD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTPD2PS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTSS2SD] = {
        .r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTSD2SS] = {
        .r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x5Au), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTDQ2PS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTPS2DQ] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VCVTTPS2DQ] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x5Bu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },


    [X86_64_OPCODE_VHADDPD] = {
        .r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
        .r256_vvvv_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
    },
    [X86_64_OPCODE_VHADDPS] = {
        .r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
        .r256_vvvv_rm256		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x7Cu), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
    },
    [X86_64_OPCODE_VHSUBPD] = {
        .r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
        .r256_vvvv_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
    },
    [X86_64_OPCODE_VHSUBPS] = {
        .r128_vvvv_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
        .r256_vvvv_rm256		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x7Du), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)) },
    },

    [X86_64_OPCODE_VMOVAPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm256_r256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVAPD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x28u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm256_r256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x29u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVLPS] = {
        .r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_m(0)), },
        .rm128_r128			= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x13u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVHPS] = {
        .r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_m(0)), },
        .rm128_r128			= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x17u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVHPD] = {
        .r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_m(0)), },
        .rm128_r128			= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x17u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVHLPS] = {
        .r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_r(0)), },
    },
    [X86_64_OPCODE_VMOVLHPS] = {
        .r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_r(0)), },
    },
    [X86_64_OPCODE_VMOVLPD] = {
        .r128_vvvv_rm128	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_m(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x13u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVMSKPS] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
        .r32_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
    },
    [X86_64_OPCODE_VMOVMSKPD] = {
        .r32_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
        .r32_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x50u), .operands = MAKE_OPERANDS(mod_reg(0), mod_r(0)), },
    },
    [X86_64_OPCODE_VMOVNTPS] = {
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
        .rm256_r256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
    },
    [X86_64_OPCODE_VMOVNTPD] = {
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
        .rm256_r256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x2Bu), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)), },
    },
    [X86_64_OPCODE_VMOVUPS] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm256_r256		= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVUPD] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .rm256_r256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVSS] = {
        .r128_vvvv_rm128 	= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_r(0)), },
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVSD] = {
        .r128_vvvv_rm128 	= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_r(0)), },
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x10u), .operands = MAKE_OPERANDS(mod_reg(0), mod_m(0)), },
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x11u), .operands = MAKE_OPERANDS(mod_m(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVSLDUP] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },
    [X86_64_OPCODE_VMOVSHDUP] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x16u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },
    [X86_64_OPCODE_VMOVDDUP] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x12u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)), },
    },

    [X86_64_OPCODE_VMOVD] = {
        .r128_rm32		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm32_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVQ] = {
        .r128_rm64		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm64_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x7Eu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },

    [X86_64_OPCODE_VMOVDQA] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm256_r256		= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },
    [X86_64_OPCODE_VMOVDQU] = {
        .r128_rm128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm128_r128		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
        .r256_rm256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .rm256_r256		= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x6Fu), .operands = MAKE_OPERANDS(mod_rm(0), mod_reg(0)) },
    },

    [X86_64_OPCODE_VPSHUFD] = {
        .r128_rm128_i8	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
        .r256_rm256_i8	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
    },
    [X86_64_OPCODE_VPSHUFHW] = {
        .r128_rm128_i8	= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
        .r256_rm256_i8	= { .opcode = BUILD_OPCODE(vex, F3, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
    },
    [X86_64_OPCODE_VPSHUFLW] = {
        .r128_rm128_i8	= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
        .r256_rm256_i8	= { .opcode = BUILD_OPCODE(vex, F2, 2B, 0x70u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0), imm) },
    },

    [X86_64_OPCODE_VUCOMISS] = {
        .r128_rm128	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x2Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VUCOMISD] = {
        .r128_rm128	= { .opcode = BUILD_OPCODE(vex, 66, 2B, 0x2Eu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [X86_64_OPCODE_VUNPCKLPS] = {
        .r128_vvvv_rm128 	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x14u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)), },
        .r256_vvvv_rm256 	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x14u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)), },
    },
    [X86_64_OPCODE_VUNPCKHPS] = {
        .r128_vvvv_rm128 	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x15u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)), },
        .r256_vvvv_rm256 	= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x15u), .operands = MAKE_OPERANDS(mod_reg(0), vex_vvvv, mod_rm(0)), },
    },

    [X86_64_OPCODE_VZEROALL] = {
        .noarg				= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x77u), .operands = 0x10u },
    },
    [X86_64_OPCODE_VZEROUPPER] = {
        .noarg				= { .opcode = BUILD_OPCODE(vex, none, 2B, 0x77u) },
    },
};

/*
            |- (prefix_{none, 66, f3, f2}, prefix_{1b, 2b, 3b})
           | op
    00 00 00 00

*/

const long long int x86_64_instructions_table_size = sizeof(x86_64_instructions_table);

X86_64_Variant_Kind oc_x86_64_get_inverse_compare(unsigned int /* OPCODE*/ opcode) {
    switch (opcode) {
    case X86_64_OPCODE_JO: return X86_64_OPCODE_JNO;
    case X86_64_OPCODE_JNO: return X86_64_OPCODE_JO;
    case X86_64_OPCODE_JA: return X86_64_OPCODE_JBE;
    case X86_64_OPCODE_JAE: return X86_64_OPCODE_JB;
    case X86_64_OPCODE_JB: return X86_64_OPCODE_JAE;
    case X86_64_OPCODE_JBE: return X86_64_OPCODE_JA;
    case X86_64_OPCODE_JC: return X86_64_OPCODE_JAE;
    case X86_64_OPCODE_JE: return X86_64_OPCODE_JNE;
    case X86_64_OPCODE_JNE: return X86_64_OPCODE_JE;
    case X86_64_OPCODE_JS: return X86_64_OPCODE_JNS;
    case X86_64_OPCODE_JNS: return X86_64_OPCODE_JS;
    case X86_64_OPCODE_JPE: return X86_64_OPCODE_JPO;
    case X86_64_OPCODE_JPO: return X86_64_OPCODE_JPE;
    case X86_64_OPCODE_JG: return X86_64_OPCODE_JLE;
    case X86_64_OPCODE_JGE: return X86_64_OPCODE_JL;
    case X86_64_OPCODE_JL: return X86_64_OPCODE_JGE;
    case X86_64_OPCODE_JLE: return X86_64_OPCODE_JG;
    default: assert(false); return 0;
    }
}

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

    case X86_64_VARIANT_KIND_r16_rm8:
    case X86_64_VARIANT_KIND_r32_rm8:
    case X86_64_VARIANT_KIND_r64_rm8:
    case X86_64_VARIANT_KIND_r32_rm16:
    case X86_64_VARIANT_KIND_r64_rm16:
        return true;

    default: return false;
    }
}

void oc_x86_64_write_nop(OC_Machine_Code_Writer* b, unsigned char byte_count) {
#define WRITE_NOP(...) do { 										\
            unsigned char bytes[] = {__VA_ARGS__};					\
            b->append_many((void*)b, bytes, oc_len(bytes));	\
        } while (0)
    switch (byte_count) {
    case 1: WRITE_NOP(0x90); break;
    case 2: WRITE_NOP(0x66, 0x90); break;
    case 3: WRITE_NOP(0x0F, 0x1F, 0x00); break;
    case 4: WRITE_NOP(0x0F, 0x1F, 0x40, 0x00); break;
    case 5: WRITE_NOP(0x0F, 0x1F, 0x44, 0x00, 0x00); break;
    case 6: WRITE_NOP(0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00); break;
    case 7: WRITE_NOP(0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00); break;
    case 8: WRITE_NOP(0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00); break;
    case 9: WRITE_NOP(0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00); break;
    default: assert(false);
    }
#undef WRITE_NOP
}

void oc_x86_64_write_instruction(OC_Machine_Code_Writer* w, X86_64_Variant_Kind variant, X86_64_Instruction_Variant instruction, X86_64_Instruction_Parameters parameters) {
    unsigned char mod, sib, rex = 0, prefix[4], prefixi = 0;
    unsigned short vex;
    int i;

    if (instruction.opcode == 0) {
        w->log_error(w, "invalid opcode: %d");

        __builtin_debugtrap();
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
    case X86_64_VARIANT_KIND_r64_rm8:
    case X86_64_VARIANT_KIND_r64_rm16:
    case X86_64_VARIANT_KIND_r64_rm32:
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
        unsigned int op##n = GET_OPERAND##n(instruction.operands);		\
        switch (op##n >> 5u) {										\
        case OPERANDS_TYPE_modr:									\
        case OPERANDS_TYPE_modreg:									\
            if ((parameters.reg ## n & 0xFu) >= X86_64_OPERAND_REGISTER_r8)	{ 	\
                rex |= 0x44u; 										\
            } 														\
            break;													\
        case OPERANDS_TYPE_modm:									\
        case OPERANDS_TYPE_modrm:									\
            if ((parameters.reg ## n & 0xFu) >= X86_64_OPERAND_REGISTER_r8)	{ 	\
                rex |= 0x41u; 										\
            } 														\
            break;													\
        case OPERANDS_TYPE_add_to_opcode:                           \
            instruction.opcode += parameters.reg ## n;              \
            break;                                                  \
        default: break;												\
        }

        SWITCH_OP(0)
        SWITCH_OP(1)
        SWITCH_OP(2)
#undef SWITCH_OP

    bool has_vex_vvvv = false;

    if (X86_64_OPCODE_GET_VEX(instruction.opcode) == X86_64_OPCODE_PREFIX_vex) {
        /* ---- VEX ---- */
        vex = X86_64_OPCODE_GET_PREFIX(instruction.opcode);

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
        unsigned int op##n = GET_OPERAND##n(instruction.operands);	\
        switch (op##n >> 5u) {								\
        case OPERANDS_TYPE_vex_vvvv:						\
            vex = vex & ~(unsigned short)(0xFu << 3u) | ((unsigned short)(~parameters.reg ## n & 0xFu) << 3u);	\
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
            X86_64_APPEND_OP_SEGMENT((unsigned char)0xC4u);
            X86_64_APPEND_OP_SEGMENT((unsigned char)(vex >> 8u));
            X86_64_APPEND_OP_SEGMENT((unsigned char)(vex & 0xFFu));
        } else {
            X86_64_APPEND_OP_SEGMENT((unsigned char)0xC5u);
            X86_64_APPEND_OP_SEGMENT((unsigned char)(((vex & 0x8000u) >> 8u) | (vex & 0x7Fu)));
        }

        X86_64_APPEND_OP_SEGMENT((unsigned char)(instruction.opcode & 0xFF));
    } else {
        /* if ((instruction.opcode >> 16) & 0xFF) { */
        /* 	X86_64_APPEND_OP_SEGMENT((unsigned char)((instruction.opcode >> 16) & 0xFF)); */
        /* } */
        /* if ((instruction.opcode >> 8) & 0xFF) { */
        /* 	X86_64_APPEND_OP_SEGMENT((unsigned char)((instruction.opcode >> 8) & 0xFF)); */
        /* } */

        if (parameters.rep) {
            // oc_todo: add check for valid instructions
            X86_64_APPEND_OP_SEGMENT(parameters.rep);
        }

        switch (X86_64_OPCODE_GET_PREFIX(instruction.opcode)) {
        case X86_64_OPCODE_PREFIX_none: 
            break;
        case X86_64_OPCODE_PREFIX_66: 
            X86_64_APPEND_OP_SEGMENT((unsigned char)0x66u);
            break;
        case X86_64_OPCODE_PREFIX_F3: 
            X86_64_APPEND_OP_SEGMENT((unsigned char)0xF3u);
            break;
        case X86_64_OPCODE_PREFIX_F2: 
            X86_64_APPEND_OP_SEGMENT((unsigned char)0xF2u);
            break;
        default: assert(false); break;
        }

        if (rex) X86_64_APPEND_OP_SEGMENT(rex);
        for (i = 0; i < prefixi; ++i) {
            X86_64_APPEND_OP_SEGMENT(prefix[i]);
        }

        switch (X86_64_OPCODE_GET_BYTECOUNT(instruction.opcode)) {
        case X86_64_OPCODE_PREFIX_1B: 
            break;
        case X86_64_OPCODE_PREFIX_2B: 
            X86_64_APPEND_OP_SEGMENT((unsigned char)0x0Fu);
            break;
        default: assert(false);
        }

        X86_64_APPEND_OP_SEGMENT((unsigned char)(instruction.opcode & 0xFF));
    }

    if (x86_64_uses_modrm(variant)) {
        mod = 0;
        sib = 0;

#define SWITCH_OP(n)										\
        unsigned int op##n = GET_OPERAND##n(instruction.operands);	\
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

        unsigned char base_reg = (parameters.reg0 & X86_64_REG_BASE) ? (parameters.reg0 & 0x7u) :
            (parameters.reg1 & X86_64_REG_BASE) ? (parameters.reg1 & 0x7u) : 
            (parameters.reg2 & X86_64_REG_BASE) ? (parameters.reg2 & 0x7u) : 0;


        if ((parameters.reg0 & X86_64_REG_BASE) || (parameters.reg1 & X86_64_REG_BASE) || (parameters.reg2 & X86_64_REG_BASE)) {

            long long int offset = parameters.displacement;

            if (!parameters.use_sib && base_reg == X86_64_OPERAND_REGISTER_rsp) {
                parameters.use_sib = 1;
                sib = (X86_64_OPERAND_REGISTER_rsp << 3u) | X86_64_OPERAND_REGISTER_rsp;
            }

            if (parameters.use_sib) {
                mod |= 0x4u;
                if (parameters.use_sib & X86_64_SIB_SCALE) {
                    if (!(parameters.use_sib & X86_64_SIB_INDEX)) {
                        w->assert_abort(w, "scale requires index");
                    }
                    sib |= parameters.scale << 6u;
                }
                if (parameters.use_sib & X86_64_SIB_INDEX) {
                    if (parameters.index == X86_64_OPERAND_REGISTER_rsp)  {
                        w->assert_abort(w, "invalid use of rsp as index with rbp as base");
                    }
                    sib |= (parameters.index & 7u) << 3u;
                } else {
                    sib |= X86_64_OPERAND_REGISTER_rsp << 3u; // rsp as index means no index
                }

                if (offset == 0)  {
                    // rbp as base means no base, only if mod is 0
                    if (base_reg == X86_64_OPERAND_REGISTER_rbp) {
                        // oc_todo: i feel like this might be wrong, below too
                        assert((mod >> 6) == 0);
                        if (parameters.use_sib & X86_64_SIB_SCALE) {
                        } else if (!parameters.rbp_is_rip) {
                            mod |= 0x40u;
                        }
                    }
                    X86_64_APPEND_OP_SEGMENT(mod);
                    X86_64_APPEND_OP_SEGMENT(sib);
                    if (base_reg == X86_64_OPERAND_REGISTER_rbp) {
                        // rbp needs dsp32, so we just add 0
                        if (parameters.use_sib & X86_64_SIB_SCALE) {
                            X86_64_APPEND_OP_SEGMENT((unsigned int)0);
                        } else {
                            X86_64_APPEND_OP_SEGMENT((unsigned char)0);
                        }
                    }
                } else if (offset >= INT8_MIN && offset <= INT8_MAX) {
                    mod |= 0x40;
                    X86_64_APPEND_OP_SEGMENT(mod);
                    X86_64_APPEND_OP_SEGMENT(sib);
                    X86_64_APPEND_OP_SEGMENT(oc_pun((signed char)offset, unsigned char));
                } else if (offset >= INT32_MIN && offset <= INT32_MAX) {
                    mod |= 0x80;
                    X86_64_APPEND_OP_SEGMENT(mod);
                    X86_64_APPEND_OP_SEGMENT(sib);
                    X86_64_APPEND_OP_SEGMENT(oc_pun((int)offset, unsigned int));
                }
            } else if (base_reg == X86_64_OPERAND_REGISTER_rbp && parameters.rbp_is_rip) {
                X86_64_APPEND_OP_SEGMENT(mod);
                X86_64_APPEND_OP_SEGMENT(oc_pun((int)offset, unsigned int));
            } else if (offset == 0 && base_reg != X86_64_OPERAND_REGISTER_rbp) {
                X86_64_APPEND_OP_SEGMENT(mod);
            } else if (offset >= INT8_MIN && (signed char)offset <= INT8_MAX) {
                mod |= 0x40;
                X86_64_APPEND_OP_SEGMENT(mod);
                X86_64_APPEND_OP_SEGMENT(oc_pun((signed char)offset, unsigned char));
            } else if (offset >= INT32_MIN && (int)offset <= INT32_MAX) {
                mod |= 0x80;
                X86_64_APPEND_OP_SEGMENT(mod);
                X86_64_APPEND_OP_SEGMENT(oc_pun((int)offset, unsigned int));
            } else {
                assert(false);
            }
        } else {
            /* if (!has_vex_vvvv) { */
                // oc_todo: do we need this?
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
        X86_64_APPEND_OP_SEGMENT(oc_pun((signed char)parameters.immediate, unsigned char));
           break;

    case X86_64_VARIANT_KIND_ax_i16:
    case X86_64_VARIANT_KIND_rm16_i16:
    case X86_64_VARIANT_KIND_r16_i16:
    case X86_64_VARIANT_KIND_i16:
    case X86_64_VARIANT_KIND_r16_rm16_i16:
        X86_64_APPEND_OP_SEGMENT(oc_pun((short)parameters.immediate, unsigned short));
        break;

    case X86_64_VARIANT_KIND_rax_i32:
    case X86_64_VARIANT_KIND_eax_i32:
    case X86_64_VARIANT_KIND_rm32_i32:
    case X86_64_VARIANT_KIND_r32_i32:
    case X86_64_VARIANT_KIND_rm64_i32:
    case X86_64_VARIANT_KIND_i32:
    case X86_64_VARIANT_KIND_r32_rm32_i32:
       case X86_64_VARIANT_KIND_r64_rm64_i32:
        X86_64_APPEND_OP_SEGMENT(oc_pun((int)parameters.immediate, unsigned int));
        break;

    case X86_64_VARIANT_KIND_r64_i64:
        X86_64_APPEND_OP_SEGMENT(oc_pun((long long int)parameters.immediate, unsigned long long int));
        break;


    /* Relative addresses */
    case X86_64_VARIANT_KIND_rel8:
        X86_64_APPEND_OP_SEGMENT(oc_pun((signed char)parameters.relative, unsigned char));
           break;

    case X86_64_VARIANT_KIND_rel16:
        X86_64_APPEND_OP_SEGMENT(oc_pun((short)parameters.relative, unsigned short));
           break;

    case X86_64_VARIANT_KIND_rel32:
        X86_64_APPEND_OP_SEGMENT(oc_pun((int)parameters.relative, unsigned int));
           break;

    default: break;
    }

    w->end_instruction(w);
}


X86_64_Instruction_Variant oc_x86_64_get_variant(const X86_64_Instruction* inst, X86_64_Variant_Kind kind) {
    X86_64_Instruction_Variant result;
#define MAKE_VARIANT(v) case X86_64_VARIANT_KIND_ ## v: result = inst-> v; break;
    switch (kind) {
        MAKE_VARIANT(noarg);

        MAKE_VARIANT(al_i8);
        MAKE_VARIANT(ax_i16);
        MAKE_VARIANT(eax_i32);
        MAKE_VARIANT(rax_i32);

        MAKE_VARIANT(ax_i8);
        MAKE_VARIANT(eax_i8);

        MAKE_VARIANT(ax_r16);
        MAKE_VARIANT(eax_r32);

        MAKE_VARIANT(al_dx);
        MAKE_VARIANT(ax_dx);
        MAKE_VARIANT(eax_dx);

        MAKE_VARIANT(rm8);
        MAKE_VARIANT(rm16);
        MAKE_VARIANT(rm32);
        MAKE_VARIANT(rm64);

        MAKE_VARIANT(rm8_r8);
        MAKE_VARIANT(rm16_r16);
        MAKE_VARIANT(rm32_r32);
        MAKE_VARIANT(rm64_r64);

        MAKE_VARIANT(rm8_cl);
        MAKE_VARIANT(rm16_cl);
        MAKE_VARIANT(rm32_cl);
        MAKE_VARIANT(rm64_cl);

        MAKE_VARIANT(r16);
        MAKE_VARIANT(r32);
        MAKE_VARIANT(r64);

        MAKE_VARIANT(r8_rm8);
        MAKE_VARIANT(r16_rm16);
        MAKE_VARIANT(r32_rm32);
        MAKE_VARIANT(r64_rm64);

        MAKE_VARIANT(r8_i8);
        MAKE_VARIANT(r16_i16);
        MAKE_VARIANT(r32_i32);
        MAKE_VARIANT(r64_i64);

        MAKE_VARIANT(rm8_i8);
        MAKE_VARIANT(rm16_i16);
        MAKE_VARIANT(rm32_i32);
        MAKE_VARIANT(rm64_i32);
        MAKE_VARIANT(rm16_i8);
        MAKE_VARIANT(rm32_i8);
        MAKE_VARIANT(rm64_i8);

        MAKE_VARIANT(r16_rm8);
        MAKE_VARIANT(r32_rm8);
        MAKE_VARIANT(r64_rm8);
        MAKE_VARIANT(r32_rm16);
        MAKE_VARIANT(r64_rm16);

        MAKE_VARIANT(i8);
        MAKE_VARIANT(i16);
        MAKE_VARIANT(i32);

        MAKE_VARIANT(rel8);
        MAKE_VARIANT(rel16);
        MAKE_VARIANT(rel32);

        MAKE_VARIANT(r16_rm16_i8);
        MAKE_VARIANT(r32_rm32_i8);
        MAKE_VARIANT(r64_rm64_i8);
        MAKE_VARIANT(r16_rm16_i16);
        MAKE_VARIANT(r32_rm32_i32);
        MAKE_VARIANT(r64_rm64_i32);

        MAKE_VARIANT(r128_vvvv_rm128);
        MAKE_VARIANT(r256_vvvv_rm256);

        MAKE_VARIANT(r128_vvvv_rm32);
        MAKE_VARIANT(r128_vvvv_rm64);
        MAKE_VARIANT(r128_rm32);
        MAKE_VARIANT(r128_rm64);
        MAKE_VARIANT(rm32_r128);
        MAKE_VARIANT(rm64_r128);
        MAKE_VARIANT(r32_rm128);
        MAKE_VARIANT(r64_rm128);
        MAKE_VARIANT(r32_rm256);
        MAKE_VARIANT(rm32_r64);
        MAKE_VARIANT(r64_rm32);
        default: assert(false);
    }
#undef MAKE_VARIANT
    return result;
}









const AArch64_Instruction aarch64_instructions_table[] = {
    [AARCH64_OPCODE_CBZ] = {
        .wd_imm19 = { .value = 0x34000000u },
        .xd_imm19 = { .value = 0xB4000000u },
    },
    [AARCH64_OPCODE_CBNZ] = {
        .wd_imm19 = { .value = 0x35000000u },
        .xd_imm19 = { .value = 0xB5000000u },
    },
    [AARCH64_OPCODE_TBZ] = {
        .xd_imm14_b40_b5 = { .value = 0x36000000u },
    },
    [AARCH64_OPCODE_TBNZ] = {
        .xd_imm14_b40_b5 = { .value = 0x37000000u },
    },
    [AARCH64_OPCODE_B_cond] = {
        .imm19_cond = { .value = 0x54000000u },
    },
    [AARCH64_OPCODE_SVC] = {
        .imm16 = { .value = 0xD4000001u },
    },
    [AARCH64_OPCODE_HVC] = {
        .imm16 = { .value = 0xD4000002u },
    },
    [AARCH64_OPCODE_SMC] = {
        .imm16 = { .value = 0xD4000003u },
    },
    [AARCH64_OPCODE_BRK] = {
        .imm16 = { .value = 0xD4200000u },
    },
    [AARCH64_OPCODE_HLT] = {
        .imm16 = { .value = 0xD4400000u },
    },
    [AARCH64_OPCODE_DCPS1] = {
        .imm16 = { .value = 0xD4A00001u },
    },
    [AARCH64_OPCODE_DCPS2] = {
        .imm16 = { .value = 0xD4A00002u },
    },
    [AARCH64_OPCODE_DCPS3] = {
        .imm16 = { .value = 0xD4A00003u },
    },
    [AARCH64_OPCODE_MSR] = {
        .crm_op1_op2 = { .value = 0xD500401Fu },
        .xd_crn_crm_op1_op2 = { .value = 0xD5100000u },
    },
    [AARCH64_OPCODE_HINT] = {
        .crm_op2 = { .value = 0xD503201Fu },
    },
    [AARCH64_OPCODE_CLREX] = {
        .crm = { .value = 0xD503305Fu },
    },
    [AARCH64_OPCODE_DSB] = {
        .crm = { .value = 0xD503309Fu },
    },
    [AARCH64_OPCODE_DMB] = {
        .crm = { .value = 0xD50330BFu },
    },
    [AARCH64_OPCODE_ISB] = {
        .crm = { .value = 0xD50330DFu },
    },
    [AARCH64_OPCODE_SYS] = {
        .xd_crn_crm_op1_op2 = { .value = 0xD5080000u },
    },
    [AARCH64_OPCODE_SYSL] = {
        .xd_crn_crm_op1_op2 = { .value = 0xD5280000u },
    },
    [AARCH64_OPCODE_MRS] = {
        .xd_crn_crm_op1_op2 = { .value = 0xD5300000u },
    },
    [AARCH64_OPCODE_BR] = {
        .xn = { .value = 0xD61F0000u },
    },
    [AARCH64_OPCODE_BLR] = {
        .xn = { .value = 0xD63F0000u },
    },
    [AARCH64_OPCODE_RET] = {
        .xn = { .value = 0xD65F0000u },
    },
    [AARCH64_OPCODE_ERET] = {
        .noarg = { .value = 0xD69F03E0u },
    },
    [AARCH64_OPCODE_DRPS] = {
        .noarg = { .value = 0xD6BF03E0u },
    },
    [AARCH64_OPCODE_B] = {
        .imm26 = { .value = 0x14000000u },
    },
    [AARCH64_OPCODE_BL] = {
        .imm26 = { .value = 0x94000000u },
    },
    [AARCH64_OPCODE_STXRB] = {
        .xd_xd2_xn_xs = { .value = 0x8000000u },
    },
    [AARCH64_OPCODE_STLXRB] = {
        .xd_xd2_xn_xs = { .value = 0x8008000u },
    },
    [AARCH64_OPCODE_LDXRB] = {
        .xd_xd2_xn_xs = { .value = 0x8400000u },
    },
    [AARCH64_OPCODE_LDAXRB] = {
        .xd_xd2_xn_xs = { .value = 0x8408000u },
    },
    [AARCH64_OPCODE_STLRB] = {
        .xd_xd2_xn_xs = { .value = 0x8808000u },
    },
    [AARCH64_OPCODE_LDARB] = {
        .xd_xd2_xn_xs = { .value = 0x8C08000u },
    },
    [AARCH64_OPCODE_STXRH] = {
        .xd_xd2_xn_xs = { .value = 0x48000000u },
    },
    [AARCH64_OPCODE_STLXRH] = {
        .xd_xd2_xn_xs = { .value = 0x48008000u },
    },
    [AARCH64_OPCODE_LDXRH] = {
        .xd_xd2_xn_xs = { .value = 0x48400000u },
    },
    [AARCH64_OPCODE_LDAXRH] = {
        .xd_xd2_xn_xs = { .value = 0x48408000u },
    },
    [AARCH64_OPCODE_STLRH] = {
        .xd_xd2_xn_xs = { .value = 0x48808000u },
    },
    [AARCH64_OPCODE_LDARH] = {
        .xd_xd2_xn_xs = { .value = 0x48C08000u },
    },
    [AARCH64_OPCODE_STXR] = {
        .wd_wd2_wn_ws = { .value = 0x88000000u },
        .xd_xd2_xn_xs = { .value = 0xC8000000u },
    },
    [AARCH64_OPCODE_STLXR] = {
        .wd_wd2_wn_ws = { .value = 0x88008000u },
        .xd_xd2_xn_xs = { .value = 0xC8008000u },
    },
    [AARCH64_OPCODE_STXP] = {
        .wd_wd2_wn_ws = { .value = 0x88200000u },
        .xd_xd2_xn_xs = { .value = 0xC8200000u },
    },
    [AARCH64_OPCODE_STLXP] = {
        .wd_wd2_wn_ws = { .value = 0x88208000u },
        .xd_xd2_xn_xs = { .value = 0xC8208000u },
    },
    [AARCH64_OPCODE_LDXR] = {
        .wd_wd2_wn_ws = { .value = 0x88400000u },
        .xd_xd2_xn_xs = { .value = 0xC8400000u },
    },
    [AARCH64_OPCODE_LDAXR] = {
        .wd_wd2_wn_ws = { .value = 0x88408000u },
        .xd_xd2_xn_xs = { .value = 0xC8408000u },
    },
    [AARCH64_OPCODE_LDXP] = {
        .wd_wd2_wn_ws = { .value = 0x88600000u },
        .xd_xd2_xn_xs = { .value = 0xC8600000u },
    },
    [AARCH64_OPCODE_LDAXP] = {
        .wd_wd2_wn_ws = { .value = 0x88608000u },
        .xd_xd2_xn_xs = { .value = 0xC8608000u },
    },
    [AARCH64_OPCODE_STLR] = {
        .wd_wd2_wn_ws = { .value = 0x88808000u },
        .xd_xd2_xn_xs = { .value = 0xC8808000u },
    },
    [AARCH64_OPCODE_LDAR] = {
        .wd_wd2_wn_ws = { .value = 0x88C08000u },
        .xd_xd2_xn_xs = { .value = 0xC8C08000u },
    },
    [AARCH64_OPCODE_LDR] = {
        .wd_imm19 = { .value = 0x18000000u },
        .sd_imm19 = { .value = 0x1C000000u },
        .xd_imm19 = { .value = 0x58000000u },
        .dd_imm19 = { .value = 0x5C000000u },
        .qd_imm19 = { .value = 0x9C000000u },
        .bd_bn_imm12 = { .value = 0x3D400000u },
        .qd_qn_imm12 = { .value = 0x3DC00000u },
        .hd_hn_imm12 = { .value = 0x7D400000u },
        .wd_wn_imm12 = { .value = 0xB9400000u },
        .sd_sn_imm12 = { .value = 0xBD400000u },
        .xd_xn_imm12 = { .value = 0xF9400000u },
        .dd_dn_imm12 = { .value = 0xFD400000u },
    },
    [AARCH64_OPCODE_LDRSW] = {
        .xd_imm19 = { .value = 0x98000000u },
        .xd_xn_imm12 = { .value = 0xB9800000u },
    },
    [AARCH64_OPCODE_PRFM] = {
        .xd_imm19 = { .value = 0xD8000000u },
        .xd_xn_imm12 = { .value = 0xF9800000u },
    },
    [AARCH64_OPCODE_STNP] = {
        .wd_wd2_wn_imm7 = { .value = 0x28000000u },
        .sd_sd2_sn_imm7 = { .value = 0x2C000000u },
        .dd_dd2_dn_imm7 = { .value = 0x6C000000u },
        .xd_xd2_xn_imm7 = { .value = 0xA8000000u },
        .qd_qd2_qn_imm7 = { .value = 0xAC000000u },
    },
    [AARCH64_OPCODE_LDNP] = {
        .wd_wd2_wn_imm7 = { .value = 0x28400000u },
        .sd_sd2_sn_imm7 = { .value = 0x2C400000u },
        .dd_dd2_dn_imm7 = { .value = 0x6C400000u },
        .xd_xd2_xn_imm7 = { .value = 0xA8400000u },
        .qd_qd2_qn_imm7 = { .value = 0xAC400000u },
    },
    [AARCH64_OPCODE_STP_post] = {
        .wd_wd2_wn_imm7 = { .value = 0x28800000u },
        .sd_sd2_sn_imm7 = { .value = 0x2C800000u },
        .dd_dd2_dn_imm7 = { .value = 0x6C800000u },
        .xd_xd2_xn_imm7 = { .value = 0xA8800000u },
        .qd_qd2_qn_imm7 = { .value = 0xAC800000u },
    },
    [AARCH64_OPCODE_LDP_post] = {
        .wd_wd2_wn_imm7 = { .value = 0x28C00000u },
        .sd_sd2_sn_imm7 = { .value = 0x2CC00000u },
        .dd_dd2_dn_imm7 = { .value = 0x6CC00000u },
        .xd_xd2_xn_imm7 = { .value = 0xA8C00000u },
        .qd_qd2_qn_imm7 = { .value = 0xACC00000u },
    },
    [AARCH64_OPCODE_LDPSW_post] = {
        .xd_xd2_xn_imm7 = { .value = 0x68C00000u },
    },
    [AARCH64_OPCODE_STP_off] = {
        .wd_wd2_wn_imm7 = { .value = 0x29000000u },
        .sd_sd2_sn_imm7 = { .value = 0x2D000000u },
        .dd_dd2_dn_imm7 = { .value = 0x6D000000u },
        .xd_xd2_xn_imm7 = { .value = 0xA9000000u },
        .qd_qd2_qn_imm7 = { .value = 0xAD000000u },
    },
    [AARCH64_OPCODE_LDP_off] = {
        .wd_wd2_wn_imm7 = { .value = 0x29400000u },
        .sd_sd2_sn_imm7 = { .value = 0x2D400000u },
        .dd_dd2_dn_imm7 = { .value = 0x6D400000u },
        .xd_xd2_xn_imm7 = { .value = 0xA9400000u },
        .qd_qd2_qn_imm7 = { .value = 0xAD400000u },
    },
    [AARCH64_OPCODE_LDPSW_off] = {
        .xd_xd2_xn_imm7 = { .value = 0x69400000u },
    },
    [AARCH64_OPCODE_STP_pre] = {
        .wd_wd2_wn_imm7 = { .value = 0x29800000u },
        .sd_sd2_sn_imm7 = { .value = 0x2D800000u },
        .dd_dd2_dn_imm7 = { .value = 0x6D800000u },
        .xd_xd2_xn_imm7 = { .value = 0xA9800000u },
        .qd_qd2_qn_imm7 = { .value = 0xAD800000u },
    },
    [AARCH64_OPCODE_LDP_pre] = {
        .wd_wd2_wn_imm7 = { .value = 0x29C00000u },
        .sd_sd2_sn_imm7 = { .value = 0x2DC00000u },
        .dd_dd2_dn_imm7 = { .value = 0x6DC00000u },
        .xd_xd2_xn_imm7 = { .value = 0xA9C00000u },
        .qd_qd2_qn_imm7 = { .value = 0xADC00000u },
    },
    [AARCH64_OPCODE_LDPSW_pre] = {
        .xd_xd2_xn_imm7 = { .value = 0x69C00000u },
    },
    [AARCH64_OPCODE_STURB] = {
        .xd_xn_imm9 = { .value = 0x38000000u },
    },
    [AARCH64_OPCODE_LDURB] = {
        .xd_xn_imm9 = { .value = 0x38400000u },
    },
    [AARCH64_OPCODE_LDURSB] = {
        .xd_xn_imm9 = { .value = 0x38800000u },
        .wd_wn_imm9 = { .value = 0x38C00000u },
    },
    [AARCH64_OPCODE_STUR] = {
        .bd_bn_imm9 = { .value = 0x3C000000u },
        .qd_qn_imm9 = { .value = 0x3C800000u },
        .hd_hn_imm9 = { .value = 0x7C000000u },
        .wd_wn_imm9 = { .value = 0xB8000000u },
        .sd_sn_imm9 = { .value = 0xBC000000u },
        .xd_xn_imm9 = { .value = 0xF8000000u },
        .dd_dn_imm9 = { .value = 0xFC000000u },
    },
    [AARCH64_OPCODE_LDUR] = {
        .bd_bn_imm9 = { .value = 0x3C400000u },
        .qd_qn_imm9 = { .value = 0x3CC00000u },
        .hd_hn_imm9 = { .value = 0x7C400000u },
        .wd_wn_imm9 = { .value = 0xB8400000u },
        .sd_sn_imm9 = { .value = 0xBC400000u },
        .xd_xn_imm9 = { .value = 0xF8400000u },
        .dd_dn_imm9 = { .value = 0xFC400000u },
    },
    [AARCH64_OPCODE_STURH] = {
        .xd_xn_imm9 = { .value = 0x78000000u },
    },
    [AARCH64_OPCODE_LDURH] = {
        .xd_xn_imm9 = { .value = 0x78400000u },
    },
    [AARCH64_OPCODE_LDURSH] = {
        .xd_xn_imm9 = { .value = 0x78800000u },
        .wd_wn_imm9 = { .value = 0x78C00000u },
    },
    [AARCH64_OPCODE_LDURSW] = {
        .xd_xn_imm9 = { .value = 0xB8800000u },
    },
    [AARCH64_OPCODE_PRFUM] = {
        .xd_xn_imm9 = { .value = 0xF8800000u },
    },
    [AARCH64_OPCODE_STRB_post] = {
        .xd_xn_imm9 = { .value = 0x38000400u },
    },
    [AARCH64_OPCODE_LDRB_post] = {
        .xd_xn_imm9 = { .value = 0x38400400u },
    },
    [AARCH64_OPCODE_LDRSB_post] = {
        .xd_xn_imm9 = { .value = 0x38800400u },
        .wd_wn_imm9 = { .value = 0x38C00400u },
    },
    [AARCH64_OPCODE_STR_post] = {
        .bd_bn_imm9 = { .value = 0x3C000400u },
        .qd_qn_imm9 = { .value = 0x3C800400u },
        .hd_hn_imm9 = { .value = 0x7C000400u },
        .wd_wn_imm9 = { .value = 0xB8000400u },
        .sd_sn_imm9 = { .value = 0xBC000400u },
        .xd_xn_imm9 = { .value = 0xF8000400u },
        .dd_dn_imm9 = { .value = 0xFC000400u },
    },
    [AARCH64_OPCODE_LDR_post] = {
        .bd_bn_imm9 = { .value = 0x3C400400u },
        .qd_qn_imm9 = { .value = 0x3CC00400u },
        .hd_hn_imm9 = { .value = 0x7C400400u },
        .wd_wn_imm9 = { .value = 0xB8400400u },
        .sd_sn_imm9 = { .value = 0xBC400400u },
        .xd_xn_imm9 = { .value = 0xF8400400u },
        .dd_dn_imm9 = { .value = 0xFC400400u },
    },
    [AARCH64_OPCODE_STRH_post] = {
        .xd_xn_imm9 = { .value = 0x78000400u },
    },
    [AARCH64_OPCODE_LDRH_post] = {
        .xd_xn_imm9 = { .value = 0x78400400u },
    },
    [AARCH64_OPCODE_LDRSH_post] = {
        .xd_xn_imm9 = { .value = 0x78800400u },
        .wd_wn_imm9 = { .value = 0x78C00400u },
    },
    [AARCH64_OPCODE_LDRSW_post] = {
        .xd_xn_imm9 = { .value = 0xB8800400u },
    },
    [AARCH64_OPCODE_STTRB] = {
        .xd_xn_imm9 = { .value = 0x38000800u },
    },
    [AARCH64_OPCODE_LDTRB] = {
        .xd_xn_imm9 = { .value = 0x38400800u },
    },
    [AARCH64_OPCODE_LDTRSB] = {
        .xd_xn_imm9 = { .value = 0x38800800u },
        .wd_wn_imm9 = { .value = 0x38C00800u },
    },
    [AARCH64_OPCODE_STTRH] = {
        .xd_xn_imm9 = { .value = 0x78000800u },
    },
    [AARCH64_OPCODE_LDTRH] = {
        .xd_xn_imm9 = { .value = 0x78400800u },
    },
    [AARCH64_OPCODE_LDTRSH] = {
        .xd_xn_imm9 = { .value = 0x78800800u },
        .wd_wn_imm9 = { .value = 0x78C00800u },
    },
    [AARCH64_OPCODE_STTR] = {
        .wd_wn_imm9 = { .value = 0xB8000800u },
        .xd_xn_imm9 = { .value = 0xF8000800u },
    },
    [AARCH64_OPCODE_LDTR] = {
        .wd_wn_imm9 = { .value = 0xB8400800u },
        .xd_xn_imm9 = { .value = 0xF8400800u },
    },
    [AARCH64_OPCODE_LDTRSW] = {
        .xd_xn_imm9 = { .value = 0xB8800800u },
    },
    [AARCH64_OPCODE_STRB_pre] = {
        .xd_xn_imm9 = { .value = 0x38000C00u },
    },
    [AARCH64_OPCODE_LDRB_pre] = {
        .xd_xn_imm9 = { .value = 0x38400C00u },
    },
    [AARCH64_OPCODE_LDRSB_pre] = {
        .xd_xn_imm9 = { .value = 0x38800C00u },
        .wd_wn_imm9 = { .value = 0x38C00C00u },
    },
    [AARCH64_OPCODE_STR_pre] = {
        .bd_bn_imm9 = { .value = 0x3C000C00u },
        .qd_qn_imm9 = { .value = 0x3C800C00u },
        .hd_hn_imm9 = { .value = 0x7C000C00u },
        .wd_wn_imm9 = { .value = 0xB8000C00u },
        .sd_sn_imm9 = { .value = 0xBC000C00u },
        .xd_xn_imm9 = { .value = 0xF8000C00u },
        .dd_dn_imm9 = { .value = 0xFC000C00u },
    },
    [AARCH64_OPCODE_LDR_pre] = {
        .bd_bn_imm9 = { .value = 0x3C400C00u },
        .qd_qn_imm9 = { .value = 0x3CC00C00u },
        .hd_hn_imm9 = { .value = 0x7C400C00u },
        .wd_wn_imm9 = { .value = 0xB8400C00u },
        .sd_sn_imm9 = { .value = 0xBC400C00u },
        .xd_xn_imm9 = { .value = 0xF8400C00u },
        .dd_dn_imm9 = { .value = 0xFC400C00u },
    },
    [AARCH64_OPCODE_STRH_pre] = {
        .xd_xn_imm9 = { .value = 0x78000C00u },
    },
    [AARCH64_OPCODE_LDRH_pre] = {
        .xd_xn_imm9 = { .value = 0x78400C00u },
    },
    [AARCH64_OPCODE_LDRSH_pre] = {
        .xd_xn_imm9 = { .value = 0x78800C00u },
        .wd_wn_imm9 = { .value = 0x78C00C00u },
    },
    [AARCH64_OPCODE_LDRSW_pre] = {
        .xd_xn_imm9 = { .value = 0xB8800C00u },
    },
    [AARCH64_OPCODE_STRB_off] = {
        .xd_xn_xm_option_s = { .value = 0x38200800u },
    },
    [AARCH64_OPCODE_LDRB_off] = {
        .xd_xn_xm_option_s = { .value = 0x38600800u },
    },
    [AARCH64_OPCODE_LDRSB_off] = {
        .xd_xn_xm_option_s = { .value = 0x38A00800u },
        .wd_wn_wm_option_s = { .value = 0x38E00800u },
    },
    [AARCH64_OPCODE_STR_off] = {
        .bd_bn_bm_option_s = { .value = 0x3C200800u },
        .qd_qn_qm_option_s = { .value = 0x3CA00800u },
        .hd_hn_hm_option_s = { .value = 0x7C200800u },
        .wd_wn_wm_option_s = { .value = 0xB8200800u },
        .sd_sn_sm_option_s = { .value = 0xBC200800u },
        .xd_xn_xm_option_s = { .value = 0xF8200800u },
        .dd_dn_dm_option_s = { .value = 0xFC200800u },
    },
    [AARCH64_OPCODE_LDR_off] = {
        .bd_bn_bm_option_s = { .value = 0x3C600800u },
        .qd_qn_qm_option_s = { .value = 0x3CE00800u },
        .hd_hn_hm_option_s = { .value = 0x7C600800u },
        .wd_wn_wm_option_s = { .value = 0xB8600800u },
        .sd_sn_sm_option_s = { .value = 0xBC600800u },
        .xd_xn_xm_option_s = { .value = 0xF8600800u },
        .dd_dn_dm_option_s = { .value = 0xFC600800u },
    },
    [AARCH64_OPCODE_STRH_off] = {
        .xd_xn_xm_option_s = { .value = 0x78200800u },
    },
    [AARCH64_OPCODE_LDRH_off] = {
        .xd_xn_xm_option_s = { .value = 0x78600800u },
    },
    [AARCH64_OPCODE_LDRSH_off] = {
        .xd_xn_xm_option_s = { .value = 0x78A00800u },
        .wd_wn_wm_option_s = { .value = 0x78E00800u },
    },
    [AARCH64_OPCODE_LDRSW_off] = {
        .xd_xn_xm_option_s = { .value = 0xB8A00800u },
    },
    [AARCH64_OPCODE_PRFM_off] = {
        .xd_xn_xm_option_s = { .value = 0xF8A00800u },
    },
    [AARCH64_OPCODE_STRB] = {
        .xd_xn_imm12 = { .value = 0x39000000u },
    },
    [AARCH64_OPCODE_LDRB] = {
        .xd_xn_imm12 = { .value = 0x39400000u },
    },
    [AARCH64_OPCODE_LDRSB] = {
        .xd_xn_imm12 = { .value = 0x39800000u },
        .wd_wn_imm12 = { .value = 0x39C00000u },
    },
    [AARCH64_OPCODE_STR] = {
        .bd_bn_imm12 = { .value = 0x3D000000u },
        .qd_qn_imm12 = { .value = 0x3D800000u },
        .hd_hn_imm12 = { .value = 0x7D000000u },
        .wd_wn_imm12 = { .value = 0xB9000000u },
        .sd_sn_imm12 = { .value = 0xBD000000u },
        .xd_xn_imm12 = { .value = 0xF9000000u },
        .dd_dn_imm12 = { .value = 0xFD000000u },
    },
    [AARCH64_OPCODE_STRH] = {
        .xd_xn_imm12 = { .value = 0x79000000u },
    },
    [AARCH64_OPCODE_LDRH] = {
        .xd_xn_imm12 = { .value = 0x79400000u },
    },
    [AARCH64_OPCODE_LDRSH] = {
        .xd_xn_imm12 = { .value = 0x79800000u },
        .wd_wn_imm12 = { .value = 0x79C00000u },
    },
    [AARCH64_OPCODE_ADR] = {
        .xd_immhi_immlo = { .value = 0x10000000u },
    },
    [AARCH64_OPCODE_ADRP] = {
        .xd_immhi_immlo = { .value = 0x90000000u },
    },
    [AARCH64_OPCODE_ADD] = {
        .wd_wn_imm12_shift = { .value = 0x11000000u },
        .xd_xn_imm12_shift = { .value = 0x91000000u },
        .wd_wn_wm_imm6_shift = { .value = 0xB000000u },
        .xd_xn_xm_imm6_shift = { .value = 0x8B000000u },
        .wd_wn_wm_imm3_option = { .value = 0xB200000u },
        .xd_xn_xm_imm3_option = { .value = 0x8B200000u },
    },
    [AARCH64_OPCODE_ADDS] = {
        .wd_wn_imm12_shift = { .value = 0x31000000u },
        .xd_xn_imm12_shift = { .value = 0xB1000000u },
        .wd_wn_wm_imm6_shift = { .value = 0x2B000000u },
        .xd_xn_xm_imm6_shift = { .value = 0xAB000000u },
        .wd_wn_wm_imm3_option = { .value = 0x2B200000u },
        .xd_xn_xm_imm3_option = { .value = 0xAB200000u },
    },
    [AARCH64_OPCODE_SUB] = {
        .wd_wn_imm12_shift = { .value = 0x51000000u },
        .xd_xn_imm12_shift = { .value = 0xD1000000u },
        .wd_wn_wm_imm6_shift = { .value = 0x4B000000u },
        .xd_xn_xm_imm6_shift = { .value = 0xCB000000u },
        .wd_wn_wm_imm3_option = { .value = 0x4B200000u },
        .xd_xn_xm_imm3_option = { .value = 0xCB200000u },
    },
    [AARCH64_OPCODE_SUBS] = {
        .wd_wn_imm12_shift = { .value = 0x71000000u },
        .xd_xn_imm12_shift = { .value = 0xF1000000u },
        .wd_wn_wm_imm6_shift = { .value = 0x6B000000u },
        .xd_xn_xm_imm6_shift = { .value = 0xEB000000u },
        .wd_wn_wm_imm3_option = { .value = 0x6B200000u },
        .xd_xn_xm_imm3_option = { .value = 0xEB200000u },
    },
    [AARCH64_OPCODE_AND] = {
        .wd_wn_imms_immr = { .value = 0x12000000u },
        .xd_xn_imms_immr_n = { .value = 0x92000000u },
        .wd_wn_wm_imm6_shift = { .value = 0xA000000u },
        .xd_xn_xm_imm6_shift = { .value = 0x8A000000u },
    },
    [AARCH64_OPCODE_MOV] = {
        .wd_wn_imms_immr = { .value = 0x32000000u },
        .xd_xn_imms_immr_n = { .value = 0xB2000000u },
        .wd_wn_wm_imm6_shift = { .value = 0x2A000000u },
        .xd_xn_xm_imm6_shift = { .value = 0xAA000000u },
    },
    [AARCH64_OPCODE_ORR] = {
        .wd_wn_imms_immr = { .value = 0x32000000u },
        .xd_xn_imms_immr_n = { .value = 0xB2000000u },
        .wd_wn_wm_imm6_shift = { .value = 0x2A000000u },
        .xd_xn_xm_imm6_shift = { .value = 0xAA000000u },
    },
    [AARCH64_OPCODE_EOR] = {
        .wd_wn_imms_immr = { .value = 0x52000000u },
        .xd_xn_imms_immr_n = { .value = 0xD2000000u },
        .wd_wn_wm_imm6_shift = { .value = 0x4A000000u },
        .xd_xn_xm_imm6_shift = { .value = 0xCA000000u },
    },
    [AARCH64_OPCODE_ANDS] = {
        .wd_wn_imms_immr = { .value = 0x72000000u },
        .xd_xn_imms_immr_n = { .value = 0xF2000000u },
        .wd_wn_wm_imm6_shift = { .value = 0x6A000000u },
        .xd_xn_xm_imm6_shift = { .value = 0xEA000000u },
    },
    [AARCH64_OPCODE_MOVN] = {
        .wd_imm16_hw = { .value = 0x12800000u },
        .xd_imm16_hw = { .value = 0x92800000u },
    },
    [AARCH64_OPCODE_MOVZ] = {
        .wd_imm16_hw = { .value = 0x52800000u },
        .xd_imm16_hw = { .value = 0xD2800000u },
    },
    [AARCH64_OPCODE_MOVK] = {
        .wd_imm16_hw = { .value = 0x72800000u },
        .xd_imm16_hw = { .value = 0xF2800000u },
    },
    [AARCH64_OPCODE_SBFM] = {
        .wd_wn_imms_immr = { .value = 0x13000000u },
        .xd_xn_imms_immr = { .value = 0x93400000u },
    },
    [AARCH64_OPCODE_BFM] = {
        .wd_wn_imms_immr = { .value = 0x33000000u },
        .xd_xn_imms_immr = { .value = 0xB3400000u },
    },
    [AARCH64_OPCODE_UBFM] = {
        .wd_wn_imms_immr = { .value = 0x53000000u },
        .xd_xn_imms_immr = { .value = 0xD3400000u },
    },
    [AARCH64_OPCODE_EXTR] = {
        .wd_wn_wm_imms = { .value = 0x13800000u },
        .xd_xn_xm_imms = { .value = 0x93C08000u },
    },
    [AARCH64_OPCODE_BIC] = {
        .wd_wn_wm_imm6_shift = { .value = 0xA200000u },
        .xd_xn_xm_imm6_shift = { .value = 0x8A200000u },
    },
    [AARCH64_OPCODE_ORN] = {
        .wd_wn_wm_imm6_shift = { .value = 0x2A200000u },
        .xd_xn_xm_imm6_shift = { .value = 0xAA200000u },
    },
    [AARCH64_OPCODE_EON] = {
        .wd_wn_wm_imm6_shift = { .value = 0x4A200000u },
        .xd_xn_xm_imm6_shift = { .value = 0xCA200000u },
    },
    [AARCH64_OPCODE_BICS] = {
        .wd_wn_wm_imm6_shift = { .value = 0x6A200000u },
        .xd_xn_xm_imm6_shift = { .value = 0xEA200000u },
    },
    [AARCH64_OPCODE_ADC] = {
        .wd_wn_wm = { .value = 0x1A000000u },
        .xd_xn_xm = { .value = 0x9A000000u },
    },
    [AARCH64_OPCODE_ADCS] = {
        .wd_wn_wm = { .value = 0x3A000000u },
        .xd_xn_xm = { .value = 0xBA000000u },
    },
    [AARCH64_OPCODE_SBC] = {
        .wd_wn_wm = { .value = 0x5A000000u },
        .xd_xn_xm = { .value = 0xDA000000u },
    },
    [AARCH64_OPCODE_SBCS] = {
        .wd_wn_wm = { .value = 0x7A000000u },
        .xd_xn_xm = { .value = 0xFA000000u },
    },
    [AARCH64_OPCODE_CCMN] = {
        .wn_nzcv_imm5_cond = { .value = 0x3A400000u },
        .xn_nzcv_imm5_cond = { .value = 0xBA400000u },
    },
    [AARCH64_OPCODE_CCMP] = {
        .wn_nzcv_imm5_cond = { .value = 0x7A400000u },
        .xn_nzcv_imm5_cond = { .value = 0xFA400000u },
    },
    [AARCH64_OPCODE_CSEL] = {
        .wd_wn_wm_cond = { .value = 0x1A800000u },
        .xd_xn_xm_cond = { .value = 0x9A800000u },
    },
    [AARCH64_OPCODE_CSINC] = {
        .wd_wn_wm_cond = { .value = 0x1A800400u },
        .xd_xn_xm_cond = { .value = 0x9A800400u },
    },
    [AARCH64_OPCODE_CSINV] = {
        .wd_wn_wm_cond = { .value = 0x5A800000u },
        .xd_xn_xm_cond = { .value = 0xDA800000u },
    },
    [AARCH64_OPCODE_CSNEG] = {
        .wd_wn_wm_cond = { .value = 0x5A800400u },
        .xd_xn_xm_cond = { .value = 0xDA800400u },
    },
    [AARCH64_OPCODE_MADD] = {
        .wd_wn_wm_wa = { .value = 0x1B000000u },
        .xd_xn_xm_xa = { .value = 0x9B000000u },
    },
    [AARCH64_OPCODE_SMADDL] = {
        .xd_xn_xm_xa = { .value = 0x9B200000u },
    },
    [AARCH64_OPCODE_UMADDL] = {
        .xd_xn_xm_xa = { .value = 0x9BA00000u },
    },
    [AARCH64_OPCODE_MSUB] = {
        .wd_wn_wm_wa = { .value = 0x1B008000u },
        .xd_xn_xm_xa = { .value = 0x9B008000u },
    },
    [AARCH64_OPCODE_SMSUBL] = {
        .xd_xn_xm_xa = { .value = 0x9B208000u },
    },
    [AARCH64_OPCODE_UMSUBL] = {
        .xd_xn_xm_xa = { .value = 0x9BA08000u },
    },
    [AARCH64_OPCODE_SMULH] = {
        .xd_xn_xm_xa = { .value = 0x9B400000u },
    },
    [AARCH64_OPCODE_UMULH] = {
        .xd_xn_xm_xa = { .value = 0x9BC00000u },
    },
    [AARCH64_OPCODE_CRC32X] = {
        .xd_xn_xm = { .value = 0x9AC04C00u },
    },
    [AARCH64_OPCODE_CRC32CX] = {
        .xd_xn_xm = { .value = 0x9AC05C00u },
    },
    [AARCH64_OPCODE_CRC32B] = {
        .xd_xn_xm = { .value = 0x1AC04000u },
    },
    [AARCH64_OPCODE_CRC32CB] = {
        .xd_xn_xm = { .value = 0x1AC05000u },
    },
    [AARCH64_OPCODE_CRC32H] = {
        .xd_xn_xm = { .value = 0x1AC04400u },
    },
    [AARCH64_OPCODE_CRC32CH] = {
        .xd_xn_xm = { .value = 0x1AC05400u },
    },
    [AARCH64_OPCODE_CRC32W] = {
        .xd_xn_xm = { .value = 0x1AC04800u },
    },
    [AARCH64_OPCODE_CRC32CW] = {
        .xd_xn_xm = { .value = 0x1AC05800u },
    },
    [AARCH64_OPCODE_UDIV] = {
        .wd_wn_wm = { .value = 0x1AC00800u },
        .xd_xn_xm = { .value = 0x9AC00800u },
    },
    [AARCH64_OPCODE_SDIV] = {
        .wd_wn_wm = { .value = 0x1AC00C00u },
        .xd_xn_xm = { .value = 0x9AC00C00u },
    },
    [AARCH64_OPCODE_LSLV] = {
        .wd_wn_wm = { .value = 0x1AC02000u },
        .xd_xn_xm = { .value = 0x9AC02000u },
    },
    [AARCH64_OPCODE_LSRV] = {
        .wd_wn_wm = { .value = 0x1AC02400u },
        .xd_xn_xm = { .value = 0x9AC02400u },
    },
    [AARCH64_OPCODE_ASRV] = {
        .wd_wn_wm = { .value = 0x1AC02800u },
        .xd_xn_xm = { .value = 0x9AC02800u },
    },
    [AARCH64_OPCODE_RORV] = {
        .wd_wn_wm = { .value = 0x1AC02C00u },
        .xd_xn_xm = { .value = 0x9AC02C00u },
    },
    [AARCH64_OPCODE_RBIT] = {
        .wd_wn = { .value = 0x5AC00000u },
        .xd_xn = { .value = 0xDAC00000u },
    },
    [AARCH64_OPCODE_CLZ] = {
        .wd_wn = { .value = 0x5AC01000u },
        .xd_xn = { .value = 0xDAC01000u },
    },
    [AARCH64_OPCODE_CLS] = {
        .wd_wn = { .value = 0x5AC01400u },
        .xd_xn = { .value = 0xDAC01400u },
    },
    [AARCH64_OPCODE_REV] = {
        .wd_wn = { .value = 0x5AC00800u },
        .xd_xn = { .value = 0xDAC00C00u },
    },
    [AARCH64_OPCODE_REV16] = {
        .wd_wn = { .value = 0xDAC00400u },
        .xd_xn = { .value = 0x5AC00400u },
    },
    [AARCH64_OPCODE_REV32] = {
        .xd_xn = { .value = 0xDAC00800u },
    },
};


const long long int aarch64_instructions_table_size = sizeof(aarch64_instructions_table);


AArch64_Variant_Kind oc_aarch64_get_inverse_compare(unsigned int /* OPCODE */ opcode) {
    (void)opcode;

    return 0;
}

void oc_aarch64_write_nop(OC_Machine_Code_Writer* b, unsigned char byte_count) {
    assert(byte_count == 4);
    // b->append_u32();
    OC_AARCH64_WRITE_INSTRUCTION(b, AARCH64_OPCODE_MOV, xd_xn_xm_shift, ((AArch64_Instruction_Parameters) { 0 }));
}

void oc_aarch64_write_instruction(OC_Machine_Code_Writer* b, AArch64_Variant_Kind variant, AArch64_Instruction_Variant instruction, AArch64_Instruction_Parameters parameters) {
    unsigned int value = instruction.value | parameters.word;

    unsigned int imm_offset = 0;
    unsigned int imm_size = 0;

    unsigned int cond_offset = 0;
    unsigned int shift_offset = 22;
    unsigned int shift_mask = 0b11;

    switch (variant) {
    case AARCH64_VARIANT_KIND_wd_wn_wm_option_s:
    case AARCH64_VARIANT_KIND_xd_xn_xm_option_s:
    case AARCH64_VARIANT_KIND_bd_bn_bm_option_s:
    case AARCH64_VARIANT_KIND_hd_hn_hm_option_s:
    case AARCH64_VARIANT_KIND_dd_dn_dm_option_s:
    case AARCH64_VARIANT_KIND_qd_qn_qm_option_s:
    case AARCH64_VARIANT_KIND_sd_sn_sm_option_s:
        shift_mask = 0b1u;
        shift_offset = 22u;
        // fallthrough

    case AARCH64_VARIANT_KIND_wd_wn_wm_imm3_option:
    case AARCH64_VARIANT_KIND_xd_xn_xm_imm3_option:
    case AARCH64_VARIANT_KIND_bd_bn_bm_imm3_option:
    case AARCH64_VARIANT_KIND_hd_hn_hm_imm3_option:
    case AARCH64_VARIANT_KIND_dd_dn_dm_imm3_option:
    case AARCH64_VARIANT_KIND_qd_qn_qm_imm3_option:
    case AARCH64_VARIANT_KIND_sd_sn_sm_imm3_option:

    case AARCH64_VARIANT_KIND_wd_wn_wm_option:
    case AARCH64_VARIANT_KIND_xd_xn_xm_option:
    case AARCH64_VARIANT_KIND_bd_bn_bm_option:
    case AARCH64_VARIANT_KIND_hd_hn_hm_option:
    case AARCH64_VARIANT_KIND_dd_dn_dm_option:
    case AARCH64_VARIANT_KIND_qd_qn_qm_option:
    case AARCH64_VARIANT_KIND_sd_sn_sm_option:
        if (!parameters.option) value |= (0b110u << 13);
        else value |= (parameters.option << 13);
        break;
    default: break;
    }

    switch (variant) {
    case AARCH64_VARIANT_KIND_wd_immhi_immlo:
    case AARCH64_VARIANT_KIND_xd_immhi_immlo:
    case AARCH64_VARIANT_KIND_bd_immhi_immlo:
    case AARCH64_VARIANT_KIND_hd_immhi_immlo:
    case AARCH64_VARIANT_KIND_dd_immhi_immlo:
    case AARCH64_VARIANT_KIND_qd_immhi_immlo:
    case AARCH64_VARIANT_KIND_sd_immhi_immlo:
        value = (parameters.immediate & 0b11u) << 29u;
        value = ((parameters.immediate >> 2u) & 0b1111111111111111111u) << 5u;
        break;

    case AARCH64_VARIANT_KIND_wd_wn_imms_immr_n:
    case AARCH64_VARIANT_KIND_xd_xn_imms_immr_n:
    case AARCH64_VARIANT_KIND_bd_bn_imms_immr_n:
    case AARCH64_VARIANT_KIND_hd_hn_imms_immr_n:
    case AARCH64_VARIANT_KIND_dd_dn_imms_immr_n:
    case AARCH64_VARIANT_KIND_qd_qn_imms_immr_n:
    case AARCH64_VARIANT_KIND_sd_sn_imms_immr_n:
        value = (parameters.immediate & 0b111111u) << 10u;
        value = ((parameters.immediate >> 6u) & 0b111111u) << 16u;
        value = ((parameters.immediate >> 12u) & 0b1u) << 22u;
        break;

    case AARCH64_VARIANT_KIND_wd_wn_imms_immr:
    case AARCH64_VARIANT_KIND_xd_xn_imms_immr:
    case AARCH64_VARIANT_KIND_bd_bn_imms_immr:
    case AARCH64_VARIANT_KIND_hd_hn_imms_immr:
    case AARCH64_VARIANT_KIND_dd_dn_imms_immr:
    case AARCH64_VARIANT_KIND_qd_qn_imms_immr:
    case AARCH64_VARIANT_KIND_sd_sn_imms_immr:
        value = (parameters.immediate & 0b111111u) << 10u;
        value = ((parameters.immediate >> 6u) & 0b111111u) << 10u;
        break;

    case AARCH64_VARIANT_KIND_wd_wn_wm_imm3_option:
    case AARCH64_VARIANT_KIND_xd_xn_xm_imm3_option:
    case AARCH64_VARIANT_KIND_bd_bn_bm_imm3_option:
    case AARCH64_VARIANT_KIND_hd_hn_hm_imm3_option:
    case AARCH64_VARIANT_KIND_dd_dn_dm_imm3_option:
    case AARCH64_VARIANT_KIND_qd_qn_qm_imm3_option:
    case AARCH64_VARIANT_KIND_sd_sn_sm_imm3_option:
		imm_offset = 10;
		imm_size = 3;
        break;
        
    case AARCH64_VARIANT_KIND_wn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_xn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_bn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_hn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_dn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_qn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_sn_nzcv_imm5_cond:
		imm_offset = 16;
		imm_size = 5;
		break;

    case AARCH64_VARIANT_KIND_wd_wn_wm_imm6_shift:
    case AARCH64_VARIANT_KIND_xd_xn_xm_imm6_shift:
    case AARCH64_VARIANT_KIND_bd_bn_bm_imm6_shift:
    case AARCH64_VARIANT_KIND_hd_hn_hm_imm6_shift:
    case AARCH64_VARIANT_KIND_dd_dn_dm_imm6_shift:
    case AARCH64_VARIANT_KIND_qd_qn_qm_imm6_shift:
    case AARCH64_VARIANT_KIND_sd_sn_sm_imm6_shift:
		imm_offset = 16;
		imm_size = 6;
		break;

    case AARCH64_VARIANT_KIND_wd_wd2_wn_imm7:
    case AARCH64_VARIANT_KIND_xd_xd2_xn_imm7:
    case AARCH64_VARIANT_KIND_bd_bd2_bn_imm7:
    case AARCH64_VARIANT_KIND_hd_hd2_hn_imm7:
    case AARCH64_VARIANT_KIND_dd_dd2_dn_imm7:
    case AARCH64_VARIANT_KIND_qd_qd2_qn_imm7:
    case AARCH64_VARIANT_KIND_sd_sd2_sn_imm7:
		imm_offset = 15;
		imm_size = 7;
		break;

    case AARCH64_VARIANT_KIND_wd_wn_imm9:
    case AARCH64_VARIANT_KIND_xd_xn_imm9:
    case AARCH64_VARIANT_KIND_bd_bn_imm9:
    case AARCH64_VARIANT_KIND_hd_hn_imm9:
    case AARCH64_VARIANT_KIND_dd_dn_imm9:
    case AARCH64_VARIANT_KIND_qd_qn_imm9:
    case AARCH64_VARIANT_KIND_sd_sn_imm9:
		imm_offset = 12;
		imm_size = 9;
		break;

    case AARCH64_VARIANT_KIND_wd_wn_imm12_shift:
    case AARCH64_VARIANT_KIND_wd_wn_imm12:
    case AARCH64_VARIANT_KIND_xd_xn_imm12_shift:
    case AARCH64_VARIANT_KIND_xd_xn_imm12:
    case AARCH64_VARIANT_KIND_bd_bn_imm12_shift:
    case AARCH64_VARIANT_KIND_bd_bn_imm12:
    case AARCH64_VARIANT_KIND_hd_hn_imm12_shift:
    case AARCH64_VARIANT_KIND_hd_hn_imm12:
    case AARCH64_VARIANT_KIND_dd_dn_imm12_shift:
    case AARCH64_VARIANT_KIND_dd_dn_imm12:
    case AARCH64_VARIANT_KIND_qd_qn_imm12_shift:
    case AARCH64_VARIANT_KIND_qd_qn_imm12:
    case AARCH64_VARIANT_KIND_sd_sn_imm12_shift:
    case AARCH64_VARIANT_KIND_sd_sn_imm12:
		imm_offset = 10;
		imm_size = 12;
		break;

    case AARCH64_VARIANT_KIND_xd_imm14_b40_b5:
    case AARCH64_VARIANT_KIND_wd_imm14_b40:
    case AARCH64_VARIANT_KIND_xd_imm14_b40:
    case AARCH64_VARIANT_KIND_bd_imm14_b40:
    case AARCH64_VARIANT_KIND_hd_imm14_b40:
    case AARCH64_VARIANT_KIND_dd_imm14_b40:
    case AARCH64_VARIANT_KIND_qd_imm14_b40:
    case AARCH64_VARIANT_KIND_sd_imm14_b40:
		imm_offset = 5;
		imm_size = 14;
		break;

    case AARCH64_VARIANT_KIND_imm16:
    case AARCH64_VARIANT_KIND_wd_imm16:
    case AARCH64_VARIANT_KIND_xd_imm16:
    case AARCH64_VARIANT_KIND_bd_imm16:
    case AARCH64_VARIANT_KIND_hd_imm16:
    case AARCH64_VARIANT_KIND_dd_imm16:
    case AARCH64_VARIANT_KIND_qd_imm16:
    case AARCH64_VARIANT_KIND_sd_imm16:
		imm_offset = 5;
		imm_size = 16;
		break;


    case AARCH64_VARIANT_KIND_imm19_cond:
    case AARCH64_VARIANT_KIND_wd_imm19:
    case AARCH64_VARIANT_KIND_xd_imm19:
    case AARCH64_VARIANT_KIND_bd_imm19:
    case AARCH64_VARIANT_KIND_hd_imm19:
    case AARCH64_VARIANT_KIND_dd_imm19:
    case AARCH64_VARIANT_KIND_qd_imm19:
    case AARCH64_VARIANT_KIND_sd_imm19:
		imm_offset = 5;
		imm_size = 19;
		break;

    case AARCH64_VARIANT_KIND_imm26:
		imm_offset = 0;
		imm_size = 26;
		break;

    default: break;
    }
	
    switch (variant) {
    case AARCH64_VARIANT_KIND_wn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_xn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_bn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_hn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_dn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_qn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_sn_nzcv_imm5_cond:
    case AARCH64_VARIANT_KIND_wd_wn_wm_cond:
    case AARCH64_VARIANT_KIND_xd_xn_xm_cond:
    case AARCH64_VARIANT_KIND_bd_bn_bm_cond:
    case AARCH64_VARIANT_KIND_hd_hn_hm_cond:
    case AARCH64_VARIANT_KIND_dd_dn_dm_cond:
    case AARCH64_VARIANT_KIND_qd_qn_qm_cond:
    case AARCH64_VARIANT_KIND_sd_sn_sm_cond:
		cond_offset = 12;
		break;

    case AARCH64_VARIANT_KIND_imm19_cond:
		cond_offset = 0;
		break;

    default: break;
    }

    value |= (parameters.shift & shift_mask) << shift_offset;
    value |= ((((uint32_t)-1) >> (32 - imm_size)) & parameters.immediate) << imm_offset;
    value |= (parameters.cond << cond_offset);

    b->append_u32(b, value);
}



#endif // OC_MACHINE_CODE_IMPLEMENTATION
