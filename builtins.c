#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <string.h>

#define MAX_PATH_LEN 512
char *last_wd;
char *curr_wd = NULL;

// TODO error checking
// TODO argument checking
// TODO error reporting

void init_wds() {
	last_wd = malloc(sizeof(char) * MAX_PATH_LEN);
	curr_wd = malloc(sizeof(char) * MAX_PATH_LEN);
	getcwd(curr_wd, MAX_PATH_LEN);
	*last_wd = '\0';
}

void update_wds() {
	strcpy(last_wd, curr_wd);
	getcwd(curr_wd, MAX_PATH_LEN);
}

int cd(int argc, char **argv)
{
	char *path;
	if (argc > 2) {
		fprintf(stderr, "cd: too many arguments\n");
		return 1;
	}

	if (argc == 1) {
		// no argument -> go to home
		path = getenv("HOME");
		if (path == NULL) {
			// if $HOME is not set, get it from user's passwd
			struct passwd *pwd = getpwuid(getuid());
			path = pwd->pw_dir;
		}
	} else {
		path = argv[1];
	}

	chdir(path);
	
	// if we changed directories, save new and previous dir
	update_wds();
	return 0;
}

int pwd(int argc, char **argv)
{
	if (argc > 1) {
		fprintf(stderr, "pwd: too many arguments\n");
		return 1;
	}

	printf("%s\n", curr_wd);
	return 0;
}
