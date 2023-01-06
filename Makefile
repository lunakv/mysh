LEX := flex
CC := gcc

.PHONY: all
all: lex.yy.c main.c builtins.c
	$(CC) -o mysh lex.yy.c builtins.c main.c

lex.yy.c: lexer.lex
	$(LEX) -o lex.yy.c lexer.lex 

.PHONY: clean
clean:
	@rm -f lex.yy.c mysh

.PHONY: in
in:
	./mysh < in
