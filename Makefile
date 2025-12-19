CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -Werror=vla

all: main

main: main.c src/lang/lexer.c src/utils.c src/lang/parser.c src/cfg.c
	$(CC) $(CFLAGS) main.c src/lang/lexer.c src/utils.c src/lang/parser.c src/cfg.c -o main
