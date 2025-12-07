#pragma once

#include <stddef.h>

#include "common.h"
#include "ast.h"

typedef struct scope_map_entry {
    struct scope_map* scope;
    struct scope_map_entry* next;
} LL_Scope_Map_Entry;

typedef enum {
    LL_SCOPE_KIND_LOCAL,
	LL_SCOPE_KIND_FIELD,
    LL_SCOPE_KIND_PARAMETER,
    LL_SCOPE_KIND_MACRO_PARAMETER,
    LL_SCOPE_KIND_FUNCTION,
    LL_SCOPE_KIND_LOOP,
    LL_SCOPE_KIND_BLOCK,
    LL_SCOPE_KIND_BLOCK_VALUE,
    LL_SCOPE_KIND_MACRO_EXPANSION,
    LL_SCOPE_KIND_STRUCT,
    LL_SCOPE_KIND_PACKAGE,
} LL_Scope_Kind;

typedef struct scope_map {
    LL_Scope_Kind kind;
    struct scope_map* parent;
    Ast_Ident* ident;
    Ast_Base* decl;
    LL_Scope_Map_Entry* children[LL_DEFAULT_MAP_ENTRY_COUNT];
    uint32_t break_block_ref;
    size_t next_anon;
    uint32_t break_value;
} LL_Scope;

typedef struct {
    LL_Scope_Kind kind;
    struct scope_map* parent;
    Ast_Ident* ident;
    Ast_Base* decl;
} LL_Scope_Simple;

typedef struct {
    LL_Scope_Kind kind;
    struct scope_map* parent;
    Ast_Ident* ident;
    Ast_Base* decl;
    Ast_Base* value;
} LL_Scope_Macro_Parameter;

typedef enum {
    LL_TYPE_VOID,
    LL_TYPE_INT,
    LL_TYPE_UINT,
    LL_TYPE_ANYINT,
    LL_TYPE_FLOAT,
    LL_TYPE_ANYFLOAT,
    LL_TYPE_BOOL,
    LL_TYPE_ANYBOOL,
    LL_TYPE_STRING,
    LL_TYPE_FUNCTION,
    LL_TYPE_POINTER,
    LL_TYPE_ARRAY,
    LL_TYPE_SLICE,
    LL_TYPE_STRUCT,
    LL_TYPE_CODE_REF,
    LL_TYPE_NAMED,
} LL_Type_Kind;

typedef struct ll_type {
    LL_Type_Kind kind;
    union {
        size_t width;
        size_t struct_alignment;
    };
} LL_Type;

typedef struct {
    size_t capacity;
} LL_Type_List;

typedef struct {
    LL_Type base;
    LL_Type* element_type;
} LL_Type_Pointer;


typedef struct {
    LL_Type base;
    LL_Type* element_type;
} LL_Type_Array;

typedef struct {
    LL_Type base;
    LL_Type* element_type;
} LL_Type_Slice;

typedef struct {
    LL_Type base;
    LL_Type* return_type;
    size_t parameter_count;
    LL_Type** parameters;
    bool is_variadic;
} LL_Type_Function;
// static_assert(sizeof(LL_Type*) == 8);

typedef struct {
    LL_Type base;
    size_t field_count;
    LL_Type** fields;
    uint32_t* offsets;
    bool has_offsets;
} LL_Type_Struct;

typedef struct {
    LL_Type base;
    LL_Scope* scope;
    LL_Type* actual_type;
} LL_Type_Named;

typedef struct ll_type_intern_map_entry {
    LL_Type* value;
    struct ll_type_intern_map_entry* next;
} LL_Type_Intern_Map_Entry;

typedef struct {
    LL_Scope* scope;
    Ast_Base** this_arg;
} LL_Typer_Resolve_Result;

typedef struct {
    uint32_t count, capacity;
    LL_Type** items;
} LL_Typer_Current_Record;

typedef struct {
    LL_Scope* field_scope;
    LL_Eval_Value value;
    bool has_init;
} LL_Typer_Record_Value;

typedef struct {
    uint32_t count, capacity;
    LL_Typer_Record_Value* items;
} LL_Typer_Record_Values;

typedef struct ll_typer {
    LL_Type_Intern_Map_Entry* interned_types[LL_DEFAULT_MAP_ENTRY_COUNT];
    LL_Type *ty_int8, *ty_int16, *ty_int32, *ty_int64,
            *ty_uint8, *ty_uint16, *ty_uint32, *ty_uint64,
            *ty_anyint,
            *ty_float16, *ty_float32, *ty_float64,
            *ty_anyfloat,
            *ty_void,
            *ty_string,
            *ty_bool8, *ty_bool16, *ty_bool32, *ty_bool64, *ty_bool, *ty_anybool, *ty_code_ref;
    
    LL_Type_Function* current_fn;
    LL_Type* block_type;
    LL_Scope* current_scope, *root_scope;

    LL_Type_Named* current_named;
    LL_Typer_Current_Record* current_record;
    LL_Typer_Record_Values* current_record_values;
} LL_Typer;


LL_Type* ll_intern_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* type);
size_t ll_type_hash(LL_Type* type, size_t seed);
LL_Type* ll_typer_get_fn_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* return_type, LL_Type** parameter_types, size_t parameter_count, bool is_variadic);

LL_Typer ll_typer_create(Compiler_Context* cc);
void ll_typer_run(Compiler_Context* cc, LL_Typer* typer, Ast_Base* node);

LL_Type* ll_typer_type_statement(Compiler_Context* cc, LL_Typer* typer, Ast_Base** stmt);
LL_Type* ll_typer_type_expression(Compiler_Context* cc, LL_Typer* typer, Ast_Base** expr, LL_Type* expected_type, LL_Typer_Resolve_Result *resolve_result);
LL_Type* ll_typer_get_type_from_typename(Compiler_Context* cc, LL_Typer* typer, Ast_Base* typename);
void ll_print_type_raw(LL_Type* type, Oc_Writer* w);
void ll_print_type(LL_Type* type);

bool ll_typer_can_implicitly_cast(Compiler_Context* cc, LL_Typer* typer, LL_Type* src_type, LL_Type* dst_type);
bool ll_typer_can_implicitly_cast_const_value(Compiler_Context* cc, LL_Typer* typer, LL_Type* src_type, LL_Eval_Value* src_value, LL_Type* dst_type);
bool ll_typer_can_implicitly_cast_expression(Compiler_Context* cc, LL_Typer* typer, Ast_Base* expr, LL_Type* dst_type);

void ll_typer_scope_put(Compiler_Context* cc, LL_Typer* typer, LL_Scope* scope, bool hoist);
LL_Scope* ll_scope_get(LL_Scope* scope, string symbol_name);
LL_Scope* ll_typer_find_symbol_up_scope(Compiler_Context* cc, LL_Typer* typer, Ast_Ident* ident);

void ll_scope_print(LL_Scope* scope, int indent, Oc_Writer* w);


void ll_typer_add_implicit_cast(Compiler_Context* cc, LL_Typer* typer, Ast_Base** expr, LL_Type* expected_type);
LL_Type* ll_typer_get_ptr_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* element_type);
