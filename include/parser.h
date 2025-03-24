#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include <string.h>

/*AST Node Types*/
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
    AST_IDENTIFIER,
    AST_FACTORIAL
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

static const char* ast_type_to_string(ASTType type);


int isKeyword(const Token t, const char *kw);
int isOperator(const Token t, const char *op);
int isDelimiter(const Token t, const char *delim);
int get_precedence(const char* op);

static const char* TYPES[] = {"int", "uint", "string", "float", "char"};
static const char* KEYWORDS[] = {"while", "repeat", "for"};
static const char* ASSIGNMENTS[] = {"=", "+=", "-=", "/=", "*=", "%=", "&=", "|=", "<<=", ">>="};

static const char* UNARY[] = { "++", "--", "~", "!", };
static const char* FACTOR[] = { "*", "/", "%"};
static const char* ADD_SUB[] = { "+", "-"};
static const char* BITSHIFTS[] = { "<<", ">>"};
static const char* COMPARISON[] = { "<=", "=>", "<", ">"};
static const char* EQUALITY[] = { "!=", "==" };
static const char* BITAND[] = { "&" };
static const char* BITXOR[] = { "^" };
static const char* BITOR[] = { "|" };
static const char* LOGAND[] = { "&&" };
static const char* LOGOR[] = { "||" };

int str_is_in(const char* str, const char* arr[], int num);


typedef struct _Parser {
    Token* tokens;
    Token current;
    int position;
    int scope_level;
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
ASTNode* parse_factorial(Parser* parser);

// Define the enum and its string conversion function
// might be good to add a custom
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

static const char* error_to_string(ParserErrorType e) {
    switch (e) {
        #define X(name) case name: return #name;
        ERRORS
        #undef X
    }
    return "Unknown";
}


#define PARSE_ERROR(parser, error_type, message, ...)\
    fprintf(stderr, "\n[PARSER ERROR] Error near token '%s' on line %d; \n\t Error: %s " message "\n", parser->current.lexeme, parser->current.line, error_to_string(error_type), ##__VA_ARGS__);\

#define PARSE_ERROR_S(parser, error_type, ...)\
    fprintf(stderr, "\n[PARSER ERROR] Error near token '%s' on line %d; \n\t Error: %s\n", parser->current.lexeme, parser->current.line, error_to_string(error_type), ##__VA_ARGS__);\


//#define DEBUG
#ifdef DEBUG
#define PARSE_INFO(message, ...) fprintf(stdout, "[PARSER DEBUG] " message , ##__VA_ARGS__);
#else
#define PARSE_INFO(message, ...)
#endif

#define CONTAINS_STR(val, str)\
    str_is_in(str, val, sizeof(val)/sizeof(*val))

#endif
