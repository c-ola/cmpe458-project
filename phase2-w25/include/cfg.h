#ifndef CFG
#define CFG

#define DEPTH 5
#define STATES 6

// tells the state array what to do
typedef enum {
    EMPTY,
    SHIFT,
    REDUCE,
    ACCEPT,
} Action;

// used in the Syntax Tree
typedef enum {
    S_EOF,
    S_LITERAL,
    S_OPERATOR,
    S_KEYWORD,
    S_IDENTIFIER,
    S_DELIMITER,
    S_1,
    S_2,
    S_3,
    S_4,
    S_5,
    S_6,
    S_7,
} Syntax;

typedef struct{
    Syntax opt1[DEPTH];
    Syntax opt2[DEPTH];
} State;
#endif