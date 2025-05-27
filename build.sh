#!/bin/bash
clang -std=c17 main.c lexer.c common.c -g -gdwarf-3 -o main 
