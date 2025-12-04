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
        OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_rbp,
            .reg1 = X86_64_OPERAND_REGISTER_rax,
        }));

    mov rdx, 0x78
        OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r16_i16, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_rdx,
            .immediate = 0x78
        }));

    mov BYTE [rbp + 0x10], al
        OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm8_r8, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp,
            .reg1 = X86_64_OPERAND_REGISTER_rax,
            .displacement = 0x10
        }));

    mov ecx, DWORD [RBP + 0x10]
        OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_rcx,
            .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp,
            .displacement = 0x10, .immediate = 0x78
        }));


    mov eax, DWORD [rdi + rcx*2 + 0x07]
                    ^ reg1 |  |     |
                    base ^ index  |
                            ^ scale
                                    ^ displacement
        OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) {
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
        OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVSS, r128_rm128, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_xmm(0),
            .reg1 = X86_64_OPERAND_REGISTER_xmm(1)
        }));


    vcvtsi2sd xmm1, xmm2, rdx
        OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VCVTSI2SD, r128_vvvv_rm64, ((X86_64_Instruction_Parameters) {
            .reg0 = X86_64_OPERAND_REGISTER_xmm(1),
            .reg1 = X86_64_OPERAND_REGISTER_xmm(2),
            .reg2 = X86_64_OPERAND_REGISTER_rdx
        }));

No arguments:

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CPUID, noarg, ((X86_64_Instruction_Parameters) {}));
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CLI  , noarg, ((X86_64_Instruction_Parameters) {}));
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_INT3 , noarg, ((X86_64_Instruction_Parameters) {}));
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
    OPCODE_CPUID,
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
    OPCODE_JO,
    OPCODE_JNO,
    OPCODE_JA,
    OPCODE_JAE,
    OPCODE_JB,
    OPCODE_JBE,
    OPCODE_JC,
    OPCODE_JCXZ,
    OPCODE_JECXZ,
    OPCODE_JRCXZ,
    OPCODE_JE,
    OPCODE_JNE,
    OPCODE_JS,
    OPCODE_JNS,
    OPCODE_JPE,
    OPCODE_JPO,
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
    OPCODE_MOVSX,
    OPCODE_MOVSXD,
    OPCODE_MOVZX,

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

    OPCODE_SETO,
    OPCODE_SETNO,
    OPCODE_SETB,
    OPCODE_SETAE,
    OPCODE_SETE,
    OPCODE_SETNE,
    OPCODE_SETBE,
    OPCODE_SETA,
    OPCODE_SETS,
    OPCODE_SETNS,
    OPCODE_SETPE,
    OPCODE_SETPO,
    OPCODE_SETL,
    OPCODE_SETGE,
    OPCODE_SETLE,
    OPCODE_SETG,

    OPCODE_TEST,
    OPCODE_XCHG,
    OPCODE_XLAT,
    OPCODE_XOR,



    OPCODE_ADDPS,
    OPCODE_ADDPD,
    OPCODE_ADDSS,
    OPCODE_ADDSD,
    OPCODE_MULPS,
    OPCODE_MULPD,
    OPCODE_MULSS,
    OPCODE_MULSD,
    OPCODE_SUBPS,
    OPCODE_SUBPD,
    OPCODE_SUBSS,
    OPCODE_SUBSD,
    OPCODE_MINPS,
    OPCODE_MINPD,
    OPCODE_MINSS,
    OPCODE_MINSD,
    OPCODE_DIVPS,
    OPCODE_DIVPD,
    OPCODE_DIVSS,
    OPCODE_DIVSD,
    OPCODE_MAXPS,
    OPCODE_MAXPD,
    OPCODE_MAXSS,
    OPCODE_MAXSD,
    OPCODE_SQRTPS,
    OPCODE_SQRTPD,
    OPCODE_SQRTSS,
    OPCODE_SQRTSD,
    OPCODE_RSQRTPS,
    OPCODE_RSQRTSS,
    OPCODE_RCPPS,
    OPCODE_RCPSS,

    OPCODE_ANDPS,
    OPCODE_ANDPD,
    OPCODE_ANDNPS,
    OPCODE_ANDNPD,
    OPCODE_ORPS,
    OPCODE_ORPD,
    OPCODE_XORPS,
    OPCODE_XORPD,

    OPCODE_PUNPCKLBW,
    OPCODE_PUNPCKLWD,
    OPCODE_PUNPCKLDQ,
    OPCODE_PUNPCKLQDQ,
    OPCODE_PACKSSWB,
    OPCODE_PCMPGTB,
    OPCODE_PCMPGTW,
    OPCODE_PCMPGTD,
    OPCODE_PACKUSWB,
    OPCODE_PUNPCKHBW,
    OPCODE_PUNPCKHWD,
    OPCODE_PUNPCKHDQ,
    OPCODE_PUNPCKHQDQ,
    OPCODE_PACKSSDW,

    OPCODE_PSHUFB,
    OPCODE_PSHUFW,
    OPCODE_PSHUFD,
    OPCODE_PSHUFHW,
    OPCODE_PSHUFLW,

    OPCODE_PSRLW,
    OPCODE_PSRLD,
    OPCODE_PSRLQ,
    OPCODE_PADDQ,
    OPCODE_PMULLW,
    OPCODE_PAVGB,
    OPCODE_PSRAW,
    OPCODE_PSRAD,
    OPCODE_PAVGW,
    OPCODE_PMULHUW,
    OPCODE_PMULHW,
    OPCODE_PSLLW,
    OPCODE_PSLLD,
    OPCODE_PSLLQ,
    OPCODE_PMULUDQ,
    OPCODE_PMADDWD,
    OPCODE_PSADBW,
    OPCODE_PSUBUSB,
    OPCODE_PSUBUSW,
    OPCODE_PMINUB,
    OPCODE_PAND,
    OPCODE_PADDUSB,
    OPCODE_PADDUSW,
    OPCODE_PMAXUB,
    OPCODE_PANDN,
    OPCODE_PSUBSB,
    OPCODE_PSUBSW,
    OPCODE_PMINSW,
    OPCODE_POR,
    OPCODE_PADDSB,
    OPCODE_PADDSW,
    OPCODE_PMAXSW,
    OPCODE_PXOR,
    OPCODE_PSUBB,
    OPCODE_PSUBW,
    OPCODE_PSUBD,
    OPCODE_PSUBQ,
    OPCODE_PADDB,
    OPCODE_PADDW,
    OPCODE_PADDD,

    OPCODE_CMPEQB,
    OPCODE_CMPEQW,
    OPCODE_CMPEQD,
    OPCODE_COMISS,
    OPCODE_COMISD,

    OPCODE_CVTDQ2PD,
    OPCODE_CVTDQ2PS,
    OPCODE_CVTPD2DQ,
    OPCODE_CVTPD2PI,
    OPCODE_CVTPD2PS,
    OPCODE_CVTPI2PD,
    OPCODE_CVTPI2PS,
    OPCODE_CVTPS2DQ,
    OPCODE_CVTPS2PD,
    OPCODE_CVTPS2PI,
    OPCODE_CVTSD2SI,
    OPCODE_CVTSD2SS,
    OPCODE_CVTSI2SD,
    OPCODE_CVTSI2SS,
    OPCODE_CVTSS2SD,
    OPCODE_CVTSS2SI,
    OPCODE_CVTTPD2DQ,
    OPCODE_CVTTPD2PI,
    OPCODE_CVTTPS2DQ,
    OPCODE_CVTTPS2PI,
    OPCODE_CVTTSD2SI,
    OPCODE_CVTTSS2SI,

    OPCODE_EMMS,
    OPCODE_HADDPD,
    OPCODE_HADDPS,
    OPCODE_HSUBPD,
    OPCODE_HSUBPS,

    OPCODE_MOVAPS,
    OPCODE_MOVAPD,
    OPCODE_MOVLPS,
    OPCODE_MOVHPS,
    OPCODE_MOVHPD,
    OPCODE_MOVHLPS,
    OPCODE_MOVLHPS,
    OPCODE_MOVLPD,
    OPCODE_MOVMSKPS,
    OPCODE_MOVMSKPD,
    OPCODE_MOVNTPS,
    OPCODE_MOVNTPD,
    OPCODE_MOVUPS,
    OPCODE_MOVUPD,
    OPCODE_MOVSS,
    /* OPCODE_MOVSD, */
    OPCODE_MOVSLDUP,
    OPCODE_MOVSHDUP,
    OPCODE_MOVDDUP,
    OPCODE_MOVD,
    OPCODE_MOVQ,
    OPCODE_MOVDQA,
    OPCODE_MOVDQU,
    OPCODE_UCOMISS,
    OPCODE_UCOMISD,
    OPCODE_UNPCKLPS,
    OPCODE_UNPCKHPS,

    OPCODE_VADDPS,
    OPCODE_VADDPD,
    OPCODE_VADDSS,
    OPCODE_VADDSD,
    OPCODE_VMULPS,
    OPCODE_VMULPD,
    OPCODE_VMULSS,
    OPCODE_VMULSD,
    OPCODE_VSUBPS,
    OPCODE_VSUBPD,
    OPCODE_VSUBSS,
    OPCODE_VSUBSD,
    OPCODE_VMINPS,
    OPCODE_VMINPD,
    OPCODE_VMINSS,
    OPCODE_VMINSD,
    OPCODE_VDIVPS,
    OPCODE_VDIVPD,
    OPCODE_VDIVSS,
    OPCODE_VDIVSD,
    OPCODE_VMAXPS,
    OPCODE_VMAXPD,
    OPCODE_VMAXSS,
    OPCODE_VMAXSD,
    OPCODE_VSQRTPS,
    OPCODE_VSQRTPD,
    OPCODE_VSQRTSS,
    OPCODE_VSQRTSD,
    OPCODE_VRSQRTPS,
    OPCODE_VRSQRTSS,
    OPCODE_VRCPPS,
    OPCODE_VRCPSS,

    OPCODE_VANDPS,
    OPCODE_VANDPD,
    OPCODE_VANDNPS,
    OPCODE_VANDNPD,
    OPCODE_VORPS,
    OPCODE_VORPD,
    OPCODE_VXORPS,
    OPCODE_VXORPD,

    OPCODE_VHADDPD,
    OPCODE_VHADDPS,
    OPCODE_VHSUBPD,
    OPCODE_VHSUBPS,

    OPCODE_VPUNPCKLBW,
    OPCODE_VPUNPCKLWD,
    OPCODE_VPUNPCKLDQ,
    OPCODE_VPUNPCKLQDQ,
    OPCODE_VPACKSSWB,
    OPCODE_VPCMPGTB,
    OPCODE_VPCMPGTW,
    OPCODE_VPCMPGTD,
    OPCODE_VPACKUSWB,
    OPCODE_VPUNPCKHBW,
    OPCODE_VPUNPCKHWD,
    OPCODE_VPUNPCKHDQ,
    OPCODE_VPUNPCKHQDQ,
    OPCODE_VPACKSSDW,
    OPCODE_VPSHUFD,
    OPCODE_VPSHUFHW,
    OPCODE_VPSHUFLW,

    OPCODE_VPSRLW,
    OPCODE_VPSRLD,
    OPCODE_VPSRLQ,
    OPCODE_VPADDQ,
    OPCODE_VPMULLW,
    OPCODE_VPAVGB,
    OPCODE_VPSRAW,
    OPCODE_VPSRAD,
    OPCODE_VPAVGW,
    OPCODE_VPMULHUW,
    OPCODE_VPMULHW,
    OPCODE_VPSLLW,
    OPCODE_VPSLLD,
    OPCODE_VPSLLQ,
    OPCODE_VPMULUDQ,
    OPCODE_VPMADDWD,
    OPCODE_VPSADBW,
    OPCODE_VPSUBUSB,
    OPCODE_VPSUBUSW,
    OPCODE_VPMINUB,
    OPCODE_VPAND,
    OPCODE_VPADDUSB,
    OPCODE_VPADDUSW,
    OPCODE_VPMAXUB,
    OPCODE_VPANDN,
    OPCODE_VPSUBSB,
    OPCODE_VPSUBSW,
    OPCODE_VPMINSW,
    OPCODE_VPOR,
    OPCODE_VPADDSB,
    OPCODE_VPADDSW,
    OPCODE_VPMAXSW,
    OPCODE_VPXOR,
    OPCODE_VPSUBB,
    OPCODE_VPSUBW,
    OPCODE_VPSUBD,
    OPCODE_VPSUBQ,
    OPCODE_VPADDB,
    OPCODE_VPADDW,
    OPCODE_VPADDD,

    OPCODE_VCMPEQB,
    OPCODE_VCMPEQW,
    OPCODE_VCMPEQD,
    OPCODE_VCOMISS,
    OPCODE_VCOMISD,
    OPCODE_VCVTSI2SD,
    OPCODE_VCVTSI2SS,
    OPCODE_VCVTSS2SI,
    OPCODE_VCVTTSS2SI,
    OPCODE_VCVTSD2SI,
    OPCODE_VCVTTSD2SI,
    OPCODE_VCVTPS2PD,
    OPCODE_VCVTPD2PS,
    OPCODE_VCVTSS2SD,
    OPCODE_VCVTSD2SS,
    OPCODE_VCVTDQ2PS,
    OPCODE_VCVTPS2DQ,
    OPCODE_VCVTTPS2DQ,
    OPCODE_VMOVAPS,
    OPCODE_VMOVAPD,
    OPCODE_VMOVLPS,
    OPCODE_VMOVHPS,
    OPCODE_VMOVHPD,
    OPCODE_VMOVHLPS,
    OPCODE_VMOVLHPS,
    OPCODE_VMOVLPD,
    OPCODE_VMOVMSKPS,
    OPCODE_VMOVMSKPD,
    OPCODE_VMOVNTPS,
    OPCODE_VMOVNTPD,
    OPCODE_VMOVUPS,
    OPCODE_VMOVUPD,
    OPCODE_VMOVSS,
    OPCODE_VMOVSD,
    OPCODE_VMOVSLDUP,
    OPCODE_VMOVSHDUP,
    OPCODE_VMOVDDUP,
    OPCODE_VMOVD,
    OPCODE_VMOVQ,
    OPCODE_VMOVDQA,
    OPCODE_VMOVDQU,
    OPCODE_VUCOMISS,
    OPCODE_VUCOMISD,
    OPCODE_VUNPCKLPS,
    OPCODE_VUNPCKHPS,
    OPCODE_VZEROALL,
    OPCODE_VZEROUPPER,
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
    X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_SUB, 0x28u, 5u),
    X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_XOR, 0x30u, 6u),
    X86_64_OPCODE_MAKE_ARITHMETIC(OPCODE_CMP, 0x38u, 7u),
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
        .rm64  		= { .opcode = 0xFFu, .operands = MAKE_OPERANDS(mod_rm(4)) | 0x10 },
        // oc_todo: add far jumps
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
    [OPCODE_MOVSX] = {
        .r16_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBEu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r32_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBEu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBEu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r32_rm16  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBFu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm16  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xBFu), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [OPCODE_MOVSXD] = {
        .r16_rm16  	= { .opcode = 0x63u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm32  	= { .opcode = 0x63u, .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
    },
    [OPCODE_MOVZX] = {
        .r16_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB6u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r32_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB6u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm8  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB6u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r32_rm16  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB7u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
        .r64_rm16  	= { .opcode = BUILD_OPCODE(none, none, 2B, 0xB7u), .operands = MAKE_OPERANDS(mod_reg(0), mod_rm(0)) },
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

    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRLW,		OPCODE_VPSRLW,		0xD1u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRLD,		OPCODE_VPSRLD,		0xD2u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRLQ,		OPCODE_VPSRLQ,		0xD3u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDQ,		OPCODE_VPADDQ,		0xD4u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMULLW,	OPCODE_VPMULLW,		0xD5u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBUSB,	OPCODE_VPSUBUSB,	0xD8u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBUSW,	OPCODE_VPSUBUSW,	0xD9u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMINUB,	OPCODE_VPMINUB,		0xDAu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PAND,		OPCODE_VPAND,		0xDBu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDUSB,	OPCODE_VPADDUSB,	0xDCu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDUSW,	OPCODE_VPADDUSW,	0xDDu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMAXUB,	OPCODE_VPMAXUB,		0xDEu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PANDN,		OPCODE_VPANDN,		0xDFu),

    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PAVGB,		OPCODE_VPAVGB,		0xE0u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRAW,		OPCODE_VPSRAW,		0xE1u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSRAD,		OPCODE_VPSRAD,		0xE2u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PAVGW,		OPCODE_VPAVGW,		0xE3u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMULHUW,	OPCODE_VPMULHUW,	0xE4u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMULHW,	OPCODE_VPMULHW,		0xE5u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBSB,	OPCODE_VPSUBSB,		0xE8u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBSW,	OPCODE_VPSUBSW,		0xE9u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMINSW,	OPCODE_VPMINSW,		0xEAu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_POR,		OPCODE_VPOR,		0xEBu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDSB,	OPCODE_VPADDSB,		0xECu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDSW,	OPCODE_VPADDSW,		0xEDu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMAXSW,	OPCODE_VPMAXSW,		0xEEu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PXOR,		OPCODE_VPXOR,		0xEFu),

    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSLLW,		OPCODE_VPSLLW,		0xF1u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSLLD,		OPCODE_VPSLLD,		0xF2u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSLLQ,		OPCODE_VPSLLQ,		0xF3u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMULUDQ,	OPCODE_VPMULUDQ,	0xF4u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PMADDWD,	OPCODE_VPMADDWD,	0xF5u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSADBW,	OPCODE_VPSADBW,		0xF6u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBB,		OPCODE_VPSUBB,		0xF8u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBW,		OPCODE_VPSUBW,		0xF9u),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBD,		OPCODE_VPSUBD,		0xFAu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PSUBQ,		OPCODE_VPSUBQ,		0xFBu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDB,		OPCODE_VPADDB,		0xFCu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDW,		OPCODE_VPADDW,		0xFDu),
    X86_64_OPCODE_MAKE_VECTOR_PqQq_VxHxWx(OPCODE_PADDD,		OPCODE_VPADDD,		0xFEu),


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

const long long int x86_64_instructions_table_size = sizeof(x86_64_instructions_table);

X86_64_Variant_Kind oc_x86_64_get_inverse_compare(unsigned int /* OPCODE*/ opcode) {
    switch (opcode) {
    case OPCODE_JO: return OPCODE_JNO;
    case OPCODE_JNO: return OPCODE_JO;
    case OPCODE_JA: return OPCODE_JBE;
    case OPCODE_JAE: return OPCODE_JB;
    case OPCODE_JB: return OPCODE_JAE;
    case OPCODE_JBE: return OPCODE_JA;
    case OPCODE_JC: return OPCODE_JAE;
    case OPCODE_JE: return OPCODE_JNE;
    case OPCODE_JNE: return OPCODE_JE;
    case OPCODE_JS: return OPCODE_JNS;
    case OPCODE_JNS: return OPCODE_JS;
    case OPCODE_JPE: return OPCODE_JPO;
    case OPCODE_JPO: return OPCODE_JPE;
    case OPCODE_JG: return OPCODE_JLE;
    case OPCODE_JGE: return OPCODE_JL;
    case OPCODE_JL: return OPCODE_JGE;
    case OPCODE_JLE: return OPCODE_JG;
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
        __asm__("int3");
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

        switch (OPCODE_GET_PREFIX(instruction.opcode)) {
        case OPCODE_PREFIX_none: 
            break;
        case OPCODE_PREFIX_66: 
            X86_64_APPEND_OP_SEGMENT((unsigned char)0x66u);
            break;
        case OPCODE_PREFIX_F3: 
            X86_64_APPEND_OP_SEGMENT((unsigned char)0xF3u);
            break;
        case OPCODE_PREFIX_F2: 
            X86_64_APPEND_OP_SEGMENT((unsigned char)0xF2u);
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

#endif // OC_MACHINE_CODE_IMPLEMENTATION
