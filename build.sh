#!/bin/bash
clang -std=c17 main.c lexer.c common.c parser.c typer.c -g -gdwarf-3 -Wno-switch -o main 
