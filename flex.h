// interface between the lexer and the main program
#ifndef MYSH_FLEX_H_
#define MYSH_FLEX_H_

typedef enum InputToken InputToken;
enum InputToken {
    Eof,
    Eol,
    Semicolon,
    Argument,
    ErrTokenTooLong,
    ErrMultipleSemicolons,
};

#define MAX_TOKEN_LENGTH 1024

/* Main lexer function generated by flex */
#define YY_DECL InputToken yylex (void)

YY_DECL;

/* Function that defines where flex reads input from */
void set_input(int argc, char **argv);

#endif  // MYSH_FLEX_H_
