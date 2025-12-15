#include "callconv.h"

const static X86_64_Operand_Register call_convention_registers_systemv[] = {
    X86_64_OPERAND_REGISTER_rcx,
    X86_64_OPERAND_REGISTER_rdx,
    X86_64_OPERAND_REGISTER_r8,
    X86_64_OPERAND_REGISTER_r9,
};

const static X86_64_Operand_Register call_convention_vector_registers_systemv[] = {
    X86_64_OPERAND_REGISTER_xmm(0),
    X86_64_OPERAND_REGISTER_xmm(1),
    X86_64_OPERAND_REGISTER_xmm(2),
    X86_64_OPERAND_REGISTER_xmm(3),
};


X86_64_Call_Convention x86_64_call_convention_systemv() {
    X86_64_Call_Convention result;
    result.registers = call_convention_registers_systemv;
    result.register_count = oc_len(call_convention_registers_systemv);
    result.register_next = 0;
    result.vector_registers = call_convention_vector_registers_systemv;
    result.vector_register_count = oc_len(call_convention_vector_registers_systemv);
    result.vector_register_next = 0;

    result.stack_offset = 0x20;
    return result;
}

X86_64_Operand_Register x86_64_call_convention_next_reg(X86_64_Call_Convention* cc, LL_Type* type) {
    if (type->kind == LL_TYPE_FLOAT) {
        if (cc->vector_register_next >= cc->vector_register_count) {
            return X86_64_OPERAND_REGISTER_invalid;
        } else {
            return cc->vector_registers[cc->vector_register_next++];
        }
    } else {
        if (cc->register_next >= cc->register_count) {
            return X86_64_OPERAND_REGISTER_invalid;
        } else {
            return cc->registers[cc->register_next++];
        }
    }
}

uint32_t x86_64_call_convention_next_mem(X86_64_Call_Convention* cc, LL_Type* type, uint32_t* stack_used_for_args) {
    (void)type;
    uint32_t stack_offset = cc->stack_offset;
    cc->stack_offset += 8;
    if (cc->stack_offset > *stack_used_for_args) {
        *stack_used_for_args = cc->stack_offset;
    }
    return stack_offset;
}

X86_64_Parameter x86_64_call_convention_next(X86_64_Call_Convention* cc, LL_Type* type, uint32_t* stack_used_for_args) {
    X86_64_Parameter result;
    result.reg = x86_64_call_convention_next_reg(cc, type);
    if (result.reg != X86_64_OPERAND_REGISTER_invalid) {
        result.is_reg = true;
    } else {
        result.is_reg = false;
        result.stack_offset = x86_64_call_convention_next_mem(cc, type, stack_used_for_args);
    }
    return result;
}

