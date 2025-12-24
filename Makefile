CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -Werror=vla -ggdb

all: main

main: main.c src/lang/lexer.c src/utils.c src/lang/parser.c src/lang/cfg.c src/abstract_interval_domain.c
	$(CC) $(CFLAGS) main.c src/lang/lexer.c src/utils.c src/lang/parser.c src/lang/cfg.c src/abstract_interval_domain.c -o main
