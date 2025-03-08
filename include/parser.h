#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"

/*AST Node Types-*/
typedef enum {
    AST_PROGRAM,
    AST_BLOCK,
    AST_VARDECL,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_REPEAT,
    AST_PRINT,
    AST_FUNCTION_CALL,
    AST_FUNCTION_ARGS,
    AST_BINOP,
    AST_UNARYOP,
    AST_LITERAL,
    AST_IDENTIFIER
} ASTType;

/*AST Node Structure*/
typedef struct ASTNode {
    ASTType           type;    // e.g. AST_IF, AST_WHILE, etc.
    Token             current; // The token associated with this node
    struct ASTNode   *left;    // Commonly used for subexpressions or conditions
    struct ASTNode   *right;   // Commonly used for subexpressions or "then" block
    struct ASTNode   *next;    // For linking statements in a block
    struct ASTNode   *body;    // For the statements inside a block, or function-call arguments
} ASTNode;

/*
Function Prototypes
*/

// Create a node with the given AST type and (optional) associated token
ASTNode* create_node(ASTType type, const Token* tk);

// Build a token array from raw input
Token* make_table(char* input);

// Parse the array of tokens into an AST
void parse_table(Token* table);

// Print (for debugging) the AST
void print_ast(ASTNode* root);

#endif
