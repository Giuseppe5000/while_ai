CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -Werror=vla -ggdb

all: main test

main: main.c src/lang/lexer.c src/utils.c src/lang/parser.c src/lang/cfg.c src/domain/abstract_interval_domain.c src/abstract_analyzer.c
	$(CC) $(CFLAGS) main.c src/lang/lexer.c src/utils.c src/lang/parser.c src/lang/cfg.c src/domain/abstract_interval_domain.c src/abstract_analyzer.c -o main

test: abstract_interval_domain_test

abstract_interval_domain_test: test/abstract_interval_domain_test.c src/utils.c
	$(CC) $(CFLAGS) src/utils.c test/abstract_interval_domain_test.c -o test/abstract_interval_domain_test
	./test/abstract_interval_domain_test
	rm ./test/abstract_interval_domain_test
