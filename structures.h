#ifndef MYSH_STRUCTURES_H
#define MYSH_STRUCTURES_H

#include <sys/queue.h>

enum ArgumentType {
    Regular,
    RedirectInput,
    RedirectOutput,
    RedirectAppend,
};
typedef enum ArgumentType ArgumentType;

struct CommandArgument {
    char *text;
    ArgumentType type;
    SLIST_ENTRY(CommandArgument) next;
};
typedef struct CommandArgument CommandArgument;
typedef SLIST_HEAD(Command, CommandArgument) Command;

#endif //MYSH_STRUCTURES_H
