#include "handlers.h"
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
#include <stdio.h>
#include <errno.h>

struct Redirects {
    char *in;
    char *out;
    bool append;

    int pipe_in;
    int pipe_out;
};
typedef struct Redirects Redirects;

struct StdDescriptors {
    int stdin;
    int stdout;
};
typedef struct StdDescriptors StdDescriptors;

int *child_pids;
int child_count = 0;

/* free allocated structures after command is run */
void free_command(Command *command) {
    while (!SLIST_EMPTY(command)) {
       CommandArgument *head = SLIST_FIRST(command);
       SLIST_REMOVE_HEAD(command, next);
       free(head->text);
       free(head);
    }
}

void free_pipeline(Pipe *pipeline) {
    while (!SLIST_EMPTY(pipeline)) {
        PipeSegment *head = SLIST_FIRST(pipeline);
        SLIST_REMOVE_HEAD(pipeline, next);
        free_command(head->command);
        free(head);
    }
}

int command_argument_count(Command *command) {
    int len = 0;
    CommandArgument *arg;
    SLIST_FOREACH(arg, command, next) {
       if (arg->type == Regular)
           len++;
    }
    return len;
}

int pipe_segment_count(Pipe *pipe) {
    int len = 0;
    PipeSegment *segment;
    SLIST_FOREACH(segment, pipe, next)
        len++;
    return len;
}

Redirects parse_arguments(PipeSegment *segment, char *args[], int in_fd, int out_fd) {
    Command *command = segment->command;
    Redirects redirects = {NULL, NULL, false, in_fd, out_fd };
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
    } else if (redirects.pipe_in != -1) {
        // only redirect to pipe if an explicit redirect from file is not set
        new_desc.stdin = redirects.pipe_in;
    }
    if (redirects.out != NULL) {
        debug("Opening new stdout at '%s'", redirects.out);
        int mode = O_WRONLY | O_CREAT;
        mode |= redirects.append ? O_APPEND : O_TRUNC;
        new_desc.stdout = UNWRAP(open(redirects.out, mode, 0666));
        debug("New stdout created at %d", new_desc.stdout);
    } else if (redirects.pipe_out != -1) {
        // only redirect to pipe if an explicit redirect to file is not set
        new_desc.stdout = redirects.pipe_out;
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

    // builtin handlers
    if (!strcmp(command, "cd")) {
        cd(argc, argv);
        return 0;
    }
    if (!strcmp(command, "exit")) {
        builtin_exit();
    }

    // not a builtin, fork+exec
    int pid;
    switch (pid = fork()) {
        case -1:
            // error
            warn(NULL);
            set_return_status(1);
            return -1;
        case 0:
            // child process
            set_descriptors(open_redirects(redirects));
            execvp(*argv, argv);
            // exec returns only on error - we can kill the child in that case
            warn("%s", *argv);
            exit(127);
        default:
            // parent process
            return pid;
    }
}

int handle_segment(PipeSegment *segment, int in_fd, int out_fd) {
    Command *command = segment->command;
    int arg_count = command_argument_count(command);
    char *args[arg_count + 1];
    Redirects redirects = parse_arguments(segment, args, in_fd, out_fd);

    return exec_command(arg_count, args, redirects);
}


void handle_invocation(Pipe *pipeline) {
    int size = pipe_segment_count(pipeline);
    debug("Handling pipe with %d segments", size);
    child_count = 0;
    free(child_pids);
    child_pids = malloc_safe(size * sizeof(int));
    child_count = size;

    int in_fd = STDIN_FILENO;
    int out_fd = STDOUT_FILENO;
    int fds[2] = {in_fd, out_fd};
    int i = 0;
    PipeSegment *segment;
    // start all sub-processes, connected with pipes
    SLIST_FOREACH(segment, pipeline, next) {
        in_fd = fds[0];
        if (i < size - 1) {
            UNWRAP(pipe(fds));
            out_fd = fds[1];
        } else {
            out_fd = STDOUT_FILENO;
        }

        debug("Creating subprocess, pipe in: %d, pipe out: %d", in_fd, out_fd);
        int child_pid = UNWRAP(handle_segment(segment, in_fd, out_fd));
        debug("Subprocess created with pid %d", child_pid);

        // child already duped the file descriptors, we should close them
        if (in_fd != STDIN_FILENO) {
            debug("closing fd %d", in_fd);
            UNWRAP(close(in_fd));
        }
        if (out_fd != STDOUT_FILENO) {
            debug("closing fd %d", out_fd);
            UNWRAP(close(out_fd));
        }

        child_pids[i++] = child_pid;
        if (child_pid <= 0) {
            break;
        }
    }

    // wait until all subprocesses end, save last child status
    int status;
    for (i = 0; i < size; i++) {
        int id = child_pids[i];
        debug("Waiting on child %d", id);
        if (id > 0) {
            int result = waitpid(child_pids[i], &status, 0);
            if (result == -1 && errno != EINTR)
                warn(NULL);
            debug("Child %d exited with status %d", id, status);
        }
        else
            break;
    }

    set_return_status(status);
    // workaround for "file not found" manually exited children
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
        set_return_status(127);
    free_pipeline(pipeline);
}

int *get_child_pids() {
    return child_pids;
}

int get_child_count() {
    return child_count;
}