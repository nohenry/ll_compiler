#include "common.h"
#include "parser.h"
#include "typer.h"
#include "../core/machine_code.h"
#include "eval.h"
#include "backend.h"

#define shift(xs, xs_sz) (oc_assert((xs_sz) > 0), (xs_sz)--, *(xs)++)

int main(int argc, char** argv) {
	bool quiet = false;
	bool run = false;
    char* filename = NULL;
    bool output_ir = false;
	shift(argv, argc);
    bool exit_0 = false;

    while (argc) {
        char* arg = shift(argv, argc);

        if (strcmp(arg, "--quiet") == 0) {
            quiet = true;
        } else if (strcmp(arg, "--run") == 0) {
            run = true;
        } else if (strcmp(arg, "--ir") == 0) {
            output_ir = true;
        } else if (strcmp(arg, "--exit-0") == 0) {
            exit_0 = true;
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
    cc.exit_0 = exit_0;

    LL_Typer typer = ll_typer_create(&cc);
    cc.typer = &typer;

    LL_Eval_Context eval_context = { 0 };
    ll_eval_init(&cc, &eval_context);

    LL_Backend backend_elf = ll_backend_init(&cc, LL_BACKEND_LINUX_X86_64_ELF);
    LL_Backend backend_ir = ll_backend_init(&cc, LL_BACKEND_IR);

    cc.typer = &typer;
    cc.eval_context = &eval_context;
    cc.bir = backend_ir.backend;

    LL_Parser parser = parser_create_from_file(&cc, filename);
    cc.lexer = &parser.lexer;

    Code* root = parser_parse_file(&cc, &parser);

    // print_things(&cc, &parser);
    // if (!cc.quiet) {
    //     print("typer queued: {}\n", typer.queue.count);
    //     ll_typer_run(&cc, &typer, root);
    //     print("typer queued: {}\n", typer.queue.count);
    //     for (uint32 i = 0; i < typer.queue.count; ++i) {
    //         if (typer.queue.items[i]->stmt_yielded_index == (uint32)-1) {
    //             // typer.queue.items[i]->decl_yielded_hash
    //             // Code_Declaration** v = hash_map_get_from_hash(&cc.arena, &typer.queue.items[i]->yielded_in_scope->declarations, typer.queue.items[i]->decl_str, typer.queue.items[i]->decl_yielded_hash);

    //             print("  decl {} -> {}\n", (void*)typer.queue.items[i]->yielded_in_scope, typer.queue.items[i]->decl_str);
    //         } else {
    //             print("  stmt {} -> {}\n", (void*)typer.queue.items[i]->yielded_in_scope, typer.queue.items[i]->stmt_yielded_index);
    //         }
    //     }
    // }

#if 1
    cc.target = &backend_elf;
    cc.native_target = &backend_elf;

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
#endif

    free(parser.lexer.source.ptr);

    return 0;
}
