CFLAGS=-Wall -Wextra -std=c99 -pedantic -Werror=vla -ggdb
SOURCES = $(shell find src -name "*.c")

all: cli test

cli: cli.c $(SOURCES)
	$(CC) $(CFLAGS) $^ -o cli

test: abstract_interval_domain_test

abstract_interval_domain_test: test/abstract_interval_domain_test.c src/common.c src/lang/parser.c src/lang/lexer.c
	$(CC) $(CFLAGS) $^ -o test/abstract_interval_domain_test
	./test/abstract_interval_domain_test
	rm ./test/abstract_interval_domain_test
