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
    Pipe pipe;
    Command *command;
    CommandArgument *argument;
    ArgumentType argumentType;
}


%token Eol
%token Semicolon
%token Argument
%token ErrTokenTooLong
%token Lt
%token Gt
%token DoubleGt
%token Vert

%type<pipe> pipe
%type<command> command command_ne
%type<argument> argument redirect argument_or_redirect
%type<argumentType> redirect_token
%type<text> Argument

%%

program: | line | Eol program | line Eol program;

line: invocation | invocation Semicolon | invocation Semicolon line;

invocation: pipe { handle_invocation(&$1); }

pipe:
      command {
        Pipe pipe;
        SLIST_INIT(&pipe);
        PipeSegment *segment = malloc_safe(sizeof(PipeSegment));
        segment->command = $1;
        SLIST_INSERT_HEAD(&pipe, segment, next);
        $$ = pipe;
      }
    | command Vert pipe {
    	PipeSegment *segment = malloc_safe(sizeof(PipeSegment));
    	segment->command = $1;
	SLIST_INSERT_HEAD(&$3, segment, next);
	$$ = $3;
      }
    ;

command:
      argument command_ne {
        SLIST_INSERT_HEAD($2, $1, next);
        $$ = $2;
      }
    | redirect command {
        SLIST_INSERT_HEAD($2, $1, next);
        $$ = $2;
      }
    | argument {
        Command *command = malloc_safe(sizeof(Command));
        SLIST_INIT(command);
        SLIST_INSERT_HEAD(command, $1, next);
        $$ = command;
      }
    ;

command_ne:
      argument_or_redirect command_ne {
        SLIST_INSERT_HEAD($2, $1, next);
        $$ = $2;
      }
    | argument_or_redirect {
        Command *command = malloc_safe(sizeof(Command));
        SLIST_INIT(command);
        SLIST_INSERT_HEAD(command, $1, next);
        $$ = command;
      }
    ;

argument_or_redirect: argument | redirect;

redirect: redirect_token Argument {
    CommandArgument *ca = malloc_safe(sizeof(CommandArgument));
    ca->text = $2;
    ca->type = $1;
    $$ = ca;
};

argument: Argument {
    CommandArgument *ca = malloc_safe(sizeof(CommandArgument));
    ca->text = $1;
    ca->type = Regular;
    $$ = ca;
};

redirect_token:
      Lt { $$ = RedirectInput; }
    | Gt { $$ = RedirectOutput; }
    | DoubleGt { $$ = RedirectAppend; }
    ;

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
