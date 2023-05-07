/*
 * cat.lex: A demonstration of YY_NEW_FILE.
 */

%{
#include <string.h>
#include <stdio.h>
#include "structures.h"
#include "parser.h"
#include "check.h"
int MAX_TOKEN_LENGTH = 100;

%}

%option noyywrap nounput noinput

%%

  /* comment */
#.*\n           return Eol;

  /* special symbols */
;               return Semicolon;
\n              return Eol;
\<               return Lt;
\>               return Gt;
\>\>              return DoubleGt;

  /* actual command words */
[^;\n \t#><]+     {
    yylval.text = UNWRAP_P(strdup(yytext));
    return Argument;
}

  /* whitespace is ignored */
[ \t]+          { }

%%

void set_input(char *file) {
    yyin = UNWRAP_P(fopen(file, "r"));
}
