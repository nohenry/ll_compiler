
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "parser.h"
#include "lexer.h"
#include "ast.h"

LL_Parser parser_create_from_file(Compiler_Context* cc, char* filename) {
    FILE* input = fopen(filename, "r");

    fseek(input, 0, SEEK_END);
    size_t input_size = ftell(input);
    fseek(input, 0, SEEK_SET);

    uint8_t* input_contents = malloc(input_size);
    if (!input_contents) ABORT("OOM");

    fread(input_contents, 1, input_size, input);
	fclose(input);

	LL_Parser result = {
		.lexer = {
			.pos = 0,
			.source.ptr = (char*)input_contents,
			.source.len = input_size,
		}
	};

	return result;
}

#define PEEK(out) (lexer_peek_token(cc, &(parser)->lexer, out))
#define CONSUME() lexer_consume(cc, &(parser)->lexer)
#define UNEXPECTED() unexpected_token(cc, parser, __FILE__, __LINE__)
#define EXPECT(kind, out) expect_token(cc, parser, kind, out, __FILE__, __LINE__)
#define CREATE_NODE(_kind, value) ({ __typeof__(value) v = (value); _CREATE_ASSIGN_KIND((_kind), value); create_node(cc, parser, (Ast_Base*)&v, sizeof(v)); })

#define _CREATE_ASSIGN_KIND(_kind, type) _Generic((type), \
        Ast_Base : 1,                       \
        default : v.base.kind = _kind                    \
    )

#include "math.h"
static Ast_Base* create_node(Compiler_Context* cc, LL_Parser* parser, Ast_Base* node, size_t size) {
    return arena_memdup(&cc->arena, node, size);
}

static void unexpected_token(Compiler_Context* cc, LL_Parser* parser, char* file, int line) {
    LL_Token tok;
    PEEK(&tok);
    fprintf(stderr, "%s line %d:\x1b[31;1merror\x1b[0m\x1b[1m: unexpected token '", file, line);
    lexer_print_token_raw(&parser->lexer, &tok);
    fprintf(stderr, "'\x1b[0m\n");
}

static bool expect_token(Compiler_Context* cc, LL_Parser* parser, LL_Token_Kind kind, LL_Token* out, char* file, int line) {
    LL_Token tok;
    PEEK(&tok);
    if (tok.kind != kind) {
        unexpected_token(cc, parser, file, line);
        return false;
    } else {
        if (out != NULL)
            memcpy(out, &tok, sizeof(tok));
        CONSUME();
        return true;
    }
}

void parser_parse_file(Compiler_Context* cc, LL_Parser* parser) {
	LL_Token token;
	PEEK(&token);
}

Ast_Base* parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, int last_precedence) {
    Ast_Base* left = parser_parse_primary(cc, parser);
    return left;
}

Ast_Base* parser_parse_primary(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token pk;
    Ast_Base* result;
    PEEK(&pk);

    switch (pk.kind) {
    case '(':
        CONSUME();
        result = parser_parse_expression(cc, parser, 0);
        EXPECT(')', NULL);
        break;
    
    case LL_TOKEN_KIND_INT:
        result = CREATE_NODE(AST_KIND_LITERAL_INT, ((Ast_Literal){ .i64 = 0 }));
        break;

    default:
        UNEXPECTED();
        break;
    }

    return result;
}