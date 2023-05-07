#ifndef MYSH_CHECK_H_
#define MYSH_CHECK_H_

#include <err.h>
#include <stddef.h>

// macros that return from the caller with a warning on error (only used in builtin functions)
#define CHCK(N) \
    if ((N) == -1) { warn(NULL); set_return_status(1); return; }

#define CHCK_P(P) \
    if ((P) == NULL) { warn(NULL); set_return_status(1); return; }

// functions that exit the shell entirely on error
// these aren't macros, because we want to avoid repeated evaluation of the parameter
int UNWRAP(int n);
void *UNWRAP_P(void *p);

// wrapper function that calls malloc and kills the shell when there's an error
void *malloc_safe(size_t n);

#endif  // MYSH_CHECK_H_
