#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"

#define DEPTH 5
#define STATES 6

// tells the state array what to do
typedef enum {
    EMPTY,
    SHIFT,
    REDUCE,
    ACCEPT,
} Action;

typedef enum {
    AST_PROGRAM,        // Program node
    AST_VARDECL,        // Variable declaration (int x)
    AST_ASSIGN,         // Assignment (x = 5)
    AST_PRINT,          // Print statement
    AST_LITERAL,        // Literal
    AST_IDENTIFIER,     // Variable name
    // TODO: Add more node types as needed
} ASTType;

typedef enum {
    PARSE_ERROR_NONE,
    PARSE_ERROR_UNEXPECTED_TOKEN,
    PARSE_ERROR_MISSING_SEMICOLON,
    PARSE_ERROR_MISSING_IDENTIFIER,
    PARSE_ERROR_MISSING_EQUALS,
    PARSE_ERROR_INVALID_EXPRESSION
} ParseError;

// astnode
typedef struct _ASTN {
    ASTType type;
    Token current;
    struct _ASTN* left;
    struct _ASTN* right;
} ASTN;

static ASTN* create_node(ASTType type, Token* t);

#endif
