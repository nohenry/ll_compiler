#pragma once

#include "lexer.h"
#include "ast.h"

typedef struct {
	LL_Lexer lexer;
} LL_Parser;

LL_Parser parser_create_from_file(Compiler_Context* cc, char* filename);
void parser_parse_file(Compiler_Context* cc, LL_Parser* parser);
Ast_Base* parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, int last_precedence);
Ast_Base* parser_parse_primary(Compiler_Context* cc, LL_Parser* parser);