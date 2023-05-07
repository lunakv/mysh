#include <err.h>
#include <stdlib.h>

int UNWRAP(int n){
    if (n == -1)
        err(1, NULL);
    return n;
}

void *UNWRAP_P(void *p) {
    if (p == NULL)
        err(1, NULL);
    return p;
}

void *malloc_safe(size_t size) {
    return UNWRAP_P(malloc(size));
}
