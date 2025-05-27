#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

typedef enum {
	LL_TOKEN_KIND_NONE = 0,
	LL_TOKEN_KIND_IDENT = 256,
	LL_TOKEN_KIND_BUILTIN,
	LL_TOKEN_KIND_INT,

	/* assigns */
    LL_TOKEN_KIND_ASSIGN_PLUS,
    LL_TOKEN_KIND_ASSIGN_MINUS,
    LL_TOKEN_KIND_ASSIGN_TIMES,
    LL_TOKEN_KIND_ASSIGN_DIVIDE,
    LL_TOKEN_KIND_ASSIGN_PERCENT,

	/* comparison */
    LL_TOKEN_KIND_EQUALS,
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
bool lexer_next_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out);
void lexer_print_token(LL_Lexer* lexer, LL_Token* token);
