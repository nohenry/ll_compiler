#pragma once

#include "lexer.h"
#include "ast.h"
#include "typer.h"
#include "typer2.h"

Enum(LL_Operation_Kind, uint32_t,
	LL_OPERATION_ADD,
	LL_OPERATION_ASSIGN,
	COUNT_OF_LL_OPERATION,
);

Enum(Ast_Result_Kind, uint32,
	FIRST_RESULT_KIND = COUNT_OF_LL_OPERATION * 3,
	RESULT_KIND_IDENT = FIRST_RESULT_KIND,
	RESULT_KIND_DECL,
	RESULT_KIND_INT,
	RESULT_KIND_FLOAT,
	COUNT_OF_AST_RESULT,
);

typedef struct {
	Ast_Result_Kind kind;
	uint32 value;
} LL_Usage;

typedef struct {
    Array(uint32, LL_Type*) types;
    Array(uint32, LL_Usage) direct_usages;
} LL_Typecheck_Value;


typedef struct ll_parser {
    LL_Lexer lexer;

	union {
		LL_Typecheck_Value linear_grid[COUNT_OF_AST_RESULT];
		struct {
			Array(uint32, Code*) ops[COUNT_OF_LL_OPERATION];

			LL_Typecheck_Value ops_lhs[COUNT_OF_LL_OPERATION];
			LL_Typecheck_Value ops_rhs[COUNT_OF_LL_OPERATION];
			LL_Typecheck_Value idents;
			LL_Typecheck_Value decls;
			LL_Typecheck_Value ints;
			LL_Typecheck_Value floats;
		};
	};

    Code_Scope* current_scope;
} LL_Parser;

typedef struct {
	Code* code;
	Ast_Result_Kind kind;
	uint32 value;
} Parse_Result;

LL_Parser parser_create_from_file(Compiler_Context* cc, char* filename);
Parse_Result parser_parse_file(Compiler_Context* cc, LL_Parser* parser);
Parse_Result parser_parse_statement(Compiler_Context* cc, LL_Parser* parser);
Code_Scope* parser_parse_block(Compiler_Context* cc, LL_Parser* parser, Code_Declaration* decl);
Code_Parameter parser_parse_parameter(Compiler_Context* cc, LL_Parser* parser);
Parse_Result parser_parse_declaration(Compiler_Context* cc, LL_Parser* parser, Parse_Result* type, LL_Storage_Class storage_class);
Parse_Result parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, Parse_Result* left, int last_precedence, bool from_statement);
Parse_Result parser_parse_primary(Compiler_Context* cc, LL_Parser* parser, bool from_statement);
Parse_Result parser_parse_struct(Compiler_Context* cc, LL_Parser* parser);

void print_node(Code* node, uint32_t indent, Oc_Writer* w);

#define TC_ALIGNMENT (64)

static inline uint32 parser_append_typecheck_value(Compiler_Context* cc, LL_Typecheck_Value* tcv, LL_Type* type, Ast_Result_Kind usage_kind, uint32 usage_index) {
    uint32 index = tcv->types.count;
    oc_array_aligned_append(&cc->arena, &tcv->types, TC_ALIGNMENT, type);
    oc_array_aligned_append(&cc->arena, &tcv->direct_usages, TC_ALIGNMENT, ((LL_Usage) { usage_kind, usage_index}));
    return index;
}

static inline uint32 parser_extend_uninit_typecheck_value(Compiler_Context* cc, LL_Typecheck_Value* tcv, uint32 count) {
    uint32 index = tcv->types.count;
    oc_array_aligned_extend_count_unint(&cc->arena, &tcv->types, TC_ALIGNMENT, count);
    oc_array_aligned_extend_count_unint(&cc->arena, &tcv->direct_usages, TC_ALIGNMENT, count);
    return index;
}

static inline uint32 parser_extend_zeroed_typecheck_value(Compiler_Context* cc, LL_Typecheck_Value* tcv, uint32 count) {
    uint32 index = tcv->types.count;
    oc_array_aligned_extend_count_unint(&cc->arena, &tcv->types, TC_ALIGNMENT, count);
    oc_array_aligned_extend_count_unint(&cc->arena, &tcv->direct_usages, TC_ALIGNMENT, count);
    memset(tcv->types.items + index, 0, count * sizeof(tcv->types.items[0]));
    memset(tcv->direct_usages.items + index, 0, count * sizeof(tcv->direct_usages.items[0]));
    return index;
}
