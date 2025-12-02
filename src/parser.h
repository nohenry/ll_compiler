#pragma once

#include "lexer.h"
#include "ast.h"

typedef struct {
    LL_Lexer lexer;
} LL_Parser;

LL_Parser parser_create_from_file(Compiler_Context* cc, char* filename);
Ast_Base* parser_parse_file(Compiler_Context* cc, LL_Parser* parser);
Ast_Base* parser_parse_statement(Compiler_Context* cc, LL_Parser* parser);
Ast_Block* parser_parse_block(Compiler_Context* cc, LL_Parser* parser);
Ast_Parameter parser_parse_parameter(Compiler_Context* cc, LL_Parser* parser);
Ast_Base* parser_parse_declaration(Compiler_Context* cc, LL_Parser* parser, Ast_Base* type, LL_Storage_Class storage_class);
Ast_Base* parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, Ast_Base* left, int last_precedence, bool from_statement);
Ast_Base* parser_parse_primary(Compiler_Context* cc, LL_Parser* parser);
Ast_Base* parser_parse_struct(Compiler_Context* cc, LL_Parser* parser);

void print_node(Ast_Base* node, uint32_t indent, Oc_Writer* w);
