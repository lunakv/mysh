/*
 * cat.lex: A demonstration of YY_NEW_FILE.
 */

%{
#include "flex.h"
#include <string.h>
#include <stdio.h>
%}

%option noyywrap nounput noinput

%%

  /* comment */
#.*\n	return Eol;

;		return Semicolon;
\n		return Eol;
<<EOF>>	return Eof;

[ \t]+  { }

[^;\n \t#]+	{
		if (yyleng >= MAX_TOKEN_LENGTH)
			return ErrTokenTooLong;
		return Argument;
}

%%

void set_input(int argc, char **argv)
{
	// TODO handle multiple input arguments
	if (argc == 1)
		yyin = stdin;
	else
		yyin = fopen(argv[1], "r");
}
