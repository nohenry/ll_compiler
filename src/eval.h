#pragma once

#include "../backends/ir.h"
#include "../backends/x86_64.h"
#include "callconv.h"
#include "eval_value.h"

typedef struct {
    uint32_t count, capacity;
    LL_Eval_Value* items;
} LL_Eval_Registers;

typedef struct ll_eval_frame {
    LL_Ir_Block_Ref next_block;
    LL_Eval_Registers locals;
    LL_Eval_Registers registers;
    LL_Eval_Registers parameters;

    LL_Eval_Value return_value;
} LL_Eval_Frame;

typedef struct {
    LL_Eval_Frame* items;
    uint32_t count, capacity;
} LL_Eval_Frame_List;

typedef struct ll_eval_context {
    Oc_Arena* arena;
    LL_Eval_Frame_List frames;
    LL_Native_Function_Map native_funcs;

    X86_64_Invoke_Prealloc_List invoke_prealloc;

    OC_Machine_Code_Writer native_fn_stub_writer;
    X86_64_Section native_fn_stub_section;

    uint8_t* native_fn_exe_code;
    uword native_fn_exe_code_size;
} LL_Eval_Context;

LL_Eval_Value ll_eval_node(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, Ast_Base* expr);
void ll_eval_init(Compiler_Context* cc, LL_Eval_Context* b);
LL_Eval_Value ll_eval_fn(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, uint32_t fn_index, uint32_t argument_cound, LL_Ir_Operand* arguments);
