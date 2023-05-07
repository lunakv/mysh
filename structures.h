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

struct PipeSegment {
    Command *command;
    SLIST_ENTRY(PipeSegment) next;
};
typedef struct PipeSegment PipeSegment;
typedef SLIST_HEAD(Pipe, PipeSegment) Pipe;

#endif //MYSH_STRUCTURES_H
