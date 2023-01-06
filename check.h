#ifndef MYSH_CHECK_H_
#define MYSH_CHECK_H_

#include <err.h>
#include <stddef.h>

// macros that return from the caller with a warning on error
#define CHCK(N) \
    if ((N) == -1) { warn(NULL); return 1; }

#define CHCK_P(P) \
    if ((P) == NULL) { warn(NULL); return 1; }

// functions that exit the shell entirely on error
// these aren't macros, because they can be inlined, and
// we want to avoid repeated evaluation of the parameter
int UNWRAP(int n) {
    if (n == -1)
        err(1, NULL);
    return n;
}

void *UNWRAP_P(void *p) {
    if (p == NULL)
        err(1, NULL);
    return p;
}

#endif  // MYSH_CHECK_H_
