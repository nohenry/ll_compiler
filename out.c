# 1 "backends/x86_64_common.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 361 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "backends/x86_64_common.c" 2

# 1 "backends/x86_64_common.h" 1


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
# 51 "backends/x86_64_common.h"
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
 uint32_t operands;
} X86_64_Instruction_Variant;

typedef struct {
 X86_64_Instruction_Variant
  noarg,
  al_i8, ax_i16, eax_i32, rax_i32,
      ax_i8, eax_i8,
         ax_r16, eax_r32, rax_r64,
  al_dx, ax_dx, eax_dx,
  rm8, rm16, rm32, rm64,
  rm8_cl, rm16_cl, rm32_cl, rm64_cl,
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

.al_i8 = |.ax_i16 = |.eax_i32 = |.rax_i32 = |.ax_i8 = |.eax_i8 = |.ax_r16 = |.eax_r32 = |.rax_r64 = |.al_dx = |.ax_dx = |.eax_dx = |.rm8 = |.rm16 = |.rm32 = |.rm64 = |.rm8_cl = |.rm16_cl = |.rm32_cl = |.rm64_cl = |.rm8_r8 = |.rm16_r16 = |.rm32_r32 = |.rm64_r64 = |.r16 = |.r32 = |.r64 = |.r8_rm8 = |.r16_rm16 = |.r32_rm32 = |.r64_rm64 = |.r8_i8 = |.r16_i16 = |.r32_i32 = |.r64_i64 = |.rm8_i8 = |.rm16_i16 = |.rm32_i32 = |.rm64_i32 = |.rm16_i8 = |.rm32_i8 = |.rm64_i8 = |.i8 = |.i16 = |.i32 = |.rel8 = |.rel16 = |.rel32 = |.r16_rm16_i8 = |.r32_rm32_i8 = |.r64_rm64_i8 = |.r16_rm16_i16 = |.r32_rm32_i32 = |.r64_rm64_i32;

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



typedef enum {
 X86_64_PREFIX_REP = 0xF3,
 X86_64_PREFIX_REPE = 0xF3,
 X86_64_PREFIX_REPNE = 0xF2,
} X86_64_Prefixes;




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
# 3 "backends/x86_64_common.c" 2
# 68 "backends/x86_64_common.c"
const X86_64_Instruction x86_64_instructions_table[] = {
 [OPCODE_ADD] = { .al_i8 = { .opcode = 0x04u + 0x00u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .ax_i16 = { .opcode = 0x05u + 0x00u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .eax_i32 = { .opcode = 0x05u + 0x00u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rax_i32 = { .opcode = 0x05u + 0x00u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rm8_i8 = { .opcode = 0x80u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i16 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_r8 = { .opcode = 0x00u + 0x00u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm16_r16 = { .opcode = 0x01u + 0x00u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm32_r32 = { .opcode = 0x01u + 0x00u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm64_r64 = { .opcode = 0x01u + 0x00u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r8_rm8 = { .opcode = 0x02u + 0x00u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r16_rm16 = { .opcode = 0x03u + 0x00u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r32_rm32 = { .opcode = 0x03u + 0x00u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r64_rm64 = { .opcode = 0x03u + 0x00u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, },
 [OPCODE_OR] = { .al_i8 = { .opcode = 0x04u + 0x04u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .ax_i16 = { .opcode = 0x05u + 0x04u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .eax_i32 = { .opcode = 0x05u + 0x04u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rax_i32 = { .opcode = 0x05u + 0x04u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rm8_i8 = { .opcode = 0x80u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i16 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_r8 = { .opcode = 0x00u + 0x04u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm16_r16 = { .opcode = 0x01u + 0x04u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm32_r32 = { .opcode = 0x01u + 0x04u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm64_r64 = { .opcode = 0x01u + 0x04u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r8_rm8 = { .opcode = 0x02u + 0x04u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r16_rm16 = { .opcode = 0x03u + 0x04u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r32_rm32 = { .opcode = 0x03u + 0x04u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r64_rm64 = { .opcode = 0x03u + 0x04u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, },
 [OPCODE_ADC] = { .al_i8 = { .opcode = 0x04u + 0x10u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .ax_i16 = { .opcode = 0x05u + 0x10u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .eax_i32 = { .opcode = 0x05u + 0x10u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rax_i32 = { .opcode = 0x05u + 0x10u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rm8_i8 = { .opcode = 0x80u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i16 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_r8 = { .opcode = 0x00u + 0x10u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm16_r16 = { .opcode = 0x01u + 0x10u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm32_r32 = { .opcode = 0x01u + 0x10u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm64_r64 = { .opcode = 0x01u + 0x10u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r8_rm8 = { .opcode = 0x02u + 0x10u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r16_rm16 = { .opcode = 0x03u + 0x10u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r32_rm32 = { .opcode = 0x03u + 0x10u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r64_rm64 = { .opcode = 0x03u + 0x10u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, },
 [OPCODE_SBB] = { .al_i8 = { .opcode = 0x04u + 0x14u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .ax_i16 = { .opcode = 0x05u + 0x14u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .eax_i32 = { .opcode = 0x05u + 0x14u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rax_i32 = { .opcode = 0x05u + 0x14u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rm8_i8 = { .opcode = 0x80u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i16 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_r8 = { .opcode = 0x00u + 0x14u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm16_r16 = { .opcode = 0x01u + 0x14u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm32_r32 = { .opcode = 0x01u + 0x14u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm64_r64 = { .opcode = 0x01u + 0x14u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r8_rm8 = { .opcode = 0x02u + 0x14u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r16_rm16 = { .opcode = 0x03u + 0x14u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r32_rm32 = { .opcode = 0x03u + 0x14u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r64_rm64 = { .opcode = 0x03u + 0x14u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, },
 [OPCODE_AND] = { .al_i8 = { .opcode = 0x04u + 0x20u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .ax_i16 = { .opcode = 0x05u + 0x20u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .eax_i32 = { .opcode = 0x05u + 0x20u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rax_i32 = { .opcode = 0x05u + 0x20u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rm8_i8 = { .opcode = 0x80u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i16 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_r8 = { .opcode = 0x00u + 0x20u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm16_r16 = { .opcode = 0x01u + 0x20u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm32_r32 = { .opcode = 0x01u + 0x20u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm64_r64 = { .opcode = 0x01u + 0x20u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r8_rm8 = { .opcode = 0x02u + 0x20u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r16_rm16 = { .opcode = 0x03u + 0x20u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r32_rm32 = { .opcode = 0x03u + 0x20u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r64_rm64 = { .opcode = 0x03u + 0x20u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, },
 [OPCODE_SUB] = { .al_i8 = { .opcode = 0x04u + 0x24u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .ax_i16 = { .opcode = 0x05u + 0x24u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .eax_i32 = { .opcode = 0x05u + 0x24u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rax_i32 = { .opcode = 0x05u + 0x24u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rm8_i8 = { .opcode = 0x80u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i16 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_r8 = { .opcode = 0x00u + 0x24u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm16_r16 = { .opcode = 0x01u + 0x24u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm32_r32 = { .opcode = 0x01u + 0x24u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm64_r64 = { .opcode = 0x01u + 0x24u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r8_rm8 = { .opcode = 0x02u + 0x24u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r16_rm16 = { .opcode = 0x03u + 0x24u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r32_rm32 = { .opcode = 0x03u + 0x24u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r64_rm64 = { .opcode = 0x03u + 0x24u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, },
 [OPCODE_XOR] = { .al_i8 = { .opcode = 0x04u + 0x30u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .ax_i16 = { .opcode = 0x05u + 0x30u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .eax_i32 = { .opcode = 0x05u + 0x30u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rax_i32 = { .opcode = 0x05u + 0x30u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rm8_i8 = { .opcode = 0x80u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i16 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_r8 = { .opcode = 0x00u + 0x30u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm16_r16 = { .opcode = 0x01u + 0x30u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm32_r32 = { .opcode = 0x01u + 0x30u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm64_r64 = { .opcode = 0x01u + 0x30u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r8_rm8 = { .opcode = 0x02u + 0x30u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r16_rm16 = { .opcode = 0x03u + 0x30u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r32_rm32 = { .opcode = 0x03u + 0x30u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r64_rm64 = { .opcode = 0x03u + 0x30u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, },
 [OPCODE_CMP] = { .al_i8 = { .opcode = 0x04u + 0x34u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .ax_i16 = { .opcode = 0x05u + 0x34u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .eax_i32 = { .opcode = 0x05u + 0x34u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rax_i32 = { .opcode = 0x05u + 0x34u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rm8_i8 = { .opcode = 0x80u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i16 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i32 = { .opcode = 0x81u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0x83u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7u) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_r8 = { .opcode = 0x00u + 0x34u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm16_r16 = { .opcode = 0x01u + 0x34u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm32_r32 = { .opcode = 0x01u + 0x34u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .rm64_r64 = { .opcode = 0x01u + 0x34u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r8_rm8 = { .opcode = 0x02u + 0x34u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r16_rm16 = { .opcode = 0x03u + 0x34u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r32_rm32 = { .opcode = 0x03u + 0x34u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, .r64_rm64 = { .opcode = 0x03u + 0x34u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) }, },
 [OPCODE_CALL] = {
  .rel32 = { .opcode = 0xE8, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .rm64 = { .opcode = 0xFF, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) | 0x10u },
 },
 [OPCODE_CBW] = { .noarg = { .opcode = 0x98, .operands = 0x20, }, },
 [OPCODE_CWDE] = { .noarg = { .opcode = 0x98 }, },
 [OPCODE_CDQE] = { .noarg = { .opcode = 0x98, .operands = 0x10 }, },
 [OPCODE_CWD] = { .noarg = { .opcode = 0x99, .operands = 0x20, }, },
 [OPCODE_CDQ] = { .noarg = { .opcode = 0x99 }, },
 [OPCODE_CQO] = { .noarg = { .opcode = 0x99, .operands = 0x10 }, },

 [OPCODE_CLC] = { .noarg = { .opcode = 0xF8 } },
 [OPCODE_CLI] = { .noarg = { .opcode = 0xFA } },
 [OPCODE_CLD] = { .noarg = { .opcode = 0xFC } },
 [OPCODE_CMC] = { .noarg = { .opcode = 0xF5u }, },

 [OPCODE_CMOVO] = { .r16_rm16 = { .opcode = 0x0f40, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f40, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f40, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVNO] = { .r16_rm16 = { .opcode = 0x0f41, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f41, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f41, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVB] = { .r16_rm16 = { .opcode = 0x0f42, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f42, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f42, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVAE] = { .r16_rm16 = { .opcode = 0x0f43, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f43, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f43, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVE] = { .r16_rm16 = { .opcode = 0x0f44, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f44, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f44, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVNE] = { .r16_rm16 = { .opcode = 0x0f45, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f45, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f45, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVBE] = { .r16_rm16 = { .opcode = 0x0f46, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f46, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f46, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVA] = { .r16_rm16 = { .opcode = 0x0f47, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f47, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f47, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVS] = { .r16_rm16 = { .opcode = 0x0f48, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f48, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f48, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVNS] = { .r16_rm16 = { .opcode = 0x0f49, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f49, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f49, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVPE] = { .r16_rm16 = { .opcode = 0x0f4A, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f4A, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f4A, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVPO] = { .r16_rm16 = { .opcode = 0x0f4B, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f4B, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f4B, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVL] = { .r16_rm16 = { .opcode = 0x0f4C, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f4C, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f4C, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVGE] = { .r16_rm16 = { .opcode = 0x0f4D, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f4D, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f4D, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVLE] = { .r16_rm16 = { .opcode = 0x0f4E, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f4E, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f4E, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_CMOVG] = { .r16_rm16 = { .opcode = 0x0f4F, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r32_rm32 = { .opcode = 0x0f4F, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .r64_rm64 = { .opcode = 0x0f4F, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },

 [OPCODE_CMPSB] = { .noarg = { .opcode = 0xA6u, .operands = 0x00u }, },
 [OPCODE_CMPSW] = { .noarg = { .opcode = 0xA7u, .operands = 0x20u }, },
 [OPCODE_CMPSD] = { .noarg = { .opcode = 0xA7u, .operands = 0x00u }, },
 [OPCODE_CMPSQ] = { .noarg = { .opcode = 0xA7u, .operands = 0x10u }, },
 [OPCODE_DEC] = {
  .rm8 = { .opcode = 0xFEu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) },
  .rm16 = { .opcode = 0xFFu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) },
  .rm32 = { .opcode = 0xFFu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0xFFu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) },
 },
 [OPCODE_DIV] = {
  .rm8 = { .opcode = 0xF6u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6) & 0xF)) << 24u) },
  .rm16 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6) & 0xF)) << 24u) },
  .rm32 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6) & 0xF)) << 24u) },
 },
 [OPCODE_FWAIT] = { .noarg = { .opcode = 0x9bu } },
 [OPCODE_HLT] = {
  .noarg = { .opcode = 0xF4u },
 },
 [OPCODE_IDIV] = {
  .rm8 = { .opcode = 0xF6u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) },
  .rm16 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) },
  .rm32 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) },
 },
 [OPCODE_IMUL] = {
  .rm8 = { .opcode = 0xF6u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) },
  .rm16 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) },
  .rm32 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) },

  .r16_rm16 = { .opcode = 0x0FAFu, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r32_rm32 = { .opcode = 0x0FAFu, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r64_rm64 = { .opcode = 0x0FAFu, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },

  .r16_rm16_i8 = { .opcode = 0x6Bu, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r32_rm32_i8 = { .opcode = 0x6Bu, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r64_rm64_i8 = { .opcode = 0x6Bu, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },

  .r16_rm16_i16 = { .opcode = 0x69u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r32_rm32_i32 = { .opcode = 0x69u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r64_rm64_i32 = { .opcode = 0x69u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
 },
 [OPCODE_IN] = {
  .al_i8 = { .opcode = 0xE4u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .ax_i8 = { .opcode = 0xE5u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .eax_i8 = { .opcode = 0xE5u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .al_dx = { .opcode = 0xECu, .operands = 0x00u },
  .ax_dx = { .opcode = 0xEDu, .operands = 0x00u },
  .eax_dx = { .opcode = 0xEDu, .operands = 0x00u },
 },
 [OPCODE_INSB] = { .noarg = { .opcode = 0x6Cu, } },
 [OPCODE_INSW] = { .noarg = { .opcode = 0x6Du, .operands = 0x20u } },
 [OPCODE_INSD] = { .noarg = { .opcode = 0x6Du, } },
 [OPCODE_INC] = {
  .rm8 = { .opcode = 0xFEu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) },
  .rm16 = { .opcode = 0xFFu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) },
  .rm32 = { .opcode = 0xFFu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0xFFu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) },
 },

 [OPCODE_INT] = {
  .i8 = { .opcode = 0xCDu, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
 },
 [OPCODE_INT1] = {
  .noarg = { .opcode = 0xF1u, .operands = 0x00u },
 },
 [OPCODE_INT3] = {
  .noarg = { .opcode = 0xCCu, .operands = 0x00u },
 },
 [OPCODE_INTO] = {
  .noarg = { .opcode = 0xF1u, .operands = 0x00u },
 },
 [OPCODE_IRET] = { .noarg = { .opcode = 0xCF, .operands = 0x20 } },
 [OPCODE_IRETD] = { .noarg = { .opcode = 0xCF } },
 [OPCODE_IRETQ] = { .noarg = { .opcode = 0xCF, .operands = 0x10 } },
 [OPCODE_JMP] = {
  .rel8 = { .opcode = 0xEBu, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .rel32 = { .opcode = 0xE9u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .rm64 = { .opcode = 0xFFu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) },

 },
 [OPCODE_JA] = { .rel8 = { .opcode = 0x77, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x77, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x77, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JAE] = { .rel8 = { .opcode = 0x73, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x73, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x73, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JB] = { .rel8 = { .opcode = 0x72, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x72, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x72, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JBE] = { .rel8 = { .opcode = 0x76, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x76, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x76, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JC] = { .rel8 = { .opcode = 0x72, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x72, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x72, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JCXZ] = { .rel8 = { .opcode = 0xE3, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0xE3, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0xE3, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JECXZ] = { .rel8 = { .opcode = 0x67E3, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x67E3, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x67E3, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JRCXZ] = { .rel8 = { .opcode = 0xE3, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0xE3, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0xE3, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JE] = { .rel8 = { .opcode = 0x74, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x74, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x74, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JG] = { .rel8 = { .opcode = 0x7F, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x7F, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x7F, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JGE] = { .rel8 = { .opcode = 0x7D, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x7D, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x7D, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JL] = { .rel8 = { .opcode = 0x7C, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x7C, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x7C, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_JLE] = { .rel8 = { .opcode = 0x7E, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel16 = { .opcode = 0x7E, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, .rel32 = { .opcode = 0x7E, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) }, },
 [OPCODE_LAHF] = { .noarg = { .opcode = 0x9F } },
 [OPCODE_LEA] = {
  .r16_rm16 = { .opcode = 0x8D, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r32_rm32 = { .opcode = 0x8D, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r64_rm64 = { .opcode = 0x8D, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
 },
 [OPCODE_LODSB] = { .noarg = { .opcode = 0xACu } },
 [OPCODE_LODSW] = { .noarg = { .opcode = 0xADu, .operands = 0x20u, } },
 [OPCODE_LODSD] = { .noarg = { .opcode = 0xADu } },
 [OPCODE_LODSQ] = { .noarg = { .opcode = 0xADu, .operands = 0x10u, } },
 [OPCODE_LOOP] = { .rel8 = { .opcode = 0xE2u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) } },
 [OPCODE_LOOPE] = { .rel8 = { .opcode = 0xE1u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) } },
 [OPCODE_LOOPNE] = { .rel8 = { .opcode = 0xE0u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) } },
 [OPCODE_MOV] = {
  .rm8_r8 = { .opcode = 0x88u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .rm16_r16 = { .opcode = 0x89u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .rm32_r32 = { .opcode = 0x89u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .rm64_r64 = { .opcode = 0x89u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },

  .r8_rm8 = { .opcode = 0x8Au, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r16_rm16 = { .opcode = 0x8Bu, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r32_rm32 = { .opcode = 0x8Bu, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r64_rm64 = { .opcode = 0x8Bu, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },

  .r8_i8 = { .opcode = 0xB0u, .operands = ((((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
  .r16_i16 = { .opcode = 0xB8u, .operands = ((((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
  .r32_i32 = { .opcode = 0xB8u, .operands = ((((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
  .r64_i64 = { .opcode = 0xB8u, .operands = ((((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },

  .rm8_i8 = { .opcode = 0xC6u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
  .rm16_i16 = { .opcode = 0xC7u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
  .rm32_i32 = { .opcode = 0xC7u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
  .rm64_i32 = { .opcode = 0xC7u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
 },
 [OPCODE_MOVSB] = { .noarg = { .opcode = 0xA4u, .operands = 0x00u }, },
 [OPCODE_MOVSW] = { .noarg = { .opcode = 0xA5u, .operands = 0x20u }, },
 [OPCODE_MOVSD] = { .noarg = { .opcode = 0xA5u, .operands = 0x00u }, },
 [OPCODE_MOVSQ] = { .noarg = { .opcode = 0xA5u, .operands = 0x10u }, },
 [OPCODE_MOVXSD] = {
  .r16_rm16 = { .opcode = 0x63u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r32_rm32 = { .opcode = 0x63u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r64_rm64 = { .opcode = 0x63u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
 },
 [OPCODE_MUL] = {
  .rm8 = { .opcode = 0xF6u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) },
  .rm16 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) },
  .rm32 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) },
 },
 [OPCODE_NEG] = {
  .rm8 = { .opcode = 0xF6u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) },
  .rm16 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) },
  .rm32 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) },
 },
 [OPCODE_NOT] = {
  .rm8 = { .opcode = 0xF6u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) },
  .rm16 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) },
  .rm32 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0xF7u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) },
 },
 [OPCODE_OUT] = {
  .al_i8 = { .opcode = 0xE6u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .ax_i8 = { .opcode = 0xE7u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .eax_i8 = { .opcode = 0xE7u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .al_dx = { .opcode = 0xEEu, .operands = 0x00u },
  .ax_dx = { .opcode = 0xEFu, .operands = 0x00u },
  .eax_dx = { .opcode = 0xEFu, .operands = 0x00u },
 },
 [OPCODE_OUTSB] = { .noarg = { .opcode = 0x6Eu, } },
 [OPCODE_OUTSW] = { .noarg = { .opcode = 0x6Fu, .operands = 0x20u } },
 [OPCODE_OUTSD] = { .noarg = { .opcode = 0x6Fu, } },
 [OPCODE_POP] = {
  .rm16 = { .opcode = 0x8Fu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0x8Fu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | 0x10 },
  .r16 = { .opcode = 0x58u, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) },
  .r64 = { .opcode = 0x58u, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) | 0x10 },
 },
 [OPCODE_POPF] = { .noarg = { .opcode = 0x9D, .operands = 0x20u } },
 [OPCODE_POPFQ] = { .noarg = { .opcode = 0x9D, .operands = 0x00u } },
 [OPCODE_PUSH] = {
  .rm16 = { .opcode = 0xFFu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6) & 0xF)) << 24u) },
  .rm64 = { .opcode = 0xFFu, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)6) & 0xF)) << 24u) | 0x10 },
  .r16 = { .opcode = 0x50u, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) },
  .r64 = { .opcode = 0x50u, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) | 0x10 },

  .i8 = { .opcode = 0x6Au, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) },
  .i16 = { .opcode = 0x68u, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) },
  .i32 = { .opcode = 0x68u, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) },
 },
 [OPCODE_PUSHF] = { .noarg = { .opcode = 0x9C, .operands = 0x20u } },
 [OPCODE_PUSHFQ] = { .noarg = { .opcode = 0x9C, .operands = 0x00u } },
 [OPCODE_RET] = {
  .noarg = { .opcode = 0xC3, .operands = 0x00u },
  .i16 = { .opcode = 0xC2, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
 },
 [OPCODE_RET_FAR] = {
  .noarg = { .opcode = 0xCB, .operands = 0x00u },
  .i16 = { .opcode = 0xCA, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
 },
 [OPCODE_SAHF] = { .noarg = { .opcode = 0x9E } },
 [OPCODE_RCL] = { .rm8 = { .opcode = 0xD0u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) }, .rm16 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) }, .rm32 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) }, .rm64 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) }, .rm8_i8 = { .opcode = 0xC0u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_cl = { .opcode = 0xD2u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) }, .rm16_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) }, .rm32_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) }, .rm64_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)2) & 0xF)) << 24u) }, },
 [OPCODE_RCR] = { .rm8 = { .opcode = 0xD0u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) }, .rm16 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) }, .rm32 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) }, .rm64 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) }, .rm8_i8 = { .opcode = 0xC0u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_cl = { .opcode = 0xD2u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) }, .rm16_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) }, .rm32_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) }, .rm64_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)3) & 0xF)) << 24u) }, },
 [OPCODE_ROL] = { .rm8 = { .opcode = 0xD0u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .rm16 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .rm32 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .rm64 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .rm8_i8 = { .opcode = 0xC0u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_cl = { .opcode = 0xD2u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .rm16_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .rm32_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, .rm64_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) }, },
 [OPCODE_ROR] = { .rm8 = { .opcode = 0xD0u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) }, .rm16 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) }, .rm32 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) }, .rm64 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) }, .rm8_i8 = { .opcode = 0xC0u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_cl = { .opcode = 0xD2u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) }, .rm16_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) }, .rm32_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) }, .rm64_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)1) & 0xF)) << 24u) }, },
 [OPCODE_SAL] = { .rm8 = { .opcode = 0xD0u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm16 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm32 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm64 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm8_i8 = { .opcode = 0xC0u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_cl = { .opcode = 0xD2u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm16_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm32_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm64_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, },
 [OPCODE_SAR] = { .rm8 = { .opcode = 0xD0u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) }, .rm16 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) }, .rm32 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) }, .rm64 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) }, .rm8_i8 = { .opcode = 0xC0u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_cl = { .opcode = 0xD2u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) }, .rm16_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) }, .rm32_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) }, .rm64_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)7) & 0xF)) << 24u) }, },
 [OPCODE_SHL] = { .rm8 = { .opcode = 0xD0u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm16 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm32 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm64 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm8_i8 = { .opcode = 0xC0u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_cl = { .opcode = 0xD2u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm16_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm32_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, .rm64_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)4) & 0xF)) << 24u) }, },
 [OPCODE_SHR] = { .rm8 = { .opcode = 0xD0u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) }, .rm16 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) }, .rm32 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) }, .rm64 = { .opcode = 0xD1u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) }, .rm8_i8 = { .opcode = 0xC0u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm16_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm32_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm64_i8 = { .opcode = 0xC1u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) }, .rm8_cl = { .opcode = 0xD2u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) }, .rm16_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) }, .rm32_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) }, .rm64_cl = { .opcode = 0xD3u, .operands = ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)5) & 0xF)) << 24u) }, },
 [OPCODE_SCASB] = { .noarg = { .opcode = 0xAEu } },
 [OPCODE_SCASW] = { .noarg = { .opcode = 0xAFu, .operands = 0x20u } },
 [OPCODE_SCASD] = { .noarg = { .opcode = 0xAFu } },
 [OPCODE_SCASQ] = { .noarg = { .opcode = 0xAFu, .operands = 0x10u } },
 [OPCODE_STC] = { .noarg = { .opcode = 0xF9 } },
 [OPCODE_STI] = { .noarg = { .opcode = 0xFB } },
 [OPCODE_STD] = { .noarg = { .opcode = 0xFD } },
 [OPCODE_STOSB] = { .noarg = { .opcode = 0xAAu } },
 [OPCODE_STOSW] = { .noarg = { .opcode = 0xABu, .operands = 0x20u, } },
 [OPCODE_STOSD] = { .noarg = { .opcode = 0xABu } },
 [OPCODE_STOSQ] = { .noarg = { .opcode = 0xABu, .operands = 0x10u, } },
 [OPCODE_SYSCALL] = { .noarg = { .opcode = 0x0F05 } },
 [OPCODE_TEST] = {
  .al_i8 = { .opcode = 0xA8u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .ax_i16 = { .opcode = 0xA9u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .eax_i32 = { .opcode = 0xA9u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },
  .rax_i32 = { .opcode = 0xA9u, .operands = (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 24u) },

  .rm8_i8 = { .opcode = 0xF6u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
  .rm16_i16 = { .opcode = 0xF7u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
  .rm32_i32 = { .opcode = 0xF7u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },
  .rm64_i32 = { .opcode = 0xF7u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | (((((uint32_t)OPERANDS_TYPE_imm) << 5)) << 16u)) },

  .rm8_r8 = { .opcode = 0x84u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .rm16_r16 = { .opcode = 0x85u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .rm32_r32 = { .opcode = 0x85u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .rm64_r64 = { .opcode = 0x85u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
 },
 [OPCODE_XCHG] = {
  .ax_r16 = { .opcode = 0x90u, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) },
  .eax_r32 = { .opcode = 0x90u, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) },
  .rax_r64 = { .opcode = 0x90u, .operands = (((((uint32_t)OPERANDS_TYPE_add_to_opcode) << 5)) << 24u) },

  .rm8_r8 = { .opcode = 0x87u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .rm16_r16 = { .opcode = 0x87u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .rm32_r32 = { .opcode = 0x87u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .rm64_r64 = { .opcode = 0x87u, .operands = (((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },

  .r8_rm8 = { .opcode = 0x86u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r16_rm16 = { .opcode = 0x87u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r32_rm32 = { .opcode = 0x87u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
  .r64_rm64 = { .opcode = 0x87u, .operands = (((((((uint32_t)OPERANDS_TYPE_modreg) << 5) | ((uint32_t)0) & 0xF)) << 24u) | ((((((uint32_t)OPERANDS_TYPE_modrm) << 5) | ((uint32_t)0) & 0xF)) << 16u)) },
 },
 [OPCODE_XLAT] = { .noarg = { .opcode = 0xD7u, .operands = 0x00u }, },
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

  _Generic(parameters.rep, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, parameters.rep);
 }
# 469 "backends/x86_64_common.c"
  uint32_t op0 = ((instruction.operands >> 24u) & 0xFF); switch (op0 >> 5u) { case OPERANDS_TYPE_modrm: if ((parameters.reg0 & 0xFu) >= X86_64_OPERAND_REGISTER_r8) { rex |= 0x41u; } break; default: break; }
  uint32_t op1 = ((instruction.operands >> 16u) & 0xFF); switch (op1 >> 5u) { case OPERANDS_TYPE_modrm: if ((parameters.reg1 & 0xFu) >= X86_64_OPERAND_REGISTER_r8) { rex |= 0x41u; } break; default: break; }


 if (rex) _Generic(rex, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, rex);
 for (i = 0; i < prefixi; ++i) {
  _Generic(prefix[i], uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, prefix[i]);
 }

 if ((instruction.opcode >> 8) & 0xFF) {
  _Generic((uint8_t)((instruction.opcode >> 8) & 0xFF), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, (uint8_t)((instruction.opcode >> 8) & 0xFF));
 }
 _Generic((uint8_t)(instruction.opcode & 0xFF), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, (uint8_t)(instruction.opcode & 0xFF));

 if (x86_64_uses_modrm(variant)) {
  mod = 0;
  sib = 0;
# 505 "backends/x86_64_common.c"
  uint32_t op0 = ((instruction.operands >> 24u) & 0xFF); switch (op0 >> 5u) { case OPERANDS_TYPE_modreg: mod |= ((op0 & 0xFu) << 3u); mod |= (parameters.reg0 & 0x7u) << 3u; break; case OPERANDS_TYPE_modrm: mod |= ((op0 & 0xFu) << 3u); if (parameters.use_sib) { sib |= parameters.reg0 & 0x7u; } else { mod |= parameters.reg0 & 0x7u; } break; default: break; }
  uint32_t op1 = ((instruction.operands >> 16u) & 0xFF); switch (op1 >> 5u) { case OPERANDS_TYPE_modreg: mod |= ((op1 & 0xFu) << 3u); mod |= (parameters.reg1 & 0x7u) << 3u; break; case OPERANDS_TYPE_modrm: mod |= ((op1 & 0xFu) << 3u); if (parameters.use_sib) { sib |= parameters.reg1 & 0x7u; } else { mod |= parameters.reg1 & 0x7u; } break; default: break; }




  uint8_t base_reg = (parameters.reg0 & (0x10u)) ? (parameters.reg0 & 0x7u) :
   (parameters.reg1 & (0x10u)) ? (parameters.reg1 & 0x7u) : 0;


  if ((parameters.reg0 & (0x10u)) || (parameters.reg1 & (0x10u))) {

   int64_t offset = parameters.displacement;

   if (!parameters.use_sib && base_reg == X86_64_OPERAND_REGISTER_rsp) {
    parameters.use_sib = 1;
    sib = (X86_64_OPERAND_REGISTER_rsp << 3u) | X86_64_OPERAND_REGISTER_rsp;
   }

   if (parameters.use_sib) {
    mod |= 0x4u;
    if (parameters.use_sib & (1u << 2u)) {
     if (!(parameters.use_sib & (1u << 1u))) {
      fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: scale requires index\n");
      abort();
     }
     sib |= parameters.scale << 6u;
    }
    if (parameters.use_sib & (1u << 1u)) {
     if (parameters.index == X86_64_OPERAND_REGISTER_rsp) {
      fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: invalid use of rsp as index with rbp as base\x1b[0m\n");
      abort();
     }
     sib |= (parameters.index & 7u) << 3u;
    } else {
     sib |= X86_64_OPERAND_REGISTER_rsp << 3u;
    }

    if (offset == 0) {

     if (base_reg == X86_64_OPERAND_REGISTER_rbp) {

      assert((mod >> 6) == 0);
      if (parameters.use_sib & (1u << 2u)) {
      } else {
       mod |= 0x40u;
      }
     }
     _Generic(mod, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, mod);
     _Generic(sib, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, sib);
     if (base_reg == X86_64_OPERAND_REGISTER_rbp) {

      if (parameters.use_sib & (1u << 2u)) {
       _Generic(PUN((int32_t)0u, uint32_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int32_t)0u, uint32_t));
      } else {
       _Generic(PUN((int8_t)0u, uint8_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int8_t)0u, uint8_t));
      }
     }
    } else if (offset >= INT8_MIN && offset <= INT8_MAX) {
     mod |= 0x40;
     _Generic(mod, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, mod);
     _Generic(sib, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, sib);
     _Generic(PUN((int8_t)offset, uint8_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int8_t)offset, uint8_t));
    } else if (offset >= INT32_MIN && offset <= INT32_MAX) {
     mod |= 0x80;
     _Generic(mod, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, mod);
     _Generic(sib, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, sib);
     _Generic(PUN((int32_t)offset, uint32_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int32_t)offset, uint32_t));
    }
   } else if (offset == 0 && base_reg != X86_64_OPERAND_REGISTER_rbp) {
    _Generic(mod, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, mod);
   } else if (offset >= INT8_MIN && offset <= INT8_MAX) {
    mod |= 0x40;
    _Generic(mod, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, mod);
    _Generic(PUN((int8_t)offset, uint8_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int8_t)offset, uint8_t));
   } else if (offset >= INT32_MIN && offset <= INT32_MAX) {
    mod |= 0x80;
    _Generic(mod, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, mod);
    _Generic(PUN((int32_t)offset, uint32_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int32_t)offset, uint32_t));
   } else assert(false);
  } else {
   mod |= 0xc0;
   _Generic(mod, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, mod);
   if (parameters.use_sib) {
    _Generic(sib, uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, sib);
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
  _Generic(PUN((int8_t)parameters.immediate, uint8_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int8_t)parameters.immediate, uint8_t));
     break;

 case X86_64_VARIANT_KIND_ax_i16:
 case X86_64_VARIANT_KIND_rm16_i16:
 case X86_64_VARIANT_KIND_r16_i16:
 case X86_64_VARIANT_KIND_i16:
 case X86_64_VARIANT_KIND_r16_rm16_i16:
  _Generic(PUN((int16_t)parameters.immediate, uint16_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int16_t)parameters.immediate, uint16_t));
  break;

 case X86_64_VARIANT_KIND_rax_i32:
 case X86_64_VARIANT_KIND_eax_i32:
 case X86_64_VARIANT_KIND_rm32_i32:
 case X86_64_VARIANT_KIND_r32_i32:
 case X86_64_VARIANT_KIND_rm64_i32:
 case X86_64_VARIANT_KIND_i32:
 case X86_64_VARIANT_KIND_r32_rm32_i32:
    case X86_64_VARIANT_KIND_r64_rm64_i32:
  _Generic(PUN((int32_t)parameters.immediate, uint32_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int32_t)parameters.immediate, uint32_t));
  break;

 case X86_64_VARIANT_KIND_r64_i64:
  _Generic(PUN((int64_t)parameters.immediate, uint64_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int64_t)parameters.immediate, uint64_t));
  break;



 case X86_64_VARIANT_KIND_rel8:
  _Generic(PUN((int8_t)parameters.relative, uint8_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int8_t)parameters.relative, uint8_t));
     break;

 case X86_64_VARIANT_KIND_rel16:
  _Generic(PUN((int16_t)parameters.relative, uint16_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int16_t)parameters.relative, uint16_t));
     break;

 case X86_64_VARIANT_KIND_rel32:
  _Generic(PUN((int32_t)parameters.relative, uint32_t), uint8_t : b->append_u8, uint16_t : b->append_u16, uint32_t : b->append_u32, uint64_t : b->append_u64 )(cc, b, PUN((int32_t)parameters.relative, uint32_t));
     break;

 default: break;
 }

}

void test_movs(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_r8, x86_64_instructions_table[OPCODE_MOV] . rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_r16, x86_64_instructions_table[OPCODE_MOV] . rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_r32, x86_64_instructions_table[OPCODE_MOV] . rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_r64, x86_64_instructions_table[OPCODE_MOV] . rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_r8, x86_64_instructions_table[OPCODE_MOV] . rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_r16, x86_64_instructions_table[OPCODE_MOV] . rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_r32, x86_64_instructions_table[OPCODE_MOV] . rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_r64, x86_64_instructions_table[OPCODE_MOV] . rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r8_rm8, x86_64_instructions_table[OPCODE_MOV] . r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16_rm16, x86_64_instructions_table[OPCODE_MOV] . r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r64_rm64, x86_64_instructions_table[OPCODE_MOV] . r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r8_rm8, x86_64_instructions_table[OPCODE_MOV] . r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16_rm16, x86_64_instructions_table[OPCODE_MOV] . r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r64_rm64, x86_64_instructions_table[OPCODE_MOV] . r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r8_i8, x86_64_instructions_table[OPCODE_MOV] . r8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16_i16, x86_64_instructions_table[OPCODE_MOV] . r16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_i32, x86_64_instructions_table[OPCODE_MOV] . r32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r64_i64, x86_64_instructions_table[OPCODE_MOV] . r64_i64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_MOV] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i16, x86_64_instructions_table[OPCODE_MOV] . rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i32, x86_64_instructions_table[OPCODE_MOV] . rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i32, x86_64_instructions_table[OPCODE_MOV] . rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_MOV] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i16, x86_64_instructions_table[OPCODE_MOV] . rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i32, x86_64_instructions_table[OPCODE_MOV] . rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i32, x86_64_instructions_table[OPCODE_MOV] . rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rax }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rdx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rsp }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rsi }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rdi }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_r12 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_r13 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rdx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rsp, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rsi, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rdi, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rcx }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rax, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rdx, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rbx, .scale = 2 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_r12, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rbp, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rsi, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rdi, .scale = 2 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rdx, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbx, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rsp, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rsi, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rdi, .displacement = 0x07 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rdi, .displacement = 0 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rax, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rdx, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rbx, .scale = 2 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rbp, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rsi, .scale = 2 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rdi, .scale = 2 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rax, .scale = 0, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 0, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rdx, .scale = 0, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rbx, .scale = 0, .displacement = 0x07 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rbp, .scale = 0, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rsi, .scale = 0, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u), .index = X86_64_OPERAND_REGISTER_rdi, .scale = 0, .displacement = 0x07 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rdx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbx, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rsp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rsi, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_MOV] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rdi, .use_sib = 1 | (1u << 1u) | (1u << 2u), .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }));
}

void test_shifts(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
# 786 "backends/x86_64_common.c"
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_SAL] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_SAL] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_SAL] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_SAL] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_SAL] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[OPCODE_SAL] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[OPCODE_SAL] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[OPCODE_SAL] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_cl, x86_64_instructions_table[OPCODE_SAL] . rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_cl, x86_64_instructions_table[OPCODE_SAL] . rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_cl, x86_64_instructions_table[OPCODE_SAL] . rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_cl, x86_64_instructions_table[OPCODE_SAL] . rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_SAL] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_SAL] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_SAL] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_SAL] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_SAL] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[OPCODE_SAL] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[OPCODE_SAL] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[OPCODE_SAL] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_cl, x86_64_instructions_table[OPCODE_SAL] . rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_cl, x86_64_instructions_table[OPCODE_SAL] . rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_cl, x86_64_instructions_table[OPCODE_SAL] . rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_cl, x86_64_instructions_table[OPCODE_SAL] . rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_SAR] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_SAR] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_SAR] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_SAR] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_SAR] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[OPCODE_SAR] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[OPCODE_SAR] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[OPCODE_SAR] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_cl, x86_64_instructions_table[OPCODE_SAR] . rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_cl, x86_64_instructions_table[OPCODE_SAR] . rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_cl, x86_64_instructions_table[OPCODE_SAR] . rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_cl, x86_64_instructions_table[OPCODE_SAR] . rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_SAR] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_SAR] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_SAR] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_SAR] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_SAR] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[OPCODE_SAR] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[OPCODE_SAR] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[OPCODE_SAR] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_cl, x86_64_instructions_table[OPCODE_SAR] . rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_cl, x86_64_instructions_table[OPCODE_SAR] . rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_cl, x86_64_instructions_table[OPCODE_SAR] . rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_cl, x86_64_instructions_table[OPCODE_SAR] . rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_SHL] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_SHL] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_SHL] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_SHL] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_SHL] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[OPCODE_SHL] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[OPCODE_SHL] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[OPCODE_SHL] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_cl, x86_64_instructions_table[OPCODE_SHL] . rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_cl, x86_64_instructions_table[OPCODE_SHL] . rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_cl, x86_64_instructions_table[OPCODE_SHL] . rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_cl, x86_64_instructions_table[OPCODE_SHL] . rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_SHL] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_SHL] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_SHL] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_SHL] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_SHL] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[OPCODE_SHL] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[OPCODE_SHL] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[OPCODE_SHL] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_cl, x86_64_instructions_table[OPCODE_SHL] . rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_cl, x86_64_instructions_table[OPCODE_SHL] . rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_cl, x86_64_instructions_table[OPCODE_SHL] . rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_cl, x86_64_instructions_table[OPCODE_SHL] . rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_SHR] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_SHR] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_SHR] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_SHR] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_SHR] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[OPCODE_SHR] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[OPCODE_SHR] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[OPCODE_SHR] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_cl, x86_64_instructions_table[OPCODE_SHR] . rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_cl, x86_64_instructions_table[OPCODE_SHR] . rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_cl, x86_64_instructions_table[OPCODE_SHR] . rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_cl, x86_64_instructions_table[OPCODE_SHR] . rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_SHR] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_SHR] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_SHR] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_SHR] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[OPCODE_SHR] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[OPCODE_SHR] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[OPCODE_SHR] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[OPCODE_SHR] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_cl, x86_64_instructions_table[OPCODE_SHR] . rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_cl, x86_64_instructions_table[OPCODE_SHR] . rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_cl, x86_64_instructions_table[OPCODE_SHR] . rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_cl, x86_64_instructions_table[OPCODE_SHR] . rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }));

}

void test_arith(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, int opcode) {
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_al_i8, x86_64_instructions_table[opcode] . al_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_ax_i16, x86_64_instructions_table[opcode] . ax_i16, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_eax_i32, x86_64_instructions_table[opcode] . eax_i32, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rax_i32, x86_64_instructions_table[opcode] . rax_i32, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_r8, x86_64_instructions_table[opcode] . rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_r16, x86_64_instructions_table[opcode] . rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_r32, x86_64_instructions_table[opcode] . rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_r64, x86_64_instructions_table[opcode] . rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_r8, x86_64_instructions_table[opcode] . rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_r16, x86_64_instructions_table[opcode] . rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_r32, x86_64_instructions_table[opcode] . rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_r64, x86_64_instructions_table[opcode] . rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r8_rm8, x86_64_instructions_table[opcode] . r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16_rm16, x86_64_instructions_table[opcode] . r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[opcode] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r64_rm64, x86_64_instructions_table[opcode] . r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r8_rm8, x86_64_instructions_table[opcode] . r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16_rm16, x86_64_instructions_table[opcode] . r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[opcode] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r64_rm64, x86_64_instructions_table[opcode] . r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[opcode] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i16, x86_64_instructions_table[opcode] . rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i32, x86_64_instructions_table[opcode] . rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i32, x86_64_instructions_table[opcode] . rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8_i8, x86_64_instructions_table[opcode] . rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i16, x86_64_instructions_table[opcode] . rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i32, x86_64_instructions_table[opcode] . rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i32, x86_64_instructions_table[opcode] . rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[opcode] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[opcode] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[opcode] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .immediate = 0x78 }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16_i8, x86_64_instructions_table[opcode] . rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32_i8, x86_64_instructions_table[opcode] . rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64_i8, x86_64_instructions_table[opcode] . rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }));
}

void test_imul(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_IMUL] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16_rm16, x86_64_instructions_table[OPCODE_IMUL] . r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_rax }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16_rm16, x86_64_instructions_table[OPCODE_IMUL] . r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_rax }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16_rm16_i8, x86_64_instructions_table[OPCODE_IMUL] . r16_rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r64_rm64_i8, x86_64_instructions_table[OPCODE_IMUL] . r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16_rm16_i16, x86_64_instructions_table[OPCODE_IMUL] . r16_rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32_i32, x86_64_instructions_table[OPCODE_IMUL] . r32_rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = (0x10u) | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }));

}

void test_incdec(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
# 864 "backends/x86_64_common.c"
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_INC] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_INC] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_INC] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_INC] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_INC] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_INC] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_INC] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_INC] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16, x86_64_instructions_table[OPCODE_INC] . r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32, x86_64_instructions_table[OPCODE_INC] . r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_DEC] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_DEC] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_DEC] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_DEC] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm8, x86_64_instructions_table[OPCODE_DEC] . rm8, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_DEC] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm32, x86_64_instructions_table[OPCODE_DEC] . rm32, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_DEC] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16, x86_64_instructions_table[OPCODE_DEC] . r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp })); x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32, x86_64_instructions_table[OPCODE_DEC] . r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }));


}

void test_push(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_PUSH] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_PUSH] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm16, x86_64_instructions_table[OPCODE_PUSH] . rm16, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rax }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_rm64, x86_64_instructions_table[OPCODE_PUSH] . rm64, ((X86_64_Instruction_Parameters) { .reg0 = (0x10u) | X86_64_OPERAND_REGISTER_rbx }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r16, x86_64_instructions_table[OPCODE_PUSH] . r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r64, x86_64_instructions_table[OPCODE_PUSH] . r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx }));

 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_i8, x86_64_instructions_table[OPCODE_PUSH] . i8, ((X86_64_Instruction_Parameters) { .immediate = 0x69 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_i16, x86_64_instructions_table[OPCODE_PUSH] . i16, ((X86_64_Instruction_Parameters) { .immediate = 0x69 }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_i32, x86_64_instructions_table[OPCODE_PUSH] . i32, ((X86_64_Instruction_Parameters) { .immediate = 0x69 }));
}

void x86_64_run_tests(Compiler_Context* cc, X86_64_Machine_Code_Writer* b) {
# 926 "backends/x86_64_common.c"
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r32_rm32, x86_64_instructions_table[OPCODE_CMOVA] . r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx }));
 x86_64_write_instruction(cc, (X86_64_Machine_Code_Writer*)b, X86_64_VARIANT_KIND_r64_rm64, x86_64_instructions_table[OPCODE_CMOVA] . r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx }));
# 942 "backends/x86_64_common.c"
}
