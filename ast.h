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

    AST_KIND_TYPE_POINTER,
} Ast_Kind;

struct ll_type;

typedef struct {
    Ast_Kind kind;
	struct ll_type* type;
} Ast_Base;

typedef struct {
    Ast_Base base;
    String_View str;
	struct scope_map* resolved_scope;
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
	Ast_Base* type;
	Ast_Ident* ident;
	Ast_Base* initializer optional;
} Ast_Variable_Declaration;

typedef struct {
    Ast_Base base;
	Ast_Base* return_type;
	Ast_Ident* ident;
	Ast_Parameter_List parameters;
	Ast_Base* body optional;
} Ast_Function_Declaration;

typedef struct {
	Ast_Base base;
	Ast_Base* element;
} Ast_Type_Pointer;

#define AST_AS(value, Type) ((Type*)(value))
