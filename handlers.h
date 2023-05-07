#ifndef MYSH_HANDLERS_H
#define MYSH_HANDLERS_H

#include "structures.h"
#include <stdlib.h>

void handle_invocation(Pipe *pipeline);

int *get_child_pids();
int get_child_count();

#endif //MYSH_HANDLERS_H
