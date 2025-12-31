#pragma once

#include "common.h"
#include "ast.h"

struct ll_parser;

typedef struct ll_typer2 {
    char a;
} LL_Typer2;

LL_Typer2 ll_typer2_create(Compiler_Context* cc);
void ll_typer2_run(Compiler_Context* cc, LL_Typer2* typer, struct ll_parser* parser, Code* root);

