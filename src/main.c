#include "common.h"
#include "parser.h"
#include "emit.h"

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

    LL_Parser parser = parser_create_from_file(&cc, filename);
    cc.lexer = &parser.lexer;

    Code_Scope* root = parser_parse_file(&cc, &parser);
    if (!cc.quiet) print_node((Code*)root);

    LL_Emitter emit = ll_emitter_create(&cc);
    ll_emitter_run(&cc, &emit, root);

    print("{}\n", oc_sb_to_string(&emit.sb));

    return 0;
}
