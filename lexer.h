#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

typedef enum {
	LL_TOKEN_KIND_NONE = 0,
	LL_TOKEN_KIND_IDENT = 256,
	LL_TOKEN_KIND_INT,
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
} LL_Lexer;

bool lexer_next_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out);
void lexer_print_token(LL_Lexer* lexer, LL_Token* token);
