#ifndef LEXER_H
#define LEXER_H 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "tokens.h"

#define NUM_KEYWORDS 14
#define NUM_OPERATORS 32
#define NUM_DELIMITERS 8

#define MAXBUFLEN 1000000
#define MAX_TABLE_SIZE 100000


static char* keywords[] = {
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
    "print",
    "return",
    "fn",
};

static char* operators[] = {
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

static char delimiters[] = {
    '}',
    '{',
    ']',
    '[',
    ')',
    '(',
    ',',
    ';',
};

int is_keyword(char* str, int len);
int is_operator(char* str, int len);
int is_delimiter(char c);

void print_error(ErrorType error, int line, const char *lexeme);
void print_token(Token token);

Token get_next_token(const char *input, int *pos, TokenType last_token_type);
void print_token_stream(const char* input);
#endif
