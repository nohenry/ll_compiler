
#include <stdio.h>

#include "common.h"
#include "parser.h"

int main() {
    Compiler_Context cc = ll_compiler_context_create();
	LL_Parser parser = parser_create_from_file(&cc, "test.t");

	parser_parse_file(&cc, &parser);
    
    /* LL_Token token; */
    /* while (lexer_next_token(&cc, &lexer, &token)) { */
    /*     lexer_print_token(&lexer, &token); */
    /* } */
	return 0;
}
