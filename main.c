
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "lexer.h"

int main() {
    Compiler_Context cc = { 0 };
    
    FILE* input = fopen("test.t", "r");

    fseek(input, 0, SEEK_END);
    size_t input_size = ftell(input);
    fseek(input, 0, SEEK_SET);

    printf("%d\n", input_size);
    uint8_t* input_contents = malloc(input_size);
    if (!input_contents) ABORT("OOM");

    fread(input_contents, 1, input_size, input);
    printf("%s\n", input_contents);

    LL_Lexer lexer = {
        .pos = 0,
        .source.ptr = (char*)input_contents,
        .source.len = input_size,
    };
    LL_Token token;
    while (lexer_next_token(&cc, &lexer, &token)) {
        lexer_print_token(&lexer, &token);
    }
	return 0;
}
