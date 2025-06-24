#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

typedef enum {
	LL_TOKEN_KIND_NONE = 0,
	LL_TOKEN_KIND_IDENT = 256,
	LL_TOKEN_KIND_BUILTIN,
	LL_TOKEN_KIND_INT,
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
    union {
        int64_t i64;
        String_View str;
    };
} LL_Token;

typedef struct {
    String_View source;
    size_t pos;
	LL_Token peeked_token;
	bool has_peeked_token;
} LL_Lexer;


bool lexer_peek_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out);
void lexer_consume(Compiler_Context *cc, LL_Lexer* lexer);
bool lexer_next_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out);
void lexer_print_token_raw(LL_Token* token);
void lexer_print_token_kind(LL_Token_Kind, FILE* fd);
void lexer_print_token_raw_to_fd(LL_Token* token, FILE* fd);
void lexer_print_token(LL_Token* token);
