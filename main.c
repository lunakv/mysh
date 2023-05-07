#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "builtins.h"
#include "status.h"
#include "parser.h"

void run_interactive() {
    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, stdin) != -1) {
        parse_line(line);
        free(line);
        line = NULL;
    }
    free(line);
}

int main(int argc, char **argv) {
    init_wds();
    if (argc < 2) {
        debug("interactive mode");
        run_interactive();
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

