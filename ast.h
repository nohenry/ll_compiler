#pragma once

#include "common.h"
#include "lexer.h"

typedef enum {
    AST_KIND_LITERAL_INT,
    AST_KIND_LITERAL_STRING,
    AST_KIND_IDENT,
    AST_KIND_BLOCK,
} Ast_Kind;

typedef struct {
    Ast_Kind kind;
} Ast_Base;

typedef struct {
    Ast_Base base;

    size_t count;
    size_t capacity;
    Ast_Base* items;
} Ast_Block;

typedef struct {
    Ast_Base base;
    String_View str;
} Ast_Ident;

typedef struct {
    Ast_Base base;
    union {
        int64_t i64;
        String_View str;
    };
} Ast_Literal;

typedef struct {
    Ast_Base base;
    Ast_Block* left;
    Ast_Block* right;
    LL_Token_Kind op;
} Ast_Operation;
