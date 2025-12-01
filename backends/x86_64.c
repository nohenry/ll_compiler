
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
    X86_64_Locals_List parameters;

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
    X86_64_OPERAND_REGISTER_r12,
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
    // oc_x86_64_write_nop(w, 1);
    // oc_array_append_many(b->arena, &b->section_text, (uint8_t*)bytes, count);
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

void native_write(int u) {
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

static void x86_64_generate_load_cast(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand dst, LL_Ir_Operand src, bool load) {
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top];

    uint32_t opcode;
    X86_64_Variant_Kind kind;
    LL_Type* src_type;
    LL_Type* dst_type = ir_get_operand_type(b->fn, dst);

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        src_type = dst_type;
        break;
    default:
        src_type = ir_get_operand_type(b->fn, src);
        break;
    }

    if (dst_type != src_type) {
        switch (src_type->kind) {
        case LL_TYPE_UINT:
            switch (dst_type->kind) {
            case LL_TYPE_BOOL:
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
                if (src_type->width < dst_type->width) {
                    if (src_type->width <= 8) {
                        opcode = OPCODE_MOVZX;
                        if (dst_type->width <= 16) {
                            kind = X86_64_VARIANT_KIND_r16_rm8;
                            break;
                        } else if (dst_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm8;
                            break;
                        } else if (dst_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm8;
                            break;
                        }
                    } else if (src_type->width <= 16) {
                        opcode = OPCODE_MOVZX;
                        if (dst_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm16;
                            break;
                        } else if (dst_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm16;
                            break;
                        }
                    }
                }
                opcode = OPCODE_MOV;
                kind = x86_64_get_variant_raw(cc, b, bir, dst_type, (X86_64_Get_Variant_Params) { .mem_right = true });
                break;
            default: oc_todo(""); break;
            }
            break;
        case LL_TYPE_INT:
            switch (dst_type->kind) {
            case LL_TYPE_BOOL:
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
                if (src_type->width < dst_type->width) {
                    if (src_type->width <= 8) {
                        opcode = OPCODE_MOVSX;
                        if (dst_type->width <= 16) {
                            kind = X86_64_VARIANT_KIND_r16_rm8;
                            break;
                        } else if (dst_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm8;
                            break;
                        } else if (dst_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm8;
                            break;
                        }
                    } else if (src_type->width <= 16) {
                        opcode = OPCODE_MOVSX;
                        if (dst_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm16;
                            break;
                        } else if (dst_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm16;
                            break;
                        }
                    } else if (src_type->width <= 32) {
                        opcode = OPCODE_MOVSXD;
                        if (dst_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm32;
                            break;
                        }
                    }
                }
                opcode = OPCODE_MOV;
                kind = x86_64_get_variant_raw(cc, b, bir, dst_type, (X86_64_Get_Variant_Params) { .mem_right = true });
                break;
            default: oc_todo(""); break;
            }

            break;
        default: oc_todo(""); break;
        }
    }

    switch (OPD_TYPE(src)) {
        case LL_IR_OPERAND_REGISTER_BIT:
            oc_assert(!load && "load should only be used for locals, parameters, and immediates");
            if (src_type == dst_type) {
                b->registers.items[OPD_VALUE(dst)] = b->registers.items[OPD_VALUE(src)];
            } else {
                params.reg0 = reg;
                params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                params.displacement = -b->registers.items[OPD_VALUE(src)];
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);

                uword offset = x86_64_move_reg_to_stack(cc, b, bir, dst_type, reg);
                b->registers.items[OPD_VALUE(dst)] = offset;
            }
            break;
        case LL_IR_OPERAND_LOCAL_BIT:
            if (src_type == dst_type) {
                b->registers.items[OPD_VALUE(dst)] = b->locals.items[OPD_VALUE(src)];
            } else {
                params.reg0 = reg;
                params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                params.displacement = -b->locals.items[OPD_VALUE(src)];
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);

                uword offset = x86_64_move_reg_to_stack(cc, b, bir, dst_type, reg);
                b->registers.items[OPD_VALUE(dst)] = offset;
            }
            break;
        case LL_IR_OPERAND_PARMAETER_BIT:
            if (src_type == dst_type) {
                b->registers.items[OPD_VALUE(dst)] = b->parameters.items[OPD_VALUE(src)];
            } else {
                params.reg0 = reg;
                params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                params.displacement = -b->parameters.items[OPD_VALUE(src)];
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);

                uword offset = x86_64_move_reg_to_stack(cc, b, bir, dst_type, reg);
                b->registers.items[OPD_VALUE(dst)] = offset;
            }
            break;
        case LL_IR_OPERAND_IMMEDIATE_BIT:
            X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top];
            params.immediate = OPD_VALUE(src);
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant(dst_type, .immediate = true), params);

            uword offset = x86_64_move_reg_to_stack(cc, b, bir, dst_type, reg);
            b->registers.items[OPD_VALUE(dst)] = offset;
            break;
        default: oc_unreachable("unsupported operand"); break;
    }
}

static X86_64_Operand_Register x86_64_load_operand_with_type(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand operand, LL_Type* operand_type) {
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Get_Variant_Params get_variant = { 0 };
    X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top++];

    params.reg0 = reg;
    switch (OPD_TYPE(operand)) {
    case LL_IR_OPERAND_REGISTER_BIT:
        get_variant.mem_right = true;
        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = -b->registers.items[OPD_VALUE(operand)];
        break;
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        get_variant.immediate = true;
        params.immediate = OPD_VALUE(operand);
        break;
    case LL_IR_OPERAND_PARMAETER_BIT:
        get_variant.mem_right = true;
        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = -b->parameters.items[OPD_VALUE(operand)];
        break;
    default: oc_unreachable("unsupported type"); break;
    }

    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, operand_type, get_variant), params);

    return reg;
}

static X86_64_Operand_Register x86_64_load_operand(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand operand) {
    LL_Type* type = ir_get_operand_type(b->fn, operand);
    return x86_64_load_operand_with_type(cc, b, bir, operand, type);
}

static void x86_64_generate_block(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
    size_t i;
    int32_t opcode1;
    X86_64_Get_Variant_Params get_variant = { 0 };

    block->generated_offset = (int64_t)b->section_text.count;

    if (block->ref1) {
        int32_t* dst_offset = (int32_t*)&b->section_text.items[bir->blocks.items[block->ref1].fixup_offset];
        *dst_offset = (int32_t)(block->generated_offset - bir->blocks.items[block->ref1].fixup_offset - 4);
    }
    if (block->ref2) {
        int32_t* dst_offset = (int32_t*)&b->section_text.items[bir->blocks.items[block->ref2].fixup_offset];
        *dst_offset = (int32_t)(block->generated_offset - bir->blocks.items[block->ref2].fixup_offset - 4);
    }

    for (i = 0; i < block->ops.count; ) {
        LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
        LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];
        X86_64_Instruction_Parameters params = { 0 };
        uword invoke_offset = 0;

        switch (opcode) {
        case LL_IR_OPCODE_STORE: {
            x86_64_generate_mov(cc, b, bir, operands[0], operands[1], true);
        } break;
        case LL_IR_OPCODE_CAST: {
            x86_64_generate_load_cast(cc, b, bir, operands[0], operands[1], false);
        } break;
        case LL_IR_OPCODE_LOAD: {
            x86_64_generate_load_cast(cc, b, bir, operands[0], operands[1], true);
        } break;

        case LL_IR_OPCODE_AND:
            opcode1 = OPCODE_AND;
            goto DO_OPCODE_ARITHMETIC;
        case LL_IR_OPCODE_OR:
            opcode1 = OPCODE_OR;
            goto DO_OPCODE_ARITHMETIC;
        case LL_IR_OPCODE_XOR:
            opcode1 = OPCODE_XOR;
            goto DO_OPCODE_ARITHMETIC;
        case LL_IR_OPCODE_SUB:
            opcode1 = OPCODE_SUB;
            goto DO_OPCODE_ARITHMETIC;
        case LL_IR_OPCODE_ADD: {
            LL_Type* type;
            opcode1 = OPCODE_ADD;

DO_OPCODE_ARITHMETIC:
            type = ir_get_operand_type(b->fn, operands[0]);
            switch (type->kind) {
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT: {
                params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
                params.reg1 = x86_64_load_operand_with_type(cc, b, bir, operands[2], type);
                b->active_register_top -= 2;

                OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, x86_64_get_variant_raw(cc, b, bir, type, get_variant), params);

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

            params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
            params.reg1 = x86_64_load_operand_with_type(cc, b, bir, operands[2], type);
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

        } break;
        case LL_IR_OPCODE_DIV: {
            params.reg0 = X86_64_OPERAND_REGISTER_rdx;
            params.reg1 = X86_64_OPERAND_REGISTER_rdx;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_XOR, r64_rm64, params);
            LL_Type* type = ir_get_operand_type(b->fn, operands[0]);

            params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
            params.reg1 = x86_64_load_operand_with_type(cc, b, bir, operands[2], type);
            b->active_register_top -= 2;
            oc_assert(params.reg0 == X86_64_OPERAND_REGISTER_rax);

            params.reg0 = params.reg1;
            params.reg1 = 0;

            switch (type->kind) {
            case LL_TYPE_INT:
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_IDIV, x86_64_get_variant(type, .single = true), params);
                break;
            case LL_TYPE_UINT:
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_DIV, x86_64_get_variant(type, .single = true), params);
                break;
            default: oc_todo("implement other types"); break;
            }

            uword offset = x86_64_move_reg_to_stack(cc, b, bir, type, X86_64_OPERAND_REGISTER_rax);
            b->registers.items[OPD_VALUE(operands[0])] = offset;

        } break;

        case LL_IR_OPCODE_EQ:
            b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_NEQ:
            b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JNE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_GTE:
            b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JGE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_GT:
            b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JG;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_LTE:
            b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JLE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_LT: {
            b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JL;
            LL_Type* type;
DO_OPCODE_COMPARE:
            type = ir_get_operand_type(b->fn, operands[1]);
            switch (type->kind) {
            case LL_TYPE_ANYBOOL:
            case LL_TYPE_BOOL:
            case LL_TYPE_ANYINT:
            case LL_TYPE_UINT:
            case LL_TYPE_INT: {
                params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
                params.reg1 = x86_64_load_operand_with_type(cc, b, bir, operands[2], type);
                b->active_register_top -= 2;

                OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_CMP, x86_64_get_variant(type), params);
                break;
            }
            default: oc_todo("handle other type"); break;
            }
            break;
        }

        case LL_IR_OPCODE_NEG: {
            LL_Type* type;
            opcode1 = OPCODE_NEG;

DO_OPCODE_ARITHMETIC_PREOP:
            type = ir_get_operand_type(b->fn, operands[0]);

            switch (type->kind) {
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT: {
                params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
                b->active_register_top -= 1;

                OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, x86_64_get_variant(type, .single = true), params);

                uword offset = x86_64_move_reg_to_stack(cc, b, bir, type, params.reg0);
                b->registers.items[OPD_VALUE(operands[0])] = offset;
            } break;
            default: oc_todo("handle other type"); break;
            }
        } break;

        case LL_IR_OPCODE_BRANCH:
            if (bir->blocks.items[operands[0]].generated_offset != -1) {
                params.relative = bir->blocks.items[operands[0]].generated_offset - (int64_t)b->section_text.count;
                if (params.relative >= INT8_MIN && params.relative <= INT8_MAX) {
                    params.relative -= 2;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_JMP, rel8, params);
                } else {
                    params.relative -= 5;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_JMP, rel32, params);
                }
            } else {
                params.relative = 0;
                OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_JMP, rel32, params);
                block->fixup_offset = (int64_t)b->section_text.count - 4u;
            }
            break;
        case LL_IR_OPCODE_BRANCH_COND:
            params.relative = 0;
            if (operands[1] == block->next) {
                // then block is next

                OC_X86_64_WRITE_INSTRUCTION(b, oc_x86_64_get_inverse_compare(b->registers.items[OPD_VALUE(operands[0])]), rel32, params);
                bir->blocks.items[operands[1]].ref1 = 0;
                bir->blocks.items[operands[2]].ref1 = bir->blocks.items[operands[1]].prev;
            } else if (operands[2] == block->next) {
                // else block is next
                OC_X86_64_WRITE_INSTRUCTION(b, b->registers.items[OPD_VALUE(operands[0])], rel32, params);
                bir->blocks.items[operands[2]].ref1 = 0;
                bir->blocks.items[operands[1]].ref1 = bir->blocks.items[operands[2]].prev;
            } else oc_assert(false);
            block->fixup_offset = (int64_t)b->section_text.count - 4u;
            break;

        case LL_IR_OPCODE_INVOKEVALUE:
            invoke_offset = 1;
        case LL_IR_OPCODE_INVOKE: {
            uint8_t reg;
            uint32_t invokee = operands[invoke_offset++];
            uint32_t count = operands[invoke_offset++];
            X86_64_Call_Convention callconv = x86_64_call_convention_systemv(b);
            LL_Ir_Function* invokee_fn = &bir->fns.items[OPD_VALUE(invokee)];
            LL_Type_Function* fn_type = (LL_Type_Function*)invokee_fn->ident->base.type;
            assert(fn_type->base.kind == LL_TYPE_FUNCTION);

            get_variant.mem_right = true;
            for (uint32_t j = 0; j < count; ++j) {
                LL_Ir_Operand arg_operand = operands[invoke_offset++];
                switch (OPD_TYPE(arg_operand)) {
                    case LL_IR_OPERAND_REGISTER_BIT: 
                        reg = x86_64_call_convention_next_reg(b, &callconv);
                        LL_Type* type = ir_get_operand_type(b->fn, arg_operand);

                        if (reg == X86_64_OPERAND_REGISTER_invalid) {
                            int32_t offset = x86_64_call_convention_next_mem(b, &callconv);
                            params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
                            params.displacement = offset;
                            params.reg1 = b->registers.items[OPD_VALUE(arg_operand)];
                            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);
                        } else {
                            params.reg0 = reg;
                            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                            params.displacement = -b->registers.items[OPD_VALUE(arg_operand)];
                            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(cc, b, bir, type, get_variant), params);
                        }
                        break;
                    case LL_IR_OPERAND_IMMEDIATE_BIT:
                        reg = x86_64_call_convention_next_reg(b, &callconv);
                        if (reg == X86_64_OPERAND_REGISTER_invalid) {
                            int32_t offset = -x86_64_call_convention_next_mem(b, &callconv);
                            params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                            params.displacement = offset;
                            params.immediate = OPD_VALUE(arg_operand);
                            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_i32, params);
                        } else {
                            params.reg0 = reg;
                            params.immediate = OPD_VALUE(arg_operand);
                            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_i32, params);
                        }
                        break;
                    default: oc_todo("unhadnled argument operand: {x}", OPD_TYPE(arg_operand));
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

        } break;
        case LL_IR_OPCODE_RET:
            params.relative = 0;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_JMP, rel32, params);
            break;
        case LL_IR_OPCODE_RETVALUE:
            LL_Type_Function* fn_type = (LL_Type_Function*)b->fn->ident->base.type;
            params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[0], fn_type->return_type);
            assert(params.reg0 == X86_64_OPERAND_REGISTER_rax);
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




        // generate prologue
        size_t function_offset = b->section_text.count;
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));
        // OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbx }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rsp }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_SUB, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = 0xface0102 }));
        size_t stack_size_offset = b->section_text.count - 4;
        fn->generated_offset = (int64_t)function_offset;

        // move registers to stack, if already on stack just use that offset
        X86_64_Call_Convention callconv = x86_64_call_convention_systemv(b);
        LL_Type_Function* fn_type = (LL_Type_Function*)fn->ident->base.type;
        oc_array_reserve(&cc->tmp_arena, &b->parameters, fn_type->parameter_count);
        assert(fn_type->base.kind == LL_TYPE_FUNCTION);

        for (uint32_t j = 0; j < fn_type->parameter_count; ++j) {
            X86_64_Parameter param = x86_64_call_convention_next(b, &callconv);
            if (param.is_reg) {
                uword offset = x86_64_move_reg_to_stack(cc, b, bir, fn_type->parameters[j], param.reg);
                b->parameters.items[j] = offset;
            } else {
                b->parameters.items[j] = param.stack_offset;
            }
        }


        // generate body
        while (block) {
            x86_64_generate_block(cc, b, bir, &bir->blocks.items[block]);
            block = bir->blocks.items[block].next;
        }

        // generate epilogue
        uint32_t stack_used = oc_align_forward(b->stack_used, 16);
        int32_t* pstack_size = (int32_t*)&b->section_text.items[stack_size_offset];
        *pstack_size = stack_used;
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_ADD, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = stack_used }));
        // OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbx }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_RET, noarg, ((X86_64_Instruction_Parameters){ 0 }));

    }

    b->entry_index = fi - 1;
}

void x86_64_run(Compiler_Context* cc, X86_64_Backend* b) {
    (void)cc;
    uint8* code = VirtualAlloc(NULL, b->section_text.count, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!code) {
        eprint("Unable to allocate memory\n");
        return;
    }
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
    if (fopen_s(&fptr, filepath, "wb")) {
        eprint("Unable to open output file: %s\n", filepath);
        return false;
    }

    bool s =  fwrite(b->section_text.items, 1, b->section_text.count, fptr) == b->section_text.count;
    fclose(fptr);
    return s;
}
