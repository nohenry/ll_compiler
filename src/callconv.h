#pragma once
#include "../core/machine_code.h"
#include "typer.h"

typedef struct {
    const X86_64_Operand_Register* registers;
    const X86_64_Operand_Register* vector_registers;
    uint8_t register_count, register_next;
    uint8_t vector_register_count, vector_register_next;
    uint32_t stack_offset;
} X86_64_Call_Convention;

typedef struct {
    bool is_reg;
    union {
        X86_64_Operand_Register reg;
        int32_t stack_offset;
    };
} X86_64_Parameter;

typedef struct {
    int64_t immediate_displacement;
    uint32_t opcode;
    uint32_t reg0;
    X86_64_Variant_Kind variant;
} X86_64_Invoke_Prealloc;

typedef struct {
    size_t count, capacity;
    X86_64_Invoke_Prealloc* items;
} X86_64_Invoke_Prealloc_List;

extern const X86_64_Operand_Register call_convention_registers_systemv[];
extern const X86_64_Operand_Register call_convention_vector_registers_systemv[];
// extern const X86_64_Operand_Register x86_64_backend_active_registers[];


X86_64_Call_Convention x86_64_call_convention_systemv(void);
X86_64_Operand_Register x86_64_call_convention_next_reg(X86_64_Call_Convention* cc, LL_Type* type);
uint32_t x86_64_call_convention_next_mem(X86_64_Call_Convention* cc, LL_Type* type, uint32_t* stack_used_for_args);
X86_64_Parameter x86_64_call_convention_next(X86_64_Call_Convention* cc, LL_Type* type, uint32_t* stack_used_for_args);
