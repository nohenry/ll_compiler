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
    LL_TYPE_BOOL1,
    LL_TYPE_BOOL8,
    LL_TYPE_BOOL32,
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

typedef uint32 LL_Type_Index;

typedef struct ll_type {
    struct {
		LL_Type_Kind  kind  : 5;
		LL_Type_Group group : 3;
	};
    uint8 padding;
    uint16 cast;
    uint16 implicit_cast;
    uint16 a;
} LL_Type;

_Static_assert(sizeof(LL_Type) == 8, "expected to be packed");

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

typedef struct {
    LL_Type *expected, *actual;
    LL_Token_Info main_token;
    LL_Token_Info highlight_start, highlight_end;
} LL_Error;

#define ll_typer_report_error(error, fmt, ...) do { OC_MAP_SEQ(OC_MAKE_GENERIC1, __VA_ARGS__); ll_typer_report_error_raw((cc), (typer), (error), fmt OC_MAP_SEQ(OC_MAKE_GENERIC1_PARAM, __VA_ARGS__)); } while (0)
#define ll_typer_report_error_info(error, fmt, ...) do { OC_MAP_SEQ(OC_MAKE_GENERIC1, __VA_ARGS__); ll_typer_report_error_info_raw((cc), (typer), (error), fmt OC_MAP_SEQ(OC_MAKE_GENERIC1_PARAM, __VA_ARGS__)); } while (0)
#define ll_typer_report_error_note(error, fmt, ...) do { OC_MAP_SEQ(OC_MAKE_GENERIC1, __VA_ARGS__); ll_typer_report_error_note_raw((cc), (typer), (error), fmt OC_MAP_SEQ(OC_MAKE_GENERIC1_PARAM, __VA_ARGS__)); } while (0)
#define ll_typer_report_error_no_src(fmt, ...) do { OC_MAP_SEQ(OC_MAKE_GENERIC1, __VA_ARGS__); ll_typer_report_error_no_src_raw((cc), (typer), fmt OC_MAP_SEQ(OC_MAKE_GENERIC1_PARAM, __VA_ARGS__)); } while (0)

LL_Typer ll_typer_create(Compiler_Context* cc);
void ll_typer_run(Compiler_Context* cc, LL_Typer* typer, struct ll_parser* parser, Code* node);
LL_Type* ll_typer_get_type_from_typename(Compiler_Context* cc, LL_Typer* typer, Code* typename);

void ll_print_type_raw(LL_Type* type, Oc_Writer* w);
void ll_print_type(LL_Type* type);

void ll_typer_print_error_line(Compiler_Context* cc, LL_Typer* typer, LL_Line_Info line_info, LL_Token_Info start_info, LL_Token_Info end_info, bool print_dot_dot_dot, bool print_underline);
void ll_typer_report_error_raw(Compiler_Context* cc, LL_Typer* typer, LL_Error error, const char* fmt, ...);
void ll_typer_report_error_note_raw(Compiler_Context* cc, LL_Typer* typer, LL_Error error, const char* fmt, ...);
void ll_typer_report_error_info_raw(Compiler_Context* cc, LL_Typer* typer, LL_Error error, const char* fmt, ...);
void ll_typer_report_error_no_src_raw(Compiler_Context* cc, LL_Typer* typer, const char* fmt, ...);
void ll_typer_report_error_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* type);
void ll_typer_report_error_type_no_fmt(Compiler_Context* cc, LL_Typer* typer, LL_Type* type);
void ll_typer_report_error_done(Compiler_Context* cc, LL_Typer* typer);
