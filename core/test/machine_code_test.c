#define OC_MACHINE_CODE_IMPLEMENTATION 1
#include "../machine_code.h"

void test_movs(OC_Machine_Code_Writer* w) {
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r64_i64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsp }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsi }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_r12 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_r13 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsp, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsi, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rax, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rdx, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rbx, .scale = 2 }) );
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rsp, .scale = 2 }) ); */
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_r12, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rbp, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rsi, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rdi, .scale = 2 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsp, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsi, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi, .displacement = 0x07 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi, .displacement = 0 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rax, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rdx, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rbx, .scale = 2 }) );
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rsp, .scale = 2 }) ); */
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rbp, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rsi, .scale = 2 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rdi, .scale = 2 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rax, .scale = 0, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 0, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rdx, .scale = 0, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rbx, .scale = 0, .displacement = 0x07 }) );
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rsp, .scale = 0, .displacement = 0x07 }) ); */
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rbp, .scale = 0, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rsi, .scale = 0, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX, .index = X86_64_OPERAND_REGISTER_rdi, .scale = 0, .displacement = 0x07 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rsi, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdi, .use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE, .index = X86_64_OPERAND_REGISTER_rcx, .scale = 2, .displacement = 0x07 }) );
}

void test_shifts(OC_Machine_Code_Writer* w) {
#define DO_TESTS(op) \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = 0x4 }) ); \
\
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm8_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm16_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm32_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm64_cl, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .displacement = 0x60, .immediate = 0x4 }) );

    DO_TESTS(OPCODE_SAL)
    DO_TESTS(OPCODE_SAR)
    DO_TESTS(OPCODE_SHL)
    DO_TESTS(OPCODE_SHR)
#undef DO_TESTS
}

void test_arith(OC_Machine_Code_Writer* w, int opcode) {
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, al_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, ax_i16, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, eax_i32, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rax_i32, ((X86_64_Instruction_Parameters) { .immediate = 0x4 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, .displacement = 0x10 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm8_r8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm16_r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm32_r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );

    OC_X86_64_WRITE_INSTRUCTION(w, opcode, r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, opcode, r8_rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rax, }) );

    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .displacement = 0x10, .immediate = 0x78 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm8_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm64_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .immediate = 0x78 }) );

    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm32_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, opcode, rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10, .immediate = 0x78 }) );
}

void test_imul(OC_Machine_Code_Writer* w) {
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IMUL, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IMUL, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_rax }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IMUL, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_rax }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IMUL, r16_rm16_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IMUL, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IMUL, r16_rm16_i16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IMUL, r32_rm32_i32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax, .immediate = 0x10 }) );

}

void test_incdec(OC_Machine_Code_Writer* w) {
#define DO_TESTS(op) \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) ); \
\
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm8, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 }) );  \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbp, .displacement = 0x10 }) ); \
\
    OC_X86_64_WRITE_INSTRUCTION(w, op, r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) ); \
    OC_X86_64_WRITE_INSTRUCTION(w, op, r32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbp }) );

    DO_TESTS(OPCODE_INC)
    DO_TESTS(OPCODE_DEC)

#undef DO_TESTS
}

void test_push(OC_Machine_Code_Writer* w) {
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUSH, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUSH, rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rax }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUSH, r16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rax }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUSH, r64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx }) );

    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUSH, i8, ((X86_64_Instruction_Parameters) { .immediate = 0x69 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUSH, i16, ((X86_64_Instruction_Parameters) { .immediate = 0x69 }) );
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUSH, i32, ((X86_64_Instruction_Parameters) { .immediate = 0x69 }) );
}

void oc_x86_64_run_tests(OC_Machine_Code_Writer* w) {
    /* X86_64_OPERAND_MI_MEM(X86_64_OPCODE_MOV, qword, rbp, -0x10, 100); */

#define xmm(n) X86_64_OPERAND_REGISTER_xmm(n)
#define reg(n) X86_64_OPERAND_REGISTER_ ## n

    test_movs(w);

    /* test_shifts(cc, b); */
    /* test_arith(cc, b, OPCODE_ADD); */
    /* test_arith(cc, b, OPCODE_ADC); */
    /* test_incdec(cc, b); */
    /* test_push(cc, b); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_JECXZ, rel8, ((X86_64_Instruction_Parameters) { .relative = 0x60 }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_JRCXZ, rel8, ((X86_64_Instruction_Parameters) { .relative = 0x60 }) ); */
    /* test_imul(cc, b); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CMPSB, noarg, ((X86_64_Instruction_Parameters) {}) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CMPSW, noarg, ((X86_64_Instruction_Parameters) {}) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CMPSD, noarg, ((X86_64_Instruction_Parameters) {}) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CMPSQ, noarg, ((X86_64_Instruction_Parameters) {}) ); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IN, al_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x01u }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IN, ax_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x02u }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IN, eax_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x02u }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IN, al_dx, ((X86_64_Instruction_Parameters) {}) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IN, ax_dx, ((X86_64_Instruction_Parameters) {}) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_IN, eax_dx, ((X86_64_Instruction_Parameters) {}) ); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_OUT, al_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x01u }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_OUT, ax_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x02u }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_OUT, eax_i8, ((X86_64_Instruction_Parameters) { .immediate = 0x02u }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_OUT, al_dx, ((X86_64_Instruction_Parameters) {}) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_OUT, ax_dx, ((X86_64_Instruction_Parameters) {}) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_OUT, eax_dx, ((X86_64_Instruction_Parameters) { .rep = X86_64_PREFIX_REP }) ); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_RET, noarg, ((X86_64_Instruction_Parameters) {}) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_RET, i16, ((X86_64_Instruction_Parameters) { .immediate = 0x10 }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_RET_FAR, noarg, ((X86_64_Instruction_Parameters) {}) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_RET_FAR, i16, ((X86_64_Instruction_Parameters) { .immediate = 0x10 }) ); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CALL, rel32, ((X86_64_Instruction_Parameters) { .immediate = 0x10 }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CALL, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CALL, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rdx }) ); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CMOVA, r16_rm16, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CMOVA, r32_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CMOVA, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx, .reg1 = X86_64_OPERAND_REGISTER_rcx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOV, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVSS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVSS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVSS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVSS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVUPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVUPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVUPS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVUPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVUPS, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVUPS, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVUPS, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx, .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */

/* 	OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVUPD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
/* 	OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVUPD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
/* 	OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVUPD, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVLPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(6), .reg2 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVLPS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg1 = X86_64_OPERAND_REGISTER_xmm(4), .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVLPS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg1 = X86_64_OPERAND_REGISTER_xmm(4), .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVHLPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(5) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVHLPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg1 = X86_64_OPERAND_REGISTER_xmm(4), .reg2 = X86_64_OPERAND_REGISTER_xmm(7) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVSLDUP, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg2 = X86_64_OPERAND_REGISTER_xmm(7) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVSLDUP, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg2 = X86_64_OPERAND_REGISTER_xmm(7) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVSLDUP, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(4), .reg2 = X86_64_OPERAND_REGISTER_xmm(7) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVSD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVSD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_OPERAND_REGISTER_xmm(1), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVSD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(0), .reg1 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rcx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVSD, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_REG_BASE | X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(1) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_UNPCKLPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2)  })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_UNPCKHPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2)  })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VUNPCKLPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VUNPCKHPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VUNPCKLPS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VUNPCKHPS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVAPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2)  })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVAPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2)  })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVAPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVAPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVAPS, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVAPS, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */


    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVNTPS, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1) | X86_64_REG_BASE, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVNTPS, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1) | X86_64_REG_BASE, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVNTPS, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1) | X86_64_REG_BASE, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTPI2PS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTPI2PD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTPS2PI, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTPD2PI, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTTPS2PI, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTTPD2PI, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */


    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTSI2SS, r128_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTSI2SS, r128_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTSI2SD, r128_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTSI2SD, r128_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VCVTSI2SS, r128_vvvv_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VCVTSI2SS, r128_vvvv_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VCVTSI2SD, r128_vvvv_rm32, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VCVTSI2SD, r128_vvvv_rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_rdx })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTTSS2SI, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTTSD2SI, r64_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VCVTTSS2SI, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VCVTTSD2SI, r64_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTSS2SI, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CVTSD2SI, r64_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VCVTSS2SI, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VCVTSD2SI, r64_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */

#define DO_VECTOR_ARITH(op) \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_ ## op ## PS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_ ## op ## PD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_ ## op ## SS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_ ## op ## SD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_V ## op ## PS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_V ## op ## PS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_V ## op ## PD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_V ## op ## PD, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_V ## op ## SS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) })); \
    OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_V ## op ## SD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(5) }))

    /* DO_VECTOR_ARITH(ADD); */
    /* DO_VECTOR_ARITH(MUL); */
    /* DO_VECTOR_ARITH(SUB); */
    /* DO_VECTOR_ARITH(MIN); */
    /* DO_VECTOR_ARITH(DIV); */
    /* DO_VECTOR_ARITH(MAX); */

#undef DO_VECTOR_ARITH

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PUNPCKLBW, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VPUNPCKLBW, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_xmm(1), .reg1 = X86_64_OPERAND_REGISTER_xmm(2), .reg2 = X86_64_OPERAND_REGISTER_xmm(6) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVMSKPS, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVMSKPD, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVMSKPS, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVMSKPD, r32_rm128, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVMSKPS, r32_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVMSKPD, r32_rm256, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rbx, .reg1 = X86_64_OPERAND_REGISTER_xmm(2) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_EMMS, noarg, ((X86_64_Instruction_Parameters) { })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VZEROUPPER, noarg, ((X86_64_Instruction_Parameters) { })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VZEROALL, noarg, ((X86_64_Instruction_Parameters) { })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PSHUFW, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PSHUFD, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PSHUFHW, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_PSHUFLW, r64_rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VPSHUFD, r128_rm128_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VPSHUFHW, r128_rm128_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VPSHUFLW, r128_rm128_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VPSHUFD, r256_rm256_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VPSHUFHW, r256_rm256_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VPSHUFLW, r256_rm256_i8, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(1), .immediate = 0x40u })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVD, r64_rm32, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVD, rm32_r64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVD, r128_rm32, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVD, rm32_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVQ, r64_rm64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVQ, rm64_r64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVQ, r128_rm64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVQ, rm64_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVD, r128_rm32, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVD, rm32_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVQ, r128_rm64, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVQ, rm64_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVDQA, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVDQA, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVDQU, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_MOVDQU, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVDQA, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVDQA, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVDQA, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVDQA, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVDQU, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVDQU, rm128_r128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVDQU, r256_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VMOVDQU, rm256_r256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_HADDPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_HADDPD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VHADDPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VHADDPS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VHADDPD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VHADDPD, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_HSUBPS, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_HSUBPD, r128_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = reg(rbx) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VHSUBPS, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VHSUBPS, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VHSUBPD, r128_vvvv_rm128, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_VHSUBPD, r256_vvvv_rm256, ((X86_64_Instruction_Parameters) { .reg0 = xmm(2), .reg1 = xmm(5), .reg2 = xmm(3) })); */
    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_JA, rel32, ((X86_64_Instruction_Parameters) { .relative = 0x10u })); */

    /* OC_X86_64_WRITE_INSTRUCTION(w, OPCODE_CPUID, noarg, ((X86_64_Instruction_Parameters) {})); */

#undef xmm
#undef reg
}

#include <string.h>

#define hex_dump(data, count, strides, strides_count) _hex_dump(data, count, strides, strides_count, 0, -1)
void _hex_dump(void* data, int count, unsigned int* strides, int strides_count, int indent, int mark_mod) {
    unsigned char *bytes = (unsigned char*)data;
    for (int i = 0; i < indent; i++) {
        printf("    ");
    }
    int stride_index = 0;
    for (int i = 0; i < count; i++) {
        // if (mark_mod > -1) {
        //     if ((i % 16) == 0 && (i > 0) && (i % mark_mod) == 0) {
        //         printf("\n");
        //         for (int j = 0; j < indent; j++) {
        //             printf("    ");
        //         }
        //     }
        // } else {
        //     if (i % 16 == 0 && i > 0) {
        //         printf("\n");
        //         for (int j = 0; j < indent; j++) {
        //             printf("    ");
        //         }
        //     } else if (i % 8 == 0 && i > 0) {
        //         printf(" ");
        //     }
        // }

        if (bytes[i] > 0) {
            printf("\x1b[44m%02x\x1b[0m", bytes[i]);
        } else {
            printf("%02x", bytes[i]);
        }

        if (stride_index < strides_count) {
            if (strides[stride_index] == i + 1) {
                printf("\n");
                stride_index++;
            } else {
                printf(" ");
            }
        } else {
            printf(" ");
        }
        // if (mark_mod > 0 && (i + 1) % mark_mod == 0 && (i + 1) > 0) {
        //     printf("|");
        // } else {
        //     printf(" ");
        // }
    }
}

int main() {
    OC_Default_Machine_Code_Writer w;
    memcpy(&w, &oc_default_machine_code_writer, sizeof(w));
    oc_x86_64_run_tests(&w.base);

    printf("%p - %u bytes written:\n", (void*)w.buffer, w.count);
    hex_dump(w.buffer, w.count, w.strides, w.strides_count); 

    FILE* f = fopen("x86_64_machine_code.bin", "wb");
    fwrite(w.buffer, 1, w.count, f);
    fclose(f);

    return 0;
}