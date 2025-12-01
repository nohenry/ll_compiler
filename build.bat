
clang -std=gnu11 src/main.c src/lexer.c src/common.c src/parser.c src/typer.c src/backend.c src/eval.c -g -Wall -Wextra -o main.exe
@REM clang -std=gnu11 src/main.c src/lexer.c src/common.c src/parser.c src/typer.c src/backend.c src/eval.c -g -Wall -Wextra -fsanitize=address  -o main.exe

@REM clang -std=gnu11 src/main.c src/lexer.c src/common.c src/parser.c src/typer.c src/backend.c src/eval.c -g -Wall  -o main.exe
@REM cl   /std:c11  src\main.c src\lexer.c src\common.c src\parser.c src\typer.c src\backend.c src\eval.c /DEBUYG /Wall /wd4464 /wd4820  /OUT:main.cl.exe
@REM clang -std=gnu11 src/backend.c -g -Wall 