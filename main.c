#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <stdbool.h>
#include "debug.h"
#include "builtins.h"
#include "handlers.h"
#include "status.h"
#include "parser.h"

void sigint_interactive_handler() {
    printf("\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

int handled_signal = -1;
void sigint_executable_handler(int signal) {
    int *pids = get_child_pids();
    for (int i = 0; i < get_child_count(); ++i) {
        int id = pids[i];
        if (id > 0) kill(id, signal);
    }
    handled_signal = signal;
    printf("\n");
    rl_on_new_line();
}


void run_interactive(struct sigaction *signal) {
    signal->sa_handler = sigint_interactive_handler;
    sigaction(SIGINT, signal, NULL);
    char *line = NULL;
    while ((line = readline("mysh$ "))) {
        signal->sa_handler = sigint_executable_handler;
        sigaction(SIGINT, signal, NULL);

        parse_line(line);

        if (handled_signal != -1) {
            set_return_status(128 + handled_signal);
            handled_signal = -1;
        }
        signal->sa_handler = sigint_interactive_handler;
        sigaction(SIGINT, signal, NULL);
        free(line);
        line = NULL;
    }
    free(line);
}

int main(int argc, char **argv) {
    init_wds();
    struct sigaction sig = {};
    sig.sa_handler = sigint_executable_handler;
    sigaction(SIGINT, &sig, NULL);

    if (argc < 2) {
        debug("interactive mode");
        run_interactive(&sig);
    } else if (!strcmp("-c", argv[1])) {
        debug("parsing argument string");
        if (argc < 3) {
            fprintf(stderr, "Option '-c' requires an argument.\n");
            return 3;
        }
        parse_line(argv[2]);
    } else {
        debug("parsing file");
        parse_file(argv[1]);
    }
    return get_return_status();
}

