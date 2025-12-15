#pragma once

#include "../src/typer.h"
#include "../src/backend.h"
#include "../core/machine_code.h"

typedef struct {
    bool immediate, mem_right, single, large_immediate;
} X86_64_Get_Variant_Params;

typedef struct {
    size_t count, capacity;
    uint8_t* items; // data/instructions
} X86_64_Section;

extern const X86_64_Operand_Register x86_64_backend_active_registers[];

X86_64_Variant_Kind x86_64_get_variant_raw(LL_Type* type, X86_64_Get_Variant_Params params);
LL_Backend_Layout x86_64_get_layout(LL_Type* ty);
