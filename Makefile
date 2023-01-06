LEX := flex
CC := gcc
CCFLAGS := -o mysh

.PHONY: all debug clean in test test1
all: lex.yy.c
	$(CC) $(CCFLAGS) lex.yy.c builtins.c main.c

debug: CCFLAGS+= -D DEBUG
debug: all

lex.yy.c: lexer.lex
	$(LEX) -o lex.yy.c lexer.lex 

clean:
	@rm -f lex.yy.c mysh

test1:
	cd tests && ./run-tests.sh `cat phase-1.tests`

test:
	cd tests && ./run-tests.sh

