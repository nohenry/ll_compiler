#pragma once

#include "lexer.h"
#include "ast.h"

Enum(LL_Operation_Kind, uint32_t,
	LL_OPERATION_ADD,
	LL_OPERATION_ASSIGN,
	COUNT_OF_LL_OPERATION,
);

typedef struct ll_parser {
    LL_Lexer lexer;

	union {
        Array(uint32, Code*) ops[COUNT_OF_LL_OPERATION];
	};

    Code_Scope* current_scope;
} LL_Parser;

LL_Parser parser_create_from_file(Compiler_Context* cc, char* filename);
Code_Scope* parser_parse_file(Compiler_Context* cc, LL_Parser* parser);
Code* parser_parse_statement(Compiler_Context* cc, LL_Parser* parser);
Code_Scope* parser_parse_block(Compiler_Context* cc, LL_Parser* parser, Code_Declaration* decl);
Code_Parameter parser_parse_parameter(Compiler_Context* cc, LL_Parser* parser);
Code_Type_Parameter parser_parse_type_parameter(Compiler_Context* cc, LL_Parser* parser);
Code* parser_parse_declaration(Compiler_Context* cc, LL_Parser* parser, LL_Storage_Class storage_class);
Code* parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, Code* left, int last_precedence, bool from_statement);
Code* parser_parse_primary(Compiler_Context* cc, LL_Parser* parser, bool from_statement);
Code* parser_parse_class(Compiler_Context* cc, LL_Parser* parser);

Code* parser_parse_type(Compiler_Context* cc, LL_Parser* parser);

void print_node(Code* node, uint32_t indent, Oc_Writer* w);
