#include "builtins.h"
#include "check.h"
#include "status.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <string.h>

#define MAX_PATH_LEN 512
// last and current working directory (we need both due to "cd -")
char *last_wd;
char *curr_wd;

/* Initializes the working directory variables on startup */
void init_wds() {
    last_wd = malloc_safe(sizeof(char) * MAX_PATH_LEN);
    curr_wd = malloc_safe(sizeof(char) * MAX_PATH_LEN);
    // TODO gracefully handle when getcwd fails
    UNWRAP_P(getcwd(curr_wd, MAX_PATH_LEN));
    strcpy(last_wd, curr_wd);
}

void update_wds() {
    strcpy(last_wd, curr_wd);
    // TODO gracefully handle when getcwd fails (increasing buffer size?)
    UNWRAP_P(getcwd(curr_wd, MAX_PATH_LEN));
}

void cd(int argc, char **argv) {
    char *path;
    if (argc > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        set_return_status(1);
        return;
    }

    if (argc == 1) {
        // no argument -> go to home directory
        path = getenv("HOME");
        if (path == NULL) {
            // if $HOME is not set, get it from user's passwd
            struct passwd *pwd = getpwuid(getuid());
            CHCK_P(pwd);
            path = pwd->pw_dir;
        }
    } else if (!strcmp(argv[1], "-")) {
        // special case - go to last directory
        path = last_wd;
        printf("%s\n", path);
    } else {
        path = argv[1];
    }

    CHCK(chdir(path));

    // if we changed directories, update new and previous dir
    update_wds();
    set_return_status(0);
}

void builtin_exit() {
    exit(get_return_status());
}
