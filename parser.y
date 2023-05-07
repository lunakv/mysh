%define parse.error detailed

%code requires {
#include "structures.h"

void parse_line(char *input);
void parse_file(char *path); // defined in lexer.lex
}

%{
#include <stdio.h>
#include <sys/queue.h>
#include <string.h>
#include "handlers.h"
#include "check.h"
#include "status.h"

extern int yylex();
extern void yyerror(const char *err);
extern void yy_scan_string(char *line);
extern void yylex_destroy();
extern void set_input(char *path);

%}

%union {
    char *text;
    Command command;
    CommandArgument *argument;
}


%token Eol
%token Semicolon
%token Argument
%token ErrTokenTooLong
%token Lt
%token Gt

%type<command> command
%type<argument> argument
%type<text> Argument

%%

program: | line | Eol program | line Eol program;

line: invocation | invocation Semicolon | invocation Semicolon line;

invocation: command { handle_invocation(&$1); }

command:
    argument command {
        SLIST_INSERT_HEAD(&$2, $1, next);
        $$ = $2;
    }
    | argument {
        Command command;
        SLIST_INIT(&command);
        SLIST_INSERT_HEAD(&command, $1, next);
        $$ = command;
    };

argument: Argument {
    CommandArgument *ca = malloc_safe(sizeof(CommandArgument));
    ca->text = $1;
    $$ = ca;
}

%%

void yyerror(const char *err) {
    fprintf(stderr, "ERROR: %s\n", err);
    set_return_status(2);
}

void parse_line(char *line) {
    yy_scan_string(line);
    yyparse();
    yylex_destroy();
}

void parse_file(char *path) {
    set_input(path);
    yyparse();
}
