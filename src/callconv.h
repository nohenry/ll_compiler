#pragma once
#include "../core/machine_code.h"
#include "typer.h"

typedef struct {
    const X86_64_Operand_Register* registers;
    const X86_64_Operand_Register* vector_registers;
    uint8 register_count, register_next;
    uint8 vector_register_count, vector_register_next;
    uint32 stack_offset;
} X86_64_Call_Convention;

typedef struct {
    bool is_reg;
    union {
        uint32 reg;
        sint32 stack_offset;
    };
} Call_Convention_Parameter;

typedef struct {
    sint64 immediate_displacement;
    uint32 opcode;
    uint32 reg0;
    uint32 variant;
} X86_64_Invoke_Prealloc;

typedef struct {
    uword count, capacity;
    X86_64_Invoke_Prealloc* items;
} X86_64_Invoke_Prealloc_List;

extern const X86_64_Operand_Register call_convention_registers_systemv[];
extern const X86_64_Operand_Register call_convention_vector_registers_systemv[];
// extern const X86_64_Operand_Register x86_64_backend_active_registers[];


X86_64_Call_Convention x86_64_call_convention_systemv(void);
uint32 x86_64_call_convention_next_reg(X86_64_Call_Convention* cc, LL_Type* type);
uint32 x86_64_call_convention_next_mem(X86_64_Call_Convention* cc, LL_Type* type, uint32* stack_used_for_args);
Call_Convention_Parameter x86_64_call_convention_next(X86_64_Call_Convention* cc, LL_Type* type, uint32* stack_used_for_args);


typedef struct {
    uint8 next_register;
    uint8 next_vector_register;
    uint32 stack_offset;
} AArch64_Call_Convention;

AArch64_Call_Convention aarch64_call_convention_systemv(void);
uint32 aarch64_call_convention_next_reg(AArch64_Call_Convention* cc, LL_Type* type);
uint32 aarch64_call_convention_next_mem(AArch64_Call_Convention* cc, LL_Type* type, uint32* stack_used_for_args);
Call_Convention_Parameter aarch64_call_convention_next(AArch64_Call_Convention* cc, LL_Type* type, uint32* stack_used_for_args);