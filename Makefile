CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pedantic -Werror=vla -ggdb

all: cli test

cli: cli.c src/lang/lexer.c src/common.c src/lang/parser.c src/lang/cfg.c src/domain/abstract_interval_domain.c src/abstract_analyzer.c src/domain/wrappers/abstract_interval_domain_wrap.c
	$(CC) $(CFLAGS) cli.c src/lang/lexer.c src/common.c src/lang/parser.c src/lang/cfg.c src/domain/abstract_interval_domain.c src/abstract_analyzer.c src/domain/wrappers/abstract_interval_domain_wrap.c -o cli

test: abstract_interval_domain_test

abstract_interval_domain_test: test/abstract_interval_domain_test.c src/common.c
	$(CC) $(CFLAGS) src/common.c test/abstract_interval_domain_test.c -o test/abstract_interval_domain_test
	./test/abstract_interval_domain_test
	rm ./test/abstract_interval_domain_test
