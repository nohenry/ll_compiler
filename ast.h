#pragma once

#include "common.h"
#include "lexer.h"

#define optional

typedef enum {
    AST_KIND_LITERAL_INT,
    AST_KIND_LITERAL_STRING,
    AST_KIND_IDENT,

    AST_KIND_BINARY_OP,
    AST_KIND_PRE_OP,
    AST_KIND_INVOKE,

	AST_KIND_VARIABLE_DECLARATION,
	AST_KIND_FUNCTION_DECLARATION,
	AST_KIND_PARAMETER,
    AST_KIND_BLOCK,

    AST_KIND_RETURN,
    AST_KIND_IF,
    AST_KIND_FOR,

	AST_KIND_INDEX,
    AST_KIND_TYPE_POINTER,
} Ast_Kind;

typedef enum {
	LL_STORAGE_CLASS_EXTERN = (1 << 0),
	LL_STORAGE_CLASS_STATIC = (1 << 1),
} LL_Storage_Class;

typedef enum {
	LL_PARAMETER_FLAG_VARIADIC = (1 << 0),
} LL_Parameter_Flags;

struct ll_type;

typedef struct {
    Ast_Kind kind;
	struct ll_type* type;
} Ast_Base;

#define AST_IDENT_SYMBOL_INVALID ((int32_t)-1)

typedef struct {
    Ast_Base base;
    String_View str;
	struct scope_map* resolved_scope;
	int32_t symbol_index;
} Ast_Ident;

typedef struct {
    size_t count;
    size_t capacity;
    Ast_Base** items;
} Ast_List;

typedef struct {
	Ast_Base base;
	Ast_Base* type;
	Ast_Ident* ident;
	LL_Parameter_Flags flags;
	uint32_t ir_index;
} Ast_Parameter;

typedef struct {
    size_t count;
    size_t capacity;
    Ast_Parameter* items;
} Ast_Parameter_List;

typedef struct {
    Ast_Base base;

    size_t count;
    size_t capacity;
    Ast_Base** items;
} Ast_Block;

typedef struct {
    Ast_Base base;
    union {
        int64_t i64;
        String_View str;
    };
} Ast_Literal;

typedef struct {
    Ast_Base base;
    Ast_Base* left;
    Ast_Base* right;
    LL_Token op;
} Ast_Operation;

typedef struct {
    Ast_Base base;
    Ast_Base* expr;
	Ast_List arguments;
} Ast_Invoke;

typedef struct {
    Ast_Base base;
    Ast_Base* expr;
} Ast_Control_Flow;

typedef struct {
    Ast_Base base;
    Ast_Base* init;
    Ast_Base* cond;
    Ast_Base* update;
    Ast_Base* body;
} Ast_Loop;

typedef struct {
    Ast_Base base;
    Ast_Base* cond;
    Ast_Base* body;
    Ast_Base* else_clause;
} Ast_If;

typedef struct {
    Ast_Base base;
	Ast_Base* type;
	Ast_Ident* ident;
	Ast_Base* initializer optional;
	LL_Storage_Class storage_class;
	uint32_t ir_index;
} Ast_Variable_Declaration;

typedef struct {
    Ast_Base base;
	Ast_Base* return_type;
	Ast_Ident* ident;
	Ast_Parameter_List parameters;
	Ast_Base* body optional;
	LL_Storage_Class storage_class;
	uint32_t ir_index;
} Ast_Function_Declaration;

typedef struct {
	Ast_Base base;
	Ast_Base* element;
} Ast_Type_Pointer;

#define AST_AS(value, Type) ((Type*)(value))
