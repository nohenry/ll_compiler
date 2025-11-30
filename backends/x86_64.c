
#include "../src/elf.h"
#include "../src/backend.h"
#include "../src/common.h"
#include "../src/ast.h"
#include "../src/typer.h"

#include "../core/machine_code.h"

typedef struct {
    size_t count, capacity;
    uint8_t* items;
} X86_64_Ops_List;
;
typedef struct {
    size_t count, capacity;
    uint8_t* items; // data/instructions
} X86_64_Section;

typedef struct {
    size_t text_rel_byte_offset;
    uint32_t data_item;
} X86_64_Internal_Relocation;

typedef struct {
    size_t count, capacity;
    X86_64_Internal_Relocation* items;
} X86_64_Internal_Relocation_List;

typedef struct {
    size_t count, capacity;
    uint64_t* items;
} X86_64_Locals_List;

struct ll_native_function_map_entry {
    struct ll_native_function_map_entry* next;
    string name;
    void* ptr;
};

typedef struct {
    OC_Machine_Code_Writer w;
    LL_Backend_Ir* ir;
    X86_64_Ops_List ops;
    Oc_Arena* arena;

    LL_Ir_Function* fn;
    X86_64_Locals_List locals;
    X86_64_Locals_List registers;

    X86_64_Section section_text;
    X86_64_Section section_data;

    X86_64_Internal_Relocation_List internal_relocations;
    uint32_t stack_used;

    uint32_t active_register_top;

    struct ll_native_function_map_entry* native_funcs[LL_DEFAULT_MAP_ENTRY_COUNT];

    int entry_index;
} X86_64_Backend;

typedef struct {
    bool is_reg;
    union {
        X86_64_Operand_Register reg;
        int32_t stack_offset;
    };
} X86_64_Parameter;

typedef struct {
    const X86_64_Operand_Register* registers;
    uint8_t register_count, register_next;
    int32_t stack_offset;
} X86_64_Call_Convention;

const static X86_64_Operand_Register call_convention_registers_systemv[] = {
    X86_64_OPERAND_REGISTER_rcx,
    X86_64_OPERAND_REGISTER_rdx,
    X86_64_OPERAND_REGISTER_r8,
    X86_64_OPERAND_REGISTER_r9,
};

const static X86_64_Operand_Register x86_64_backend_active_registers[] = {
    X86_64_OPERAND_REGISTER_rax,
    X86_64_OPERAND_REGISTER_rbx,
};

X86_64_Call_Convention x86_64_call_convention_systemv(X86_64_Backend* b) {
    (void)b;
    X86_64_Call_Convention result;
    result.registers = call_convention_registers_systemv;
    result.register_count = oc_len(call_convention_registers_systemv);
    result.register_next = 0;
    result.stack_offset = 0;
    return result;
}

X86_64_Operand_Register x86_64_call_convention_next_reg(X86_64_Backend* b, X86_64_Call_Convention* cc) {
    (void)b;
    if (cc->register_next >= cc->register_count) {
        return X86_64_OPERAND_REGISTER_invalid;
    } else {
        return cc->registers[cc->register_next++];
    }
}

uint32_t x86_64_call_convention_next_mem(X86_64_Backend* b, X86_64_Call_Convention* cc) {
    uint32_t stack_offset = cc->stack_offset;
    cc->stack_offset += 8;
    b->stack_used += 8;
    return stack_offset;
}

X86_64_Parameter x86_64_call_convention_next(X86_64_Backend* b, X86_64_Call_Convention* cc) {
    X86_64_Parameter result;
    result.reg = x86_64_call_convention_next_reg(b, cc);
    if (result.reg != X86_64_OPERAND_REGISTER_invalid) {
        result.is_reg = true;
    } else {
        result.is_reg = false;
        result.stack_offset = x86_64_call_convention_next_mem (b, cc);
    }
    return result;
}

X86_64_Parameter x86_64_call_convention_nth_parameter(X86_64_Backend* b, X86_64_Call_Convention* cc, uint32_t n) {
    while (n > 0) {
        x86_64_call_convention_next(b, cc);
        n--;
    }
    return x86_64_call_convention_next(b, cc);
}


void x86_64_append_op_segment_u8(void* w, uint8_t segment) {
    X86_64_Backend* b = w;
    segment = AS_LITTLE_ENDIAN_U8(segment);
    oc_array_append(b->arena, &b->section_text, segment);
}

void x86_64_append_op_segment_u16(void* w, uint16_t segment) {
    X86_64_Backend* b = w;
    segment = AS_LITTLE_ENDIAN_U16(segment);
    oc_array_append_many(b->arena, &b->section_text, (uint8_t*)&segment, 2);
}

void x86_64_append_op_segment_u32(void* w, uint32_t segment) {
    X86_64_Backend* b = w;
    segment = AS_LITTLE_ENDIAN_U32(segment);
    oc_array_append_many(b->arena, &b->section_text, (uint8_t*)&segment, 4);
}

void x86_64_append_op_segment_u64(void* w, uint64_t segment) {
    X86_64_Backend* b = w;
    segment = AS_LITTLE_ENDIAN_U64(segment);
    oc_array_append_many(b->arena, &b->section_text, (uint8_t*)&segment, 8);
}

void x86_64_append_op_many(void* w, uint8_t* bytes, uint64_t count) {
    X86_64_Backend* b = w;
    oc_array_append_many(b->arena, &b->section_text, (uint8_t*)bytes, count);
}

void x86_64_end_instruction(void* w) {
    (void)w;
    // x86_64 *ww = (x86_64*)w;
    // default_code_writer_append_stride(ww, ww->count);
}

#include <stdio.h>
void x86_64_log_error(void* w, const char* fmt, ...) {
    (void)w;
    va_list f;
    va_start(f, fmt);
    vfprintf(stderr, fmt, f);
    va_end(f);
}

void x86_64_assert_abort(void* w, const char* fmt, ...) {
    (void)w;
    va_list f;
    va_start(f, fmt);
    vfprintf(stderr, fmt, f);
    va_end(f);
    __asm__("int3");
    oc_exit(-1);
    // abort();
}



void ll_native_fn_put(Compiler_Context* cc, X86_64_Backend* b, string name, void* ptr) {
    size_t hash = stbds_hash_string(name, MAP_DEFAULT_SEED) % oc_len(b->native_funcs);
    struct ll_native_function_map_entry* current = b->native_funcs[hash];
    struct ll_native_function_map_entry* new_entry = oc_arena_alloc(&cc->arena, sizeof(*new_entry));
    new_entry->next = current;
    new_entry->name = name;
    new_entry->ptr = ptr;
    b->native_funcs[hash] = new_entry;
}

void* ll_native_fn_get(Compiler_Context* cc, X86_64_Backend* b, string name) {
    (void)cc;
    size_t hash = stbds_hash_string(name, MAP_DEFAULT_SEED) % oc_len(b->native_funcs);
    struct ll_native_function_map_entry* current = b->native_funcs[hash];

    while (current) {
        if (string_eql(current->name, name)) break;
        current = current->next;
    }

    if (current) return current->ptr;
    return NULL;
}

void native_write(uword u) {
    print("{}\n", u);
}

void x86_64_backend_init(Compiler_Context* cc, X86_64_Backend* b) {
    b->arena = &cc->arena;
    b->w.append_u8 = x86_64_append_op_segment_u8;
    b->w.append_u16 = x86_64_append_op_segment_u16;
    b->w.append_u32 = x86_64_append_op_segment_u32;
    b->w.append_u64 = x86_64_append_op_segment_u64;
    b->w.append_many = x86_64_append_op_many;
    b->w.end_instruction = x86_64_end_instruction;
    b->w.log_error = x86_64_log_error;
    b->w.assert_abort = x86_64_assert_abort;
    memset(&b->ops, 0, sizeof(b->ops));
    memset(&b->internal_relocations, 0, sizeof(b->internal_relocations));


    ll_native_fn_put(cc, b, lit("write_int"), native_write);
}


LL_Backend_Layout x86_64_get_layout(LL_Type* ty) {
    LL_Backend_Layout sub_layout;
    switch (ty->kind) {
    case LL_TYPE_INT: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_UINT: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_FLOAT: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_POINTER: return (LL_Backend_Layout) { .size = 8, .alignment = 8 };
    case LL_TYPE_STRING: return (LL_Backend_Layout) { .size = 8, .alignment = 8 };
    case LL_TYPE_ARRAY: {
        sub_layout = x86_64_get_layout(((LL_Type_Array*)ty)->element_type);
        return (LL_Backend_Layout) { .size = max(sub_layout.size, sub_layout.alignment) * ty->width, .alignment = sub_layout.alignment };
    }
    default: return (LL_Backend_Layout) { .size = 0, .alignment = 1 };
    }
}

typedef struct {
    bool immediate, mem_right, single;
} X86_64_Get_Variant_Params;

static X86_64_Variant_Kind x86_64_get_variant_raw(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Type* type, X86_64_Get_Variant_Params params) {
    (void)cc;
    (void)b;
    (void)bir;
    oc_assert(!params.immediate || !params.mem_right);
    switch (type->kind) {
    case LL_TYPE_STRING:
    case LL_TYPE_POINTER:
        if (params.immediate) return X86_64_VARIANT_KIND_rm64_i32;
        else if (params.mem_right) return X86_64_VARIANT_KIND_r64_rm64;
        else return X86_64_VARIANT_KIND_rm64_r64;
    case LL_TYPE_BOOL:
    case LL_TYPE_UINT:
    case LL_TYPE_INT:
        if (type->width <= 8u) {
            if (params.immediate) return X86_64_VARIANT_KIND_rm8_i8;
            else if (params.mem_right) return X86_64_VARIANT_KIND_r8_rm8;
            else if (params.single) return X86_64_VARIANT_KIND_rm8;
            else return X86_64_VARIANT_KIND_rm8_r8;
        } else if (type->width <= 16u) {
            if (params.immediate) return X86_64_VARIANT_KIND_rm16_i16;
            else if (params.mem_right) return X86_64_VARIANT_KIND_r16_rm16;
            else if (params.single) return X86_64_VARIANT_KIND_rm16;
            else return X86_64_VARIANT_KIND_rm16_r16;
        } else if (type->width <= 32u) {
            if (params.immediate) return X86_64_VARIANT_KIND_rm32_i32;
            else if (params.mem_right) return X86_64_VARIANT_KIND_r32_rm32;
            else if (params.single) return X86_64_VARIANT_KIND_rm32;
            else return X86_64_VARIANT_KIND_rm32_r32;
        } else if (type->width <= 64u) {
            if (params.immediate) return X86_64_VARIANT_KIND_rm64_i32;
            else if (params.mem_right) return X86_64_VARIANT_KIND_r64_rm64;
            else if (params.single) return X86_64_VARIANT_KIND_rm64;
            else return X86_64_VARIANT_KIND_rm64_r64;
        }
    default: break;
    }
    asm("int3");
    oc_exit(-1);
    return (X86_64_Variant_Kind)-1;
}

#define x86_64_get_variant(type, ...) x86_64_get_variant_raw(cc, b, bir, (type), (X86_64_Get_Variant_Params) { __VA_ARGS__ })

static void x86_64_generate_mov_to_register(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Type* type, X86_64_Operand_Register result, LL_Ir_Operand src, bool store) {
    X86_64_Instruction_Parameters params = { 0 };
    params.reg0 = result;
    if (store) {
        params.reg0 |= X86_64_REG_BASE;
    }

    switch (type->kind) {
    case LL_TYPE_STRING:
    case LL_TYPE_ANYINT:
    case LL_TYPE_UINT:
    case LL_TYPE_INT: {
        /* oc_todo: max immeidiate is 28 bits */
        switch (OPD_TYPE(src)) {
        case LL_IR_OPERAND_IMMEDIATE_BIT:
            params.immediate = OPD_VALUE(src);

            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
            break;
        case LL_IR_OPERAND_REGISTER_BIT:
            params.reg1 = b->registers.items[OPD_VALUE(src)];
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, type, (X86_64_Get_Variant_Params) { 0 }), params);
            break;
        case LL_IR_OPERAND_LOCAL_BIT:
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -b->locals.items[OPD_VALUE(src)];
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_LEA, x86_64_get_variant_raw(cc, b, bir, type, (X86_64_Get_Variant_Params) { .mem_right = true }), params);
            break;
        case LL_IR_OPERAND_DATA_BIT:
            params.immediate = 0;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, params);
            oc_todo("handle relocation");
            // oc_array_append(&cc->tmp_arena, &b->relocations, ((Elf64_Rela) { .r_offset = b->current_section->count - 8, .r_info = ELF64_R_INFO(3, R_X86_64_64), .r_addend = bir->data_items.items[OPD_VALUE(src)].binary_offset }));
            break;
        default: oc_todo("handle other operands"); break;
        }
        break;
    }
    default: oc_todo("handle other types"); break;
    }
}

static void x86_64_generate_mov(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand result, LL_Ir_Operand src, bool store) {
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Instruction_Parameters src_params = { 0 };
    LL_Type* type = ir_get_operand_type(b->fn, result);
    switch (OPD_TYPE(result)) {
    case LL_IR_OPERAND_LOCAL_BIT: {
        params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = -b->locals.items[OPD_VALUE(result)];

        switch (type->kind) {
        case LL_TYPE_POINTER:
        case LL_TYPE_STRING:
        case LL_TYPE_UINT:
        case LL_TYPE_INT: {
            /* oc_todo: max immeidiate is 28 bits */
            switch (OPD_TYPE(src)) {
            case LL_IR_OPERAND_IMMEDIATE_BIT:
                params.immediate = OPD_VALUE(src);
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
                break;
            case LL_IR_OPERAND_REGISTER_BIT:
                src_params.reg0 = x86_64_backend_active_registers[b->active_register_top];
                src_params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                src_params.displacement = -b->registers.items[OPD_VALUE(src)];
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, type, (X86_64_Get_Variant_Params) { .mem_right = true }), src_params);

                params.reg1 = src_params.reg0;
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, type, (X86_64_Get_Variant_Params) { 0 }), params);
                break;
            case LL_IR_OPERAND_DATA_BIT: {
                /* oc_array_append(&cc->tmp_arena, &b->internal_relocations, ((Linux_x86_64_Internal_Relocation) {  })); */
                /* OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params); */
                /* OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, , params); */
                break;
            }
            case LL_IR_OPERAND_LOCAL_BIT:
                break;
            case LL_IR_OPERAND_PARMAETER_BIT: {
                break;
            }
            default: oc_todo("handle other operands"); break;
            }
            break;
        }
        default: oc_todo("handle other types"); break;
        }

        break;
    }
    case LL_IR_OPERAND_REGISTER_BIT:
        x86_64_generate_mov_to_register(cc, b, bir, type, b->registers.items[OPD_VALUE(result)], src, store);
        break;
    }
}

static void x86_64_generate_load_cast(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, X86_64_Operand_Register result_register, LL_Ir_Operand result, LL_Ir_Operand src, bool load) {
    uint32_t opcode;
    X86_64_Variant_Kind kind;
    LL_Type* from_type;
    X86_64_Instruction_Parameters params = { 0 };
    LL_Type* to_type = ir_get_operand_type(b->fn, result);

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        from_type = to_type;
        break;
    default:
        from_type = ir_get_operand_type(b->fn, src);
        break;
    }

    params.reg0 = result_register;

    if (to_type != from_type) {
        switch (from_type->kind) {
        case LL_TYPE_UINT:
            switch (to_type->kind) {
            case LL_TYPE_BOOL:
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
                if (from_type->width < to_type->width) {
                    if (from_type->width <= 8) {
                        opcode = OPCODE_MOVZX;
                        if (to_type->width <= 16) {
                            kind = X86_64_VARIANT_KIND_r16_rm8;
                            break;
                        } else if (to_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm8;
                            break;
                        } else if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm8;
                            break;
                        }
                    } else if (from_type->width <= 16) {
                        opcode = OPCODE_MOVZX;
                        if (to_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm16;
                            break;
                        } else if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm16;
                            break;
                        }
                    }
                }
                opcode = OPCODE_MOV;
                kind = x86_64_get_variant_raw(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .mem_right = true });
                break;
            default: oc_todo(""); break;
            }
            break;
        case LL_TYPE_INT:
            switch (to_type->kind) {
            case LL_TYPE_BOOL:
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
                if (from_type->width < to_type->width) {
                    if (from_type->width <= 8) {
                        opcode = OPCODE_MOVSX;
                        if (to_type->width <= 16) {
                            kind = X86_64_VARIANT_KIND_r16_rm8;
                            break;
                        } else if (to_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm8;
                            break;
                        } else if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm8;
                            break;
                        }
                    } else if (from_type->width <= 16) {
                        opcode = OPCODE_MOVSX;
                        if (to_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm16;
                            break;
                        } else if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm16;
                            break;
                        }
                    } else if (from_type->width <= 32) {
                        opcode = OPCODE_MOVSXD;
                        if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm32;
                            break;
                        }
                    }
                }
                opcode = OPCODE_MOV;
                kind = x86_64_get_variant_raw(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .mem_right = true });
                break;
            default: oc_todo(""); break;
            }

            break;
        default: oc_todo(""); break;
        }
    } else {
        opcode = OPCODE_MOV;
        kind = x86_64_get_variant_raw(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .mem_right = true });
    }

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_LOCAL_BIT: {
        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = -b->locals.items[OPD_VALUE(src)];
        OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);
    } break;
    case LL_IR_OPERAND_REGISTER_BIT: {
        if (load) {
            // pointer dereference
            params.reg1 = b->registers.items[OPD_VALUE(src)] | X86_64_REG_BASE;
        } else {
            // cast
            params.reg1 = b->registers.items[OPD_VALUE(src)];
        }
        OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);
    } break;
    case LL_IR_OPERAND_PARMAETER_BIT: {
        X86_64_Call_Convention conv = x86_64_call_convention_systemv(b);
        X86_64_Parameter parameter = x86_64_call_convention_nth_parameter(b, &conv, OPD_VALUE(src));

        if (parameter.is_reg) {
            params.reg1 = parameter.reg;
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);
        } else {
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = parameter.stack_offset + 16;
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);
        }
        break;
    }
    case LL_IR_OPERAND_IMMEDIATE_BIT: {
        params.immediate = OPD_VALUE(src);
        OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, x86_64_get_variant_raw(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
    } break;
    default: oc_todo("add load operands"); break;
    }
}

static uword x86_64_move_reg_to_stack(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Type* type, X86_64_Operand_Register reg) {
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Get_Variant_Params get_variant = { 0 };
    LL_Backend_Layout l = x86_64_get_layout(type);
    uword stride = max(l.size, l.alignment);
    uword offset = b->stack_used;
    b->stack_used = oc_align_forward(offset + stride, l.alignment);

    params.reg1 = reg;
    params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
    params.displacement = -b->stack_used;
    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, type, get_variant), params);

    return b->stack_used;
}

static X86_64_Operand_Register x86_64_load_operand(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand operand) {
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Get_Variant_Params get_variant = { .mem_right = true };
    X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top++];
    LL_Type* type = ir_get_operand_type(b->fn, operand);

    assert(OPD_TYPE(operand) == LL_IR_OPERAND_REGISTER_BIT);

    get_variant.mem_right = true;
    params.reg0 = reg;
    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
    params.displacement = -b->registers.items[OPD_VALUE(operand)];
    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, type, get_variant), params);

    return reg;
}

static void x86_64_generate_block(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
    size_t i;
    int32_t opcode1, opcode2;
    X86_64_Get_Variant_Params get_variant = { 0 };
    int offset = 0;

    for (i = 0; i < block->ops.count; ) {
        LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
        LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];
        X86_64_Instruction_Parameters params = { 0 };

        switch (opcode) {
        case LL_IR_OPCODE_STORE: {
            x86_64_generate_mov(cc, b, bir, operands[0], operands[1], true);
        } break;
        case LL_IR_OPCODE_CAST: {
            X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top++];
            x86_64_generate_load_cast(cc, b, bir, reg, operands[0], operands[1], true);
        } break;
        case LL_IR_OPCODE_LOAD: {
            // X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top++];
            // x86_64_generate_load_cast(cc, b, bir, reg, operands[0], operands[1], true);
            switch (OPD_TYPE(operands[1])) {
                case LL_IR_OPERAND_LOCAL_BIT:
                    b->registers.items[OPD_VALUE(operands[0])] = b->locals.items[OPD_VALUE(operands[1])];
                    break;
                case LL_IR_OPERAND_IMMEDIATE_BIT:
                    LL_Type* type = ir_get_operand_type(b->fn, operands[0]);
                    

                    X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top];
                    params.immediate = OPD_VALUE(operands[1]);
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant(type, .immediate = true), params);

                    uword offset = x86_64_move_reg_to_stack(cc, b, bir, type, reg);
                    b->registers.items[OPD_VALUE(operands[0])] = offset;
                    break;
                default: break;
            }
        } break;

        case LL_IR_OPCODE_SUB:
            opcode1 = OPCODE_SUB;
            opcode2 = OPCODE_DEC;
            goto DO_OPCODE_ARITHMETIC;
        case LL_IR_OPCODE_ADD: {
            LL_Type* type;
            opcode1 = OPCODE_ADD;
            opcode2 = OPCODE_INC;

DO_OPCODE_ARITHMETIC:
            type = ir_get_operand_type(b->fn, operands[0]);
            switch (type->kind) {
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT: {
                params.reg0 = x86_64_load_operand(cc, b, bir, operands[1]);
                params.reg1 = x86_64_load_operand(cc, b, bir, operands[2]);
                b->active_register_top -= 2;

                if (get_variant.immediate && params.immediate == 1) {
                    get_variant.immediate = false;
                    get_variant.single = true;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode2, x86_64_get_variant_raw(cc, b, bir, type, get_variant), params);
                } else {
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, x86_64_get_variant_raw(cc, b, bir, type, get_variant), params);
                }

                uword offset = x86_64_move_reg_to_stack(cc, b, bir, type, params.reg0);
                b->registers.items[OPD_VALUE(operands[0])] = offset;
            } break;
            default: oc_todo("handle other type"); break;
            }
        } break;
        case LL_IR_OPCODE_MUL: {
            params.reg0 = X86_64_OPERAND_REGISTER_rdx;
            params.reg1 = X86_64_OPERAND_REGISTER_rdx;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_XOR, r64_rm64, params);
            LL_Type* type = ir_get_operand_type(b->fn, operands[0]);

            params.reg0 = x86_64_load_operand(cc, b, bir, operands[1]);
            params.reg1 = x86_64_load_operand(cc, b, bir, operands[2]);
            b->active_register_top -= 2;
            oc_assert(params.reg0 == X86_64_OPERAND_REGISTER_rax);

            params.reg0 = params.reg1;
            params.reg1 = 0;

            switch (type->kind) {
            case LL_TYPE_INT:
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_IMUL, x86_64_get_variant(type, .single = true), params);
                break;
            case LL_TYPE_UINT:
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MUL, x86_64_get_variant(type, .single = true), params);
                break;
            default: oc_todo("implement other types"); break;
            }

            uword offset = x86_64_move_reg_to_stack(cc, b, bir, type, X86_64_OPERAND_REGISTER_rax);
            b->registers.items[OPD_VALUE(operands[0])] = offset;

            break;
        }

        case LL_IR_OPCODE_INVOKEVALUE:
            offset = 1;
        case LL_IR_OPCODE_INVOKE: {
            uint8_t reg;
            uint32_t invokee = operands[0 + offset];
            uint32_t count = operands[1 + offset];
            X86_64_Call_Convention callconv = x86_64_call_convention_systemv(b);
            LL_Type_Function* fn_type = (LL_Type_Function*)ir_get_operand_type(b->fn, invokee);
            assert(fn_type->base.kind == LL_TYPE_FUNCTION);

            get_variant.mem_right = true;
            for (uint32_t j = offset; j < count; ++j) {
                switch (OPD_TYPE(operands[2 + j])) {
                    case LL_IR_OPERAND_REGISTER_BIT: 
                        reg = x86_64_call_convention_next_reg(b, &callconv);
                        LL_Type* type = ir_get_operand_type(b->fn, operands[2 + j]);

                        if (reg == X86_64_OPERAND_REGISTER_invalid) {
                            int32_t offset = x86_64_call_convention_next_mem(b, &callconv);
                            params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
                            params.displacement = offset;
                            params.reg1 = b->registers.items[OPD_VALUE(operands[2 + j])];
                            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);
                        } else {
                            params.reg0 = reg;
                            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                            params.displacement = -b->registers.items[OPD_VALUE(operands[2 + j])];
                            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, type, get_variant), params);
                        }
                        break;
                    case LL_IR_OPERAND_IMMEDIATE_BIT:
                        reg = x86_64_call_convention_next_reg(b, &callconv);
                        if (reg == X86_64_OPERAND_REGISTER_invalid) {
                            int32_t offset = -x86_64_call_convention_next_mem(b, &callconv);
                            params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                            params.displacement = offset;
                            params.immediate = OPD_VALUE(operands[2 + j]);
                            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_i32, params);
                        } else {
                            params.reg0 = reg;
                            params.immediate = OPD_VALUE(operands[2 + j]);
                            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_i32, params);
                        }
                        break;
                    default: oc_todo("unhadnled argument operand");
                }
            }

            switch (OPD_TYPE(invokee)) {
            case LL_IR_OPERAND_FUNCTION_BIT: {
                LL_Ir_Function* fn = &bir->fns.items[OPD_VALUE(invokee)];

                if (fn->flags & LL_IR_FUNCTION_FLAG_EXTERN || fn->generated_offset == LL_IR_FUNCTION_OFFSET_INVALID) {
                    params.relative = 0;

                    // OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_CALL, rel32, params);
                    oc_todo("implement extern");
                } else {
                    params.relative = (int32_t)(fn->generated_offset - (int64_t)b->section_text.count) - 5;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_CALL, rel32, params);
                }
                break;
            }
            default: oc_todo("handle inveok type"); break;
            }

            if (opcode == LL_IR_OPCODE_INVOKEVALUE) {
                uword offset = x86_64_move_reg_to_stack(cc, b, bir, fn_type->return_type, X86_64_OPERAND_REGISTER_rax);
                b->registers.items[OPD_VALUE(operands[0])] = offset;
            }

            break;
        }
        case LL_IR_OPCODE_RET:
            params.relative = 0;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_JMP, rel32, params);
            break;
        default:
            print("implement op: {x}\n", opcode);
            break;
        }

        size_t count = ir_get_op_count(cc, bir, block->ops.items, i);
        i += count;
    }
}

void x86_64_backend_generate(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir) {
    b->ir = bir;
    size_t fi;
    // we skip fi 0 since it's used for constant evaluation

    for (fi = 1; fi < bir->data_items.count; ++fi) {
        bir->data_items.items[fi].binary_offset = b->section_data.count;
        oc_array_append_many(&cc->arena, &b->section_data, bir->data_items.items[fi].ptr, bir->data_items.items[fi].len);
        oc_array_append(&cc->arena, &b->section_data, 0);
    }

    for (fi = 1; fi < bir->fns.count; ++fi) {
        LL_Ir_Function* fn = &bir->fns.items[fi];
        LL_Ir_Block_Ref block = fn->entry;
        if (fn->flags & LL_IR_FUNCTION_FLAG_NATIVE) {
            size_t function_offset = b->section_text.count;
            fn->generated_offset = (int64_t)function_offset;

            void* native_fn_ptr = ll_native_fn_get(cc, b, fn->ident->str);
            if (!native_fn_ptr) {
                eprint("Unable to find native function: {}\n", fn->ident->str);
                continue;
            }

            b->fn = fn;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rax, .immediate = (uword)native_fn_ptr }));
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_JMP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rax }));
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_INT3, noarg, ((X86_64_Instruction_Parameters){}));

            continue;
        }
        if (fn->flags & LL_IR_FUNCTION_FLAG_EXTERN) continue;
        b->fn = fn;
        b->active_register_top = 0;

        // allocate locals
        oc_array_reserve(&cc->tmp_arena, &b->locals, fn->locals.count);
        oc_array_reserve(&cc->tmp_arena, &b->registers, fn->registers.count);
        uint64_t offset = 0;
        for (uint32_t li = 0; li < fn->locals.count; ++li) {
            LL_Backend_Layout l = x86_64_get_layout(fn->locals.items[li].ident->base.type);
            offset = oc_align_forward(offset + max(l.size, l.alignment), l.alignment);
            b->locals.items[li] = offset;
        }
        b->stack_used = (uint32_t)offset;




        // generate body
        size_t function_offset = b->section_text.count;
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rsp }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_SUB, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = 0 }));
        size_t stack_size_offset = b->section_text.count - 4;
        fn->generated_offset = (int64_t)function_offset;

        while (block) {
            x86_64_generate_block(cc, b, bir, &bir->blocks.items[block]);
            block = bir->blocks.items[block].next;
        }

        uint32_t stack_used = oc_align_forward(b->stack_used, 16);
        int32_t* pstack_size = (int32_t*)&b->section_text.items[stack_size_offset];
        *pstack_size = stack_used;
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_ADD, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = stack_used }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_RET, noarg, ((X86_64_Instruction_Parameters){ 0 }));

    }

    b->entry_index = fi - 1;
}

void x86_64_run(Compiler_Context* cc, X86_64_Backend* b) {
    (void)cc;
    uint8* code = VirtualAlloc(NULL, b->section_text.count, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    memcpy(code, b->section_text.items, b->section_text.count);
    sint32 a;
    if (!VirtualProtect(code, b->section_text.count, PAGE_EXECUTE, &a)) {
        extern sint32 GetLastError();
        eprint("Unable to change protection: {x}\n", GetLastError());
        return;
    }

    void (*fn_ptr)() = (void (*)())(code + b->ir->fns.items[b->entry_index].generated_offset);
    fn_ptr();
}

bool x86_64_write_to_file(Compiler_Context* cc, X86_64_Backend* b, char* filepath) {
	if (!cc->quiet) {
		print("text: \n");
		oc_hex_dump(b->section_text.items, b->section_text.count, 1, -1);
		print("data: \n");
		oc_hex_dump(b->section_data.items, b->section_data.count, 1, -1);
	}


    FILE* fptr;
    if (fopen_s(&fptr, filepath, "w")) {
        eprint("Unable to open output file: %s\n", filepath);
        return false;
    }

    bool s =  fwrite(b->section_text.items, 1, b->section_text.count, fptr) == b->section_text.count;
    fclose(fptr);
    return s;
}
