LEX := flex
YACC := bison
YACCFLAGS := --header=parser.h
CC := gcc
CCFLAGS := -o mysh -Wall -Wextra

.PHONY: all debug clean in test test1
all: lex.yy.c parser.c builtins.c check.c debug.c handlers.c status.c main.c
	$(CC) $(CCFLAGS) $^

debug: CCFLAGS+= -D DEBUG -O0 -g
debug: all

lex.yy.c: lexer.lex
	$(LEX) -o lex.yy.c lexer.lex 

parser.c: parser.y
	$(YACC) $(YACCFLAGS) -o parser.c parser.y

clean:
	@rm -f lex.yy.c parser.c parser.h mysh

test1:
	cd tests && ./run-tests.sh `cat phase-1.tests`

test:
	cd tests && ./run-tests.sh

