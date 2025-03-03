#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../include/tokens.h"
#include "../include/cfg.h"

#define NUM_KEYWORDS 14
#define NUM_OPERATORS 32
#define NUM_DELIMITERS 8

#define MAXBUFLEN 1000000
#define MAX_TABLE_SIZE 100000


char* keywords[] = {
    "int",
    "uint",
    "float",
    "string",
    "char",
    "object",
    "if",
    "else",
    "for",
    "while",
    "loop",
    "break",
    "return",
    "fn",
};

char* operators[] = {
    "==",
    "=",
    "!",
    "!=",
    ">",
    ">=",
    "<",
    "<=",
    "+",
    "-",
    "*",
    "/",
    "%",
    ">>",
    "<<",
    "&",
    "&&",
    "|",
    "||",
    "^",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    ">>=",
    "<<=",
    "&=",
    "&&=",
    "|=",
    "||=",
    "^=",
};

char delimiters[] = {
    '}',
    '{',
    ']',
    '[',
    ')',
    '(',
    ',', // special cuz no closing bracket
    ';',
};

int is_keyword(char* str, int len);
int is_operator(char* str, int len);
int is_delimiter(char c);

void print_error(ErrorType error, int line, const char *lexeme);
void print_token(Token token);

Token get_next_token(const char *input, int *pos, TokenType last_token_type);
Token* make_table(char* in);
void parse_table(Token* table);
void init_states(State* ref);
int token_type_to_state(TokenType tt)

