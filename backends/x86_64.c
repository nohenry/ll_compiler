
#include "../src/elf.h"
#include "../src/backend.h"
#include "../src/common.h"
#include "../src/ast.h"
#include "../src/typer.h"
#include "../src/callconv.h"

#include "../core/machine_code.h"

#include "x86_64.h"

typedef struct {
    size_t count, capacity;
    uint8_t* items;
} X86_64_Ops_List;

typedef struct {
    LL_Ir_Block_Ref branch_from;
    LL_Ir_Block_Ref branch_to;
} X86_64_Branch_Relocation;

typedef struct {
    size_t count, capacity;
    X86_64_Branch_Relocation* items;
} X86_64_Branch_Relocation_List;

typedef struct {
    size_t text_rel_byte_offset;
    uint32_t data_item;
} X86_64_Internal_Relocation;

typedef struct {
    size_t count, capacity;
    X86_64_Internal_Relocation* items;
} X86_64_Internal_Relocation_List;

typedef struct {
    size_t text_rel_byte_offset;
    uint32_t fn_index;
} X86_64_Function_Relocation;

typedef struct {
    size_t count, capacity;
    X86_64_Function_Relocation* items;
} X86_64_Function_Relocation_List;

typedef struct {
    size_t count, capacity;
    uint64_t* items;
} X86_64_Locals_List;

typedef uint32_t X86_64_Register_Location;
enum {
    X86_64_REGISTER_LOCATION_REGISTER,
    X86_64_REGISTER_LOCATION_STACK,
    X86_64_REGISTER_LOCATION_FLAGS,
};

typedef struct {
    X86_64_Register_Location location;
    uint32_t value;
#ifdef _DEBUG
    uint32_t usedup;
#endif
} X86_64_Register;

typedef struct {
    size_t count, capacity;
    X86_64_Register* items;
} X86_64_Registers_List;

typedef struct {
    OC_Machine_Code_Writer w;
    LL_Backend_Ir* ir;
    X86_64_Ops_List ops;
    Oc_Arena* arena;

    LL_Ir_Function* fn;
    X86_64_Locals_List locals;
    X86_64_Registers_List registers;
    X86_64_Locals_List parameters;

    X86_64_Section section_text;
    X86_64_Section section_data;

    X86_64_Branch_Relocation_List branch_relocations;
    X86_64_Internal_Relocation_List internal_relocations;
    X86_64_Function_Relocation_List fn_relocations;
    uint32_t stack_used, stack_used_for_args;

    uint32_t active_register_top;
    X86_64_Invoke_Prealloc_List invoke_prealloc;

    // struct ll_native_function_map_entry* native_funcs[LL_DEFAULT_MAP_ENTRY_COUNT];
    LL_Native_Function_Map native_funcs;

    uword indirect_return_ptr_offset;
    bool indirect_return_type;
    int entry_index;
} X86_64_Backend;

const X86_64_Operand_Register x86_64_backend_active_registers[] = {
    X86_64_OPERAND_REGISTER_rax,
    X86_64_OPERAND_REGISTER_rcx,
    X86_64_OPERAND_REGISTER_rbx,
    X86_64_OPERAND_REGISTER_r10,
    X86_64_OPERAND_REGISTER_r11,
    X86_64_OPERAND_REGISTER_r12,
    X86_64_OPERAND_REGISTER_r13,
    X86_64_OPERAND_REGISTER_r14,
    X86_64_OPERAND_REGISTER_r15,
};


#define x86_64_alloc_tmp_regs(n) (b->active_register_top += (n), b->active_register_top - (n))
#define x86_64_tmp_regs(rg, i) (x86_64_backend_active_registers[(rg) + (i)])
#define x86_64_free_tmp_regs(rg) (b->active_register_top = (rg))
#define push_regs(_name, n) for (uint32_t _name = x86_64_alloc_tmp_regs(n), _name ## _ii = 1; _name ## _ii; (_name ## _ii --, x86_64_free_tmp_regs(_name)))

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

    oc_breakpoint();
    oc_exit(-1);
    // abort();
}

void x86_64_backend_init(Compiler_Context* cc, X86_64_Backend* b) {
    b->arena = &cc->arena;
    b->w.append_u8 = x86_64_append_op_segment_u8;
    b->w.append_u16 = x86_64_append_op_segment_u16;
    b->w.append_u32 = x86_64_append_op_segment_u32;
    b->w.append_u64 = (void (*)(void *, unsigned long long))x86_64_append_op_segment_u64;
    b->w.append_many = (void (*)(void *, unsigned char *, unsigned long long))x86_64_append_op_many;
    b->w.end_instruction = x86_64_end_instruction;
    b->w.log_error = x86_64_log_error;
    b->w.assert_abort = x86_64_assert_abort;
    memset(&b->ops, 0, sizeof(b->ops));
    memset(&b->internal_relocations, 0, sizeof(b->internal_relocations));
    memset(&b->branch_relocations, 0, sizeof(b->branch_relocations));
    memset(&b->fn_relocations, 0, sizeof(b->fn_relocations));


    ll_native_fn_put(cc, &b->native_funcs, lit("write_int"), native_write);
    ll_native_fn_put(cc, &b->native_funcs, lit("write_float32"), native_write_float32);
    ll_native_fn_put(cc, &b->native_funcs, lit("write_float64"), native_write_float64);
    ll_native_fn_put(cc, &b->native_funcs, lit("write_string"), native_write_string);
    ll_native_fn_put(cc, &b->native_funcs, lit("write_many"), native_write_many);
    ll_native_fn_put(cc, &b->native_funcs, lit("read_entire_file"), native_read_entire_file);
    ll_native_fn_put(cc, &b->native_funcs, lit("malloc"), native_malloc);
    ll_native_fn_put(cc, &b->native_funcs, lit("realloc"), native_realloc);
    ll_native_fn_put(cc, &b->native_funcs, lit("breakpoint"), native_breakpoint);
}


LL_Backend_Layout x86_64_get_layout(LL_Type* ty) {
    LL_Backend_Layout sub_layout;
    switch (ty->kind) {
    case LL_TYPE_INT: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_UINT: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_CHAR: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_FLOAT: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_POINTER: return (LL_Backend_Layout) { .size = 8, .alignment = 8 };
    case LL_TYPE_ARRAY: {
        sub_layout = x86_64_get_layout(((LL_Type_Array*)ty)->element_type);
        return (LL_Backend_Layout) { .size = max(sub_layout.size, sub_layout.alignment) * ty->width, .alignment = sub_layout.alignment };
    } break;
    case LL_TYPE_STRING:
    case LL_TYPE_SLICE: {
        return (LL_Backend_Layout) { .size = 16, .alignment = 8 };
    } break;
    case LL_TYPE_STRUCT: {
        LL_Type_Struct* struct_type = (LL_Type_Struct*)ty;
        ir_calculate_struct_offsets(ty);
        if (struct_type->field_count == 0) return (LL_Backend_Layout) { .size = 0, .alignment = 1 };

        sub_layout = x86_64_get_layout(struct_type->fields[struct_type->field_count - 1]);
        size_t size = struct_type->offsets[struct_type->field_count - 1] + max(sub_layout.size, sub_layout.alignment);
        size = oc_align_forward(size, struct_type->base.struct_alignment);

        return (LL_Backend_Layout) { .size = size, .alignment = struct_type->base.struct_alignment };
    } break;
    case LL_TYPE_NAMED: {
        return x86_64_get_layout(((LL_Type_Named*)ty)->actual_type);
    } break;
    default: return (LL_Backend_Layout) { .size = 0, .alignment = 1 };
    }
}

static inline bool is_small_struct(LL_Type* type) {
    assert(type->kind == LL_TYPE_STRUCT);
    LL_Backend_Layout struct_layout = x86_64_get_layout(type);
    size_t actual_size = max(struct_layout.size, struct_layout.alignment);
    return actual_size <= 8;
}

static inline bool is_large_aggregate_type(LL_Type* type) {
    switch (type->kind) {
    case LL_TYPE_STRUCT:
    case LL_TYPE_ARRAY: {
        LL_Backend_Layout struct_layout = x86_64_get_layout(type);
        size_t actual_size = max(struct_layout.size, struct_layout.alignment);
        return actual_size > 8;
    } break;
    case LL_TYPE_SLICE:
    case LL_TYPE_STRING:
        return true;
    default:
        return false;
    }
}

X86_64_Variant_Kind x86_64_get_variant_raw(LL_Type* type, X86_64_Get_Variant_Params params) {
    oc_assert(!params.immediate || !params.mem_right);

    if (type->kind == LL_TYPE_NAMED) {
        type = ((LL_Type_Named*)type)->actual_type;
    }

    switch (type->kind) {
    case LL_TYPE_POINTER:
        if (params.immediate) return X86_64_VARIANT_KIND_rm64_i32;
        else if (params.mem_right) return X86_64_VARIANT_KIND_r64_rm64;
        else return X86_64_VARIANT_KIND_rm64_r64;
    case LL_TYPE_STRING:
    case LL_TYPE_SLICE:
        // slice is like struct, but it's always > 64bits
        return X86_64_VARIANT_KIND_r64_rm64;
    case LL_TYPE_ARRAY:
    case LL_TYPE_STRUCT: {
        LL_Backend_Layout layout = x86_64_get_layout(type);
        size_t size = max(layout.size, layout.alignment);
        if (size <= 64u) {
            if (params.immediate) return X86_64_VARIANT_KIND_rm64_i32;
            else if (params.mem_right) return X86_64_VARIANT_KIND_r64_rm64;
            else if (params.single) return X86_64_VARIANT_KIND_rm64;
            else return X86_64_VARIANT_KIND_rm64_r64;
        } else {
            return X86_64_VARIANT_KIND_r64_rm64;
        }
    } break;
    case LL_TYPE_BOOL:
    case LL_TYPE_CHAR:
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
            if (params.large_immediate) return X86_64_VARIANT_KIND_r64_i64;
            else if (params.immediate) return X86_64_VARIANT_KIND_rm64_i32;
            else if (params.mem_right) return X86_64_VARIANT_KIND_r64_rm64;
            else if (params.single) return X86_64_VARIANT_KIND_rm64;
            else return X86_64_VARIANT_KIND_rm64_r64;
        }
    case LL_TYPE_FLOAT:
        if (type->width <= 32u) {
            if (params.immediate) oc_assert(false);
            else if (params.mem_right) return X86_64_VARIANT_KIND_r128_rm128;
            else if (params.single) oc_assert(false);
            else return X86_64_VARIANT_KIND_rm128_r128;
        } else if (type->width <= 64u) {
            if (params.large_immediate) oc_assert(false);
            else if (params.immediate) oc_assert(false);
            else if (params.mem_right) return X86_64_VARIANT_KIND_r128_rm128;
            else if (params.single) oc_assert(false);
            else return X86_64_VARIANT_KIND_rm128_r128;
        }

    default: break;
    }
    __builtin_debugtrap();
    oc_exit(-1);
    return (X86_64_Variant_Kind)-1;
}

#define x86_64_get_variant(type, ...) x86_64_get_variant_raw((type), (X86_64_Get_Variant_Params) { __VA_ARGS__ })

static void x86_64_useup_register(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, X86_64_Register* reg) {
    (void)cc;
    (void)b;
    (void)bir;
#ifndef _NDEBUG
    oc_assert(!reg->usedup);
    reg->usedup = 1;
#endif

    switch (reg->location) {
    case X86_64_REGISTER_LOCATION_REGISTER:
        oc_todo("implement register alloc");
        // print("{} {}\n", reg->value, x86_64_backend_active_registers[b->active_register_top]);
        // oc_assert(reg->value == (uint32_t)x86_64_backend_active_registers[b->active_register_top]);
        break;
    case X86_64_REGISTER_LOCATION_STACK:
        break;
    case X86_64_REGISTER_LOCATION_FLAGS:
        break;
    default: oc_todo(""); break;
    }
}

static X86_64_Operand_Register x86_64_load_register(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, X86_64_Operand_Register dst_hint, X86_64_Register* src, LL_Type* operand_type) {
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Get_Variant_Params get_variant = { .mem_right = true };

    if ((int32_t)dst_hint < 0) {
        params.reg0 = x86_64_backend_active_registers[b->active_register_top++];
    } else {
        params.reg0 = dst_hint;
    }
    x86_64_useup_register(cc, b, bir, src);
    switch (src->location) {
    case X86_64_REGISTER_LOCATION_REGISTER: {
        oc_todo("implement register alloc");
        if (src->value != (uint32_t)dst_hint) {
            params.reg1 = src->value;
        }
    } break;
    case X86_64_REGISTER_LOCATION_STACK: {
        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = -(int64_t)(int32_t)src->value;
    } break;
    default: oc_todo(""); break;
    }

    operand_type = ll_get_base_type(operand_type);

    switch (operand_type->kind) {
    case LL_TYPE_FLOAT:
        if (operand_type->width <= 32)
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOVSS, x86_64_get_variant_raw(operand_type, get_variant), params);
        else if (operand_type->width <= 64)
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOVSD, x86_64_get_variant_raw(operand_type, get_variant), params);
        else oc_todo("implement other sizes");
        break;
    case LL_TYPE_BOOL:
    case LL_TYPE_CHAR:
    case LL_TYPE_UINT:
    case LL_TYPE_INT:
    case LL_TYPE_POINTER:
    case LL_TYPE_STRING:
    case LL_TYPE_ARRAY:
    case LL_TYPE_STRUCT:
    case LL_TYPE_SLICE:
        OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(operand_type, get_variant), params);
        break;
    default: oc_todo("add types");
    }

    return params.reg0;
}

static X86_64_Operand_Register x86_64_load_operand_with_type(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand operand, LL_Type* operand_type) {
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Get_Variant_Params get_variant = { 0 };
    X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top++];

    params.reg0 = reg;
    switch (OPD_TYPE(operand)) {
    case LL_IR_OPERAND_REGISTER_BIT:
        get_variant.mem_right = true;
        X86_64_Register src_reg = b->registers.items[OPD_VALUE(operand)];
        x86_64_useup_register(cc, b, bir, &b->registers.items[OPD_VALUE(operand)]);

        switch (src_reg.location) {
        case X86_64_REGISTER_LOCATION_STACK: {
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -(int64_t)(int32_t)src_reg.value;
        } break;
        case X86_64_REGISTER_LOCATION_REGISTER: {
            oc_todo("flskdjf");
            return src_reg.value;
        } break;
        case X86_64_REGISTER_LOCATION_FLAGS: {
            switch (src_reg.value) {
            case OPCODE_JO:  src_reg.value = OPCODE_SETO; break;
            case OPCODE_JNO: src_reg.value = OPCODE_SETNO; break;
            case OPCODE_JB:  src_reg.value = OPCODE_SETB; break;
            case OPCODE_JAE: src_reg.value = OPCODE_SETAE; break;
            case OPCODE_JE:  src_reg.value = OPCODE_SETE; break;
            case OPCODE_JNE: src_reg.value = OPCODE_SETNE; break;
            case OPCODE_JBE: src_reg.value = OPCODE_SETBE; break;
            case OPCODE_JA:  src_reg.value = OPCODE_SETA; break;
            case OPCODE_JS:  src_reg.value = OPCODE_SETS; break;
            case OPCODE_JNS: src_reg.value = OPCODE_SETNS; break;
            case OPCODE_JPE: src_reg.value = OPCODE_SETPE; break;
            case OPCODE_JPO: src_reg.value = OPCODE_SETPO; break;
            case OPCODE_JL:  src_reg.value = OPCODE_SETL; break;
            case OPCODE_JGE: src_reg.value = OPCODE_SETGE; break;
            case OPCODE_JLE: src_reg.value = OPCODE_SETLE; break;
            case OPCODE_JG:  src_reg.value = OPCODE_SETG; break;
            default: oc_todo("unhandled opcode");
            }
            OC_X86_64_WRITE_INSTRUCTION(b, src_reg.value, rm8, params);
            return reg;
        } break;
        default: oc_todo(""); break;
        }
        break;
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        get_variant.immediate = true;
        params.immediate = OPD_VALUE(operand);
        break;
    case LL_IR_OPERAND_IMMEDIATE64_BIT:
        get_variant.large_immediate = true;
        params.immediate = b->fn->literals.items[OPD_VALUE(operand)].as_u64;
        break;
    case LL_IR_OPERAND_PARMAETER_BIT:
        get_variant.mem_right = true;
        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = -b->parameters.items[OPD_VALUE(operand)];
        break;
    default: oc_unreachable("unsupported type"); break;
    }

    operand_type = ll_get_base_type(operand_type);

    switch (operand_type->kind) {
    case LL_TYPE_FLOAT:
        if (operand_type->width <= 32)
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOVSS, x86_64_get_variant_raw(operand_type, get_variant), params);
        else if (operand_type->width <= 64)
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOVSD, x86_64_get_variant_raw(operand_type, get_variant), params);
        else oc_todo("implement other sizes");
        break;
    case LL_TYPE_BOOL:
    case LL_TYPE_CHAR:
    case LL_TYPE_UINT:
    case LL_TYPE_INT:
    case LL_TYPE_POINTER:
    case LL_TYPE_STRING:
    case LL_TYPE_ARRAY:
    case LL_TYPE_STRUCT:
    case LL_TYPE_SLICE:
        OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(operand_type, get_variant), params);
        break;
    default: oc_todo("add types");
    }

    return reg;
}

static X86_64_Operand_Register x86_64_load_operand(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand operand) {
    LL_Type* type = ir_get_operand_type(bir, b->fn, operand);
    return x86_64_load_operand_with_type(cc, b, bir, operand, type);
}

static void x86_64_generate_mov_to_register(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Type* type, X86_64_Operand_Register result, LL_Ir_Operand src, bool store) {
    X86_64_Instruction_Parameters params = { 0 };
    params.reg0 = result;
    if (store) {
        params.reg0 |= X86_64_REG_BASE;
        assert(type->kind == LL_TYPE_POINTER);
        type = ((LL_Type_Pointer*)type)->element_type;
    }


    if (type->kind == LL_TYPE_NAMED) {
        type = ((LL_Type_Named*)type)->actual_type;
    }

    uint32_t opcode = OPCODE_MOV;

    switch (type->kind) {
    case LL_TYPE_FLOAT:
        if (type->width <= 32)
            opcode = OPCODE_MOVSS;
        else if (type->width <= 64)
            opcode = OPCODE_MOVSD;
        else oc_todo("implement other sizes");
        // fallthrough
    case LL_TYPE_STRING:
    case LL_TYPE_ANYINT:
    case LL_TYPE_CHAR:
    case LL_TYPE_UINT:
    case LL_TYPE_INT: {
        /* oc_todo: max immeidiate is 28 bits */
        switch (OPD_TYPE(src)) {
        case LL_IR_OPERAND_IMMEDIATE_BIT:
            params.immediate = OPD_VALUE(src);
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, x86_64_get_variant_raw(type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
            break;
        case LL_IR_OPERAND_IMMEDIATE64_BIT:
            params.immediate = b->fn->literals.items[OPD_VALUE(src)].as_u64;
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, x86_64_get_variant_raw(type, (X86_64_Get_Variant_Params) { .immediate = true, .large_immediate = true }), params);
            break;
        case LL_IR_OPERAND_REGISTER_BIT: {
            X86_64_Register* src_reg = &b->registers.items[OPD_VALUE(src)];
            x86_64_useup_register(cc, b, bir, src_reg);

            switch (src_reg->location) {
            case X86_64_REGISTER_LOCATION_STACK: {
                params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                params.displacement = -(int64_t)(int32_t)src_reg->value;
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, x86_64_get_variant_raw(type, (X86_64_Get_Variant_Params) { .mem_right = true }), params);
            } break;
            case X86_64_REGISTER_LOCATION_REGISTER: {
                if (src_reg->value != (uint32_t)result) {
                    params.reg1 = src_reg->value;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, x86_64_get_variant_raw(type, (X86_64_Get_Variant_Params) { .mem_right = true }), params);
                }
            } break;
            default: oc_todo(""); break;
            }
        } break;
            break;
        case LL_IR_OPERAND_LOCAL_BIT:
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -(int64_t)b->locals.items[OPD_VALUE(src)];
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
            break;
        case LL_IR_OPERAND_PARMAETER_BIT:
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -(int64_t)b->parameters.items[OPD_VALUE(src)];
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
            break;
        case LL_IR_OPERAND_DATA_BIT:
            params.immediate = 0;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, params);

            oc_array_append(&cc->tmp_arena, &b->internal_relocations, ((X86_64_Internal_Relocation) {
                .data_item = OPD_VALUE(src),
                .text_rel_byte_offset = b->section_text.count - 8 /* sizeof displacement */
            }));
            break;
        default: oc_todo("handle other operands"); break;
        }
        break;
    }
    default: ll_print_type(type); oc_todo("handle other types"); break;
    }
}

static void x86_64_generate_memcpy_assum_rdi_rsi(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Type* type, LL_Ir_Operand byte_size, LL_Type* operand_type) {
    X86_64_Instruction_Parameters params = { 0 };
    // X86_64_Get_Variant_Params get_variant = { 0 };
    LL_Backend_Layout elem_layout = x86_64_get_layout(type);

    size_t alignment = min(8, elem_layout.alignment);

    LL_Ir_Operand size_value;
    switch (OPD_TYPE(byte_size)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        size_value = OPD_VALUE(byte_size) / alignment;
        x86_64_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rcx, size_value, false);
        break;
    case LL_IR_OPERAND_IMMEDIATE64_BIT:
        size_value = b->fn->literals.items[OPD_VALUE(byte_size)].as_u64 / alignment;
        x86_64_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rcx, size_value, false);
        break;
    case LL_IR_OPERAND_REGISTER_BIT: {
        x86_64_load_register(cc, b, bir, X86_64_OPERAND_REGISTER_rcx, &b->registers.items[OPD_VALUE(byte_size)], operand_type);

        b->active_register_top -= 1;
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_SHR, rm64_i8, ((X86_64_Instruction_Parameters) {
            .reg0 = params.reg0,
            .immediate = log2_u32(alignment << 3)
        }));
    } break;
    default:
        x86_64_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rcx, byte_size, true);
        break;
    }

    const static int opcodes[] = { [1] = OPCODE_MOVSB, [2] = OPCODE_MOVSW, [4] = OPCODE_MOVSD, [8] = OPCODE_MOVSQ };
    OC_X86_64_WRITE_INSTRUCTION(b, opcodes[alignment], noarg, ((X86_64_Instruction_Parameters) { .rep = X86_64_PREFIX_REP }) );
}

static void x86_64_generate_memcpy(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand dst, LL_Ir_Operand src, LL_Ir_Operand byte_size, LL_Type* operand_type) {
    LL_Type* type = ir_get_operand_type(bir, b->fn, dst);

    x86_64_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rdi, dst, false);
    x86_64_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rsi, src, false);
    x86_64_generate_memcpy_assum_rdi_rsi(cc, b, bir, type, byte_size, operand_type);
}

uword x86_64_make_struct_copy(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Type* type, LL_Ir_Operand reg, bool allocate_only) {
    X86_64_Instruction_Parameters params = { 0 };
    oc_assert(type->kind == LL_TYPE_ARRAY || type->kind == LL_TYPE_STRUCT || type->kind == LL_TYPE_SLICE || type->kind == LL_TYPE_STRING);

    LL_Backend_Layout l = x86_64_get_layout(type);
    uword stride = max(l.size, l.alignment);
    uword offset = b->stack_used;
    b->stack_used = oc_align_forward(offset + stride, l.alignment);

    if (!allocate_only) {
        params.reg0 = X86_64_OPERAND_REGISTER_rdi;
        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = -(long long int)b->stack_used;
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);

        oc_assert(OPD_TYPE(reg) == LL_IR_OPERAND_REGISTER_BIT);
        x86_64_load_register(cc, b, bir, X86_64_OPERAND_REGISTER_rsi, &b->registers.items[OPD_VALUE(reg)], type);

        oc_assert(l.size <= 0xFFFFFFF);
        x86_64_generate_memcpy_assum_rdi_rsi(cc, b, bir, type, LL_IR_OPERAND_IMMEDIATE_BIT | l.size, NULL);
    }

    return b->stack_used;
}

static uword x86_64_move_reg_to_stack(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Type* type, X86_64_Operand_Register reg) {
    (void)cc;
    (void)bir;
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Get_Variant_Params get_variant = { 0 };
    LL_Backend_Layout l = x86_64_get_layout(type);
    uword stride = max(l.size, l.alignment);
    uword offset = b->stack_used;
    b->stack_used = oc_align_forward(offset + stride, l.alignment);

    params.reg1 = reg;
    params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
    params.displacement = -(long long int)b->stack_used;

    type = ll_get_base_type(type);

    switch (type->kind) {
    case LL_TYPE_FLOAT:
        if (type->width <= 32)
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOVSS, x86_64_get_variant_raw(type, get_variant), params);
        else if (type->width <= 64)
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOVSD, x86_64_get_variant_raw(type, get_variant), params);
        else oc_todo("implement other sizes");
        break;
    case LL_TYPE_BOOL:
    case LL_TYPE_CHAR:
    case LL_TYPE_UINT:
    case LL_TYPE_INT:
    case LL_TYPE_POINTER:
        OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(type, get_variant), params);
        break;
    case LL_TYPE_STRING:
    case LL_TYPE_ARRAY:
    case LL_TYPE_SLICE:
    case LL_TYPE_STRUCT:
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);
        break;
    default: ll_print_type(type); oc_todo("add types");
    }

    return b->stack_used;
}

static X86_64_Register x86_64_move_reg_alloc(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Type* type, X86_64_Operand_Register reg) {
    (void)cc;
    (void)bir;
    X86_64_Register result = { 0 };
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Get_Variant_Params get_variant = { 0 };

    if (false && b->active_register_top < oc_len(x86_64_backend_active_registers)) {
        result.location = X86_64_REGISTER_LOCATION_REGISTER;
        if (x86_64_backend_active_registers[b->active_register_top] == reg) {
            b->active_register_top++;
            result.value = reg;
            return result;
        } else {
            result.value = x86_64_backend_active_registers[b->active_register_top++];
            params.reg0 = result.value;
            params.reg1 = reg;
        }
    } else {
        LL_Backend_Layout l = x86_64_get_layout(type);

        uword stride = max(l.size, l.alignment);
        uword offset = b->stack_used;
        b->stack_used = oc_align_forward(offset + stride, l.alignment);

        params.displacement = -(long long int)b->stack_used;
        params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.reg1 = reg;

        result.value = b->stack_used;
        result.location = X86_64_REGISTER_LOCATION_STACK;
    }

    type = ll_get_base_type(type);

    switch (type->kind) {
    case LL_TYPE_FLOAT:
        if (type->width <= 32)
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOVSS, x86_64_get_variant_raw(type, get_variant), params);
        else if (type->width <= 64)
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOVSD, x86_64_get_variant_raw(type, get_variant), params);
        else oc_todo("implement other sizes");
        break;
    case LL_TYPE_BOOL:
    case LL_TYPE_CHAR:
    case LL_TYPE_UINT:
    case LL_TYPE_INT:
    case LL_TYPE_POINTER:
        OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant_raw(type, get_variant), params);
        break;
    case LL_TYPE_STRING:
    case LL_TYPE_ARRAY:
    case LL_TYPE_SLICE:
    case LL_TYPE_STRUCT:
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);
        break;
    default: ll_print_type(type); oc_todo("add types");
    }

    return result;
}

typedef struct {
    uint32_t opcode, kind;
    uint32_t cast_opcode, cast_kind;
    bool needs_cast, explicit_cast_instruction;
} Move_Info;

static Move_Info x86_64_get_move_info(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Type* dst_type, LL_Type* src_type, bool mem_right) {
    (void)cc;
    (void)b;
    (void)bir;
    Move_Info result = { 0 };
    X86_64_Get_Variant_Params get_variant = { .mem_right = mem_right };

    dst_type = ll_get_base_type(dst_type);
    src_type = ll_get_base_type(src_type);

    if (mem_right) {
        result.kind = x86_64_get_variant_raw(src_type, get_variant);
    } else {
        result.kind = x86_64_get_variant_raw(dst_type, get_variant);
    }
    switch (src_type->kind) {
    case LL_TYPE_FLOAT:
        if      (src_type->width <= 32) result.opcode = OPCODE_MOVSS;
        else if (src_type->width <= 64) result.opcode = OPCODE_MOVSD;
        else oc_unreachable("invalid type");
        break;
    case LL_TYPE_UINT:
    case LL_TYPE_INT:
    case LL_TYPE_CHAR:
    case LL_TYPE_BOOL:
    case LL_TYPE_POINTER:
    case LL_TYPE_STRING:
    case LL_TYPE_ARRAY:
    case LL_TYPE_SLICE:
    case LL_TYPE_STRUCT:
        result.opcode = OPCODE_MOV;
        break;
    default: oc_todo("add get move type"); break;
    }

    if (dst_type != src_type) {
        result.needs_cast = true;
        switch (src_type->kind) {
        case LL_TYPE_CHAR:
        case LL_TYPE_UINT:
            switch (dst_type->kind) {
            case LL_TYPE_CHAR:
            case LL_TYPE_INT:
                // fallthrough
            case LL_TYPE_BOOL:
                // fallthrough
            case LL_TYPE_UINT:
                if (src_type->width < dst_type->width || dst_type->kind == LL_TYPE_INT) {
                    result.opcode = OPCODE_MOVZX;
                    if (src_type->width <= 8) {
                        if (dst_type->width <= 16) {
                            result.kind = X86_64_VARIANT_KIND_r16_rm8;
                            break;
                        } else if (dst_type->width <= 32) {
                            result.kind = X86_64_VARIANT_KIND_r32_rm8;
                            break;
                        } else if (dst_type->width <= 64) {
                            result.kind = X86_64_VARIANT_KIND_r64_rm8;
                            break;
                        }
                    } else if (src_type->width <= 16) {
                        if (dst_type->width <= 32) {
                            result.kind = X86_64_VARIANT_KIND_r32_rm16;
                            break;
                        } else if (dst_type->width <= 64) {
                            result.kind = X86_64_VARIANT_KIND_r64_rm16;
                            break;
                        }
                    }
                }
                result.opcode = OPCODE_MOV;
                result.kind = x86_64_get_variant_raw(src_type, get_variant);
                break;
            case LL_TYPE_FLOAT:
                if (dst_type->width <= 32) {
                    result.opcode = OPCODE_CVTSI2SS;
                } else if (dst_type->width <= 64) {
                    result.opcode = OPCODE_CVTSI2SD;
                } else oc_todo("add widths");

                // for 8 bit and 16 bit, need to sign extend to 32 bits since cvtsi2s only accepts 32 bit and 64 bit
                if (src_type->width <= 8) {
                    result.explicit_cast_instruction = true;

                    result.cast_opcode = result.opcode;
                    result.cast_kind = X86_64_VARIANT_KIND_r128_rm32;
                    result.opcode = OPCODE_MOVZX;
                    result.kind = X86_64_VARIANT_KIND_r32_rm8;
                } else if (src_type->width <= 16) {
                    result.explicit_cast_instruction = true;

                    result.cast_opcode = result.opcode;
                    result.cast_kind = X86_64_VARIANT_KIND_r128_rm32;
                    result.opcode = OPCODE_MOVZX;
                    result.kind = X86_64_VARIANT_KIND_r32_rm16;
                } else if (src_type->width <= 32) {
                    result.kind = X86_64_VARIANT_KIND_r128_rm32;
                } else if (src_type->width <= 64) {
                    result.kind = X86_64_VARIANT_KIND_r128_rm64;
                } else oc_todo("flksdjf");
                break;
            default: oc_todo(""); break;
            }
            break;
        case LL_TYPE_INT:
            switch (dst_type->kind) {
            case LL_TYPE_BOOL:
            case LL_TYPE_CHAR:
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
                if (src_type->width < dst_type->width) {
                    if (src_type->width <= 8) {
                        result.opcode = OPCODE_MOVSX;
                        if (dst_type->width <= 16) {
                            result.kind = X86_64_VARIANT_KIND_r16_rm8;
                            break;
                        } else if (dst_type->width <= 32) {
                            result.kind = X86_64_VARIANT_KIND_r32_rm8;
                            break;
                        } else if (dst_type->width <= 64) {
                            result.kind = X86_64_VARIANT_KIND_r64_rm8;
                            break;
                        }
                    } else if (src_type->width <= 16) {
                        result.opcode = OPCODE_MOVSX;
                        if (dst_type->width <= 32) {
                            result.kind = X86_64_VARIANT_KIND_r32_rm16;
                            break;
                        } else if (dst_type->width <= 64) {
                            result.kind = X86_64_VARIANT_KIND_r64_rm16;
                            break;
                        }
                    } else if (src_type->width <= 32) {
                        result.opcode = OPCODE_MOVSXD;
                        if (dst_type->width <= 64) {
                            result.kind = X86_64_VARIANT_KIND_r64_rm32;
                            break;
                        }
                    }
                }
                result.opcode = OPCODE_MOV;
                result.kind = x86_64_get_variant_raw(src_type, get_variant);
                break;
            case LL_TYPE_FLOAT:
                if (dst_type->width <= 32) {
                    result.opcode = OPCODE_CVTSI2SS;
                } else if (dst_type->width <= 64) {
                    result.opcode = OPCODE_CVTSI2SD;
                } else oc_todo("add widths");

                // for 8 bit and 16 bit, need to sign extend to 32 bits since cvtsi2s only accepts 32 bit and 64 bit
                if (src_type->width <= 8) {
                    result.explicit_cast_instruction = true;

                    result.cast_opcode = result.opcode;
                    result.cast_kind = X86_64_VARIANT_KIND_r128_rm32;
                    result.opcode = OPCODE_MOVSX;
                    result.kind = X86_64_VARIANT_KIND_r32_rm8;
                } else if (src_type->width <= 16) {
                    result.explicit_cast_instruction = true;

                    result.cast_opcode = result.opcode;
                    result.cast_kind = X86_64_VARIANT_KIND_r128_rm32;
                    result.opcode = OPCODE_MOVSX;
                    result.kind = X86_64_VARIANT_KIND_r32_rm16;
                } else if (src_type->width <= 32) {
                    result.kind = X86_64_VARIANT_KIND_r128_rm32;
                } else if (src_type->width <= 64) {
                    result.kind = X86_64_VARIANT_KIND_r128_rm64;
                } else oc_todo("flksdjf");
                break;
            default: oc_todo(""); break;
            }

            break;
        case LL_TYPE_POINTER:
            switch (dst_type->kind) {
            case LL_TYPE_POINTER:
                result.opcode = OPCODE_MOV;
                result.kind = x86_64_get_variant_raw(src_type, get_variant);
                break;
            default: ll_print_type(dst_type); oc_todo(""); break;
            }
            break;
        case LL_TYPE_FLOAT:
            switch (dst_type->kind) {
            case LL_TYPE_INT:
                if (src_type->width == 32 && dst_type->width <= 32) {
                    result.opcode = OPCODE_CVTSS2SI;
                    result.kind = X86_64_VARIANT_KIND_r32_rm128;
                } else if (src_type->width == 32 && dst_type->width <= 64) {
                    result.opcode = OPCODE_CVTSS2SI;
                    result.kind = X86_64_VARIANT_KIND_r64_rm128;
                } else if (src_type->width == 64 && dst_type->width <= 32) {
                    result.opcode = OPCODE_CVTSD2SI;
                    result.kind = X86_64_VARIANT_KIND_r32_rm128;
                } else if (src_type->width == 64 && dst_type->width <= 64) {
                    result.opcode = OPCODE_CVTSD2SI;
                    result.kind = X86_64_VARIANT_KIND_r64_rm128;
                } else oc_unreachable("unsupported");
                break;
            case LL_TYPE_FLOAT:
                if (src_type->width == 32 && dst_type->width == 64) {
                    result.opcode = OPCODE_CVTSS2SD;
                    result.kind = X86_64_VARIANT_KIND_r128_rm128;
                } else if (src_type->width == 64 && dst_type->width == 32) {
                    result.opcode = OPCODE_CVTSD2SS;
                    result.kind = X86_64_VARIANT_KIND_r128_rm128;
                } else oc_unreachable("unsupported");
                break;
            default: oc_todo(""); break;
            }
            break;
        case LL_TYPE_ARRAY:
            switch (dst_type->kind) {
            case LL_TYPE_STRING:
            case LL_TYPE_SLICE:
                break;
            default:
                break;
            }
            break;
        case LL_TYPE_STRING:
            break;
        default:
            ll_print_type(src_type);
            ll_print_type(dst_type);
            oc_todo("add types");
            break;
        }
    }

    return result;
}

static void x86_64_generate_clone_cast(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand dst, LL_Ir_Operand src) {
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top];
    Move_Info move_info;

    LL_Type* src_type;
    LL_Type* dst_type = ir_get_operand_type(bir, b->fn, dst);

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
    case LL_IR_OPERAND_IMMEDIATE64_BIT:
        src_type = dst_type;
        break;
    default:
        src_type = ir_get_operand_type(bir, b->fn, src);
        break;
    }

    oc_assert(OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT);
    oc_assert(src_type == dst_type);

    LL_Backend_Layout struct_layout = x86_64_get_layout(src_type);
    size_t actual_size = max(struct_layout.size, struct_layout.alignment);

    int64_t displacement;

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_LOCAL_BIT: {
        displacement = -(int64_t)b->locals.items[OPD_VALUE(src)];
    } break;
    case LL_IR_OPERAND_PARMAETER_BIT: {
        displacement = -(int64_t)b->parameters.items[OPD_VALUE(src)];
    } break;
    default: oc_todo("unpkemented"); break;
    }

    switch (src_type->kind) {
    case LL_TYPE_STRING:
    case LL_TYPE_SLICE:
        goto HANDLE_CLONE_STRUCT;
    case LL_TYPE_ARRAY:
    case LL_TYPE_STRUCT: {
        if (actual_size <= 8) goto HANDLE_CLONE_LOCAL_INTEGRAL;

HANDLE_CLONE_STRUCT: (void)0;
        uword dst_offset = x86_64_make_struct_copy(cc, b, bir, src_type, 0, true);

        { // memcpy
            params.reg0 = X86_64_OPERAND_REGISTER_rdi;
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -(long long int)dst_offset;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);

            params.reg0 = X86_64_OPERAND_REGISTER_rsi;
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = displacement;

            switch (OPD_TYPE(src)) {
            case LL_IR_OPERAND_LOCAL_BIT: {
                OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
            } break;
            case LL_IR_OPERAND_PARMAETER_BIT: {
                OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_rm64, params);
            } break;
            default: oc_todo("unpkemented"); break;
            }

            x86_64_generate_memcpy_assum_rdi_rsi(cc, b, bir, src_type, LL_IR_OPERAND_IMMEDIATE_BIT | actual_size, cc->typer->ty_uint64);
        }

        params.reg0 = reg;
        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = -(long long int)dst_offset;
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
        
        X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
        b->registers.items[OPD_VALUE(dst)] = offset;
    } break;
    case LL_TYPE_FLOAT:
    case LL_TYPE_POINTER:
    case LL_TYPE_ANYBOOL:
    case LL_TYPE_BOOL:
    case LL_TYPE_CHAR:
    case LL_TYPE_ANYINT:
    case LL_TYPE_UINT:
    case LL_TYPE_INT: {
HANDLE_CLONE_LOCAL_INTEGRAL:
        params.reg0 = reg;
        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = displacement;
        OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, move_info.kind, params);
        // TODO: should we handle move_info.explicit_cast_instruction (casting)

        X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
        b->registers.items[OPD_VALUE(dst)] = offset;
    } break;
    default: oc_todo("handle type"); break;
    }
}

static void x86_64_generate_load_cast(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand dst, LL_Ir_Operand src, bool cast) {
    X86_64_Instruction_Parameters params = { 0 };
    X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top];
    Move_Info move_info;

    LL_Type* src_type;
    LL_Type* dst_type = ir_get_operand_type(bir, b->fn, dst);

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
    case LL_IR_OPERAND_IMMEDIATE64_BIT:
        src_type = dst_type;
        break;
    default:
        src_type = ir_get_operand_type(bir, b->fn, src);
        break;
    }

    if (!cast && OPD_TYPE(src) == LL_IR_OPERAND_REGISTER_BIT) {
        assert(src_type->kind == LL_TYPE_POINTER);
        LL_Type* element_type = ((LL_Type_Pointer*)src_type)->element_type;
        while (element_type && element_type->kind == LL_TYPE_NAMED) {
            element_type = ((LL_Type_Named*)element_type)->actual_type;
        }

        switch (element_type->kind)  {
        case LL_TYPE_STRING:
        case LL_TYPE_ARRAY:
        case LL_TYPE_STRUCT: {
            LL_Backend_Layout struct_layout = x86_64_get_layout(element_type);
            size_t actual_size = max(struct_layout.size, struct_layout.alignment);

            if (actual_size <= 8) goto HANDLE_LOAD_PTR_INTEGRAL_TYPE;

            b->registers.items[OPD_VALUE(dst)] = b->registers.items[OPD_VALUE(src)];
        } break;
        case LL_TYPE_FLOAT:
        case LL_TYPE_POINTER:
        case LL_TYPE_ANYBOOL:
        case LL_TYPE_BOOL:
        case LL_TYPE_CHAR:
        case LL_TYPE_ANYINT:
        case LL_TYPE_UINT:
        case LL_TYPE_INT: {
    HANDLE_LOAD_PTR_INTEGRAL_TYPE:

            move_info = x86_64_get_move_info(cc, b, bir, dst_type, element_type, true);

            X86_64_Operand_Register loaded_addr = x86_64_load_operand(cc, b, bir, src);
            b->active_register_top -= 1;

            params.reg0 = reg;
            params.reg1 = loaded_addr | X86_64_REG_BASE;
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, move_info.kind, params);
            if (move_info.explicit_cast_instruction) {
                params.reg0 = reg;
                params.reg1 = reg;
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.cast_opcode, move_info.cast_kind, params);
            }

            X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
            b->registers.items[OPD_VALUE(dst)] = offset;

        } break;
        default: oc_todo("handle type"); break;
        }

        return;
    }

    move_info = x86_64_get_move_info(cc, b, bir, dst_type, src_type, true);

    switch (OPD_TYPE(src)) {
        case LL_IR_OPERAND_REGISTER_BIT:
            oc_assert(cast && "load should only be used for locals, parameters, and immediates");
            if (src_type == dst_type) {
                b->registers.items[OPD_VALUE(dst)] = b->registers.items[OPD_VALUE(src)];
            } else {
                X86_64_Register src_reg = b->registers.items[OPD_VALUE(src)];
                switch (src_reg.location) {
                case X86_64_REGISTER_LOCATION_REGISTER: {
                    oc_todo("implement register alloc");
                    params.reg1 = src_reg.value;
                } break;
                case X86_64_REGISTER_LOCATION_STACK: {
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -(int64_t)(int32_t)src_reg.value;
                } break;
                default: oc_todo(""); break;
                }
                params.reg0 = reg;
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, move_info.kind, params);

                if (move_info.explicit_cast_instruction) {
                    params.reg0 = reg;
                    params.reg1 = reg;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.cast_opcode, move_info.cast_kind, params);
                }

                X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
                b->registers.items[OPD_VALUE(dst)] = offset;
            }
            break;
        case LL_IR_OPERAND_DATA_BIT:
            switch (src_type->kind) {
            case LL_TYPE_ARRAY:
                assert(OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT);
                switch (dst_type->kind) {
                case LL_TYPE_STRING:
                case LL_TYPE_SLICE: {
                    
                    X86_64_Operand_Register tmp_reg = x86_64_backend_active_registers[b->active_register_top];

                    params.reg0 = tmp_reg;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, params);
                    oc_array_append(&cc->tmp_arena, &b->internal_relocations, ((X86_64_Internal_Relocation) {
                        .data_item = OPD_VALUE(src),
                        .text_rel_byte_offset = b->section_text.count - 8 /* sizeof displacement */
                    }));

                    uword offset = x86_64_make_struct_copy(cc, b, bir, dst_type, -1, true);

                    // set ptr
                    params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.reg1 = tmp_reg;
                    params.displacement = -offset;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);

                    // set len
                    params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -offset + 8;
                    params.immediate = (int64_t)src_type->width;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_i32, params);

                    // take pointer to temporary
                    params.reg0 = tmp_reg;
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -offset;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);

                    X86_64_Register moved_reg = x86_64_move_reg_alloc(cc, b, bir, dst_type, tmp_reg);
                    b->registers.items[OPD_VALUE(dst)] = moved_reg;
                } break;
                default: goto HANDLE_LOAD_STRUCT_DATA;
                }
                break;
            case LL_TYPE_STRING:
                assert(OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT);
                if (dst_type->kind == LL_TYPE_SLICE) src_type = dst_type;
                goto HANDLE_LOAD_STRUCT_DATA;
                break;
            case LL_TYPE_SLICE:
                assert(OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT);
                goto HANDLE_LOAD_STRUCT_DATA;
                break;
            case LL_TYPE_STRUCT: {
HANDLE_LOAD_STRUCT_DATA:
                assert(dst_type == src_type);
                LL_Backend_Layout struct_layout = x86_64_get_layout(src_type);
                size_t actual_size = max(struct_layout.size, struct_layout.alignment);

                if (actual_size <= 8) goto HANDLE_LOAD_LOCAL_INTEGRAL_DATA;
                assert(OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT);

                X86_64_Instruction_Parameters params = { 0 };
                params.reg0 = reg;
                OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, params);
                oc_array_append(&cc->tmp_arena, &b->internal_relocations, ((X86_64_Internal_Relocation) {
                    .data_item = OPD_VALUE(src),
                    .text_rel_byte_offset = b->section_text.count - 8 /* sizeof displacement */
                }));
                
                X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
                b->registers.items[OPD_VALUE(dst)] = offset;
            } break;
            case LL_TYPE_FLOAT:
            case LL_TYPE_POINTER:
            case LL_TYPE_ANYBOOL:
            case LL_TYPE_BOOL:
            case LL_TYPE_CHAR:
            case LL_TYPE_ANYINT:
            case LL_TYPE_UINT:
            case LL_TYPE_INT: {
        HANDLE_LOAD_LOCAL_INTEGRAL_DATA:
                if (src_type == dst_type) {
                    b->registers.items[OPD_VALUE(dst)].location = X86_64_REGISTER_LOCATION_STACK;
                    b->registers.items[OPD_VALUE(dst)].value = b->locals.items[OPD_VALUE(src)];
                    #ifndef _NDEBUG
                        b->registers.items[OPD_VALUE(dst)].usedup = 0;
                    #endif
                } else {
                    params.reg0 = reg;
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -b->locals.items[OPD_VALUE(src)];
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, move_info.kind, params);
                    if (move_info.explicit_cast_instruction) {
                        params.reg0 = reg;
                        params.reg1 = reg;
                        OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.cast_opcode, move_info.cast_kind, params);
                    }

                    X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
                    b->registers.items[OPD_VALUE(dst)] = offset;
                }
            } break;
            default: oc_todo("handle type"); break;
            }
            break;
        case LL_IR_OPERAND_LOCAL_BIT:
            switch (src_type->kind) {
            case LL_TYPE_ARRAY:
                assert(OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT);
                switch (dst_type->kind) {
                case LL_TYPE_STRING:
                case LL_TYPE_SLICE: {
                    
                    X86_64_Operand_Register tmp_reg = x86_64_backend_active_registers[b->active_register_top];

                    params.reg0 = tmp_reg;
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -b->locals.items[OPD_VALUE(src)];
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);

                    uword offset = x86_64_make_struct_copy(cc, b, bir, dst_type, -1, true);

                    // set ptr
                    params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.reg1 = tmp_reg;
                    params.displacement = -offset;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);

                    // set len
                    params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -offset + 8;
                    params.immediate = (int64_t)src_type->width;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_i32, params);

                    // take pointer to temporary
                    params.reg0 = tmp_reg;
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -offset;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);

                    X86_64_Register moved_reg = x86_64_move_reg_alloc(cc, b, bir, dst_type, tmp_reg);
                    b->registers.items[OPD_VALUE(dst)] = moved_reg;
                } break;
                default: goto HANDLE_LOAD_STRUCT;
                }
                break;
            case LL_TYPE_STRING:
                assert(OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT);
                if (dst_type->kind == LL_TYPE_SLICE) src_type = dst_type;
                goto HANDLE_LOAD_STRUCT;
                break;
            case LL_TYPE_SLICE:
                assert(OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT);
                goto HANDLE_LOAD_STRUCT;
                break;
            case LL_TYPE_STRUCT: {
HANDLE_LOAD_STRUCT:
                assert(dst_type == src_type);
                LL_Backend_Layout struct_layout = x86_64_get_layout(src_type);
                size_t actual_size = max(struct_layout.size, struct_layout.alignment);

                if (actual_size <= 8) goto HANDLE_LOAD_LOCAL_INTEGRAL;
                assert(OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT);

                X86_64_Instruction_Parameters params = { 0 };
                params.reg0 = reg;
                params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                params.displacement = -b->locals.items[OPD_VALUE(src)];
                OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
                
                X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
                b->registers.items[OPD_VALUE(dst)] = offset;
            } break;
            case LL_TYPE_FLOAT:
            case LL_TYPE_POINTER:
            case LL_TYPE_ANYBOOL:
            case LL_TYPE_BOOL:
            case LL_TYPE_CHAR:
            case LL_TYPE_ANYINT:
            case LL_TYPE_UINT:
            case LL_TYPE_INT: {
        HANDLE_LOAD_LOCAL_INTEGRAL:
                if (src_type == dst_type) {
                    b->registers.items[OPD_VALUE(dst)].location = X86_64_REGISTER_LOCATION_STACK;
                    b->registers.items[OPD_VALUE(dst)].value = b->locals.items[OPD_VALUE(src)];

                    #ifndef _NDEBUG
                        b->registers.items[OPD_VALUE(dst)].usedup = 0;
                    #endif
                } else {
                    params.reg0 = reg;
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -b->locals.items[OPD_VALUE(src)];
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, move_info.kind, params);
                    if (move_info.explicit_cast_instruction) {
                        params.reg0 = reg;
                        params.reg1 = reg;
                        OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.cast_opcode, move_info.cast_kind, params);
                    }

                    X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
                    b->registers.items[OPD_VALUE(dst)] = offset;
                }
            } break;
            default: oc_todo("handle type"); break;
            }
            break;
        case LL_IR_OPERAND_PARMAETER_BIT:
            if (src_type == dst_type) {
                b->registers.items[OPD_VALUE(dst)].location = X86_64_REGISTER_LOCATION_STACK;
                b->registers.items[OPD_VALUE(dst)].value = (uint32_t)b->parameters.items[OPD_VALUE(src)];

                #ifndef _NDEBUG
                    b->registers.items[OPD_VALUE(dst)].usedup = 0;
                #endif
            } else {
                params.reg0 = reg;
                params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                params.displacement = -(int64_t)b->parameters.items[OPD_VALUE(src)];
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, move_info.kind, params);
                if (move_info.explicit_cast_instruction) {
                    params.reg0 = reg;
                    params.reg1 = reg;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.cast_opcode, move_info.cast_kind, params);
                }

                X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
                b->registers.items[OPD_VALUE(dst)] = offset;
            }
            break;
        case LL_IR_OPERAND_IMMEDIATE_BIT: {
            X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top];
            params.immediate = OPD_VALUE(src);
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, x86_64_get_variant(dst_type, .immediate = true), params);

            X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
            b->registers.items[OPD_VALUE(dst)] = offset;
        } break;
        case LL_IR_OPERAND_IMMEDIATE64_BIT: {
            X86_64_Operand_Register reg = x86_64_backend_active_registers[b->active_register_top];
            uint32_t lit_index = OPD_VALUE(src);
            params.immediate = (int64_t)b->fn->literals.items[lit_index].as_u64;
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, x86_64_get_variant(dst_type, .large_immediate = true), params);

            X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, dst_type, reg);
            b->registers.items[OPD_VALUE(dst)] = offset;
        } break;
        default: oc_unreachable("unsupported operand"); break;
    }
}

static inline void x86_64_generate_store_cast(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand dst, LL_Ir_Operand src) {
    X86_64_Instruction_Parameters params = { 0 };
    Move_Info move_info;

    LL_Type* src_type;
    LL_Type* dst_type = ir_get_operand_type(bir, b->fn, dst);

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
    case LL_IR_OPERAND_IMMEDIATE64_BIT:
        src_type = dst_type;
        break;
    default:
        src_type = ir_get_operand_type(bir, b->fn, src);
        break;
    }

    if (OPD_TYPE(dst) == LL_IR_OPERAND_REGISTER_BIT && dst_type->kind == LL_TYPE_POINTER) {
        dst_type = ((LL_Type_Pointer*)dst_type)->element_type;
    }

    switch (dst_type->kind) {
    case LL_TYPE_STRING:
    case LL_TYPE_SLICE: {
        switch (src_type->kind) {
        case LL_TYPE_ARRAY: {
            X86_64_Operand_Register tmp_reg = x86_64_backend_active_registers[b->active_register_top];

            params.reg0 = tmp_reg;
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -b->locals.items[OPD_VALUE(src)];
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);

            switch (OPD_TYPE(dst)) {
            case LL_IR_OPERAND_LOCAL_BIT: {
                params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                params.reg1 = tmp_reg;
                params.displacement = -b->locals.items[OPD_VALUE(dst)];
                OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);
            } break;
            default: oc_todo("handle other operands"); break;
            }
        } break;
        case LL_TYPE_STRING:
        case LL_TYPE_SLICE:
            goto HANDLE_STRUCT_MEMCPY;
        default: ll_print_type(src_type); oc_todo("unkown type"); break;
        }
    } break;
    case LL_TYPE_ARRAY:
    case LL_TYPE_STRUCT: {
        assert(dst_type == src_type);
HANDLE_STRUCT_MEMCPY:
        (void)0;
        LL_Backend_Layout struct_layout = x86_64_get_layout(dst_type);
        size_t actual_size = max(struct_layout.size, struct_layout.alignment);

        if (actual_size <= 8) goto HANDLE_INTEGRAL_TYPE;

        switch (OPD_TYPE(dst)) {
        case LL_IR_OPERAND_PARMAETER_BIT: {
            params.reg0 = X86_64_OPERAND_REGISTER_rdi;
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -(int64_t)b->parameters.items[OPD_VALUE(dst)];
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_rm64, params);

            x86_64_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rsi, src, false);

            x86_64_generate_memcpy_assum_rdi_rsi(cc, b, bir, dst_type, (LL_Ir_Operand)actual_size | LL_IR_OPERAND_IMMEDIATE_BIT, cc->typer->ty_uint64);
        } break;
        case LL_IR_OPERAND_LOCAL_BIT: {
            x86_64_generate_memcpy(cc, b, bir, dst, src, (LL_Ir_Operand)actual_size | LL_IR_OPERAND_IMMEDIATE_BIT, dst_type);
        } break;
        case LL_IR_OPERAND_REGISTER_BIT: {
            x86_64_generate_memcpy(cc, b, bir, dst, src, (LL_Ir_Operand)actual_size | LL_IR_OPERAND_IMMEDIATE_BIT, dst_type);
        } break;
        default: oc_todo("handle other operands"); break;
        }

    } break; // LL_TYPE_STRUCT
    case LL_TYPE_FLOAT:
    case LL_TYPE_POINTER:
    case LL_TYPE_ANYBOOL:
    case LL_TYPE_BOOL:
    case LL_TYPE_CHAR:
    case LL_TYPE_ANYINT:
    case LL_TYPE_UINT:
    case LL_TYPE_INT: {
HANDLE_INTEGRAL_TYPE:
        switch (OPD_TYPE(dst)) {
        case LL_IR_OPERAND_LOCAL_BIT: {
            params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -b->locals.items[OPD_VALUE(dst)];

            switch (OPD_TYPE(src)) {
            case LL_IR_OPERAND_IMMEDIATE_BIT:
                if (dst_type->kind == LL_TYPE_FLOAT) {
                    assert(dst_type->width == 32);
                    params.immediate = OPD_VALUE(src);
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm32_i32, params);
                } else {
                    params.immediate = OPD_VALUE(src);
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MOV, x86_64_get_variant(dst_type, .immediate = true), params);
                }
                break;
            case LL_IR_OPERAND_IMMEDIATE64_BIT: {
                X86_64_Operand_Register tmp_reg = x86_64_backend_active_registers[b->active_register_top];
                params.reg0 = tmp_reg;
                if (dst_type->kind == LL_TYPE_FLOAT && dst_type->width <= 32) {
                    params.immediate = b->fn->literals.items[OPD_VALUE(src)].as_u32;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r32_i32, params);
                } else {
                    params.immediate = b->fn->literals.items[OPD_VALUE(src)].as_u64;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, params);
                }

                params.reg1 = tmp_reg;
                params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                params.displacement = -b->locals.items[OPD_VALUE(dst)];
                if (dst_type->kind == LL_TYPE_FLOAT && dst_type->width <= 32) {
                    params.immediate = b->fn->literals.items[OPD_VALUE(src)].as_u32;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm32_r32, params);
                } else {
                    params.immediate = b->fn->literals.items[OPD_VALUE(src)].as_u64;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);
                }
            } break;
            case LL_IR_OPERAND_REGISTER_BIT: {
                move_info = x86_64_get_move_info(cc, b, bir, dst_type, src_type, false);
                oc_assert(dst_type == src_type && "handle explicit cast from move info");

                X86_64_Operand_Register reg = x86_64_load_operand(cc, b, bir, src);
                b->active_register_top -= 1;

                params.reg1 = reg;
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, move_info.kind, params);
            } break;
            default: oc_todo("handle other operands"); break;
            }

        } break;
        case LL_IR_OPERAND_REGISTER_BIT:
            // assert(dst_type->kind == LL_TYPE_POINTER);
            // dst_type = ((LL_Type_Pointer*)dst_type)->element_type;

            switch (OPD_TYPE(src)) {
            case LL_IR_OPERAND_IMMEDIATE_BIT:
            case LL_IR_OPERAND_IMMEDIATE64_BIT:
                src_type = dst_type;
                break;
            default: break;
            }

            move_info = x86_64_get_move_info(cc, b, bir, dst_type, src_type, false);
            oc_assert(dst_type == src_type && "handle explicit cast from move info");

            X86_64_Operand_Register dst_reg = x86_64_load_operand(cc, b, bir, dst);
            X86_64_Operand_Register src_reg = x86_64_load_operand_with_type(cc, b, bir, src, dst_type);
            b->active_register_top -= 2;

            params.reg0 = dst_reg | X86_64_REG_BASE;
            params.reg1 = src_reg;
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, move_info.opcode, move_info.kind, params);

            break;
        default: oc_todo("handle other operands"); break;
        }
    } break; // LL_TYPE_INT*
    default: ll_print_type(dst_type); oc_todo("handle other types"); break;
    }
}


static void x86_64_generate_block(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block_Ref block_ref) {
    size_t i;
    int32_t opcode1;
    X86_64_Get_Variant_Params get_variant = { 0 };
    LL_Type* type;

    LL_Ir_Block* block = &bir->blocks.items[block_ref];
    block->generated_offset = (int64_t)b->section_text.count;

    for (i = 0; i < block->ops.count; ) {
        LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
        LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];
        X86_64_Instruction_Parameters params = { 0 };
        uword invoke_offset = 0;

        switch (opcode) {
        case LL_IR_OPCODE_STORE: {
            x86_64_generate_store_cast(cc, b, bir, operands[0], operands[1]);
        } break;
        case LL_IR_OPCODE_CAST: {
            x86_64_generate_load_cast(cc, b, bir, operands[0], operands[1], true);
        } break;
        case LL_IR_OPCODE_LOAD: {
            x86_64_generate_load_cast(cc, b, bir, operands[0], operands[1], false);
        } break;
        case LL_IR_OPCODE_CLONE: {
            x86_64_generate_clone_cast(cc, b, bir, operands[0], operands[1]);
        } break;
        case LL_IR_OPCODE_ALIAS: {
            oc_assert(OPD_TYPE(operands[0]) == LL_IR_OPERAND_REGISTER_BIT);
            oc_assert(OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT);

            b->registers.items[OPD_VALUE(operands[0])] = b->registers.items[OPD_VALUE(operands[1])];
        } break;

        case LL_IR_OPCODE_MEMCOPY: {
            if (OPD_TYPE(operands[3]) == LL_IR_OPERAND_IMMEDIATE_BIT) {
                x86_64_generate_memcpy(cc, b, bir, operands[0], operands[1], operands[2], cc->typer->ty_int32);
            } else {
                x86_64_generate_memcpy(cc, b, bir, operands[0], operands[1], operands[2], ir_get_operand_type(bir, b->fn, operands[3]));
            }
        } break;

        case LL_IR_OPCODE_LEA: {
            LL_Type* type = ir_get_operand_type(bir, b->fn, operands[0]);
            switch (type->kind) {
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
            case LL_TYPE_ANYINT:
            case LL_TYPE_CHAR:
            case LL_TYPE_BOOL:
            case LL_TYPE_ANYBOOL:
            case LL_TYPE_POINTER:
            case LL_TYPE_ARRAY:
            case LL_TYPE_STRUCT:
            case LL_TYPE_STRING: {
                /* oc_todo: max immeidiate is 28 bits */
                params.reg0 = x86_64_backend_active_registers[b->active_register_top];

                switch (OPD_TYPE(operands[1])) {
                case LL_IR_OPERAND_REGISTER_BIT: {
                    params.reg1 = x86_64_load_operand(cc, b, bir, operands[1]) | X86_64_REG_BASE;
                    b->active_register_top -= 1;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
                } break;
                case LL_IR_OPERAND_LOCAL_BIT: {
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -b->locals.items[OPD_VALUE(operands[1])];
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);

                    X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, type, params.reg0);
                    b->registers.items[OPD_VALUE(operands[0])] = offset;
                } break;
                case LL_IR_OPERAND_DATA_BIT: {
                    params.immediate = 0;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, params);

                    oc_array_append(&cc->tmp_arena, &b->internal_relocations, ((X86_64_Internal_Relocation) {
                        .data_item = OPD_VALUE(operands[1]),
                        .text_rel_byte_offset = b->section_text.count - 8 /* sizeof displacement */
                    }));

                    X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, type, params.reg0);
                    b->registers.items[OPD_VALUE(operands[0])] = offset;
                } break;
                default: oc_todo("add lea operands"); break;
                }
            } break;
            default: ll_print_type(type); oc_todo("add lea types"); break;
            }

        } break;

        case LL_IR_OPCODE_LEA_INDEX: {
            LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)ir_get_operand_type(bir, b->fn, operands[0]);
            assert(ptr_type->base.kind == LL_TYPE_POINTER);
            LL_Type* type = ll_get_base_type(ptr_type->element_type);

            switch (type->kind) {
            case LL_TYPE_POINTER:
            case LL_TYPE_STRUCT:
            case LL_TYPE_ARRAY:
            case LL_TYPE_STRING:
            case LL_TYPE_FLOAT:
            case LL_TYPE_ANYFLOAT:
            case LL_TYPE_ANYINT:
            case LL_TYPE_CHAR:
            case LL_TYPE_UINT:
            case LL_TYPE_INT: {
                /* oc_todo: max immeidiate is 28 bits */
                uint32_t tmp_regs = x86_64_alloc_tmp_regs(1);
                params.reg0 = x86_64_tmp_regs(tmp_regs, 0);
                int64_t displacement = 0;
                uint8_t use_sib = 0;

                switch (OPD_TYPE(operands[2])) {
                case LL_IR_OPERAND_IMMEDIATE_BIT: {
                    /* params.use_sib = true; */
                    displacement = (OPD_VALUE(operands[2]) * OPD_VALUE(operands[3]));
                } break;
                case LL_IR_OPERAND_IMMEDIATE64_BIT: {
                    /* params.use_sib = true; */
                    displacement = (b->fn->literals.items[OPD_VALUE(operands[2])].as_u64 * OPD_VALUE(operands[3]));
                } break;
                case LL_IR_OPERAND_REGISTER_BIT: {
                    use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE;
                    params.index = x86_64_load_register(cc, b, bir, -1, &b->registers.items[OPD_VALUE(operands[2])], cc->typer->ty_uint64);
                    // oc_todo("i don't think this is right");

                    switch (OPD_VALUE(operands[3])) {
                    case 1: params.scale = 0; break;
                    case 2: params.scale = 1; break;
                    case 4: params.scale = 2; break;
                    case 8: params.scale = 3; break;
                    default: oc_todo("add index size {}", OPD_VALUE(operands[3])); break;
                    }
                } break;
                default: oc_todo("add lea operands"); break;
                }

                switch (OPD_TYPE(operands[1])) {
                case LL_IR_OPERAND_REGISTER_BIT: {
                    params.reg1 = x86_64_load_operand(cc, b, bir, operands[1]) | X86_64_REG_BASE;
                    params.use_sib = use_sib;
                    params.displacement = displacement;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
                } break;
                case LL_IR_OPERAND_LOCAL_BIT: {
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.use_sib = use_sib;
                    params.displacement = displacement - b->locals.items[OPD_VALUE(operands[1])];
                    // params.displacement = displacement - b->locals.items[OPD_VALUE(operands[1])];
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
                } break;
                case LL_IR_OPERAND_DATA_BIT: {
                    params.use_sib = use_sib;
                    params.immediate = displacement;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, params);
                    oc_array_append(&cc->tmp_arena, &b->internal_relocations, ((X86_64_Internal_Relocation) {
                        .data_item = OPD_VALUE(operands[1]),
                        .text_rel_byte_offset = b->section_text.count - 8 /* sizeof displacement */
                    }));
                } break;
                case LL_IR_OPERAND_PARMAETER_BIT: {
                    LL_Type* src_type = ir_get_operand_type(bir, b->fn, operands[1]);
                    switch (src_type->kind) {
                    case LL_TYPE_STRING:
                    case LL_TYPE_SLICE: {
                        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                        params.displacement = -(int64_t)b->parameters.items[OPD_VALUE(operands[1])];
                        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_rm64, params);

                        params.reg1 = params.reg0 | X86_64_REG_BASE;
                        params.use_sib = use_sib;
                        params.displacement = displacement;
                        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
                    } break;
                    case LL_TYPE_ARRAY:
                    case LL_TYPE_STRUCT: {
                        LL_Backend_Layout struct_layout = x86_64_get_layout(src_type);
                        size_t actual_size = max(struct_layout.size, struct_layout.alignment);

                        if (actual_size <= 8) goto HANDLE_LEA_INTEGRAL_TYPE;

                        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                        params.displacement = -(int64_t)b->parameters.items[OPD_VALUE(operands[1])];
                        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_rm64, params);

                        params.reg1 = params.reg0 | X86_64_REG_BASE;
                        params.use_sib = use_sib;
                        params.displacement = displacement;
                        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
                    } break;
                    case LL_TYPE_POINTER:
                    case LL_TYPE_ANYBOOL:
                    case LL_TYPE_BOOL:
                    case LL_TYPE_CHAR:
                    case LL_TYPE_ANYINT:
                    case LL_TYPE_UINT:
                    case LL_TYPE_INT: {
                HANDLE_LEA_INTEGRAL_TYPE:
                        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                        params.use_sib = use_sib;
                        params.displacement = displacement - (int64_t)b->parameters.items[OPD_VALUE(operands[1])];
                        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
                    } break;
                    default: ll_print_type(src_type); oc_todo("add lea types"); break;
                    }
                } break;
                default: ll_print_type(type); oc_todo("add lea operands"); break;
                }

                X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, (LL_Type*)ptr_type, params.reg0);
                b->registers.items[OPD_VALUE(operands[0])] = offset;
                x86_64_free_tmp_regs(tmp_regs);
            } break;
            default: ll_print_type(type); oc_todo("add lea types"); break;
            }
        } break;

        case LL_IR_OPCODE_TEST: {
            type = ir_get_operand_type(bir, b->fn, operands[0]);
            push_regs(tmp_regs, 0) {
                params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
            }
            X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, type, params.reg0);
            b->registers.items[OPD_VALUE(operands[0])] = offset;
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
            type = ir_get_operand_type(bir, b->fn, operands[0]);
            switch (type->kind) {
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
                opcode1 = OPCODE_SUB;
                break;
            case LL_TYPE_FLOAT:
                if (type->width <= 32) opcode1 = OPCODE_SUBSS;
                else if (type->width <= 64) opcode1 = OPCODE_SUBSD;
                else oc_todo("add widths");
                break;
            default: oc_todo("add types"); break;
            }
            goto DO_OPCODE_ARITHMETIC;
        case LL_IR_OPCODE_ADD: {
            type = ir_get_operand_type(bir, b->fn, operands[0]);
            switch (type->kind) {
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
                opcode1 = OPCODE_ADD;
                break;
            case LL_TYPE_FLOAT:
                if (type->width <= 32) opcode1 = OPCODE_ADDSS;
                else if (type->width <= 64) opcode1 = OPCODE_ADDSD;
                else oc_todo("add widths");
                break;
            default: oc_todo("add types"); break;
            }

DO_OPCODE_ARITHMETIC:

            push_regs(tmp_regs, 0) {
                params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
                params.reg1 = x86_64_load_operand_with_type(cc, b, bir, operands[2], type);
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, x86_64_get_variant_raw(type, get_variant), params);
            }

            X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, type, params.reg0);
            b->registers.items[OPD_VALUE(operands[0])] = offset;
        } break;
        case LL_IR_OPCODE_MUL: {
            LL_Type* type = ir_get_operand_type(bir, b->fn, operands[0]);
            if (type->kind == LL_TYPE_INT || type->kind == LL_TYPE_UINT) {
                params.reg0 = X86_64_OPERAND_REGISTER_rdx;
                params.reg1 = X86_64_OPERAND_REGISTER_rdx;
                OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_XOR, r64_rm64, params);
            }

            push_regs(tmp_regs, 0) {
                params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
                params.reg1 = x86_64_load_operand_with_type(cc, b, bir, operands[2], type);
                oc_assert(params.reg0 == X86_64_OPERAND_REGISTER_rax);

                switch (type->kind) {
                case LL_TYPE_INT:
                    params.reg0 = params.reg1;
                    params.reg1 = 0;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_IMUL, x86_64_get_variant(type, .single = true), params);
                    break;
                case LL_TYPE_UINT:
                    params.reg0 = params.reg1;
                    params.reg1 = 0;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_MUL, x86_64_get_variant(type, .single = true), params);
                    break;
                case LL_TYPE_FLOAT:
                    if (type->width <= 32) OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MULSS, r128_rm128, params);
                    else if (type->width <= 64) OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MULSD, r128_rm128, params);
                    else oc_todo("implement");
                    break;
                default: oc_todo("implement other types"); break;
                }
            }

            X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, type, X86_64_OPERAND_REGISTER_rax);
            b->registers.items[OPD_VALUE(operands[0])] = offset;

        } break;
        case LL_IR_OPCODE_DIV: {
            LL_Type* type = ir_get_operand_type(bir, b->fn, operands[0]);

            push_regs(tmp_regs, 0) {
                params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
                params.reg1 = x86_64_load_operand_with_type(cc, b, bir, operands[2], type);
                oc_assert(params.reg0 == X86_64_OPERAND_REGISTER_rax);

                switch (type->kind) {
                case LL_TYPE_INT:
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_CQO , noarg, params);

                    params.reg0 = params.reg1;
                    params.reg1 = 0;

                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_IDIV, x86_64_get_variant(type, .single = true), params);
                    break;
                case LL_TYPE_UINT:
                    params.reg0 = X86_64_OPERAND_REGISTER_rdx;
                    params.reg1 = X86_64_OPERAND_REGISTER_rdx;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_XOR, r64_rm64, params);

                    params.reg0 = params.reg1;
                    params.reg1 = 0;

                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_DIV, x86_64_get_variant(type, .single = true), params);
                    break;
                case LL_TYPE_FLOAT:
                    if (type->width <= 32) OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_DIVSS, r128_rm128, params);
                    else if (type->width <= 64) OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_DIVSD, r128_rm128, params);
                    else oc_todo("implement");
                    break;
                default: ll_print_type(type); oc_todo("implement other types"); break;
                }
            }

            X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, type, X86_64_OPERAND_REGISTER_rax);
            b->registers.items[OPD_VALUE(operands[0])] = offset;

        } break;
        case LL_IR_OPCODE_MOD: {
            LL_Type* type = ir_get_operand_type(bir, b->fn, operands[0]);

            push_regs(tmp_regs, 0) {
                params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
                params.reg1 = x86_64_load_operand_with_type(cc, b, bir, operands[2], type);
                oc_assert(params.reg0 == X86_64_OPERAND_REGISTER_rax);


                switch (type->kind) {
                case LL_TYPE_INT:
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_CQO , noarg, params);

                    params.reg0 = params.reg1;
                    params.reg1 = 0;

                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_IDIV, x86_64_get_variant(type, .single = true), params);
                    break;
                case LL_TYPE_UINT:
                    params.reg0 = X86_64_OPERAND_REGISTER_rdx;
                    params.reg1 = X86_64_OPERAND_REGISTER_rdx;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_XOR, r64_rm64, params);

                    params.reg0 = params.reg1;
                    params.reg1 = 0;

                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_DIV, x86_64_get_variant(type, .single = true), params);
                    break;
                case LL_TYPE_FLOAT:
                    if (type->width <= 32) OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_DIVSS, r128_rm128, params);
                    else if (type->width <= 64) OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_DIVSD, r128_rm128, params);
                    else oc_todo("implement");
                    break;
                default: ll_print_type(type); oc_todo("implement other types"); break;
                }
            }

            if (type->kind == LL_TYPE_FLOAT) {
                X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, type, params.reg0);
                b->registers.items[OPD_VALUE(operands[0])] = offset;
            } else {
                X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, type, X86_64_OPERAND_REGISTER_rdx);
                b->registers.items[OPD_VALUE(operands[0])] = offset;
            }

        } break;

        case LL_IR_OPCODE_EQ:
            b->registers.items[OPD_VALUE(operands[0])].value = OPCODE_JE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_NEQ:
            b->registers.items[OPD_VALUE(operands[0])].value = OPCODE_JNE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_GTE:
            b->registers.items[OPD_VALUE(operands[0])].value = OPCODE_JGE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_GT:
            b->registers.items[OPD_VALUE(operands[0])].value = OPCODE_JG;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_LTE:
            b->registers.items[OPD_VALUE(operands[0])].value = OPCODE_JLE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_LT: {
            b->registers.items[OPD_VALUE(operands[0])].value = OPCODE_JL;
            LL_Type* type;
DO_OPCODE_COMPARE:
            b->registers.items[OPD_VALUE(operands[0])].location = X86_64_REGISTER_LOCATION_FLAGS;

            #ifndef _NDEBUG
                b->registers.items[OPD_VALUE(operands[0])].usedup = 0;
            #endif

            type = ir_get_operand_type(bir, b->fn, operands[1]);
            switch (type->kind) {
            case LL_TYPE_ANYBOOL:
            case LL_TYPE_BOOL:
            case LL_TYPE_CHAR:
            case LL_TYPE_ANYINT:
            case LL_TYPE_UINT:
            case LL_TYPE_INT: {

                push_regs(tmp_regs, 0) {
                    params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
                    params.reg1 = x86_64_load_operand_with_type(cc, b, bir, operands[2], type);
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_CMP, x86_64_get_variant(type), params);
                }
                break;
            }
            default: oc_todo("handle other type"); break;
            }
            break;
        }

        case LL_IR_OPCODE_NOT:
            opcode1 = OPCODE_NOT;
            goto DO_OPCODE_ARITHMETIC_PREOP;
        case LL_IR_OPCODE_NEG: {
            LL_Type* type;
            opcode1 = OPCODE_NEG;

DO_OPCODE_ARITHMETIC_PREOP:
            type = ir_get_operand_type(bir, b->fn, operands[0]);

            switch (type->kind) {
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT: {

                push_regs(tmp_regs, 0) {
                    params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[1], type);
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, x86_64_get_variant(type, .single = true), params);
                }

                X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, type, params.reg0);
                b->registers.items[OPD_VALUE(operands[0])] = offset;
            } break;
            default: ll_print_type(type); oc_todo("handle other type"); break;
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

                oc_array_append(&cc->arena, &b->branch_relocations, ((X86_64_Branch_Relocation) {
                    .branch_from = block_ref, // this block
                    .branch_to = operands[0],
                }));
            }
            break;
        case LL_IR_OPCODE_BRANCH_COND: {
            params.relative = 0;

            X86_64_Register* src_reg = &b->registers.items[OPD_VALUE(operands[0])];
            if (src_reg->location == X86_64_REGISTER_LOCATION_FLAGS) {
                if (operands[1] == block->next) {
                    OC_X86_64_WRITE_INSTRUCTION(b, oc_x86_64_get_inverse_compare(src_reg->value), rel32, params);
                } else if (operands[2] == block->next) {
                    OC_X86_64_WRITE_INSTRUCTION(b, src_reg->value, rel32, params);
                } else oc_unreachable("figure this out");
            } else {
                LL_Type* op_type = ir_get_operand_type(bir, b->fn, operands[0]);
                
                push_regs(tmp_regs, 0) {
                    params.reg0 = x86_64_load_register(cc, b, bir, -1, src_reg, op_type);
                    params.reg1 = params.reg0;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, OPCODE_TEST, x86_64_get_variant(op_type), params);
                }

                if (operands[1] == block->next) {
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_JE, rel32, params);
                } else if (operands[2] == block->next) {
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_JNE, rel32, params);
                } else oc_unreachable("figure this out");
            }

            block->fixup_offset = (int64_t)b->section_text.count - 4u;

            if (operands[1] == block->next) {
                // then block is next
                oc_array_append(&cc->arena, &b->branch_relocations, ((X86_64_Branch_Relocation) {
                    .branch_from = block_ref, // this block
                    .branch_to = operands[2],
                }));
            } else if (operands[2] == block->next) {
                // else block is next
                oc_array_append(&cc->arena, &b->branch_relocations, ((X86_64_Branch_Relocation) {
                    .branch_from = block_ref, // this block
                    .branch_to = operands[1],
                }));
            } else oc_unreachable("figure this out");
        } break;

        case LL_IR_OPCODE_INVOKEVALUE:
            invoke_offset = 1;
        case LL_IR_OPCODE_INVOKE: {
            uint32_t invokee = operands[invoke_offset++];
            uint32_t count = operands[invoke_offset++];
            X86_64_Call_Convention callconv = x86_64_call_convention_systemv();
            LL_Ir_Function* invokee_fn = &bir->fns.items[OPD_VALUE(invokee)];
            LL_Type_Function* fn_type = invokee_fn->fn_type;
            assert(fn_type->base.kind == LL_TYPE_FUNCTION);

            LL_Type* return_type = fn_type->return_type ? ll_get_base_type(fn_type->return_type) : NULL;

            b->invoke_prealloc.count = 0;
            oc_array_reserve(&cc->arena, &b->invoke_prealloc, count);

            get_variant.mem_right = true;
            uword pre_invoke_offset = invoke_offset;
            for (uint32_t j = 0; j < count; ++j) {
                LL_Ir_Operand arg_operand = operands[pre_invoke_offset++];
                LL_Type* type = ir_get_operand_type(bir, b->fn, arg_operand);

                uint32_t opcode = OPCODE_MOV;
                if (type->kind == LL_TYPE_FLOAT) {
                    if (type->width <= 32) opcode = OPCODE_MOVSS;
                    else if (type->width <= 64) opcode = OPCODE_MOVSD;
                    else oc_todo("add widths");
                }

                X86_64_Invoke_Prealloc prealloc = { 0 };
                switch (OPD_TYPE(arg_operand)) {
                    case LL_IR_OPERAND_REGISTER_BIT: 
                        oc_assert(b->registers.items[OPD_VALUE(arg_operand)].location == X86_64_REGISTER_LOCATION_STACK);

                        if (is_large_aggregate_type(type)) {
                            prealloc.immediate_displacement = (uint32_t)x86_64_make_struct_copy(cc, b, bir, type, arg_operand, false);
                            prealloc.opcode = OPCODE_LEA;
                            prealloc.variant = X86_64_VARIANT_KIND_r64_rm64;
                        } else {
                            prealloc.immediate_displacement = b->registers.items[OPD_VALUE(arg_operand)].value;
                            prealloc.opcode = opcode;
                            prealloc.variant = x86_64_get_variant_raw(type, get_variant);
                        }
                        break;
                    case LL_IR_OPERAND_IMMEDIATE_BIT:
                        prealloc.immediate_displacement = (int64_t)OPD_VALUE(arg_operand);
                        prealloc.opcode = OPCODE_MOV;
                        prealloc.variant = X86_64_VARIANT_KIND_rm64_i32;
                        break;
                    case LL_IR_OPERAND_IMMEDIATE64_BIT:
                        prealloc.immediate_displacement = (int64_t)b->fn->literals.items[OPD_VALUE(arg_operand)].as_u64;
                        prealloc.opcode = OPCODE_MOV;
                        prealloc.variant = X86_64_VARIANT_KIND_r64_i64;
                        break;
                    default: oc_todo("unhadnled argument operand: {x}", OPD_TYPE(arg_operand));
                }

                b->invoke_prealloc.items[j] = prealloc;
            }
            

            uword return_struct_offset;
            if (return_type && is_large_aggregate_type(return_type)) {
                X86_64_Parameter param = x86_64_call_convention_next(&callconv, return_type, &b->stack_used_for_args);

                return_struct_offset = x86_64_make_struct_copy(cc, b, bir, return_type, 0, true);
                if (param.is_reg) {
                    params.reg0 = param.reg;
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -return_struct_offset;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);
                } else {
                    X86_64_Operand_Register tmp_reg;
                    tmp_reg = x86_64_backend_active_registers[b->active_register_top];
                    params.reg0 = tmp_reg;
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -return_struct_offset;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);

                    params.displacement = param.stack_offset;
                    params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
                    params.reg1 = tmp_reg;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);
                }
            }


            for (uint32_t j = 0; j < count; ++j) {
                LL_Ir_Operand arg_operand = operands[invoke_offset++];
                LL_Type* type = ir_get_operand_type(bir, b->fn, arg_operand);

                uint32_t opcode = OPCODE_MOV;
                if (type->kind == LL_TYPE_FLOAT) {
                    if (type->width <= 32) opcode = OPCODE_MOVSS;
                    else if (type->width <= 64) opcode = OPCODE_MOVSD;
                    else oc_todo("add widths");
                }

                X86_64_Parameter parameter_location = x86_64_call_convention_next(&callconv, type, &b->stack_used_for_args);
                switch (OPD_TYPE(arg_operand)) {
                    case LL_IR_OPERAND_REGISTER_BIT: 
                        oc_assert(b->registers.items[OPD_VALUE(arg_operand)].location == X86_64_REGISTER_LOCATION_STACK);

                        if (parameter_location.is_reg) {
                            params.reg0 = parameter_location.reg;
                            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                            params.displacement = -b->invoke_prealloc.items[j].immediate_displacement;
                            OC_X86_64_WRITE_INSTRUCTION_DYN(b, b->invoke_prealloc.items[j].opcode, b->invoke_prealloc.items[j].variant, params);
                        } else {
                            X86_64_Operand_Register tmp_reg;

                            tmp_reg = x86_64_backend_active_registers[b->active_register_top];
                            params.reg0 = tmp_reg;
                            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                            params.displacement = -b->invoke_prealloc.items[j].immediate_displacement;
                            OC_X86_64_WRITE_INSTRUCTION_DYN(b, b->invoke_prealloc.items[j].opcode, b->invoke_prealloc.items[j].variant, params);

                            params.displacement = (int32_t)parameter_location.stack_offset;
                            params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
                            params.reg1 = tmp_reg;
                            OC_X86_64_WRITE_INSTRUCTION(b, opcode, rm64_r64, params);
                        }

                        break;

                    case LL_IR_OPERAND_IMMEDIATE_BIT:
                        if (parameter_location.is_reg) {
                            params.reg0 = parameter_location.reg;
                        } else {
                            params.displacement = (int32_t)parameter_location.stack_offset;
                            params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
                        }
                        params.immediate = OPD_VALUE(OPD_VALUE(arg_operand));
                        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_i32, params);

                        break;
                    case LL_IR_OPERAND_IMMEDIATE64_BIT:
                        if (parameter_location.is_reg) {
                            params.reg0 = parameter_location.reg;
                            params.immediate = b->fn->literals.items[OPD_VALUE(arg_operand)].as_u64;
                            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, params);
                        } else {
                            params.reg0 = x86_64_backend_active_registers[b->active_register_top];
                            params.immediate = b->fn->literals.items[OPD_VALUE(arg_operand)].as_u64;
                            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_i64, params);

                            params.reg1 = params.reg0;
                            params.displacement = (int32_t)parameter_location.stack_offset;
                            params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
                            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, params);
                        }
                        break;
                    default: oc_todo("unhadnled argument operand: {x}", OPD_TYPE(arg_operand));
                }
            }

            switch (OPD_TYPE(invokee)) {
            case LL_IR_OPERAND_FUNCTION_BIT: {
                LL_Ir_Function* fn = &bir->fns.items[OPD_VALUE(invokee)];

                if (fn->flags & LL_IR_FUNCTION_FLAG_EXTERN) {
                    params.relative = 0;
                    // OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_CALL, rel32, params);
                    oc_todo("implement extern");
                } else if (fn->generated_offset == LL_IR_FUNCTION_OFFSET_INVALID) {
                    params.relative = -(int64_t)b->section_text.count - 5;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_CALL, rel32, params);

                    oc_array_append(&cc->tmp_arena, &b->fn_relocations, ((X86_64_Function_Relocation) {
                        .fn_index = OPD_VALUE(invokee),
                        .text_rel_byte_offset = b->section_text.count - 4 /* sizeof displacement */
                    }));
                } else {
                    params.relative = (int32_t)(fn->generated_offset - (int64_t)b->section_text.count) - 5;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_CALL, rel32, params);
                }
                break;
            }
            default: oc_todo("handle inveok type"); break;
            }

            if (opcode == LL_IR_OPCODE_INVOKEVALUE) {
                if (return_type && is_large_aggregate_type(return_type)) {
                    params.reg0 = x86_64_backend_active_registers[b->active_register_top];
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -return_struct_offset;
                    OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LEA, r64_rm64, params);

                    X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, fn_type->return_type, params.reg0);
                    b->registers.items[OPD_VALUE(operands[0])] = offset;
                } else {
                    X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, fn_type->return_type, X86_64_OPERAND_REGISTER_rax);
                    b->registers.items[OPD_VALUE(operands[0])] = offset;
                }
            }

        } break;
        case LL_IR_OPCODE_RET:
            params.relative = 0;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_JMP, rel32, params);
            break;
        case LL_IR_OPCODE_RETVALUE: {
            LL_Type_Function* fn_type = (LL_Type_Function*)b->fn->fn_type;

            if (b->indirect_return_type) {
                LL_Type* return_type = ll_get_base_type(fn_type->return_type);
                oc_assert(is_large_aggregate_type(return_type));

                LL_Backend_Layout l = x86_64_get_layout(return_type);
                uword stride = max(l.size, l.alignment);

                params.reg0 = X86_64_OPERAND_REGISTER_rdi;
                params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                params.displacement = -b->indirect_return_ptr_offset;
                OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, r64_rm64, params);

                params.reg0 = X86_64_OPERAND_REGISTER_rsi;
                x86_64_load_register(cc, b, bir, X86_64_OPERAND_REGISTER_rsi, &b->registers.items[OPD_VALUE(operands[0])], cc->typer->ty_uint64);

                oc_assert(stride <= 0xFFFFFFF);
                x86_64_generate_memcpy_assum_rdi_rsi(cc, b, bir, return_type, LL_IR_OPERAND_IMMEDIATE_BIT | stride, NULL);
            } else {
                push_regs(tmp_regs, 0) {
                    params.reg0 = x86_64_load_operand_with_type(cc, b, bir, operands[0], fn_type->return_type);
                    oc_assert(params.reg0 == X86_64_OPERAND_REGISTER_rax);
                }
            }

        } break;
        default:
            print("\x1b[31;1mtodo: \x1b[0m implement op: {x}\n", opcode);
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

    for (fi = 0; fi < bir->data_items.count; ++fi) {
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

            void* native_fn_ptr = ll_native_fn_get(cc, &b->native_funcs, fn->ident->str);
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
            if (!cc->quiet) {
                print("local {} - size {x} - offset {x} - ", li, l.size, offset);
                ll_print_type_raw(fn->locals.items[li].ident->base.type, &stdout_writer);
                print("\n");
            }
        }
        b->stack_used = (uint32_t)offset;
        b->stack_used_for_args = 0;



        // generate prologue
        size_t function_offset = b->section_text.count;
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbx }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rdi }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsi }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_r12 }));

        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rsp }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_SUB, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = 0xface0102 }));
        size_t stack_size_offset = b->section_text.count - 4;
        fn->generated_offset = (int64_t)function_offset;

        int64_t mxcsr_offset = -1;
        if (b->fn->ident && string_eql(b->fn->ident->str, lit("main"))) {
            b->stack_used += 4;
            mxcsr_offset = b->stack_used;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_STMXCSR, rm32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE, .displacement = -(int64_t)b->stack_used }));
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_OR, rm32_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE, .displacement = -(int64_t)b->stack_used, .immediate = 0x6000 }));
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LDMXCSR, rm32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE, .displacement = -(int64_t)b->stack_used }));
            b->stack_used += 4;
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_STMXCSR, rm32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE, .displacement = -(int64_t)b->stack_used }));

            b->entry_index = fi;
        }

        // move registers to stack, if already on stack just use that offset
        X86_64_Call_Convention callconv = x86_64_call_convention_systemv();
        LL_Type_Function* fn_type = fn->fn_type;
        oc_array_reserve(&cc->tmp_arena, &b->parameters, fn_type->parameter_count);
        assert(fn_type->base.kind == LL_TYPE_FUNCTION);

        LL_Type* return_type = fn_type->return_type ? ll_get_base_type(fn_type->return_type) : NULL;

        if (return_type && is_large_aggregate_type(return_type)) {
            X86_64_Parameter param = x86_64_call_convention_next(&callconv, return_type, &b->stack_used_for_args);
            if (param.is_reg) {
                LL_Type* ptr_type = ll_typer_get_ptr_type(cc, cc->typer, fn_type->return_type);
                uword offset = x86_64_move_reg_to_stack(cc, b, bir, ptr_type, param.reg);
                b->indirect_return_ptr_offset = offset;
            } else {
                b->indirect_return_ptr_offset = -param.stack_offset - 0x30 /* rbp and return address */;
            }
            
            b->indirect_return_type = true;
        } else {
            b->indirect_return_type = false;
        }

        for (uint32_t j = 0; j < fn_type->parameter_count; ++j) {
            X86_64_Parameter param = x86_64_call_convention_next(&callconv, fn_type->parameters[j], &b->stack_used_for_args);
            if (param.is_reg) {
                uword offset = x86_64_move_reg_to_stack(cc, b, bir, fn_type->parameters[j], param.reg);
                b->parameters.items[j] = offset;
            } else {
                b->parameters.items[j] = -param.stack_offset - 0x30 /* rbp and return address */;
            }
            if (!cc->quiet) {
                print("parameter {} - offset {x}", j, b->parameters.items[j]);
                print("\n");
            }
        }

        oc_array_reserve(&cc->arena, &b->branch_relocations, fn->block_count);
        b->branch_relocations.count = 0;

        // generate body
        while (block) {
            x86_64_generate_block(cc, b, bir, block);
            block = bir->blocks.items[block].next;
        }

        for (size_t br = 0; br < b->branch_relocations.count; ++br) {
            LL_Ir_Block* from = &bir->blocks.items[b->branch_relocations.items[br].branch_from];
            LL_Ir_Block* to   = &bir->blocks.items[b->branch_relocations.items[br].branch_to];
            
            int32_t* dst_offset = (int32_t*)&b->section_text.items[from->fixup_offset];
            *dst_offset = (int32_t)(to->generated_offset - from->fixup_offset - 4);
        }

        for (uint32_t li = 0; li < b->registers.count; ++li) {
            if (!cc->quiet) {
                print("register {} - location {x} offset {x}\n", li, b->registers.items[li].location, b->registers.items[li].value);
            }
        }
        if (!cc->quiet) print("\n\n");

        if (mxcsr_offset != -1) {
            OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_LDMXCSR, rm32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE, .displacement = -mxcsr_offset }));
        }

        // generate epilogue
        // oc_assert(oc_align_forward(b->stack_used_for_args, 16) == b->stack_used_for_args);
        uint32_t stack_used = oc_align_forward(b->stack_used + b->stack_used_for_args, 16) ;
        int32_t* pstack_size = (int32_t*)&b->section_text.items[stack_size_offset];
        *pstack_size = stack_used;
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_ADD, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = stack_used }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_r12 }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsi }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rdi }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbx }));
        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));

        OC_X86_64_WRITE_INSTRUCTION(b, OPCODE_RET, noarg, ((X86_64_Instruction_Parameters){ 0 }));

    }

    for (size_t fi = 0; fi < b->fn_relocations.count; ++fi) {
        X86_64_Function_Relocation relocation = b->fn_relocations.items[fi];
        LL_Ir_Function* data_item = &bir->fns.items[relocation.fn_index];

        int32_t* dst_offset = (int32_t*)&b->section_text.items[relocation.text_rel_byte_offset];
        *dst_offset += data_item->generated_offset;
    }

}

void x86_64_run(Compiler_Context* cc, X86_64_Backend* b, LL_Backend_Ir* bir) {
    (void)cc;

#ifdef OC_PLATFORM_WINDOWS
    uint8* data = NULL;
    if (b->section_data.count) {
        data = VirtualAlloc(NULL, b->section_data.count, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        if (!data) {
            oc_assert(false && "Unable to allocate memory\n");
            return;
        }
        memcpy(data, b->section_data.items, b->section_data.count);
    } else {
        oc_assert(b->internal_relocations.count == 0);
    }

    uint8* code = VirtualAlloc(NULL, b->section_text.count, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (!code) {
        oc_assert(false && "Unable to allocate memory\n");
        return;
    }
    memcpy(code, b->section_text.items, b->section_text.count);

    for (size_t fi = 0; fi < b->internal_relocations.count; ++fi) {
        X86_64_Internal_Relocation relocation = b->internal_relocations.items[fi];

        LL_Ir_Data_Item* data_item = &bir->data_items.items[relocation.data_item];

        int64_t* dst_offset = (int64_t*)&code[relocation.text_rel_byte_offset];
        *dst_offset += ((int64_t)data + data_item->binary_offset);
    }

    sint32 a;
    if (!VirtualProtect(code, b->section_text.count, PAGE_EXECUTE, &a)) {
        extern sint32 GetLastError();
        oc_assert(false && "Unable to change protection: {x}\n");
        return;
    }

    void (*fn_ptr)() = (void (*)())(code + b->ir->fns.items[b->entry_index].generated_offset);
    fn_ptr();
#endif
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
