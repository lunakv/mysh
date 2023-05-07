#include "builtins.h"
#include "status.h"
#include "structures.h"
#include "debug.h"
#include "check.h"
#include <stdlib.h>
#include <sys/queue.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

struct Redirects {
    char *in;
    char *out;
    bool append;
};
typedef struct Redirects Redirects;

struct StdDescriptors {
    int stdin;
    int stdout;
};
typedef struct StdDescriptors StdDescriptors;

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
       if (arg->type == Regular)
           len++;
    }
    return len;
}

Redirects parse_arguments(Command *command, char *args[]) {
    Redirects redirects = { NULL, NULL, false };
    int argc = 0;
    CommandArgument *arg;
    SLIST_FOREACH(arg, command, next) {
       switch (arg->type) {
           case Regular:
               args[argc++] = arg->text;
               break;
           case RedirectInput:
               redirects.in = arg->text;
               break;
           case RedirectOutput:
               redirects.out = arg->text;
               redirects.append = false;
               break;
           case RedirectAppend:
               redirects.out = arg->text;
               redirects.append = true;
               break;
       }
    }
    args[argc] = NULL;
    return redirects;
}

// sets stdin and stdout to specified redirect values
struct StdDescriptors open_redirects(Redirects redirects) {
    // this function is called only in the forked child process, so it's OK to
    // fully exit the process if we can't open the std file descriptors
    StdDescriptors new_desc = { STDIN_FILENO, STDOUT_FILENO };
    if (redirects.in != NULL) {
        debug("Opening new stdin at '%s'", redirects.in);
        new_desc.stdin = UNWRAP(open(redirects.in, O_RDONLY, 0666));
        debug("New stdin created at %d", new_desc.stdin);
    }
    if (redirects.out != NULL) {
        debug("Opening new stdout at '%s'", redirects.out);
        int mode = O_WRONLY | O_CREAT;
        mode |= redirects.append ? O_APPEND : O_TRUNC;
        new_desc.stdout = UNWRAP(open(redirects.out, mode, 0666));
        debug("New stdout created at %d", new_desc.stdout);
    }
    return new_desc;
}

void set_descriptors(StdDescriptors desc) {
    // this function is called only in the forked child process, so it's OK to
    // fully exit the process if we can't properly set the std file descriptors
    debug("setting std file descriptors - in: %d, out: %d", desc.stdin, desc.stdout);
    if (desc.stdin != STDIN_FILENO) {
        UNWRAP(dup2(desc.stdin, STDIN_FILENO));
        UNWRAP(close(desc.stdin));
    }
    if (desc.stdout != STDOUT_FILENO) {
        UNWRAP(dup2(desc.stdout, STDOUT_FILENO));
        UNWRAP(close(desc.stdout));
    }
}

/* Run a command specified on input - argv[0] contains name of command */
int exec_command(int argc, char **argv, Redirects redirects) {
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
            set_descriptors(open_redirects(redirects));
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
    Redirects redirects = parse_arguments(command, args);

    int return_status = exec_command(arg_count, args, redirects);
    set_return_status(return_status);
    free_command(command);
}