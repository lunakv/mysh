#include "builtins.h"
#include "status.h"
#include "structures.h"
#include "debug.h"
#include <stdlib.h>
#include <sys/queue.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>

/* free allocated structures after command is run */
void free_command(Command *command) {
    while (!SLIST_EMPTY(command)) {
       CommandArgument *head = SLIST_FIRST(command);
       SLIST_REMOVE_HEAD(command, next);
       free(head->text);
       free(head);
    }
}

int argument_count(Command *command) {
    int len = 0;
    CommandArgument *arg;
    SLIST_FOREACH(arg, command, next) {
       len++;
    }
    return len;
}

/* Run a command specified on input - argv[0] contains name of command */
int exec_command(int argc, char **argv) {
    if (!argc) return 0;
    char *command = *argv;
    debug("Command: %s", command);
    for (int i = 1; i < argc; ++i)
        debug("Argument: %s", argv[i]);

    if (!strcmp(command, "cd"))
        return cd(argc, argv);
    if (!strcmp(command, "pwd"))
        return pwd(argc);

    // not a builtin, fork+exec
    int status;
    switch (fork()) {
        case -1:
            // error
            warn(NULL);
            status = 1;
            break;
        case 0:
            // child process
            execvp(*argv, argv);
            // exec returns only on error
            warn(NULL);
            status = 1;
            break;
        default:
            // parent process
            wait(&status);
    }
    return status;

}

void handle_invocation(Command *command) {
    int arg_count = argument_count(command);
    char *args[arg_count + 1];
    arg_count = 0;
    CommandArgument *cap;
    SLIST_FOREACH(cap, command, next)
        args[arg_count++] = cap->text;

    args[arg_count] = NULL;

    int return_status = exec_command(arg_count, args);
    set_return_status(return_status);
    free_command(command);
}