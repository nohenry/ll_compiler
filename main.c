
#include <stdio.h>

#include "common.h"
#include "parser.h"
#include "typer.h"
#include "eval.h"
#include "backend.h"
#include "backends/x86_64_common.h"

int main() {
    Compiler_Context cc = ll_compiler_context_create();
	LL_Parser parser = parser_create_from_file(&cc, "test.t");

	Ast_Base* root = parser_parse_file(&cc, &parser);
	LL_Typer typer = ll_typer_create(&cc);
	LL_Eval_Context eval_context = {};
	LL_Backend backend_ir = ll_backend_init(&cc, LL_BACKEND_IR);

	cc.typer = &typer;
	cc.eval_context = &eval_context;
	cc.bir = backend_ir.backend;

	ll_typer_run(&cc, &typer, root);

	ll_backend_generate_statement(&cc, &backend_ir, root);
	ll_backend_write_to_file(&cc, &backend_ir, "");

	/* LL_Backend backend = ll_backend_init(&cc, LL_BACKEND_LINUX_X86_64_GAS); */
	/* ll_backend_generate_statement_from_ir(&cc, &backend, backend_ir.backend); */
	/* ll_backend_write_to_file(&cc, &backend, "out.s"); */

	LL_Backend backend_elf = ll_backend_init(&cc, LL_BACKEND_LINUX_X86_64_ELF);
	ll_backend_generate_statement_from_ir(&cc, &backend_elf, backend_ir.backend);
	/* x86_64_run_tests(&cc, backend_elf.backend); */
	ll_backend_write_to_file(&cc, &backend_elf, "out.bin");

    
    /* LL_Token token; */
    /* while (lexer_next_token(&cc, &lexer, &token)) { */
    /*     lexer_print_token(&lexer, &token); */
    /* } */
	return 0;
}
