#pragma once

#include "common.h"
#include "lexer.h"
#include "eval_value.h"

#define optional

typedef struct code Code;

typedef struct {
    Code** items;
    uint32_t count, capacity;
} LL_Flattened;

typedef struct {
    Code* yielded_on;

    LL_Flattened flattened;
    uint32_t flattened_todo_cursor;
} LL_Queued;

typedef enum {
    CODE_KIND_LITERAL_INT,
    CODE_KIND_LITERAL_FLOAT,
    CODE_KIND_LITERAL_STRING,
    CODE_KIND_IDENT,

    CODE_KIND_BINARY_OP,
    CODE_KIND_PRE_OP,
    CODE_KIND_INVOKE,

    CODE_KIND_VARIABLE_DECLARATION,
    CODE_KIND_FUNCTION_DECLARATION,
    CODE_KIND_PARAMETER,
    CODE_KIND_BLOCK,
    CODE_KIND_CONST,
    CODE_KIND_INITIALIZER,
    CODE_KIND_ARRAY_INITIALIZER,
    CODE_KIND_KEY_VALUE,

    CODE_KIND_RETURN,
    CODE_KIND_BREAK,
    CODE_KIND_CONTINUE,
    CODE_KIND_IF,
    CODE_KIND_FOR,
    CODE_KIND_WHILE,

    CODE_KIND_STRUCT,
    CODE_KIND_INDEX,
    CODE_KIND_SLICE,
    CODE_KIND_CAST,
    CODE_KIND_GENERIC,
    CODE_KIND_TYPE_POINTER,


    // Generated
    CODE_KIND_TYPENAME,
} Code_Kind;

typedef enum {
    LL_STORAGE_CLASS_EXTERN = (1 << 0),
    LL_STORAGE_CLASS_STATIC = (1 << 1),
    LL_STORAGE_CLASS_NATIVE = (1 << 2),
    LL_STORAGE_CLASS_MACRO  = (1 << 3),
    LL_STORAGE_CLASS_CONST  = (1 << 4),
    LL_STORAGE_CLASS_POLYMORPHIC = (1 << 5),
} LL_Storage_Class;

typedef enum {
    LL_PARAMETER_FLAG_VARIADIC = (1 << 0),
} LL_Parameter_Flags;

struct code {
    Code_Kind kind;
    uint8_t has_const;
    LL_Queued* queued;
    Code* type;
    LL_Eval_Value const_value;
    LL_Token_Info token_info;
};

typedef struct ll_function_instantiation {
    struct ll_function_instantiation* next;
    struct ll_type_function* fn_type;
    Code* body optional;
    uint32_t ir_index;
} LL_Function_Instantiation;

#define CODE_IDENT_SYMBOL_INVALID ((int32_t)-1)

typedef uint32_t Code_Ident_Flags;
enum {
    CODE_IDENT_FLAG_EXPAND = (1u << 0u),
};

typedef struct {
    Code base;
    string str;
    struct Code_Declaration* resolved_decl;
    int32_t symbol_index;
    Code_Ident_Flags flags;
} Code_Ident;

typedef struct {
    size_t count;
    size_t capacity;
    Code** items;
} Code_List;

typedef struct {
    Code base;
    Code* type;
    Code_Ident* ident;
    Code* initializer;
    LL_Parameter_Flags flags;
    uint32_t ir_index;
} Code_Parameter;

typedef struct {
    uint32_t count;
    uint32_t capacity;
    Code_Parameter* items;
} Code_Parameter_List;

typedef enum {
    CODE_BLOCK_FLAG_EXPR            = (1u << 0u),
    CODE_BLOCK_FLAG_MACRO_EXPANSION = (1u << 1u),
} Code_Scope_Flags;

typedef struct Code_Declaration {
    Code base;
    Code* type;
    Code_Ident* ident;
    struct Code_Scope* within_scope;
    Code* declared_type;
} Code_Declaration;

typedef struct Code_Scope {
    Code base;
    Code_Scope_Flags flags;
    Code_Declaration* decl;

    struct Code_Scope* parent_scope;

    Array(uint32, Code*)    statements;
    Hash_Map(string, Code_Declaration*) declarations;

    LL_Token_Info c_open, c_close;
    uint32 break_value;
    uint32 break_block_ref;
} Code_Scope;

typedef struct {
    Code base;
    union {
        uint64_t u64;
        double f64;
        string str;
    };
} Code_Literal;

typedef struct {
    Code base;
    Code* left;
    Code* right;
    LL_Token_Info op;
} Code_Operation;

typedef struct {
    Code base;
    Code* ptr;
    Code* start;
    Code* stop;
} Code_Slice;

typedef struct {
    Code base;
    Code* key;
    Code* value;
} Code_Key_Value;

typedef struct {
    Code base;

    uint32_t count;
    uint32_t capacity;
    Code** items;
    LL_Token_Info c_close;
} Code_Initializer;

typedef struct {
    Code base;
    Code* expr;
} Code_Marker;

enum {
    CODE_CONTROL_FLOW_TARGET_ANY,
    CODE_CONTROL_FLOW_TARGET_DO,
    CODE_CONTROL_FLOW_TARGET_IF,
    CODE_CONTROL_FLOW_TARGET_FOR,
    CODE_CONTROL_FLOW_TARGET_WHILE,
};
typedef uint32 Code_Control_Flow_Target;

typedef struct {
    Code base;
    Code* expr;
    Code_Control_Flow_Target target;
    Code_Scope* referenced_scope;
} Code_Control_Flow;

typedef struct {
    Code base;
    Code* init;
    Code* cond;
    Code* update;
    Code* body;
    struct scope_map* scope;
} Code_Loop;

typedef struct {
    Code base;
    Code* cond;
    Code* body;
    Code* else_clause;
    LL_Token_Info else_kw;
} Code_If;

typedef struct {
    Code_Declaration base;

    Code* initializer optional;
    LL_Storage_Class storage_class;
    uint32_t ir_index; // for locals, it's the locals index
                       // for structs, it's the type field index
} Code_Variable_Declaration;

typedef struct {
    Code_Declaration base;

    Code_Parameter_List parameters;
    Code_Scope* body optional;
    LL_Storage_Class storage_class;
    uint32_t ir_index;
    LL_Token_Info p_open, p_close;

    LL_Function_Instantiation* (*instantiations)[LL_DEFAULT_MAP_ENTRY_COUNT];
} Code_Function_Declaration;

typedef struct {
    Code base;
    Code* expr;
    Code_List arguments;
	Code_List ordered_arguments;
    Code_Function_Declaration* fn_decl;
    LL_Function_Instantiation* resolved_fn_inst;
    bool has_this_arg;
    LL_Token_Info p_close;
} Code_Invoke;

typedef struct {
    Code_Declaration base;
    Code_Scope* block;
} Code_Struct;

typedef struct {
    Code base;
    Code* cast_type;
    Code* expr;
    LL_Token_Info p_open, p_close;
} Code_Cast;

typedef struct {
    Code base;
    Code_Ident* ident;
} Code_Generic;

typedef struct {
    Code base;
    Code* element;
} Code_Type_Pointer;

#define CODE_AS(value, Type) ((Type*)(value))


typedef struct {
    bool expand_first_block;
    bool convert_all_idents_to_expansion;
    Code_Scope* current_scope;
} LL_Code_Clone_Params;

const char* ast_get_node_kind(Code* node);
Code* ast_clone_node_deep(Compiler_Context* cc, Code* node, LL_Code_Clone_Params params);

#define _CREATE_ASSIGN_KIND(_kind, type) _Generic((type), \
        Code : (void)0,                                   \
        Code_Variable_Declaration : ((Code_Variable_Declaration*)&v)->base.base.kind = _kind,      \
        Code_Function_Declaration : ((Code_Function_Declaration*)&v)->base.base.kind = _kind,      \
        Code_Struct : ((Code_Struct*)&v)->base.base.kind = _kind,      \
        default : ((Code_Scope*)&v)->base.kind = _kind                     \
    )

static inline Code* create_node(Compiler_Context* cc, Code* node, size_t size) {
    return oc_arena_dup(&cc->arena, node, size);
}

static inline Code_Ident* create_ident(Compiler_Context* cc, string sym) {
    Code_Ident ident = { .str = sym, .symbol_index = CODE_IDENT_SYMBOL_INVALID };
    ident.base.kind = CODE_KIND_IDENT;

    if (ident.str.ptr[0] == '$') {
        ident.str.ptr++;
        ident.str.len--;
        ident.str = ll_intern_string(cc, ident.str);
        ident.flags |= CODE_IDENT_FLAG_EXPAND;
    }

    Code_Ident* node = (Code_Ident*)create_node(cc, (Code*)&ident, sizeof(ident));
    return node;
}
