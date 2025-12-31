#!/bin/bash

flags="-std=gnu11 -Wall -Wextra"
debug_flags="$flags -g -O0 -fno-omit-frame-pointer -D_DEBUG"
c_files="src/main.c src/lexer.c src/common.c src/parser.c src/typer.c src/typer2.c src/backend.c src/eval.c src/callconv.c"

set -x
clang $c_files $debug_flags -lm -o main.exe
