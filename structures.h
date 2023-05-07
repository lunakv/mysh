#ifndef MYSH_STRUCTURES_H
#define MYSH_STRUCTURES_H

#include <sys/queue.h>

struct CommandArgument {
    char *text;

    SLIST_ENTRY(CommandArgument) next;
};
typedef struct CommandArgument CommandArgument;
typedef SLIST_HEAD(Command, CommandArgument) Command;

#endif //MYSH_STRUCTURES_H
