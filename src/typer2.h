#pragma once

#include "common.h"
#include "ast.h"

struct ll_parser;

Enum(LL_Type_Kind, uint8,
    LL_TYPE_UNKNOWN,
    LL_TYPE_INT8,
    LL_TYPE_INT16,
    LL_TYPE_INT32,
    LL_TYPE_INT64,
    LL_TYPE_UINT8,
    LL_TYPE_UINT16,
    LL_TYPE_UINT32,
    LL_TYPE_UINT64,
    LL_TYPE_CHAR8,
    LL_TYPE_CHAR16,
    LL_TYPE_CHAR32,
    LL_TYPE_CHAR64,
    LL_TYPE_BOOL1,
    LL_TYPE_BOOL8,
    LL_TYPE_BOOL16,
    LL_TYPE_BOOL32,
    LL_TYPE_BOOL64,
    LL_TYPE_FLOAT16,
    LL_TYPE_FLOAT32,
    LL_TYPE_FLOAT64,

    LL_TYPE_STRING,
    LL_TYPE_FUNCTION,
    LL_TYPE_POINTER,
    LL_TYPE_ARRAY,
    LL_TYPE_SLICE,
    LL_TYPE_STRUCT,
    LL_TYPE_CODE_REF,
    LL_TYPE_NAMED,
    LL_TYPE_TYPE,
);

Enum(LL_Type_Group, uint8,
    LL_TYPE_GROUP_UNKNOWN,
    LL_TYPE_GROUP_INT,
    LL_TYPE_GROUP_UINT,
    LL_TYPE_GROUP_CHAR,
    LL_TYPE_GROUP_BOOL,
    LL_TYPE_GROUP_FLOAT,
	LL_TYPE_GROUP_OTHER,
);

typedef struct ll_type {
    struct {
		LL_Type_Kind  kind  : 5;
		LL_Type_Group group : 3;
	};
} LL_Type;

_Static_assert(sizeof(LL_Type) == 1, "expected to be packed");

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

typedef struct ll_type_function {
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
    Code_Declaration* scope;
    LL_Type* actual_type;
} LL_Type_Named;

typedef struct ll_type_intern_map_entry {
    LL_Type* value;
    struct ll_type_intern_map_entry* next;
} LL_Type_Intern_Map_Entry;

typedef struct {
    Code* scope;
    Code** this_arg;
} LL_Typer_Resolve_Result;

typedef struct {
    uint32_t count, capacity;
    LL_Type** items;
} LL_Typer_Current_Record;

typedef struct {
    Code_Declaration* field_scope;
    LL_Eval_Value value;
    bool has_init;
} LL_Typer_Record_Value;

typedef struct {
    uint32_t count, capacity;
    LL_Typer_Record_Value* items;
} LL_Typer_Record_Values;


typedef struct ll_typer {
	Array(uint32, LL_Type) types;
} LL_Typer;

LL_Typer ll_typer_create(Compiler_Context* cc);
void ll_typer_run(Compiler_Context* cc, LL_Typer* typer, struct ll_parser* parser, Code* node);

void ll_print_type_raw(LL_Type* type, Oc_Writer* w);
void ll_print_type(LL_Type* type);

