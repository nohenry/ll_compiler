
#include <stdlib.h>

#include "common.h"
#include "parser.h"
#include "lexer.h"

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
#define CONSUME(out) (lexer_next_token(cc, &(parser)->lexer, out))

void parser_parse_file(Compiler_Context* cc, LL_Parser* parser) {
	LL_Token token;
	PEEK(&token);
}
