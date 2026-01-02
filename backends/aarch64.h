#pragma once

#include "../core/core1.h"
#include "../core/machine_code.h"

#define aarch64_w(n) (n & 0b11111)
#define aarch64_x(n) (n & 0b11111)
#define aarch64_v(n) (n & 0b11111)


static inline void aarch64_write_literal_int(OC_Machine_Code_Writer* w, uint8 rd, uint64 value) {
    AArch64_Instruction_Parameters params = { .rd = rd };

    uint32 opcode = AARCH64_OPCODE_MOVZ;
    do {
        params.immediate = value & 0xFFFFu;
        OC_AARCH64_WRITE_INSTRUCTION(w, opcode, xd_imm16_hw, params);
        params.shift += 1;
        value >>= 16;
        opcode = AARCH64_OPCODE_MOVK;
    } while (value);
}