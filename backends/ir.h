#pragma once

#include "stdint.h"
#include "../typer.h"

typedef uint32_t LL_Ir_Operand;
#define LL_IR_OPERAND_VALUE_MASK 0xFFFFFFFu
#define LL_IR_OPERAND_TYPE_MASK (~LL_IR_OPERAND_VALUE_MASK)
#define LL_IR_OPERAND_IMMEDIATE_BIT 0x00000000u
#define LL_IR_OPERAND_REGISTER_BIT 0x10000000u
#define LL_IR_OPERAND_LOCAL_BIT 0x20000000u
#define LL_IR_OPERAND_PARMAETER_BIT 0x30000000u
#define LL_IR_OPERAND_FUNCTION_BIT 0x40000000u

typedef struct {
	Ast_Ident* ident;
} LL_Ir_Local;

typedef struct {
	uint32_t count, capacity;
	LL_Ir_Local* items;
} LL_Ir_Local_List;

typedef struct {
	LL_Type* type;
} LL_Ir_Register;

typedef struct {
	uint32_t count, capacity;
	LL_Ir_Register* items;
} LL_Ir_Register_List;

typedef struct {
	Ast_Ident* ident;
	struct ll_ir_block* entry;
	LL_Ir_Local_List locals;
	LL_Ir_Register_List registers;
} LL_Ir_Function;

typedef enum {
	LL_IR_OPCODE_RET,
	LL_IR_OPCODE_STORE,
	LL_IR_OPCODE_LOAD,
	LL_IR_OPCODE_INVOKE,
	LL_IR_OPCODE_LEA,
} LL_Ir_Opcode;

typedef struct {
	LL_Ir_Opcode opcode;
	LL_Ir_Operand operands[];
} LL_Ir;

typedef struct {
	size_t count, capacity;
	uint32_t* items;
} LL_Ir_Op_List;

typedef struct ll_ir_block {
	struct ll_ir_block *next, *prev;
	LL_Ir_Op_List ops;
	bool did_return;
} LL_Ir_Block;

typedef struct {
	size_t count, capacity;
	LL_Ir_Function* items;
} LL_Ir_Function_List;

typedef struct {
	LL_Ir_Function_List fns;
	int32_t current_function;
	LL_Ir_Block* current_block;
} LL_Backend_Ir;

void ir_init(Compiler_Context* cc, LL_Backend_Ir* b);
bool ir_write_to_file(Compiler_Context* cc, LL_Backend_Ir* b, char* filepath);
void ir_generate_statement(Compiler_Context* cc, LL_Backend_Ir* b, Ast_Base* stmt);
LL_Ir_Operand ir_generate_expression(Compiler_Context* cc, LL_Backend_Ir* b, Ast_Base* expr, bool ref);


