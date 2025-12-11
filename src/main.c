#include "common.h"
#include "parser.h"
#include "typer.h"
#include "eval.h"
#include "backend.h"

#define shift(xs, xs_sz) (oc_assert((xs_sz) > 0), (xs_sz)--, *(xs)++)

int main(int argc, char** argv) {
	bool quiet = false;
	bool run = false;
    char* filename = NULL;
    bool output_ir = false;
	shift(argv, argc);

    while (argc) {
        char* arg = shift(argv, argc);

        if (strcmp(arg, "--quiet") == 0) {
            quiet = true;
        } else if (strcmp(arg, "--run") == 0) {
            run = true;
        } else if (strcmp(arg, "--ir") == 0) {
            output_ir = true;
        } else {
            if (arg[0] == '-' && arg[1] == '-') {
				eprint("Ignoring unknown argument '{}'\n", arg);
            } else {
                if (filename) {
                    eprint("Expected only one file name, got '{}' but have '{}'\n", arg, filename);
					return 1;
                } else {
					filename = arg;
				}
            }
        }
    }
	if (!filename) {
		eprint("Expected a filename!\n");
		return 1;
	}

    Compiler_Context cc = ll_compiler_context_create();
	cc.quiet = quiet;

    LL_Parser parser = parser_create_from_file(&cc, filename);
    Ast_Base* root = parser_parse_file(&cc, &parser);
    LL_Typer typer = ll_typer_create(&cc);
    LL_Eval_Context eval_context = {};

    LL_Backend backend_elf = ll_backend_init(&cc, LL_BACKEND_LINUX_X86_64_ELF);
    LL_Backend backend_ir = ll_backend_init(&cc, LL_BACKEND_IR);
    // LL_Backend backend_c = ll_backend_init(&cc, LL_BACKEND_C);

    cc.typer = &typer;
    cc.eval_context = &eval_context;
    cc.bir = backend_ir.backend;
    cc.target = &backend_elf;
    cc.lexer = &parser.lexer;

    if (!cc.quiet) print_node(root, 0, &stdout_writer);
    ll_typer_run(&cc, &typer, root);
    if (!cc.quiet) print_node(root, 0, &stdout_writer);


    // ll_backend_generate_statement(&cc, &backend_c, root);
    // ll_backend_write_to_file(&cc, &backend_c, "out.c");

    ll_backend_generate_statement(&cc, &backend_ir, root);
    if (output_ir) ll_backend_write_to_file(&cc, &backend_ir, "out.ir");

    /* LL_Backend backend = ll_backend_init(&cc, LL_BACKEND_LINUX_X86_64_GAS); */
    /* ll_backend_generate_statement_from_ir(&cc, &backend, backend_ir.backend); */
    /* ll_backend_write_to_file(&cc, &backend, "out.s"); */

    ll_backend_generate_statement_from_ir(&cc, &backend_elf, backend_ir.backend);
    ll_backend_write_to_file(&cc, &backend_elf, "out.bin");
    if (run) ll_backend_execute(&cc, &backend_elf, backend_ir.backend);

    /* x86_64_run_tests(&cc, backend_elf.backend); */

    
    /* LL_Token token; */
    /* while (lexer_next_token(&cc, &lexer, &token)) { */
    /*     lexer_print_token(&lexer, &token); */
    /* } */
    return 0;
}
