#pragma once

#include "stdint.h"
#include "../src/typer.h"

typedef uint32_t LL_Ir_Operand;
#define LL_IR_OPERAND_VALUE_MASK 0xFFFFFFFu
#define LL_IR_OPERAND_TYPE_MASK (~LL_IR_OPERAND_VALUE_MASK)
#define LL_IR_OPERAND_IMMEDIATE_BIT 0x00000000u
#define LL_IR_OPERAND_REGISTER_BIT  0x10000000u
#define LL_IR_OPERAND_LOCAL_BIT     0x20000000u
#define LL_IR_OPERAND_PARMAETER_BIT 0x30000000u
#define LL_IR_OPERAND_FUNCTION_BIT  0x40000000u
#define LL_IR_OPERAND_DATA_BIT      0x50000000u

#define OPD_VALUE(operand) ((operand) & LL_IR_OPERAND_VALUE_MASK)
#define OPD_TYPE(operand) ((operand) & LL_IR_OPERAND_TYPE_MASK)

typedef struct {
    Ast_Ident* ident;
} LL_Ir_Local;

typedef struct {
    uint32_t count, capacity;
    LL_Ir_Local* items;
} LL_Ir_Local_List;

typedef struct {
    struct ll_type* type;
} LL_Ir_Register;

typedef struct {
    uint32_t count, capacity;
    LL_Ir_Register* items;
} LL_Ir_Register_List;

enum {
    LL_IR_OPCODE_RET,       // x
    LL_IR_OPCODE_RETVALUE,  // x
    LL_IR_OPCODE_STORE,     // x
    LL_IR_OPCODE_MEMCOPY,   // 
    LL_IR_OPCODE_LOAD,      // x
    // operands: invokee, arg_count, arg0, arg1, ... arg(arg_count-1)
    LL_IR_OPCODE_INVOKE,    // x
    // operands: result_register, invokee, arg_count, arg0, arg1, ... arg(arg_count-1)
    LL_IR_OPCODE_INVOKEVALUE, // x
    LL_IR_OPCODE_LEA,
    LL_IR_OPCODE_LEA_INDEX,
    LL_IR_OPCODE_CAST,      // x

    LL_IR_OPCODE_BRANCH,      // x
    LL_IR_OPCODE_BRANCH_COND, // x

    LL_IR_OPCODE_ADD, // x
    LL_IR_OPCODE_SUB, // x
    LL_IR_OPCODE_MUL, // x
    LL_IR_OPCODE_DIV, // x
    LL_IR_OPCODE_LT,  // x
    LL_IR_OPCODE_LTE, // x
    LL_IR_OPCODE_GT,  // x
    LL_IR_OPCODE_GTE, // x
    LL_IR_OPCODE_EQ,  // x
    LL_IR_OPCODE_NEQ, // x

    LL_IR_OPCODE_NEG, // x
    LL_IR_OPCODE_NOT, // x

    LL_IR_OPCODE_AND, // x
    LL_IR_OPCODE_OR,  // x
    LL_IR_OPCODE_XOR, // x
};
typedef uint32 LL_Ir_Opcode;

typedef struct {
    LL_Ir_Opcode opcode;
    LL_Ir_Operand operands[];
} LL_Ir;

typedef struct {
    size_t count, capacity;
    LL_Ir_Opcode* items;
} LL_Ir_Op_List;

typedef struct ll_ir_block {
    uint32_t next, prev;
    uint32_t ref1, ref2;
    /* struct ll_ir_block *next, *prev; */
    /* struct ll_ir_block *ref1, *ref2; */
    LL_Ir_Op_List ops;
    LL_Ir_Op_List rops;
    bool did_branch;
    uint32_t bi;
    int64_t generated_offset;
    int64_t fixup_offset;
} LL_Ir_Block;

typedef uint32_t LL_Ir_Block_Ref;

typedef struct {
    uint32_t count, capacity;
    LL_Ir_Block* items;
} LL_Ir_Block_List;

typedef enum {
    LL_IR_FUNCTION_FLAG_EXTERN = (1u << 0u),
    LL_IR_FUNCTION_FLAG_NATIVE = (1u << 1u),
} LL_Ir_Function_Flags;

#define LL_IR_FUNCTION_OFFSET_INVALID ((int64_t)-1)

typedef struct {
    Ast_Ident* ident;
    LL_Ir_Block_Ref entry;
    LL_Ir_Block_Ref exit;
    LL_Ir_Local_List locals;
    LL_Ir_Register_List registers;
    LL_Ir_Function_Flags flags;
    uint32_t block_count;

    int64_t generated_offset;
} LL_Ir_Function;

typedef struct {
    size_t count, capacity;
    LL_Ir_Function* items;
} LL_Ir_Function_List;

typedef struct {
    void* ptr;
    size_t len, binary_offset;
} LL_Ir_Data_Item;

typedef struct {
    size_t count, capacity;
    LL_Ir_Data_Item* items;
} LL_Ir_Data_Item_List;

typedef enum {
    LL_BACKEND_IR_FLAG_RHS_IMMEDIATE = (1u << 0u),
    LL_BACKEND_IR_FLAG_LHS_IMMEDIATE = (1u << 1u),
} LL_Backend_Ir_Flags;

#define CURRENT_CONST_STACK (0x80000000u)
#define CURRENT_INDEX (0x7FFFFFFFu)

typedef struct ll_backend_ir {
    LL_Ir_Function_List fns;
    LL_Ir_Function_List const_stack;
    LL_Ir_Data_Item_List data_items;

    uint32_t current_function;
    LL_Ir_Block_Ref current_block, return_block;
    LL_Ir_Block_List blocks;

    LL_Ir_Block_Ref free_block;
    LL_Ir_Operand block_value;

    LL_Backend_Ir_Flags flags;
    LL_Ir_Operand copy_operand;

    uint8_t* initializer_ptr;
    bool last_op_was_load;
} LL_Backend_Ir;

#define OPERAND_FMT "{}{}"
#define OPERAND_FMT_VALUE(v) ( \
        (v & LL_IR_OPERAND_TYPE_MASK) == LL_IR_OPERAND_LOCAL_BIT ? "l" : \
        (v & LL_IR_OPERAND_TYPE_MASK) == LL_IR_OPERAND_REGISTER_BIT ? "r" : \
        (v & LL_IR_OPERAND_TYPE_MASK) == LL_IR_OPERAND_PARMAETER_BIT ? "p" : \
        (v & LL_IR_OPERAND_TYPE_MASK) == LL_IR_OPERAND_FUNCTION_BIT ? "f" : \
        (v & LL_IR_OPERAND_TYPE_MASK) == LL_IR_OPERAND_DATA_BIT ? "d" : \
        "" \
        ), (v & LL_IR_OPERAND_VALUE_MASK)

void ir_init(Compiler_Context* cc, LL_Backend_Ir* b);
bool ir_write_to_file(Compiler_Context* cc, LL_Backend_Ir* b, char* filepath);
void ir_generate_statement(Compiler_Context* cc, LL_Backend_Ir* b, Ast_Base* stmt);
LL_Ir_Operand ir_generate_expression(Compiler_Context* cc, LL_Backend_Ir* b, Ast_Base* expr, bool ref);
LL_Type* ir_get_operand_type(LL_Ir_Function* fn, LL_Ir_Operand operand);

size_t ir_get_op_count(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Opcode* opcode_list, size_t i);
void ir_print_op(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Opcode* opcode_list, size_t i, Oc_Writer* w);

void ir_calculate_struct_offsets(LL_Type* type);
