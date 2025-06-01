
#include <stdio.h>

#include "common.h"
#include "parser.h"
#include "typer.h"

int main() {
    Compiler_Context cc = ll_compiler_context_create();
	LL_Parser parser = parser_create_from_file(&cc, "test.t");

	Ast_Base* root = parser_parse_file(&cc, &parser);
	LL_Typer typer = ll_typer_create(&cc);
	ll_typer_run(&cc, &typer, root);
    
    /* LL_Token token; */
    /* while (lexer_next_token(&cc, &lexer, &token)) { */
    /*     lexer_print_token(&lexer, &token); */
    /* } */
	return 0;
}
