#pragma once

// #include <stdint.h>
// #include <stdbool.h>
#include "common.h"

typedef enum {
    LL_TOKEN_KIND_NONE = 0,
    LL_TOKEN_KIND_IDENT = 256,
    LL_TOKEN_KIND_BUILTIN,
    LL_TOKEN_KIND_INT,
    LL_TOKEN_KIND_FLOAT,
    LL_TOKEN_KIND_STRING,

    /* assigns */
    LL_TOKEN_KIND_ASSIGN_PLUS,
    LL_TOKEN_KIND_ASSIGN_MINUS,
    LL_TOKEN_KIND_ASSIGN_TIMES,
    LL_TOKEN_KIND_ASSIGN_DIVIDE,
    LL_TOKEN_KIND_ASSIGN_PERCENT,

    /* comparison */
    LL_TOKEN_KIND_EQUALS,
    LL_TOKEN_KIND_NEQUALS,
    LL_TOKEN_KIND_LTE,
    LL_TOKEN_KIND_GTE,

    LL_TOKEN_KIND_RANGE,
} LL_Token_Kind;

typedef struct {
    LL_Token_Kind kind;
    size_t position;
    union {
        uint64_t u64;
        double f64;
        string str;
    };
} LL_Token;

typedef struct {
    LL_Token_Kind kind;
    size_t position;
} LL_Token_Info;

typedef struct ll_lexer {
    string filename;
    string source;
    size_t pos;
    LL_Token peeked_token;
    LL_Token_Info peeked_token_info;
    bool has_peeked_token;
} LL_Lexer;

typedef struct {
    size_t line, column;
    string line_str;
    size_t start_pos, end_pos;
} LL_Line_Info;

bool lexer_peek_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out);
void lexer_consume(Compiler_Context *cc, LL_Lexer* lexer);
bool lexer_next_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out);
void lexer_print_token_raw(LL_Token* token);
void lexer_print_token_kind(LL_Token_Kind, Oc_Writer* w);
void lexer_print_token_raw_to_writer(LL_Token* token, Oc_Writer* w);
void lexer_print_token(LL_Token* token);
LL_Token_Info lexer_get_token_info(Compiler_Context *cc, LL_Lexer* lexer);

LL_Line_Info lexer_get_line_info(LL_Lexer* lexer, LL_Token_Info token_info);

int64_t lexer_get_token_length(Compiler_Context *cc, LL_Lexer* lexer, LL_Token_Info token);
