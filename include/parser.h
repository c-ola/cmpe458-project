#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"

/*AST Node Types*/
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
    AST_IDENTIFIER,
    AST_FACTORIAL // ADDED
} ASTType;

/*AST Node Structure*/
typedef struct ASTNode {
    ASTType           type;
    Token             current;
    struct ASTNode   *left;
    struct ASTNode   *right;
    struct ASTNode   *next;
    struct ASTNode   *body;
} ASTNode;

/*Prototypes*/
ASTNode* create_node(ASTType type, const Token* tk);
Token* make_table(char* input);
void parse_table(Token* table);
void print_ast(ASTNode* root);

/*Optional helper*/
ASTNode* create_node_simple(ASTType type);

#endif
