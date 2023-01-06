#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <err.h>
#include "flex.h"
#include "debug.h"
#include "builtins.h"

// values from lexer describing parsed token
extern char *yytext;
extern int yyleng;

// shell exits with the return code of the last command
int last_status = 0;

#define MAX_ARGS 128
// arguments for the currently parsed command
char *cmd_argv[MAX_ARGS];
int cmd_argc = 0;

/* Clean up array of command arguments after command is run */
void clear_cmd() {
    for (int i = 0; i < cmd_argc; ++i) {
        free(cmd_argv[i]);
        cmd_argv[i] = NULL;
    }
    cmd_argc = 0;
}

/* Run a command specified on input - argv[0] contains name of command */
void run_command(int argc, char **argv) {
    if (!argc) return;
    char *command = *argv;
    debug("Command: %s", command);
    for (int i = 1; i < argc; ++i) {
        debug("Argument: %s", argv[i]);
    }

    if (!strcmp(command, "cd")) {
        last_status = cd(argc, argv);
    } else if (!strcmp(command, "pwd")) {
        last_status = pwd(argc, argv);
    } else {
        // not a builtin
        switch (fork()) {
            case -1:
                // error
                warn(NULL);
                last_status = 1;
                break;
            case 0:
                // child
                execvp(*argv, argv);
                // exec returns only on error
                warn(NULL);
                last_status = 1;
                break;
            default:
                // parent
                wait(&last_status);
        }
    }
}


int main(int argc, char **argv) {
    init_wds();
    set_input(argc, argv);
    debug("initialized");

    bool in_parsing_error = false;
    while (true) {
        InputToken token = yylex();
        switch (token) {
            case Semicolon:
            case Eol:
            case Eof:
                if (in_parsing_error) {
                    debug("Cleaning up parsing error - no commands run");
                    in_parsing_error = false;
                    last_status = 2;
                } else {
                    debug("Command running with %d arguments", cmd_argc);
                    cmd_argv[cmd_argc] = NULL;
                    run_command(cmd_argc, cmd_argv);
                }
                clear_cmd();
                break;
            case ErrMultipleSemicolons:
                if (!in_parsing_error)
                    fprintf(stderr, "syntax error near unexpected token `;;'\n");
                in_parsing_error = true;
                break;
            case ErrTokenTooLong:
                if (!in_parsing_error)
                    fprintf(stderr, "syntax error: encountered token larger than MAX_TOKEN_LENGTH\n");
                in_parsing_error = true;
                break;
            case Argument:
                if (cmd_argc >= MAX_ARGS) {
                    if (!in_parsing_error)
                        fprintf(stderr, "syntax error: too many arguments to a single command\n");
                    in_parsing_error = true;
                } else if (!in_parsing_error) {
                    debug("Token found (%d) '%s'", yyleng, yytext);
                    cmd_argv[cmd_argc] = malloc(sizeof(char) * (yyleng + 1));
                    strcpy(cmd_argv[cmd_argc], yytext);
                    cmd_argc++;
                }
                break;
        }

        if (token == Eof)
            return last_status;
    }
}

