#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include <string.h>

/*AST Node Types-*/
typedef enum {
    AST_PROGRAM,
    AST_BLOCK,
    AST_VARDECLTYPE,
    AST_VARDECLFUNC,
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

const char* ast_type_to_string(ASTType type);


int isKeyword(const Token t, const char *kw);
int isOperator(const Token t, const char *op);
int isDelimiter(const Token t, const char *delim);
int get_precedence(const char* op);

const char* TYPES[] = {"int", "uint", "string", "float", "char"};
const char* KEYWORDS[] = {"while", "repeat", "for"};
const char* ASSIGNMENTS[] = {"=", "+=", "-=", "/=", "*=", "%=", "&=", "|=", "<<=", ">>="};

const char* UNARY[] = { "++", "--", "~", "!", };
const char* FACTOR[] = { "*", "/", "%"};
const char* ADD_SUB[] = { "+", "-"};
const char* BITSHIFTS[] = { "<<", ">>"};
const char* COMPARISON[] = { "<=", "=>", "<", ">"};
const char* EQUALITY[] = { "!=", "==" };
const char* BITAND[] = { "&" };
const char* BITXOR[] = { "^" };
const char* BITOR[] = { "|" };
const char* LOGAND[] = { "&&" };
const char* LOGOR[] = { "||" };

int str_is_in(const char* str, const char* arr[], int num);


typedef struct _Parser {
    Token* tokens;
    Token current;
    int position;
    ASTNode* root;
} Parser;

Parser new_parser(char* input);
int parse(Parser* parser);
void free_parser(Parser parser);
void advance(Parser* parser);

ASTNode* parse_program(Parser* parser);
ASTNode* parse_expression(Parser* parser, int min_precedence);
ASTNode* parse_declaration(Parser* parser);
ASTNode* parse_assignment(Parser* parser, ASTNode* lhs);
ASTNode* parse_block(Parser* parser);
ASTNode* parse_if_statement(Parser* parser);
ASTNode* parse_while_statement(Parser* parser);
ASTNode* parse_repeat_until(Parser* parser);
ASTNode* parse_print_statement(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_primary(Parser* parser);
ASTNode* parse_function_args(Parser* parser);

#define DEFINE_ERROR_ENUM(name, ...)                 \
    typedef enum { __VA_ARGS__ } name;               \
    const char* name##_to_string(name e) {           \
        switch (e) {                                 \
            __VA_ARGS__                              \
        }                                           \
        return "Unknown";                            \
    }

// Helper macro to generate case labels
#define ENUM_CASE(e) case e: return #e;

// Define the enum and its string conversion function
#define ERRORS \
    X(EXPECTED)\
    X(UNEXPECTED)\
    X(EXPECTED_DELIMITER)\
    X(EXPECTED_ASSIGNMENT)\
    X(EXPECTED_TYPE_IN_FUNC_DECL)\
    X(EXPECTED_TYPE)\
    X(EXPECTED_IDENTIFIER)

typedef enum {
    #define X(name) name,
    ERRORS
    #undef X
} ParserErrorType;

const char* error_to_string(ParserErrorType e) {
    switch (e) {
        #define X(name) case name: return #name;
        ERRORS
        #undef X
    }
    return "Unknown";
}



#define PARSE_ERROR(parser, error_type, message, ...)\
    fprintf(stderr, "\n[PARSER ERROR] Error near token '%s' on line %d; \n\t Error: %s " message "\n", parser->current.lexeme, parser->current.line, error_to_string(error_type), ##__VA_ARGS__);\
    exit(0);\

#define PARSE_ERROR_S(parser, error_type, ...)\
    fprintf(stderr, "\n[PARSER ERROR] Error near token '%s' on line %d; \n\t Error: %s\n", parser->current.lexeme, parser->current.line, error_to_string(error_type), ##__VA_ARGS__);\
    exit(0);\

#define PARSE_INFO(message, ...)\
    fprintf(stdout, "[PARSER DEBUG] " message , ##__VA_ARGS__);\

#define CONTAINS_STR(val, str)\
    str_is_in(str, val, sizeof(val)/sizeof(*val))

#endif
