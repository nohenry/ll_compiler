#pragma once

#include "lexer.h"

typedef struct {
	LL_Lexer lexer;
} LL_Parser;

LL_Parser parser_create_from_file(Compiler_Context* cc, char* filename);
void parser_parse_file(Compiler_Context* cc, LL_Parser* parser);
