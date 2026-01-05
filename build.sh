#!/bin/bash

flags="-std=gnu11 -Wall -Wextra"
debug_flags="$flags -g -O0 -fno-omit-frame-pointer -D_DEBUG"
c_files="src/main.c src/lexer.c src/common.c src/parser.c"

set -x
# clang $c_files $debug_flags -lm -o main.exe
clang $c_files $debug_flags -fsanitize=address -g -lm -o main.exe
