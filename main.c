#include "flex.h"
#include "builtins.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

extern char *yytext;
extern int yyleng;

int last_status = 0;
#define MAX_ARGS 100

void run_command(int argc, char **argv)
{
	if (!argc) return;
	char *command = *argv;
	//printf("Command: %s", command);
	for (int i = 1; i < argc; ++i) {
		//printf(" %s", argv[i]);
	}
	if (!strcmp(command, "cd"))
		cd(argc, argv);
	else if (!strcmp(command, "pwd"))
		pwd(argc, argv);
	else {
		// not a builtin
		int child_pid;
		switch (child_pid = fork()) {
			case -1:
				// TODO handle
				break;
			case 0:
				// child
				execvp(*argv, argv);
				exit(1);
				break;
			default:
				int status;
				wait(&status);
				last_status = status;
		}
	}
}

int main(int argc, char **argv)
{
	init_wds();
	set_input(argc, argv);
	//printf("initialized\n");

	char *cmd_argv[MAX_ARGS];
	int cmd_argc = 0;

	while (1) {
		InputToken token = yylex();
		switch (token) {
			case Semicolon:
			case Eol:
			case Eof:
				//printf("command running %d\n", cmd_argc);
				cmd_argv[cmd_argc] = NULL;
				run_command(cmd_argc, cmd_argv);
				for (int i = 0; i < cmd_argc; ++i) {
					free(cmd_argv[i]);
					cmd_argv[i] = NULL;
				}
				cmd_argc = 0;
				break;
			default:
				if (argc >= MAX_ARGS) {
					// TODO handle error
				} else {
					//printf("token found %d\n", yyleng);
					cmd_argv[cmd_argc] = malloc(sizeof(char) * (yyleng + 1));
					strcpy(cmd_argv[cmd_argc], yytext);
					cmd_argc++;
				}
		}

		if (token == Eof)
			exit(last_status);
	}	
}



			


