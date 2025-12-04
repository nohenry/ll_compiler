#pragma once

#include "common.h"
#include "lexer.h"
#include "eval_value.h"

#define optional

typedef enum {
    AST_KIND_LITERAL_INT,
    AST_KIND_LITERAL_FLOAT,
    AST_KIND_LITERAL_STRING,
    AST_KIND_IDENT,

    AST_KIND_BINARY_OP,
    AST_KIND_PRE_OP,
    AST_KIND_INVOKE,

    AST_KIND_VARIABLE_DECLARATION,
    AST_KIND_FUNCTION_DECLARATION,
    AST_KIND_PARAMETER,
    AST_KIND_BLOCK,
    AST_KIND_CONST,
    AST_KIND_INITIALIZER,
    AST_KIND_ARRAY_INITIALIZER,
    AST_KIND_KEY_VALUE,

    AST_KIND_RETURN,
    AST_KIND_BREAK,
    AST_KIND_CONTINUE,
    AST_KIND_IF,
    AST_KIND_FOR,

    AST_KIND_STRUCT,
    AST_KIND_INDEX,
    AST_KIND_CAST,
    AST_KIND_TYPE_POINTER,
} Ast_Kind;

typedef enum {
    LL_STORAGE_CLASS_EXTERN = (1 << 0),
    LL_STORAGE_CLASS_STATIC = (1 << 1),
    LL_STORAGE_CLASS_NATIVE = (1 << 2),
} LL_Storage_Class;

typedef enum {
    LL_PARAMETER_FLAG_VARIADIC = (1 << 0),
} LL_Parameter_Flags;

struct ll_type;

typedef struct {
    Ast_Kind kind;
    struct ll_type* type;
    LL_Eval_Value const_value;
    uint8_t has_const;
} Ast_Base;

#define AST_IDENT_SYMBOL_INVALID ((int32_t)-1)

typedef struct {
    Ast_Base base;
    string str;
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
    Ast_Base* initializer;
    LL_Parameter_Flags flags;
    uint32_t ir_index;
} Ast_Parameter;

typedef struct {
    uint32_t count;
    uint32_t capacity;
    Ast_Parameter* items;
} Ast_Parameter_List;

typedef enum {
    AST_BLOCK_FLAG_EXPR = (1u << 0u),
} Ast_Block_Flags;

typedef struct {
    Ast_Base base;
    Ast_Block_Flags flags;

	struct scope_map* scope;

    uint32_t count;
    uint32_t capacity;
    Ast_Base** items;
} Ast_Block;

typedef struct {
    Ast_Base base;
    union {
        uint64_t u64;
        double f64;
        string str;
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
	Ast_List ordered_arguments;
    bool has_this_arg;
} Ast_Invoke;

typedef struct {
    Ast_Base base;
    Ast_Base* key;
    Ast_Base* value;
} Ast_Key_Value;

typedef struct {
    Ast_Base base;

    uint32_t count;
    uint32_t capacity;
    Ast_Base** items;
} Ast_Initializer;

typedef struct {
    Ast_Base base;
    Ast_Base* expr;
} Ast_Marker;

typedef struct {
    Ast_Base base;
    Ast_Base* expr;
    struct scope_map* referenced_scope;
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
    uint32_t ir_index; // for locals, it's the locals index
                       // for structs, it's the type field index
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
    Ast_Ident* ident;
    Ast_List body;
} Ast_Struct;

typedef struct {
    Ast_Base base;
    Ast_Base* cast_type;
    Ast_Base* expr;
} Ast_Cast;

typedef struct {
    Ast_Base base;
    Ast_Base* element;
} Ast_Type_Pointer;

#define AST_AS(value, Type) ((Type*)(value))

const char* ast_get_node_kind(Ast_Base* node);