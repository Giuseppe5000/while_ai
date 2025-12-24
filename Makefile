CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -Werror=vla -ggdb

all: main

main: main.c src/lang/lexer.c src/utils.c src/lang/parser.c src/lang/cfg.c src/domain/abstract_interval_domain.c src/abstract_analyzer.c
	$(CC) $(CFLAGS) main.c src/lang/lexer.c src/utils.c src/lang/parser.c src/lang/cfg.c src/domain/abstract_interval_domain.c src/abstract_analyzer.c -o main
