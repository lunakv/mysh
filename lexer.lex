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
#.*\n           return Eol;

  /* command separators */
;               return Semicolon;
;;+.*           return ErrMultipleSemicolons;
\n              return Eol;
<<EOF>>         return Eof;

  /* actual command words */
[^;\n \t#]+     {
    if (yyleng >= MAX_TOKEN_LENGTH)
        return ErrTokenTooLong;
    return Argument;
}

  /* whitespace is ignored */
[ \t]+          { }

%%

void set_input(int argc, char **argv) {
    // arguments beyond the first are ignored (this mirrors behavior of other shells)
    if (argc == 1)
        yyin = stdin;
    else
        yyin = fopen(argv[1], "r");
}
