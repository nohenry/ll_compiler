#include "common.h"
#include "parser.h"
#include "typer.h"
#include "typer2.h"
/* #include "eval.h" */
/* #include "backend.h" */

#define shift(xs, xs_sz) (oc_assert((xs_sz) > 0), (xs_sz)--, *(xs)++)

void print_things(Compiler_Context* cc, LL_Parser* parser) {

    Oc_String_Builder sb;
    oc_sb_init(&sb, &cc->arena);
	for (int i = 0; i < COUNT_OF_AST_RESULT; i++) {
        switch (i) {
        case 0:
            print("=== Operations ===\n");
            break;
        case COUNT_OF_LL_OPERATION:
            print("=== LHS ===\n");
            break;
        case COUNT_OF_LL_OPERATION * 2:
            print("=== RHS ===\n");
            break;
        case RESULT_KIND_IDENT:
            print("=== Identifiers ===\n");
            break;
        case RESULT_KIND_DECL:
            print("=== Declarations ===\n");
            break;
        case RESULT_KIND_INT:
            print("=== Integers ===\n");
            break;
        case RESULT_KIND_FLOAT:
            print("=== Floats ===\n");
            break;
        default:
            break;
        }

        sb.count = 0;
        wprint(&sb.writer, "{}:", i);
        print("{}", oc_sb_to_string(&sb));

		for (uint32 j = 0; j < parser->linear_grid[i].types.count; j++) {
            if (j == 0)  for (uint32 ws = 0; ws <= 4 - sb.count; ws++) print(" ");
            else print("    ");
			ll_print_type(parser->linear_grid[i].types.items[j]);
		}
        if (!parser->linear_grid[i].types.count) print("\n");
	}
}

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
    LL_Typer2 typer2 = ll_typer2_create(&cc);
    cc.typer2 = &typer2;


    LL_Parser parser = parser_create_from_file(&cc, filename);
    cc.lexer = &parser.lexer;

    Parse_Result root = parser_parse_file(&cc, &parser);
    if (!cc.quiet) print_node(root.code, 0, &stdout_writer);

    // print_things(&cc, &parser);
    ll_typer_run(&cc, &typer, root.code);
#if 0
    LL_Eval_Context eval_context = { 0 };
    ll_eval_init(&cc, &eval_context);

    LL_Backend backend_elf = ll_backend_init(&cc, LL_BACKEND_LINUX_X86_64_ELF);
    LL_Backend backend_ir = ll_backend_init(&cc, LL_BACKEND_IR);
    // LL_Backend backend_c = ll_backend_init(&cc, LL_BACKEND_C);

    cc.typer = &typer;
    cc.eval_context = &eval_context;
    cc.bir = backend_ir.backend;
    cc.target = &backend_elf;
    cc.native_target = &backend_elf;
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
#endif

    /* x86_64_run_tests(&cc, backend_elf.backend); */

    
    /* LL_Token token; */
    /* while (lexer_next_token(&cc, &lexer, &token)) { */
    /*     lexer_print_token(&lexer, &token); */
    /* } */
    return 0;
}
