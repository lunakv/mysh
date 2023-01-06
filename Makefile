DEBUG:=$(filter debug,$(MAKECMDGOALS)) 

LEX := flex
CC := gcc
CCFLAGS :=

.PHONY: all debug clean in
all: lex.yy.c main.c builtins.c
	$(CC) $(CCFLAGS) -o mysh lex.yy.c builtins.c main.c

debug: CCFLAGS=-D DEBUG
debug: all

lex.yy.c: lexer.lex
	$(LEX) -o lex.yy.c lexer.lex 

clean:
	@rm -f lex.yy.c mysh

in:
	./mysh in

