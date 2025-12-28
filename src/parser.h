#pragma once

#include "lexer.h"
#include "ast.h"
#include "typer2.h"

Enum(LL_Operation_Kind, uint32_t,
	LL_OPERATION_ADD,
	LL_OPERATION_ASSIGN,
	COUNT_OF_LL_OPERATION,
);

Enum(Ast_Result_Kind, uint32,
	FIRST_RESULT_KIND = COUNT_OF_LL_OPERATION * 2,
	RESULT_KIND_IDENT = FIRST_RESULT_KIND,
	RESULT_KIND_DECL,
	RESULT_KIND_INT,
	RESULT_KIND_FLOAT,
	COUNT_OF_AST_RESULT,
);

typedef struct {
	LL_Type type;
} LL_Typecheck_Value;


typedef struct ll_parser {
    LL_Lexer lexer;

	union {
		Array(uint32, LL_Typecheck_Value) linear_grid[COUNT_OF_AST_RESULT];
		struct {
			Array(uint32, LL_Typecheck_Value) ops[COUNT_OF_LL_OPERATION];
			Array(uint32, LL_Typecheck_Value) ops_values[COUNT_OF_LL_OPERATION];
			Array(uint32, LL_Typecheck_Value) idents;
			Array(uint32, LL_Typecheck_Value) decls;
			Array(uint32, LL_Typecheck_Value) ints;
			Array(uint32, LL_Typecheck_Value) floats;
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
Code_Scope* parser_parse_block(Compiler_Context* cc, LL_Parser* parser);
Code_Parameter parser_parse_parameter(Compiler_Context* cc, LL_Parser* parser);
Parse_Result parser_parse_declaration(Compiler_Context* cc, LL_Parser* parser, Parse_Result* type, LL_Storage_Class storage_class);
Parse_Result parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, Parse_Result* left, int last_precedence, bool from_statement);
Parse_Result parser_parse_primary(Compiler_Context* cc, LL_Parser* parser, bool from_statement);
Parse_Result parser_parse_struct(Compiler_Context* cc, LL_Parser* parser);

void print_node(Code* node, uint32_t indent, Oc_Writer* w);
