
#include "eval.h"
#include "callconv.h"
#include "../backends/x86_64.h"
#include "../backends/aarch64.h"
#include "../backends/ir.h"

#define FUNCTION() (&bir->fns.items[bir->current_function & CURRENT_INDEX])
#define FRAME() (&b->frames.items[b->frames.count - 1])
#define FRAMEN(n) (&b->frames.items[b->frames.count - 1 - (n)])

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

static inline LL_Eval_Registers* get_storage_location(
    Compiler_Context* cc,
    LL_Eval_Context* b,
    LL_Ir_Operand op
) {
    (void)cc;
    switch (OPD_TYPE(op)) {
    case LL_IR_OPERAND_LOCAL_BIT:
        return &FRAME()->locals;
    case LL_IR_OPERAND_REGISTER_BIT:
        return &FRAME()->registers;
    case LL_IR_OPERAND_PARMAETER_BIT:
        return &FRAME()->parameters;
    default: return NULL;
    }
}

#ifdef OC_PLATFORM_WINDOWS
void copy_native_code(
    Compiler_Context* cc,
    LL_Eval_Context* b
) {
    (void)cc;
    if (b->native_fn_stub_section.count > b->native_fn_exe_code_size) {
        if (b->native_fn_exe_code) {
            VirtualFree(b->native_fn_exe_code, b->native_fn_exe_code_size, MEM_COMMIT|MEM_RESERVE);
        }

        b->native_fn_exe_code = VirtualAlloc(NULL, b->native_fn_stub_section.count * 4, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        if (!b->native_fn_exe_code) {
            oc_assert(false && "Unable to allocate memory\n");
            return;
        }
        b->native_fn_exe_code_size = b->native_fn_stub_section.count * 4;
    } else {
        sint32 a;
        if (!VirtualProtect(b->native_fn_exe_code, b->native_fn_exe_code_size, PAGE_READWRITE, &a)) {
            extern sint32 GetLastError();
            oc_assert(false && "Unable to change protection: {x}\n");
            return;
        }
    }
    memcpy(b->native_fn_exe_code, b->native_fn_stub_section.items, b->native_fn_stub_section.count);

    sint32 a;
    if (!VirtualProtect(b->native_fn_exe_code, b->native_fn_exe_code_size, PAGE_EXECUTE, &a)) {
        extern sint32 GetLastError();
        oc_assert(false && "Unable to change protection: {x}\n");
        return;
    }
}
#else

void copy_native_code(
    Compiler_Context* cc,
    LL_Eval_Context* b
) {
    (void)cc;
    if (b->native_fn_stub_section.count > b->native_fn_exe_code_size) {
        if (b->native_fn_exe_code) {
            munmap(b->native_fn_exe_code, b->native_fn_exe_code_size);
        }

        b->native_fn_exe_code = mmap(NULL, b->native_fn_stub_section.count * 4, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_UNINITIALIZED, -1, 0);
        if (!b->native_fn_exe_code) {
            oc_assert(false && "Unable to allocate memory\n");
            return;
        }
        b->native_fn_exe_code_size = b->native_fn_stub_section.count * 4;
    } else {
        if (mprotect(b->native_fn_exe_code, b->native_fn_exe_code_size, PROT_EXEC)) {
            oc_assert(false && "Unable to change protection: {x}\n");
            return;
        }
    }
    memcpy(b->native_fn_exe_code, b->native_fn_stub_section.items, b->native_fn_stub_section.count);

    if (mprotect(b->native_fn_exe_code, b->native_fn_exe_code_size, PROT_EXEC)) {
        oc_assert(false && "Unable to change protection: {x}\n");
        return;
    }
}
#endif

int64_t do_native_fn_call_aarch64(
    Compiler_Context* cc,
    LL_Eval_Context* b,
    LL_Backend_Ir* bir,
    // LL_Ir_Function* invokee_fn,
    LL_Ir_Operand invokee,
    uint32_t operand_count,
    LL_Ir_Operand* operands
) {
    uint8 active_register_top = 9;
    uint32_t stack_used = 0;
    AArch64_Instruction_Parameters params = { 0 };

    AArch64_Call_Convention callconv = aarch64_call_convention_systemv();


    switch (OPD_TYPE(invokee)) {
    case LL_IR_OPERAND_FUNCTION_BIT: {
        LL_Ir_Function* fn = &bir->fns.items[OPD_VALUE(invokee)];
        fn->generated_offset = (int64_t)b->native_fn_stub_section.count;
    } break;
    default: oc_todo("fn type"); break;
    }

    OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_STP_pre, xd_xd2_xn_imm7, ((AArch64_Instruction_Parameters){ .rd = 29, .rn = 31, .rd2 = 30, .immediate = -0x10 }));
    OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_SUB, xd_xn_imm12_shift, ((AArch64_Instruction_Parameters){ .rd = 31, .rn = 31, .immediate = 0 }));
    size_t stack_size_offset = b->native_fn_stub_section.count - 4;

    LL_Ir_Function* invokee_fn = &bir->fns.items[OPD_VALUE(invokee)];
    LL_Type_Function* fn_type = (LL_Type_Function*)invokee_fn->ident->base.type;
    assert(fn_type->base.kind == LL_TYPE_FUNCTION);

    LL_Type* return_type = fn_type->return_type ? ll_get_base_type(fn_type->return_type) : NULL;

    b->invoke_prealloc.count = 0;
    oc_array_reserve(&cc->arena, &b->invoke_prealloc, operand_count);

    uword invoke_offset = 0;
    uword pre_invoke_offset = invoke_offset;
    for (uint32_t j = 0; j < operand_count; ++j) {
        LL_Ir_Operand arg_operand = operands[pre_invoke_offset++];
        LL_Type* type = ir_get_operand_type(bir, FUNCTION(), arg_operand);

        uint32_t opcode = AARCH64_OPCODE_STR;
        // if (type->kind == LL_TYPE_FLOAT) {
        //     if (type->width <= 32) opcode = X86_64_OPCODE_MOVSS;
        //     else if (type->width <= 64) opcode = X86_64_OPCODE_MOVSD;
        //     else oc_todo("add widths");
        // }

        X86_64_Invoke_Prealloc prealloc = { 0 };
        switch (OPD_TYPE(arg_operand)) {
            case LL_IR_OPERAND_REGISTER_BIT: 
                if (is_large_aggregate_type(type)) {
                    oc_todo("struct");
                    // prealloc.immediate_displacement = (uint32_t)x86_64_make_struct_copy(cc, b, bir, type, arg_operand, false);
                    // prealloc.opcode = X86_64_OPCODE_LEA;
                    // prealloc.variant = X86_64_VARIANT_KIND_r64_rm64;
                } else {
                    prealloc.immediate_displacement = FRAME()->registers.items[OPD_VALUE(arg_operand)].as_i64;
                    prealloc.opcode = opcode;
                    prealloc.variant = AARCH64_VARIANT_KIND_xd_xn_imm12;
                }
                break;
            case LL_IR_OPERAND_IMMEDIATE_BIT:
                prealloc.immediate_displacement = (int64_t)OPD_VALUE(arg_operand);
                prealloc.opcode = X86_64_OPCODE_MOV;
                prealloc.variant = X86_64_VARIANT_KIND_rm64_i32;
                break;
            case LL_IR_OPERAND_IMMEDIATE64_BIT:
                prealloc.immediate_displacement = (int64_t)FUNCTION()->literals.items[OPD_VALUE(arg_operand)].as_u64;
                prealloc.opcode = X86_64_OPCODE_MOV;
                prealloc.variant = X86_64_VARIANT_KIND_r64_i64;
                break;
            default: oc_todo("unhadnled argument operand: {x}", OPD_TYPE(arg_operand));
        }

        b->invoke_prealloc.items[j] = prealloc;
    }
    

    uword return_struct_offset;
    if (return_type && is_large_aggregate_type(return_type)) {
        oc_todo("struct return");
        Call_Convention_Parameter param = aarch64_call_convention_next(&callconv, return_type, &stack_used);

        // return_struct_offset = x86_64_make_struct_copy(cc, b, bir, return_type, 0, true);
        if (param.is_reg) {
            params.rd = param.reg;
            params.rn = 30;
            params.immediate = return_struct_offset;
            OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_SUB, wd_wn_imm12_shift, params);
        } else {
            uint8 tmp_reg;
            tmp_reg = active_register_top;
            params.rd = tmp_reg;
            params.rn = 30;
            params.immediate = return_struct_offset;
            OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_SUB, wd_wn_imm12_shift, params);

            params.immediate = param.stack_offset;
            params.rd = 30;
            params.rn = tmp_reg;
            OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_STR, xd_xn_imm12, params);
        }
    }


    for (uint32_t j = 0; j < operand_count; ++j) {
        LL_Ir_Operand arg_operand = operands[invoke_offset++];
        LL_Type* type = ir_get_operand_type(bir, FUNCTION(), arg_operand);

        uint32_t opcode = AARCH64_OPCODE_STR;
        if (type->kind == LL_TYPE_FLOAT) {
            if (type->width <= 32) opcode = X86_64_OPCODE_MOVSS;
            else if (type->width <= 64) opcode = X86_64_OPCODE_MOVSD;
            else oc_todo("add widths");
        }

        Call_Convention_Parameter parameter_location = aarch64_call_convention_next(&callconv, type, &stack_used);
        switch (OPD_TYPE(arg_operand)) {
            case LL_IR_OPERAND_REGISTER_BIT: 
                if (parameter_location.is_reg) {
                    aarch64_write_literal_int(&b->native_fn_stub_writer, parameter_location.reg, b->invoke_prealloc.items[j].immediate_displacement);
                } else {
                    uint32 tmp_reg = active_register_top;
                    params.rd = tmp_reg;
                    aarch64_write_literal_int(&b->native_fn_stub_writer, tmp_reg, b->invoke_prealloc.items[j].immediate_displacement);

                    params.immediate = (int32_t)parameter_location.stack_offset;
                    params.rd = 31;
                    params.rn = tmp_reg;
                    OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, opcode, xd_xn_imm12, params);
                }

                break;

            case LL_IR_OPERAND_IMMEDIATE_BIT:
                if (parameter_location.is_reg) {
                    aarch64_write_literal_int(&b->native_fn_stub_writer, parameter_location.reg, OPD_VALUE(OPD_VALUE(arg_operand)));
                } else {
                    uint32 tmp_reg = active_register_top;
                    aarch64_write_literal_int(&b->native_fn_stub_writer, tmp_reg, OPD_VALUE(OPD_VALUE(arg_operand)));

                    params.immediate = (int32_t)parameter_location.stack_offset;
                    params.rd = 31;
                    params.rn = tmp_reg;
                    OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, opcode, xd_xn_imm12, params);
                }

                break;
            case LL_IR_OPERAND_IMMEDIATE64_BIT:
                if (parameter_location.is_reg) {
                    aarch64_write_literal_int(&b->native_fn_stub_writer, parameter_location.reg, FUNCTION()->literals.items[OPD_VALUE(arg_operand)].as_u64);
                } else {
                    aarch64_write_literal_int(&b->native_fn_stub_writer, active_register_top, FUNCTION()->literals.items[OPD_VALUE(arg_operand)].as_u64);

                    params.rn = params.rd;
                    params.immediate = (int32_t)parameter_location.stack_offset;
                    params.rd = 31;
                    OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_STR, xd_xn_imm12, params);
                }
                break;
            default: oc_todo("unhadnled argument operand: {x}", OPD_TYPE(arg_operand));
        }
    }

    int64_t result = -1;
    switch (OPD_TYPE(invokee)) {
    case LL_IR_OPERAND_FUNCTION_BIT: {
        LL_Ir_Function* fn = &bir->fns.items[OPD_VALUE(invokee)];
        oc_assert(fn->flags & LL_IR_FUNCTION_FLAG_NATIVE);;

        void (*native_fn_ptr)() = ll_native_fn_get(cc, &b->native_funcs, fn->ident->str);
        if (!native_fn_ptr) {
            eprint("Unable to find native function: {}\n", fn->ident->str);
            return result;
        }

        result = fn->generated_offset;

        aarch64_write_literal_int(&b->native_fn_stub_writer, active_register_top, (uint64)native_fn_ptr);

        OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_BLR, xn, ((AArch64_Instruction_Parameters){ .rn = active_register_top }));

        stack_used = oc_align_forward(stack_used, 16);
        uint32* pstack_size = (uint32*)&b->native_fn_stub_section.items[stack_size_offset];
        *pstack_size = ((stack_used & 0xFFFu) << 10u) | (*pstack_size & ~(0xFFFu << 10u));

        OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_ADD, xd_xn_imm12_shift, ((AArch64_Instruction_Parameters){ .rd = 31, .rn = 31, .immediate = stack_used }));
        OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_LDP_post, xd_xd2_xn_imm7, ((AArch64_Instruction_Parameters){ .rd = 29, .rn = 31, .rd2 = 30, .immediate = 0x10 }));


        params.rn = 0b11110u;
        params.shift = 0;
        OC_AARCH64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, AARCH64_OPCODE_RET, xn, params);

        FILE* fptr;
        if (fopen_s(&fptr, "potato.bin", "wb")) {
            eprint("Unable to open output file: %s\n", "potato.bin");
            return false;
        }

        fwrite(b->native_fn_stub_section.items, 1, b->native_fn_stub_section.count, fptr);
        fclose(fptr);
        copy_native_code(cc, b);

        break;
    }
    default: oc_todo("handle inveok type"); break;
    }

    // if (opcode == LL_IR_OPCODE_INVOKEVALUE) {
    //     if (return_type && is_large_aggregate_type(return_type)) {
    //         params.reg0 = x86_64_backend_active_registers[active_register_top];
    //         params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
    //         params.displacement = -return_struct_offset;
    //         OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_LEA, r64_rm64, params);

    //         X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, fn_type->return_type, params.reg0);
    //         FRAME()->registers.items[OPD_VALUE(operands[0])] = offset;
    //     } else {
    //         X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, fn_type->return_type, X86_64_OPERAND_REGISTER_rax);
    //         FRAME()->registers.items[OPD_VALUE(operands[0])] = offset;
    //     }
    // }

    return result;
}

int64_t do_native_fn_call(
    Compiler_Context* cc,
    LL_Eval_Context* b,
    LL_Backend_Ir* bir,
    // LL_Ir_Function* invokee_fn,
    LL_Ir_Operand invokee,
    uint32_t operand_count,
    LL_Ir_Operand* operands
) {
    uint32_t active_register_top = 0;
    uint32_t stack_used = 0;
    X86_64_Instruction_Parameters params = { 0 };

    X86_64_Call_Convention callconv = x86_64_call_convention_host();


    switch (OPD_TYPE(invokee)) {
    case LL_IR_OPERAND_FUNCTION_BIT: {
        LL_Ir_Function* fn = &bir->fns.items[OPD_VALUE(invokee)];
        fn->generated_offset = (int64_t)b->native_fn_stub_section.count;
    } break;
    default: oc_todo("fn type"); break;
    }

    OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_SUB, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = 0xface0102 }));
    size_t stack_size_offset = b->native_fn_stub_section.count - 4;


    LL_Ir_Function* invokee_fn = &bir->fns.items[OPD_VALUE(invokee)];
    LL_Type_Function* fn_type = (LL_Type_Function*)invokee_fn->ident->base.type;
    assert(fn_type->base.kind == LL_TYPE_FUNCTION);

    LL_Type* return_type = fn_type->return_type ? ll_get_base_type(fn_type->return_type) : NULL;

    b->invoke_prealloc.count = 0;
    oc_array_reserve(&cc->arena, &b->invoke_prealloc, operand_count);

    uword invoke_offset = 0;
    uword pre_invoke_offset = invoke_offset;
    for (uint32_t j = 0; j < operand_count; ++j) {
        LL_Ir_Operand arg_operand = operands[pre_invoke_offset++];
        LL_Type* type = ir_get_operand_type(bir, FUNCTION(), arg_operand);

        uint32_t opcode = X86_64_OPCODE_MOV;
        // if (type->kind == LL_TYPE_FLOAT) {
        //     if (type->width <= 32) opcode = X86_64_OPCODE_MOVSS;
        //     else if (type->width <= 64) opcode = X86_64_OPCODE_MOVSD;
        //     else oc_todo("add widths");
        // }

        X86_64_Invoke_Prealloc prealloc = { 0 };
        switch (OPD_TYPE(arg_operand)) {
            case LL_IR_OPERAND_REGISTER_BIT: 
                if (is_large_aggregate_type(type)) {
                    oc_todo("struct");
                    // prealloc.immediate_displacement = (uint32_t)x86_64_make_struct_copy(cc, b, bir, type, arg_operand, false);
                    // prealloc.opcode = X86_64_OPCODE_LEA;
                    // prealloc.variant = X86_64_VARIANT_KIND_r64_rm64;
                } else {
                    prealloc.immediate_displacement = FRAME()->registers.items[OPD_VALUE(arg_operand)].as_i64;
                    prealloc.opcode = opcode;
                    prealloc.variant = X86_64_VARIANT_KIND_r64_i64;
                }
                break;
            case LL_IR_OPERAND_IMMEDIATE_BIT:
                prealloc.immediate_displacement = (int64_t)OPD_VALUE(arg_operand);
                prealloc.opcode = X86_64_OPCODE_MOV;
                prealloc.variant = X86_64_VARIANT_KIND_rm64_i32;
                break;
            case LL_IR_OPERAND_IMMEDIATE64_BIT:
                prealloc.immediate_displacement = (int64_t)FUNCTION()->literals.items[OPD_VALUE(arg_operand)].as_u64;
                prealloc.opcode = X86_64_OPCODE_MOV;
                prealloc.variant = X86_64_VARIANT_KIND_r64_i64;
                break;
            default: oc_todo("unhadnled argument operand: {x}", OPD_TYPE(arg_operand));
        }

        b->invoke_prealloc.items[j] = prealloc;
    }
    

    uword return_struct_offset;
    if (return_type && is_large_aggregate_type(return_type)) {
        oc_todo("struct return");
        Call_Convention_Parameter param = x86_64_call_convention_next(&callconv, return_type, &stack_used);

        // return_struct_offset = x86_64_make_struct_copy(cc, b, bir, return_type, 0, true);
        if (param.is_reg) {
            params.reg0 = param.reg;
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -return_struct_offset;
            OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_LEA, r64_rm64, params);
        } else {
            X86_64_Operand_Register tmp_reg;
            tmp_reg = x86_64_backend_active_registers[active_register_top];
            params.reg0 = tmp_reg;
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -return_struct_offset;
            OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_LEA, r64_rm64, params);

            params.displacement = param.stack_offset;
            params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
            params.reg1 = tmp_reg;
            OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_MOV, rm64_r64, params);
        }
    }


    for (uint32_t j = 0; j < operand_count; ++j) {
        LL_Ir_Operand arg_operand = operands[invoke_offset++];
        LL_Type* type = ir_get_operand_type(bir, FUNCTION(), arg_operand);

        uint32_t opcode = X86_64_OPCODE_MOV;
        if (type->kind == LL_TYPE_FLOAT) {
            if (type->width <= 32) opcode = X86_64_OPCODE_MOVSS;
            else if (type->width <= 64) opcode = X86_64_OPCODE_MOVSD;
            else oc_todo("add widths");
        }

        Call_Convention_Parameter parameter_location = x86_64_call_convention_next(&callconv, type, &stack_used);
        switch (OPD_TYPE(arg_operand)) {
            case LL_IR_OPERAND_REGISTER_BIT: 
                if (parameter_location.is_reg) {
                    params.reg0 = parameter_location.reg;
                    params.immediate = b->invoke_prealloc.items[j].immediate_displacement;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(&b->native_fn_stub_writer, b->invoke_prealloc.items[j].opcode, b->invoke_prealloc.items[j].variant, params);
                } else {
                    X86_64_Operand_Register tmp_reg;

                    tmp_reg = x86_64_backend_active_registers[active_register_top];
                    params.reg0 = tmp_reg;
                    params.immediate = b->invoke_prealloc.items[j].immediate_displacement;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(&b->native_fn_stub_writer, b->invoke_prealloc.items[j].opcode, b->invoke_prealloc.items[j].variant, params);

                    params.displacement = (int32_t)parameter_location.stack_offset;
                    params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
                    params.reg1 = tmp_reg;
                    OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, opcode, rm64_r64, params);
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
                OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_MOV, rm64_i32, params);

                break;
            case LL_IR_OPERAND_IMMEDIATE64_BIT:
                if (parameter_location.is_reg) {
                    params.reg0 = parameter_location.reg;
                    params.immediate = FUNCTION()->literals.items[OPD_VALUE(arg_operand)].as_u64;
                    OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_MOV, r64_i64, params);
                } else {
                    params.reg0 = x86_64_backend_active_registers[active_register_top];
                    params.immediate = FUNCTION()->literals.items[OPD_VALUE(arg_operand)].as_u64;
                    OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_MOV, r64_i64, params);

                    params.reg1 = params.reg0;
                    params.displacement = (int32_t)parameter_location.stack_offset;
                    params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
                    OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_MOV, rm64_r64, params);
                }
                break;
            default: oc_todo("unhadnled argument operand: {x}", OPD_TYPE(arg_operand));
        }
    }

    int64_t result = -1;
    switch (OPD_TYPE(invokee)) {
    case LL_IR_OPERAND_FUNCTION_BIT: {
        LL_Ir_Function* fn = &bir->fns.items[OPD_VALUE(invokee)];
        oc_assert(fn->flags & LL_IR_FUNCTION_FLAG_NATIVE);;

        void (*native_fn_ptr)() = ll_native_fn_get(cc, &b->native_funcs, fn->ident->str);
        if (!native_fn_ptr) {
            eprint("Unable to find native function: {}\n", fn->ident->str);
            return result;
        }

        result = fn->generated_offset;

        params.reg0 = X86_64_OPERAND_REGISTER_rax;
        params.immediate = (uword)native_fn_ptr;
        OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_MOV, r64_i64, params);
        OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_CALL, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rax }));

        stack_used = oc_align_forward(stack_used, 16) ;
        int32_t* pstack_size = (int32_t*)&b->native_fn_stub_section.items[stack_size_offset];
        *pstack_size = stack_used;
        OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_ADD, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = stack_used }));

        OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_RET, noarg, params);

        copy_native_code(cc, b);

        break;
    }
    default: oc_todo("handle inveok type"); break;
    }

    // if (opcode == LL_IR_OPCODE_INVOKEVALUE) {
    //     if (return_type && is_large_aggregate_type(return_type)) {
    //         params.reg0 = x86_64_backend_active_registers[active_register_top];
    //         params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
    //         params.displacement = -return_struct_offset;
    //         OC_X86_64_WRITE_INSTRUCTION(&b->native_fn_stub_writer, X86_64_OPCODE_LEA, r64_rm64, params);

    //         X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, fn_type->return_type, params.reg0);
    //         FRAME()->registers.items[OPD_VALUE(operands[0])] = offset;
    //     } else {
    //         X86_64_Register offset = x86_64_move_reg_alloc(cc, b, bir, fn_type->return_type, X86_64_OPERAND_REGISTER_rax);
    //         FRAME()->registers.items[OPD_VALUE(operands[0])] = offset;
    //     }
    // }

    return result;
}

LL_Eval_Value ll_eval_allocate_object(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, LL_Type* type) {
    (void)bir;
    LL_Backend_Layout layout = cc->native_target->get_layout(type);
    LL_Eval_Value result;
    switch (type->kind) {
    case LL_TYPE_ARRAY:
        result.as_object = oc_arena_alloc_aligned(&b->object_arena, layout.size, layout.alignment);
        break;
    case LL_TYPE_STRUCT:
        result.as_object = oc_arena_alloc_aligned(&b->object_arena, layout.size, layout.alignment);
        break;
    default: ll_print_type(type); oc_assert(false && "unsupported type"); break;
    }
    return result;
}

static void ll_eval_set_value(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, LL_Ir_Operand lvalue, LL_Eval_Value rvalue, bool store) {
    (void)cc;
    LL_Type* type;
    type = ir_get_operand_type(bir, FUNCTION(), lvalue);

    LL_Eval_Registers* storage = get_storage_location(cc, b, lvalue);

    switch (OPD_TYPE(lvalue)) {
    case LL_IR_OPERAND_LOCAL_BIT:
    case LL_IR_OPERAND_REGISTER_BIT:
    case LL_IR_OPERAND_PARMAETER_BIT:
        break;
    default: oc_assert("invalid lvalue"); break;
    }

    switch (type->kind) {
    case LL_TYPE_ANYBOOL:
    case LL_TYPE_BOOL:
    case LL_TYPE_CHAR:
    case LL_TYPE_UINT:
    case LL_TYPE_INT:
    case LL_TYPE_FLOAT:
        storage->items[OPD_VALUE(lvalue)] = rvalue;
        break;
    case LL_TYPE_POINTER:
        if (store && OPD_TYPE(lvalue) == LL_IR_OPERAND_REGISTER_BIT) {
            *storage->items[OPD_VALUE(lvalue)].as_ptr = rvalue;
        }
        storage->items[OPD_VALUE(lvalue)] = rvalue;
        break;
    case LL_TYPE_STRUCT:
    case LL_TYPE_ARRAY: {
        LL_Backend_Layout layout = cc->native_target->get_layout(type);
        if (!storage->items[OPD_VALUE(lvalue)].as_object) {
            storage->items[OPD_VALUE(lvalue)] = ll_eval_allocate_object(cc, b, bir, type);
        }
        memcpy(storage->items[OPD_VALUE(lvalue)].as_object, rvalue.as_object, layout.size);
    } break;
    default: oc_assert(false); break;
    }

}

static LL_Eval_Value ll_eval_get_value(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, LL_Ir_Operand lvalue) {
    (void)cc;
    LL_Eval_Value result;
    LL_Type* type;


    switch (OPD_TYPE(lvalue)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        result.as_u64 = (uint64_t)OPD_VALUE(lvalue);
        break;
    case LL_IR_OPERAND_IMMEDIATE64_BIT:
        result.as_u64 = FUNCTION()->literals.items[OPD_VALUE(lvalue)].as_u64;
        break;
    case LL_IR_OPERAND_LOCAL_BIT:
    case LL_IR_OPERAND_REGISTER_BIT:
    case LL_IR_OPERAND_PARMAETER_BIT: {
        LL_Eval_Registers* storage = get_storage_location(cc, b, lvalue);
        type = ir_get_operand_type(bir, FUNCTION(), lvalue);
        switch (type->kind) {
        case LL_TYPE_ANYBOOL:
        case LL_TYPE_BOOL:
        case LL_TYPE_CHAR:
        case LL_TYPE_UINT:
        case LL_TYPE_INT:
        case LL_TYPE_FLOAT:
        case LL_TYPE_POINTER:
            result = storage->items[OPD_VALUE(lvalue)];
            break;
        case LL_TYPE_ARRAY:
        case LL_TYPE_STRUCT:
            result = storage->items[OPD_VALUE(lvalue)];
            if (!result.as_object) {
                storage->items[OPD_VALUE(lvalue)] = ll_eval_allocate_object(cc, b, bir, type);
                result = storage->items[OPD_VALUE(lvalue)];
            }
            break;
        default: result.as_u64 = 0; oc_todo("add error"); break;
        }
    } break;
    case LL_IR_OPERAND_DATA_BIT: {
        type = ir_get_operand_type(bir, FUNCTION(), lvalue);
        switch (type->kind) {
        case LL_TYPE_SLICE:
        case LL_TYPE_STRING:
        case LL_TYPE_ARRAY:
        case LL_TYPE_STRUCT:
            result.as_object = bir->data_items.items[OPD_VALUE(lvalue)].ptr;
            break;
        default: result.as_u64 = 0; oc_todo("add error"); break;
        }
    } break;
    default: oc_todo("implementn get value operand type\n"); break;
    }

    return result;
}


static void ll_eval_load(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, LL_Ir_Operand result, LL_Ir_Operand src, bool load) {
    (void)cc;
    LL_Type* to_type = ir_get_operand_type(bir, FUNCTION(), result);
    // @TODO: why did we have this???
    // LL_Type* from_type;

    // switch (OPD_TYPE(src)) {
    // case LL_IR_OPERAND_LOCAL_BIT: {
    //     from_type = FUNCTION()->locals.items[OPD_VALUE(src)].ident->base.type;
    //     break;
    // }
    // case LL_IR_OPERAND_REGISTER_BIT: {
    //     from_type = ir_get_operand_type(bir, FUNCTION(), src);
    //     break;
    // }
    // case LL_IR_OPERAND_IMMEDIATE_BIT: {
    //     from_type = to_type;
    //     break;
    // }
    // default: oc_todo("add load operands"); break;
    // }

    LL_Eval_Registers* storage = get_storage_location(cc, b, src);
    LL_Eval_Value value;
    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_LOCAL_BIT:
    case LL_IR_OPERAND_PARMAETER_BIT:
        value = storage->items[OPD_VALUE(src)];
        break;
    case LL_IR_OPERAND_REGISTER_BIT:
        value = storage->items[OPD_VALUE(src)];
        if (load) {
            value = *value.as_ptr;
        }
        break;
    case LL_IR_OPERAND_IMMEDIATE_BIT: {
        switch (to_type->kind) {
        case LL_TYPE_ANYBOOL:
        case LL_TYPE_BOOL:
        case LL_TYPE_CHAR:
        case LL_TYPE_UINT:
            FRAME()->registers.items[OPD_VALUE(result)].as_u64 = OPD_VALUE(src);
            break;
        case LL_TYPE_ANYINT:
        case LL_TYPE_INT:
            FRAME()->registers.items[OPD_VALUE(result)].as_i64 = OPD_VALUE(src);
            break;
        default: oc_todo("unhandled type"); break;
        }

        return;
    }
    case LL_IR_OPERAND_IMMEDIATE64_BIT: {
        LL_Ir_Literal literal = FUNCTION()->literals.items[OPD_VALUE(src)];

        switch (to_type->kind) {
        case LL_TYPE_ANYBOOL:
        case LL_TYPE_BOOL:
        case LL_TYPE_CHAR:
        case LL_TYPE_UINT:
            FRAME()->registers.items[OPD_VALUE(result)].as_u64 = literal.as_u64;
            break;
        case LL_TYPE_ANYINT:
        case LL_TYPE_INT:
            FRAME()->registers.items[OPD_VALUE(result)].as_i64 = (int64_t)literal.as_u64;
            break;
        case LL_TYPE_ANYFLOAT:
        case LL_TYPE_FLOAT:
            if (to_type->width <= 32) {
                FRAME()->registers.items[OPD_VALUE(result)].as_f64 = literal.as_f32;
            } else if (to_type->width <= 64) {
                FRAME()->registers.items[OPD_VALUE(result)].as_f64 = literal.as_f64;
            } else oc_unreachable("invalid type");
            break;
        default: break;
        }

        return;
    }
    default: oc_todo("add load operands"); break;
    }

    switch (to_type->kind) {
    case LL_TYPE_ANYBOOL:
    case LL_TYPE_BOOL:
    case LL_TYPE_CHAR:
    case LL_TYPE_ANYINT:
    case LL_TYPE_UINT:
    case LL_TYPE_INT:
    case LL_TYPE_ANYFLOAT:
    case LL_TYPE_FLOAT:
    case LL_TYPE_POINTER:
    case LL_TYPE_ARRAY:
    case LL_TYPE_STRUCT:
        FRAME()->registers.items[OPD_VALUE(result)] = value;
        break;
    default: oc_todo("unhandled type"); break;
    }
}

static void ll_eval_block(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
    size_t i;
    int32_t opcode1, opcode2;
    LL_Type* type;

    (void)opcode1;
    (void)opcode2;

    for (i = 0; i < block->ops.count; ) {
        uint32_t invoke_offset = 0;
        LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
        LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];

        switch (opcode) {
        case LL_IR_OPCODE_BRANCH:
            FRAME()->next_block = operands[0];
            return;
        case LL_IR_OPCODE_BRANCH_COND:
            if (ll_eval_get_value(cc, b, bir, operands[0]).as_u64) {
                FRAME()->next_block = operands[1];
            } else {
                FRAME()->next_block = operands[2];
            }
            return;
        case LL_IR_OPCODE_RET:
            break;
        case LL_IR_OPCODE_RETVALUE: {
            LL_Type_Function* fn_type = (LL_Type_Function*)FUNCTION()->ident->base.type;
            oc_assert(fn_type->base.kind == LL_TYPE_FUNCTION);
            FRAME()->return_value = ll_eval_get_value(cc, b, bir, operands[0]);
        } break;
        case LL_IR_OPCODE_STORE: {
            LL_Eval_Value value = ll_eval_get_value(cc, b, bir, operands[1]);
            ll_eval_set_value(cc, b, bir, operands[0], value, true);
        } break;

#define DO_OP(op) \
            type = ir_get_operand_type(bir, FUNCTION(), operands[0]); \
            switch (type->kind) { \
            case LL_TYPE_ANYINT: \
            case LL_TYPE_INT: \
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_u64 = (FRAME()->registers.items[OPD_VALUE(operands[1])].as_i64 op FRAME()->registers.items[OPD_VALUE(operands[2])].as_i64) ? 1 : 0; \
                break; \
            case LL_TYPE_CHAR: \
            case LL_TYPE_UINT: \
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_u64 = (FRAME()->registers.items[OPD_VALUE(operands[1])].as_u64 op FRAME()->registers.items[OPD_VALUE(operands[2])].as_u64) ? 1 : 0; \
                break; \
            case LL_TYPE_ANYBOOL: \
            case LL_TYPE_BOOL: \
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_u64 = (FRAME()->registers.items[OPD_VALUE(operands[1])].as_u64 op FRAME()->registers.items[OPD_VALUE(operands[2])].as_u64) ? 1 : 0; \
                break; \
            case LL_TYPE_ANYFLOAT: \
            case LL_TYPE_FLOAT: \
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_f64 = (FRAME()->registers.items[OPD_VALUE(operands[1])].as_f64 op FRAME()->registers.items[OPD_VALUE(operands[2])].as_f64) ? 1 : 0; \
                break; \
            default: oc_todo("implement types for operations"); break; \
            }
        case LL_IR_OPCODE_EQ: DO_OP(==); break;
        case LL_IR_OPCODE_NEQ: DO_OP(!=); break;
        case LL_IR_OPCODE_GTE: DO_OP(>=); break;
        case LL_IR_OPCODE_GT: DO_OP(>); break;
        case LL_IR_OPCODE_LTE: DO_OP(<=); break;
        case LL_IR_OPCODE_LT: DO_OP(<); break;
#undef DO_OP
#define DO_OP(op) \
            type = ir_get_operand_type(bir, FUNCTION(), operands[0]); \
            switch (type->kind) { \
            case LL_TYPE_ANYINT: \
            case LL_TYPE_INT: \
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_i64 = ll_eval_get_value(cc, b, bir, operands[1]).as_i64 op ll_eval_get_value(cc, b, bir, operands[2]).as_i64; \
                break; \
            case LL_TYPE_CHAR: \
            case LL_TYPE_UINT: \
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_u64 = ll_eval_get_value(cc, b, bir, operands[1]).as_u64 op ll_eval_get_value(cc, b, bir, operands[2]).as_u64; \
                break; \
            case LL_TYPE_ANYFLOAT: \
            case LL_TYPE_FLOAT: \
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_f64 = ll_eval_get_value(cc, b, bir, operands[1]).as_f64 op ll_eval_get_value(cc, b, bir, operands[2]).as_f64; \
                break; \
            default: oc_todo("implement types for operations"); break; \
            }
        case LL_IR_OPCODE_ADD: DO_OP(+) break; 
        case LL_IR_OPCODE_SUB: DO_OP(-) break; 
        case LL_IR_OPCODE_MUL: DO_OP(*) break;
        case LL_IR_OPCODE_DIV: DO_OP(/) break;
#undef DO_OP
        case LL_IR_OPCODE_MOD:
            type = ir_get_operand_type(bir, FUNCTION(), operands[0]);
            switch (type->kind) {
            case LL_TYPE_INT:
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_i64 = ll_eval_get_value(cc, b, bir, operands[1]).as_i64 % ll_eval_get_value(cc, b, bir, operands[2]).as_i64;
                break;
            case LL_TYPE_CHAR: \
            case LL_TYPE_UINT:
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_u64 = ll_eval_get_value(cc, b, bir, operands[1]).as_u64 % ll_eval_get_value(cc, b, bir, operands[2]).as_u64;
                break;
            case LL_TYPE_ANYFLOAT: \
            case LL_TYPE_FLOAT: {
                extern double fmod(double, double);
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_f64 = fmod(ll_eval_get_value(cc, b, bir, operands[1]).as_f64, ll_eval_get_value(cc, b, bir, operands[2]).as_f64);
            } break;
            default: oc_todo("implement types for operations"); break;
            }
            break;
        
#define DO_OP(op) \
            type = ir_get_operand_type(bir, FUNCTION(), operands[0]); \
            switch (type->kind) { \
            case LL_TYPE_ANYINT: \
            case LL_TYPE_INT: \
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_i64 = ll_eval_get_value(cc, b, bir, operands[1]).as_i64 op ll_eval_get_value(cc, b, bir, operands[2]).as_i64; \
                break; \
            case LL_TYPE_ANYBOOL: \
            case LL_TYPE_BOOL: \
            case LL_TYPE_CHAR: \
            case LL_TYPE_UINT: \
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_u64 = ll_eval_get_value(cc, b, bir, operands[1]).as_u64 op ll_eval_get_value(cc, b, bir, operands[2]).as_u64; \
                break; \
            default: oc_todo("implement types for operations"); break; \
            }
        case LL_IR_OPCODE_AND: DO_OP(&) break; 
        case LL_IR_OPCODE_OR: DO_OP(|) break; 
        case LL_IR_OPCODE_XOR: DO_OP(^) break;
#undef DO_OP
        case LL_IR_OPCODE_NEG: {
            type = ir_get_operand_type(bir, FUNCTION(), operands[0]);
            LL_Eval_Value value = ll_eval_get_value(cc, b, bir, operands[1]);

            switch (type->kind) {
            case LL_TYPE_INT:
                value.as_i64 = -value.as_i64;
                break;
            case LL_TYPE_FLOAT:
                value.as_f64 = -value.as_f64;
                break;
            default: oc_todo("implement types for operations"); break;
            }

            FRAME()->registers.items[OPD_VALUE(operands[0])] = value;
        } break;

        case LL_IR_OPCODE_NOT: {
            type = ir_get_operand_type(bir, FUNCTION(), operands[0]);
            LL_Eval_Value value = ll_eval_get_value(cc, b, bir, operands[1]);

            switch (type->kind) {
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT:
                value.as_i64 = ~value.as_i64;
                break;
            case LL_TYPE_UINT:
                value.as_u64 = ~value.as_u64;
                break;
            case LL_TYPE_BOOL:
                value.as_u64 = ~value.as_u64;
                break;
            default: oc_todo("implement types for operations"); break;
            }

            FRAME()->registers.items[OPD_VALUE(operands[0])] = value;
        } break;

        case LL_IR_OPCODE_TEST: {
            type = ir_get_operand_type(bir, FUNCTION(), operands[0]);
            LL_Eval_Value value = ll_eval_get_value(cc, b, bir, operands[1]);

            switch (type->kind) {
            case LL_TYPE_CHAR:
                value.as_u64 = value.as_u64 ? (uint64_t)(-1LL) : 0uLL;
                break;
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT:
                value.as_u64 = value.as_i64 ? (uint64_t)(-1LL) : 0uLL;
                break;
            case LL_TYPE_UINT:
                value.as_u64 = value.as_u64 ? (uint64_t)(-1LL) : 0uLL;
                break;
            case LL_TYPE_BOOL:
                value.as_u64 = value.as_u64;
                break;
            default: oc_todo("implement types for operations"); break;
            }

            FRAME()->registers.items[OPD_VALUE(operands[0])] = value;
        } break;
        
        case LL_IR_OPCODE_CAST: {
            ll_eval_load(cc, b, bir, operands[0], operands[1], false);
        } break;
        case LL_IR_OPCODE_LOAD: {
            ll_eval_load(cc, b, bir, operands[0], operands[1], true);
        } break;
        case LL_IR_OPCODE_CLONE: {
            ll_eval_load(cc, b, bir, operands[0], operands[1], false);
        } break;
        case LL_IR_OPCODE_ALIAS: {
            ll_eval_load(cc, b, bir, operands[0], operands[1], false);
        } break;
        case LL_IR_OPCODE_MEMCOPY: {
            LL_Eval_Registers* storage = get_storage_location(cc, b, operands[0]);
            LL_Eval_Value value;

            switch (OPD_TYPE(operands[0])) {
            case LL_IR_OPERAND_LOCAL_BIT:
            case LL_IR_OPERAND_PARMAETER_BIT:
                if (!storage->items[OPD_VALUE(operands[0])].as_object) {
                    storage->items[OPD_VALUE(operands[0])] = ll_eval_allocate_object(cc, b, bir, ir_get_operand_type(bir, FUNCTION(), operands[0]));
                }
                value = storage->items[OPD_VALUE(operands[0])];
                break;
            case LL_IR_OPERAND_REGISTER_BIT:
                value = storage->items[OPD_VALUE(operands[0])];
                value = *value.as_ptr;
                if (!value.as_object) {
                    *value.as_ptr = ll_eval_allocate_object(cc, b, bir, ir_get_operand_type(bir, FUNCTION(), operands[0]));
                    value = *value.as_ptr;
                }
                break;
            default: oc_assert(false); break;
            }

            LL_Eval_Value rvalue = ll_eval_get_value(cc, b, bir, operands[1]);
            LL_Eval_Value size = ll_eval_get_value(cc, b, bir, operands[2]);

            memcpy(value.as_object, rvalue.as_object, size.as_u64);
        } break;
        case LL_IR_OPCODE_LEA: {
            oc_assert(OPD_TYPE(operands[0]) == LL_IR_OPERAND_REGISTER_BIT);
            switch (OPD_TYPE(operands[1])) {
            case LL_IR_OPERAND_LOCAL_BIT:
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_ptr = &FRAME()->locals.items[OPD_VALUE(operands[1])];
                break;
            case LL_IR_OPERAND_PARMAETER_BIT:
                FRAME()->registers.items[OPD_VALUE(operands[0])].as_ptr = &FRAME()->parameters.items[OPD_VALUE(operands[1])];
                break;
            case LL_IR_OPERAND_DATA_BIT:
                oc_todo("handle this");
                break;
            default: oc_assert(false && "invalid operand"); break;
            }
        } break;
        case LL_IR_OPCODE_LEA_INDEX: {
            LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)ir_get_operand_type(bir, FUNCTION(), operands[0]);
            assert(ptr_type->base.kind == LL_TYPE_POINTER);

            LL_Eval_Value base = ll_eval_get_value(cc, b, bir, operands[1]);
            LL_Eval_Value index = ll_eval_get_value(cc, b, bir, operands[2]);
            LL_Eval_Value scale = ll_eval_get_value(cc, b, bir, operands[3]);

            base.as_object += index.as_i64 * scale.as_i64;

            ll_eval_set_value(cc, b, bir, operands[0], base, false);
        } break;
        case LL_IR_OPCODE_INVOKEVALUE:
            invoke_offset = 1;
        case LL_IR_OPCODE_INVOKE: {
            uint32_t invokee = operands[invoke_offset++];
            uint32_t count = operands[invoke_offset++];
            LL_Eval_Value return_value = { 0 };

            switch (OPD_TYPE(invokee)) {
            case LL_IR_OPERAND_FUNCTION_BIT: {
                LL_Ir_Function* fn = &bir->fns.items[OPD_VALUE(invokee)];

                if (fn->flags & LL_IR_FUNCTION_FLAG_NATIVE) {
                    size_t old_section_size = b->native_fn_stub_section.count;

                    int64_t offset = do_native_fn_call_aarch64(cc, b, bir, invokee, count, operands + invoke_offset);
                    if (offset == -1) break;
                    if (!b->native_fn_exe_code) break;
                    
                    void (*fn_ptr)() = (void (*)())(b->native_fn_exe_code + offset);
                    fn_ptr();

                    b->native_fn_stub_section.count = old_section_size;
                } else {
                    return_value = ll_eval_fn(cc, b, bir, OPD_VALUE(invokee), count, operands + invoke_offset);
                }
            } break;
            default: oc_todo("handle other op");
            }

            if (opcode == LL_IR_OPCODE_INVOKEVALUE) {
                FRAME()->registers.items[OPD_VALUE(operands[0])] = return_value;
            }
            
            break;
        }
        default: oc_todo("handle other op"); break;
        }

        size_t count = ir_get_op_count(cc, bir, (LL_Ir_Opcode*)block->ops.items, i);
        i += count;
    }
}

LL_Eval_Value ll_eval_fn(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, uint32_t fn_index, uint32_t argument_count, LL_Ir_Operand* arguments) {
    LL_Ir_Function* fn = &bir->fns.items[fn_index];
    oc_array_append(&cc->arena, &b->frames, ((LL_Eval_Frame) { 0 }));

    int32_t last_function = bir->current_function;
    LL_Ir_Block_Ref last_block = bir->current_block;
    bir->current_function = fn_index;

    // @NOTE: these storage locations should be stable so we can take references to them.

    FRAME()->registers.count = 0;
    oc_array_reserve(&cc->arena, &FRAME()->registers, FUNCTION()->registers.count);

    FRAME()->locals.count = 0;
    oc_array_reserve(&cc->arena, &FRAME()->locals, FUNCTION()->locals.count);

    FRAME()->parameters.count = 0;
    oc_array_reserve(&cc->arena, &FRAME()->parameters, argument_count);
    b->frames.count--;
    for (uint32_t ai = 0; ai < argument_count; ai++) {
        LL_Eval_Value argument_value = ll_eval_get_value(cc, b, bir, arguments[ai]);
        FRAMEN(-1)->parameters.items[ai] = argument_value;
    }
    b->frames.count++;

    LL_Ir_Block_Ref current_block = fn->entry;
    while (current_block) {
        bir->current_block = current_block;
        FRAME()->next_block = bir->blocks.items[current_block].next;
        ll_eval_block(cc, b, bir, &bir->blocks.items[current_block]);
        current_block = FRAME()->next_block;
    }
    LL_Eval_Value return_value = FRAME()->return_value;

    bir->current_block = last_block;
    bir->current_function = last_function;

    return return_value;
}

void ll_eval_native_stub_append_op_segment_u8(void* w, uint8_t segment) {
    LL_Eval_Context* b = w - offsetof(LL_Eval_Context, native_fn_stub_writer);
    segment = AS_LITTLE_ENDIAN_U8(segment);
    oc_array_append(b->arena, &b->native_fn_stub_section, segment);
}

void ll_eval_native_stub_append_op_segment_u16(void* w, uint16_t segment) {
    LL_Eval_Context* b = w - offsetof(LL_Eval_Context, native_fn_stub_writer);
    segment = AS_LITTLE_ENDIAN_U16(segment);
    oc_array_append_many(b->arena, &b->native_fn_stub_section, (uint8_t*)&segment, 2);
}

void ll_eval_native_stub_append_op_segment_u32(void* w, uint32_t segment) {
    LL_Eval_Context* b = w - offsetof(LL_Eval_Context, native_fn_stub_writer);
    segment = AS_LITTLE_ENDIAN_U32(segment);
    oc_array_append_many(b->arena, &b->native_fn_stub_section, (uint8_t*)&segment, 4);
}

void ll_eval_native_stub_append_op_segment_u64(void* w, uint64_t segment) {
    LL_Eval_Context* b = w - offsetof(LL_Eval_Context, native_fn_stub_writer);
    segment = AS_LITTLE_ENDIAN_U64(segment);
    oc_array_append_many(b->arena, &b->native_fn_stub_section, (uint8_t*)&segment, 8);
}

void ll_eval_native_stub_append_op_many(void* w, uint8_t* bytes, uint64_t count) {
    LL_Eval_Context* b = w - offsetof(LL_Eval_Context, native_fn_stub_writer);
    oc_array_append_many(b->arena, &b->native_fn_stub_section, (uint8_t*)bytes, count);
}

void ll_eval_native_stub_end_instruction(void* w) {
    (void)w;
    // x86_64 *ww = (x86_64*)w;
    // default_code_writer_append_stride(ww, ww->count);
    // oc_x86_64_write_nop(w, 1);
    // oc_array_append_many(b->arena, &b->section_text, (uint8_t*)bytes, count);
}

void ll_eval_init(Compiler_Context* cc, LL_Eval_Context* b) {
    b->arena = &cc->arena;
    
    b->native_fn_stub_writer.append_u8 = ll_eval_native_stub_append_op_segment_u8;
    b->native_fn_stub_writer.append_u16 = ll_eval_native_stub_append_op_segment_u16;
    b->native_fn_stub_writer.append_u32 = ll_eval_native_stub_append_op_segment_u32;
    b->native_fn_stub_writer.append_u64 = (void (*)(void *, unsigned long long))ll_eval_native_stub_append_op_segment_u64;
    b->native_fn_stub_writer.append_many = (void (*)(void *, unsigned char *, unsigned long long))ll_eval_native_stub_append_op_many;
    b->native_fn_stub_writer.end_instruction = ll_eval_native_stub_end_instruction;
}


LL_Eval_Value ll_eval_node(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, Code* expr) {
    LL_Eval_Value result;
    LL_Ir_Operand result_op;
    LL_Ir_Block_Ref entry_block_ref = bir->free_block ? bir->free_block : bir->blocks.count;
    LL_Ir_Block entry_block = { 0 };
    entry_block.generated_offset = -1;
    if (bir->free_block) {
        bir->free_block = bir->blocks.items[bir->free_block].next;
        memcpy(&bir->blocks.items[entry_block_ref], &entry_block, sizeof(entry_block));
    } else {
        oc_array_append(&cc->arena, &bir->blocks, entry_block);
    }




    {
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






    LL_Ir_Function fn = {
        .entry = entry_block_ref,
        .exit = entry_block_ref,
        .flags = 0,
        .generated_offset = LL_IR_FUNCTION_OFFSET_INVALID,
        .block_count = 1,
    };

    int32_t last_function = bir->current_function;
    LL_Ir_Block_Ref last_block = bir->current_block;

    if (bir->const_stack.count) {
        bir->current_function = (uint32_t)bir->fns.count; // top of stack is current
        oc_array_append(&cc->arena, &bir->fns, fn);
    } else {
        bir->current_function = 0; // top of stack is current
        bir->fns.items[0] = fn;
    }
    oc_array_append(&cc->arena, &bir->const_stack, fn);

    // bir->current_function = CURRENT_CONST_STACK | (bir->const_stack.count - 1); // top of stack is current
    bir->current_block = fn.entry;

    result_op = ir_generate_expression(cc, bir, expr, false);

// LL_Backend backend_ir = { .backend = bir };
// ll_backend_write_to_file(cc, &backend_ir, "out.ir");

    ll_eval_fn(cc, b, bir, 0, 0, NULL);

    bir->current_block = last_block;
    bir->current_function = last_function;
    bir->free_block = fn.entry;

    bir->const_stack.count--;

    switch (OPD_TYPE(result_op)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT: result.as_i64 = OPD_VALUE(result_op); break;
    case LL_IR_OPERAND_REGISTER_BIT: result = FRAME()->registers.items[OPD_VALUE(result_op)]; break;
    case LL_IR_OPERAND_LOCAL_BIT: result = FRAME()->locals.items[OPD_VALUE(result_op)]; break;
    }

    return result;
}

