@echo off
set flags=-std=gnu11 -Wall -Wextra  -Wswitch
set debug_flags=%flags% -g -O0  -D_DEBUG
set c_files=src/main.c src/lexer.c src/common.c src/parser.c src/typer.c src/backend.c src/eval.c src/callconv.c

@echo on
@REM clang -std=gnu11 src/main.c src/lexer.c src/common.c src/parser.c src/typer.c src/backend.c src/eval.c -g -Wall -Wextra -o main.exe
@REM clang -std=gnu11 src/main.c src/lexer.c src/common.c src/parser.c src/typer.c src/backend.c src/eval.c -g -Wall -Wextra -fsanitize=address -D_DEBUG -o main.exe
clang %c_files% %debug_flags% -o main.exe

@REM clang %flags% -c src/main.c    -o src/main.o
@REM clang %flags% -c src/lexer.c   -o src/lexer.o
@REM clang %flags% -c src/common.c  -o src/common.o
@REM clang %flags% -c src/parser.c  -o src/parser.o
@REM clang %flags% -c src/typer.c   -o src/typer.o
@REM clang %flags% -c src/backend.c -o src/backend.o
@REM clang %flags% -c src/eval.c    -o src/eval.o

@REM clang src/main.o src/lexer.o src/common.o src/parser.o src/typer.o src/backend.o src/eval.o -g -Wall -Wextra   -fprofile-instr-generate -fcoverage-mapping  -o main.exe


@REM clang -std=gnu11 src/main.c src/lexer.c src/common.c src/parser.c src/typer.c src/backend.c src/eval.c  -Wall -Wextra -g -fsanitize=address -O3 -D_NDEBUG  -o main.exe

@REM clang -std=gnu11 src/main.c src/lexer.c src/common.c src/parser.c src/typer.c src/backend.c src/eval.c -g -Wall  -o main.exe
@REM cl   /std:c11  src\main.c src\lexer.c src\common.c src\parser.c src\typer.c src\backend.c src\eval.c /DEBUYG /Wall /wd4464 /wd4820  /OUT:main.cl.exe
@REM clang -std=gnu11 src/backend.c -g -Wall 