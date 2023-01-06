#include "builtins.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include "check.h"

#define MAX_PATH_LEN 512
// last and current working directory (we need both due to "cd -")
char *last_wd;
char *curr_wd;

/* Initializes the working directory variables on startup */
void init_wds() {
    last_wd = UNWRAP_P(malloc(sizeof(char) * MAX_PATH_LEN));
    curr_wd = UNWRAP_P(malloc(sizeof(char) * MAX_PATH_LEN));
    // TODO gracefully handle when getcwd fails
    UNWRAP_P(getcwd(curr_wd, MAX_PATH_LEN));
    strcpy(last_wd, curr_wd);
}

void update_wds() {
    strcpy(last_wd, curr_wd);
    // TODO gracefully handle when getcwd fails (increasing buffer size?)
    UNWRAP_P(getcwd(curr_wd, MAX_PATH_LEN));
}

int cd(int argc, char **argv) {
    char *path;
    if (argc > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
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
    return 0;
}

int pwd(int argc, char **_) {
    if (argc > 1) {
        fprintf(stderr, "pwd: too many arguments\n");
        return 1;
    }

    printf("%s\n", curr_wd);
    return 0;
}
