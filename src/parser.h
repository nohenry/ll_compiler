#pragma once

#include "lexer.h"
#include "ast.h"

typedef struct {
    LL_Lexer lexer;

    Code_Scope* current_scope;
} LL_Parser;

LL_Parser parser_create_from_file(Compiler_Context* cc, char* filename);
Code* parser_parse_file(Compiler_Context* cc, LL_Parser* parser);
Code* parser_parse_statement(Compiler_Context* cc, LL_Parser* parser);
Code_Scope* parser_parse_block(Compiler_Context* cc, LL_Parser* parser);
Code_Parameter parser_parse_parameter(Compiler_Context* cc, LL_Parser* parser);
Code* parser_parse_declaration(Compiler_Context* cc, LL_Parser* parser, Code* type, LL_Storage_Class storage_class);
Code* parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, Code* left, int last_precedence, bool from_statement);
Code* parser_parse_primary(Compiler_Context* cc, LL_Parser* parser, bool from_statement);
Code* parser_parse_struct(Compiler_Context* cc, LL_Parser* parser);

void print_node(Code* node, uint32_t indent, Oc_Writer* w);
