#pragma once

#include "backends/ir.h"

typedef struct {
	union {
		int64_t  ival;
		uint64_t uval;
	};
} LL_Eval_Value;

typedef struct {
	uint32_t count, capacity;
	LL_Eval_Value* items;
} LL_Eval_Registers;

typedef struct ll_eval_context {
	LL_Ir_Block_Ref next_block;
	LL_Eval_Registers locals;
	LL_Eval_Registers registers;
} LL_Eval_Context;

LL_Eval_Value ll_eval_node(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, Ast_Base* expr);

