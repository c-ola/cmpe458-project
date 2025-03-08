/* parser.c */
#include "lexer.h"
#include "tokens.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int isKeyword(const Token t, const char *kw);
static int isOperator(const Token t, const char *op);
static int isDelimiter(const Token t, const char *delim);

/* Forward declarations to stop code from yelling at me */
static ASTNode* parse_program(void);
static ASTNode* parse_expression(int min_precedence);
static ASTNode* parse_declaration(void);
static ASTNode* parse_assignment(void);
static ASTNode* parse_block(void);
static ASTNode*parse_if_statement(void);
static ASTNode* parse_while_statement(void);
static ASTNode* parse_repeat_until(void);
static ASTNode* parse_print_statement(void);
static ASTNode* parse_statement(void);
static ASTNode* parse_primary(void);
static ASTNode* parse_function_args(void);


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

static int str_is_in(const char* str, const char* arr[], int num) {
    for (int i = 0; i < num; i++) {
        if (!strcmp(str, arr[i])) {
            return 1;
        }
    }
    return 0;
}

static int get_precedence(const char* op) {
    int result = 0;
    if (str_is_in(op, UNARY, sizeof(UNARY)/sizeof(char*)))
        result = 1;
    if (str_is_in(op, FACTOR, sizeof(FACTOR)/sizeof(char*)))
        result = 2;
    if (str_is_in(op, ADD_SUB, sizeof(ADD_SUB)/sizeof(char*)))
        result = 3;
    if (str_is_in(op, BITSHIFTS, sizeof(BITSHIFTS)/sizeof(char*)))
        result = 4;
    if (str_is_in(op, COMPARISON, sizeof(COMPARISON)/sizeof(char*)))
        result = 5;
    if (str_is_in(op, EQUALITY, sizeof(EQUALITY)/sizeof(char*)))
        result = 6;
    if (str_is_in(op, BITAND, sizeof(BITAND)/sizeof(char*)))
        result = 7;
    if (str_is_in(op, BITXOR, sizeof(BITXOR)/sizeof(char*)))
        result = 8;
    if (str_is_in(op, BITOR, sizeof(BITOR)/sizeof(char*)))
        result = 9;
    if (str_is_in(op, LOGAND, sizeof(LOGAND)/sizeof(char*)))
        result = 10;
    if (str_is_in(op, LOGOR, sizeof(LOGOR)/sizeof(char*)))
        result = 11;
    return 11 - result;
}

/*
  Global parser state
*/
static Token* g_tokens  = NULL; 
static int    g_position= 0;
static Token  g_current;

#define PARSE_ERROR(message, ...)\
    fprintf(stderr, "\n[PARSER ERROR] Parse Error near token '%s' on line %d; \n\t Error: " message "\n", g_current.lexeme, g_current.line, ##__VA_ARGS__);\
    exit(0);\

#define PARSE_INFO(message, ...)\
    fprintf(stdout, "[PARSER DEBUG] " message , ##__VA_ARGS__);\

#define CONTAINS_STR(val, str)\
    str_is_in(str, val, sizeof(val)/sizeof(*val))

/*
Some Helper Functions
-*/
static int isKeyword(const Token t, const char *kw) {
    return (t.type == TOKEN_KEYWORD && strcmp(t.lexeme, kw) == 0);
}
static int isOperator(const Token t, const char *op) {
    return (t.type == TOKEN_OPERATOR && strcmp(t.lexeme, op) == 0);
}
static int isDelimiter(const Token t, const char *delim) {
    return (t.type == TOKEN_DELIMITER && strcmp(t.lexeme, delim) == 0);
}

static void advance() {
    g_current = g_tokens[g_position];
    if (g_current.type != TOKEN_EOF) {
        g_position++;
    }
    PARSE_INFO("advance() -> token='%s' (type=%d)\n",
           g_current.lexeme, g_current.type);
}

/*
 AST Node Creation & Print
*/
ASTNode* create_node(ASTType type, const Token* tk) {
    PARSE_INFO("create_node(type=%d, token='%s')\n",
           type, tk->lexeme);

    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Out of memory creating ASTNode\n");
        exit(1);
    }
    node->type    = type;
    node->current = *tk; 
    node->left    = NULL;
    node->right   = NULL;
    node->next    = NULL;
    node->body    = NULL;
    return node;
}

ASTNode* create_node_simple(ASTType type) {
    Token empty;
    memset(&empty, 0, sizeof(Token));
    return create_node(type, &empty);
}

static void print_ast_recursive(ASTNode *node, int level) {
    if (!node) return;
    for (int i = 0; i < level; i++) printf("  ");

    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n"); break;
        case AST_BLOCK:
            printf("Block\n"); break;
        case AST_VARDECL:
            printf("VarDecl: %s\n", node->current.lexeme); break;
        case AST_VARDECLTYPE:
            printf("VarDeclType: %s\n", node->current.lexeme); break;
        case AST_ASSIGN:
            printf("Assignment to: %s\n", node->current.lexeme); break;
        case AST_IF:
            printf("If\n"); break;
        case AST_WHILE:
            printf("While\n"); break;
        case AST_REPEAT:
            printf("RepeatUntil\n"); break;
        case AST_PRINT:
            printf("Print\n"); break;
        case AST_FUNCTION_CALL:
            printf("FunctionCall: %s\n", node->current.lexeme); break;
        case AST_FUNCTION_ARGS:
            printf("Function Args: %s\n", node->current.lexeme); break;
        case AST_BINOP:
            printf("BinOp: %s\n", node->current.lexeme); break;
        case AST_UNARYOP:
            printf("UnaryOp: %s\n", node->current.lexeme); break;
        case AST_LITERAL:
            printf("Literal: %s\n", node->current.lexeme); break;
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->current.lexeme); break;
        default:
            printf("Unknown AST Node\n"); break;
    }
    if (node->left) {
        print_ast_recursive(node->left,  level+1);
    }
    if (node->right) {
        print_ast_recursive(node->right, level+1);
    }

    if (node->body) {
        for (ASTNode* s = node->body; s; s = s->next) {
            print_ast_recursive(s, level+1);
        }
    }
    if (node->next) {
        //printf("Next: ");
        //print_ast_recursive(node->next, level);
    }
}

void print_ast(ASTNode* root) {
    print_ast_recursive(root, 0);
}

/*
 Build Token Table
*/
Token* make_table(char* in) {
    PARSE_INFO("make_table()\n");
    Token* table = malloc(sizeof(Token) * MAX_TABLE_SIZE);
    TokenType last = TOKEN_NONE;
    Token current;
    int position = 0;
    int lexemmes = 0;

    do {
        current = get_next_token(in, &position, last);
        if (current.error != ERROR_NONE) {
            print_error(current.error, current.line, current.lexeme);
            free(table);
            if (table == NULL) {
                fprintf(stderr, "Failed to create token table\n");
                exit(0);
            }
            return NULL;
        }
        last = current.type;
        table[lexemmes++] = current;
    } while (current.type != TOKEN_EOF);

    Token* new_table = malloc(sizeof(Token) * (lexemmes + 1));
    memcpy(new_table, table, sizeof(Token) * (lexemmes + 1));
    free(table);

    PARSE_INFO("make_table -> generated %d tokens\n", lexemmes);
    return new_table;
}

/*
 Expression Parsing (Operator Precedence)
*/

static ASTNode* parse_primary() {
    PARSE_INFO("parse_primary -> current='%s'\n", g_current.lexeme);

    if (isDelimiter(g_current, "(")) {
        PARSE_INFO("parse_primary -> '(' found\n");
        advance();
        ASTNode* expr = parse_expression(0);
        if (!isDelimiter(g_current, ")")) {
            PARSE_ERROR("Expected ')' after expression in parse_primary");
        }
        advance(); // consume ")"
        return expr;
    }
    if (g_current.type == TOKEN_NUMBER) {
        PARSE_INFO("parse_primary -> NUMBER '%s'\n", g_current.lexeme);
        ASTNode* node = create_node(AST_LITERAL, &g_current);
        advance();
        return node;
    }
    if (g_current.type == TOKEN_STRING) {
        PARSE_INFO("parse_primary -> STRING '%s'\n", g_current.lexeme);
        ASTNode* node = create_node(AST_LITERAL, &g_current);
        advance();
        return node;
    }
    if (g_current.type == TOKEN_IDENTIFIER) {
        // Could be function call or plain identifier
        Token id = g_current;
        advance();
        if (isDelimiter(g_current, "(")) {
            PARSE_INFO("parse_primary -> function call\n");
            ASTNode* callNode = create_node(AST_FUNCTION_CALL, &id);
            advance(); // consume "("
            ASTNode* argHead = NULL;
            ASTNode* argTail = NULL;
            while (!isDelimiter(g_current, ")") &&
                   g_current.type != TOKEN_EOF)
            {
                ASTNode* arg = parse_expression(0);
                if (!argHead) argHead = arg;
                else          argTail->next = arg;
                argTail = arg;

                if (isDelimiter(g_current, ",")) {
                    advance(); // consume comma
                } else {
                    break;
                }
            }
            if (!isDelimiter(g_current, ")")) {
                PARSE_ERROR("Expected ')' after function arguments in parse_primary, found %s", g_current.lexeme);
            }
            advance();
            callNode->body = argHead;
            return callNode;
        } else {
            PARSE_INFO("parse_primary -> identifier '%s'\n", id.lexeme);
            ASTNode* idNode = create_node(AST_IDENTIFIER, &id);
            return idNode;
        }
    }

    // Error if we get here
    PARSE_ERROR("Unexpected token in parse_primary");
    return NULL;
}

// Parse expression based on operator precedence
static ASTNode* parse_expression(int min_precedence) {
    PARSE_INFO("parse_expression -> start, current='%s'\n", g_current.lexeme);
    ASTNode* left = parse_primary();

    while (g_current.type == TOKEN_OPERATOR) {
        Token op = g_current; 
        int prec = get_precedence(g_current.lexeme);
        if (prec < min_precedence) {
            break;
        }
        advance();
        ASTNode* right = parse_expression(prec + 1);
        ASTNode* binNode = create_node(AST_BINOP, &op);
        binNode->left = left;
        binNode->right = right;
        left = binNode;
    }
    PARSE_INFO("parse_expression -> end %s\n", g_current.lexeme);
    return left;
}

/*
 Statement Parsing
*/

// parse_block: "{" { ... } "}"
static ASTNode* parse_block(void) {
    PARSE_INFO("parse_block -> start, current='%s'\n", g_current.lexeme);
    if (!isDelimiter(g_current, "{")) {
        PARSE_ERROR("Expected '{' at start of block");
    }
    Token braceTok = g_current;
    advance(); // consume "{"

    ASTNode* blockNode = create_node(AST_BLOCK, &braceTok);
    ASTNode* head = NULL;
    ASTNode* tail = NULL;
    while (!isDelimiter(g_current, "}") && g_current.type != TOKEN_EOF) {
        PARSE_INFO("parse_block -> reading stmt, current='%s'\n", g_current.lexeme);
        ASTNode* stmt = NULL;

        if (CONTAINS_STR(TYPES, g_current.lexeme)) {
            stmt = parse_declaration();
        } else {
            stmt = parse_statement();
        }

        if (head == NULL) // initialize the head
            head = stmt;
        else
            tail->next = stmt;

        tail = stmt;
        while (tail->next) {
            tail = tail->next;
        }
    }

    if (!isDelimiter(g_current, "}")) {
        PARSE_ERROR("Expected '}' at end of block");
    }
    advance(); // consume "}"

    // If you get a ;, just consume it to ignore it
    if (isDelimiter(g_current, ";")) {
        advance(); // consume ";"
    }
    blockNode->body = head;
    PARSE_INFO("parse_block -> end\n");
    return blockNode;
}

static ASTNode* parse_if_statement(void) {
    PARSE_INFO("parse_if_statement -> start, current='%s'\n", g_current.lexeme);
    Token ifTok = g_current; 
    advance(); // consume "if"

    if (!isDelimiter(g_current, "(")) {
        PARSE_ERROR("Expected '(' after 'if'");
    }
    advance(); // consume "("

    ASTNode* cond = parse_expression(0);

    if (!isDelimiter(g_current, ")")) {
        PARSE_ERROR("Expected ')' after if condition");
    }
    advance(); // consume ")"

    ASTNode* ifNode = create_node(AST_IF, &ifTok);
    ASTNode* thenBlock = parse_block();
    ifNode->left  = cond;
    ifNode->right = thenBlock;

    if (isKeyword(g_current, "else")) {
        PARSE_INFO("parse_if_statement -> found 'else'\n");
        advance(); // consume "else"
        ASTNode* elseBlock = parse_block();
        ifNode->body = elseBlock;
    }
    PARSE_INFO("parse_if_statement -> end\n");
    return ifNode;
}

static ASTNode* parse_while_statement(void) {
    PARSE_INFO("parse_while_statement -> start\n");
    Token whTok = g_current;
    advance(); // consume "while"

    if (!isDelimiter(g_current, "(")) {
        PARSE_ERROR("Expected '(' after 'while'");
    }
    advance(); // consume "("

    ASTNode* cond = parse_expression(0);

    if (!isDelimiter(g_current, ")")) {
        PARSE_ERROR("Expected ')' after while condition");
    }
    advance(); // consume ")"

    ASTNode* whNode = create_node(AST_WHILE, &whTok);
    ASTNode* bodyBlock = parse_block();
    whNode->left  = cond;
    whNode->right = bodyBlock;
    PARSE_INFO("parse_while_statement -> end\n");
    return whNode;
}

static ASTNode* parse_repeat_until(void) {
    PARSE_INFO("parse_repeat_until -> start\n");
    Token rptTok = g_current;
    advance(); // consume "repeat"

    ASTNode* blockNode = parse_block();

    if (!isKeyword(g_current, "until")) {
        PARSE_ERROR("Expected 'until' after repeat block");
    }
    advance(); // consume "until"

    if (!isDelimiter(g_current, "(")) {
        PARSE_ERROR("Expected '(' after 'until'");
    }
    advance(); // consume "("

    ASTNode* cond = parse_expression(0);

    if (!isDelimiter(g_current, ")")) {
        PARSE_ERROR("Expected ')' after repeat-until condition");
    }
    advance(); // consume ")"

    ASTNode* rptNode = create_node(AST_REPEAT, &rptTok);
    rptNode->left  = blockNode;
    rptNode->right = cond;
    PARSE_INFO("parse_repeat_until -> end\n");
    return rptNode;
}

static ASTNode* parse_print_statement(void) {
    PARSE_INFO("parse_print_statement -> start\n");
    Token prTok = g_current;
    advance(); // consume "print"

    ASTNode* prNode = create_node(AST_PRINT, &prTok);
    ASTNode* expr = parse_expression(0);
    prNode->right = expr;

    if (!isDelimiter(g_current, ";")) {
        PARSE_ERROR("Expected ';' after print statement");
    }
    advance(); // consume ";"
    PARSE_INFO("parse_print_statement -> end\n");
    return prNode;
}

static ASTNode* parse_statement(void) {
    PARSE_INFO("parse_statement -> start, current='%s'\n", g_current.lexeme);

    if (isKeyword(g_current, "if"))      return parse_if_statement();
    if (isKeyword(g_current, "while"))   return parse_while_statement();
    if (isKeyword(g_current, "repeat"))  return parse_repeat_until();
    if (isKeyword(g_current, "print"))   return parse_print_statement();
    if (isDelimiter(g_current, "{"))     return parse_block();

    if (g_current.type == TOKEN_IDENTIFIER) {
        // Peek next token
        Token nextTok = g_tokens[g_position];
        if (nextTok.type == TOKEN_OPERATOR && !strcmp(nextTok.lexeme, "=")) {
            return parse_assignment();
        } else {
            // expression statement
            ASTNode* expr = parse_expression(0);
            // If you get a ;, just consume it to ignore it
            if (isDelimiter(g_current, ";")) {
                advance(); // consume ";"
            }
            return expr;
        }
    }

    // fallback: expression statement
    ASTNode* expr = parse_expression(0);
    if (!isDelimiter(g_current, ";")) {
        PARSE_ERROR("Expected ';' after expression statement");
    }
    advance(); // consume ";"
    PARSE_INFO("parse_statement -> end (expression stmt)\n");
    return expr;
}


// parse args for function main(int argc, char* argv[])
static ASTNode* parse_function_args() {
    PARSE_INFO("parse_function_args -> start\n");
    ASTNode* func_args = NULL;
    if (isDelimiter(g_current, ")")) {
        advance();
        return NULL;
    }
    while (1) {
        if (!CONTAINS_STR(TYPES, g_current.lexeme)) {
            PARSE_ERROR("Expected type in function decl, found %s", g_current.lexeme);
        }
        Token type = g_current;
        // Could be strict here to make semantics easier
        // Or lax, making parser easier but semantics harder
        ASTNode* arg_type = create_node(AST_VARDECLTYPE, &g_current);
        if (func_args == NULL) {
            func_args = arg_type;
        }
        advance();
        if (g_current.type != TOKEN_IDENTIFIER) {
            PARSE_ERROR("Expected identifier in function arg with type %s, found %s", type.lexeme, g_current.lexeme);
        }
        arg_type->body = create_node(AST_VARDECL, &g_current);
        if (arg_type != func_args)
            func_args->next = arg_type;
        advance();
        if (!isDelimiter(g_current, ",")) {
            break;
        }
        advance();
    }
    if (!isDelimiter(g_current, ")")) {
        PARSE_ERROR("Expected delimiter ')', found %s", g_current.lexeme);
    }
    advance();

    // Now get the body of the function
    if (isDelimiter(g_current, "{")) {
    } else {
        PARSE_ERROR("Expected delimiter ')', found %s", g_current.lexeme);
    }

    return func_args;
}

// parse_declaration: "int x"
static ASTNode* parse_declaration() {
    PARSE_INFO("parse_declaration -> start\n");
    for (int i = 0; i < sizeof(TYPES)/sizeof(char*); i++) {
        if ((g_current.type == TOKEN_KEYWORD && strcmp(g_current.lexeme, TYPES[i]) == 0)) {
            break;
        }
        // Could not find valid type
        if (i == sizeof(TYPES)/sizeof(char*) - 1) {
            PARSE_ERROR("Expected a type, found something else");
        }
    }
    Token typeToken = g_current;
    advance();
    ASTNode* declNode = create_node(AST_VARDECLTYPE, &typeToken);
    
    if (g_current.type != TOKEN_IDENTIFIER) {
        PARSE_ERROR("Expected identifier after type");
    }
    Token idToken = g_current;
    advance(); // consume identifier
    ASTNode* declIdNode = create_node(AST_VARDECL, &idToken);
    declNode->right = declIdNode;


    // Parse assignment to an identifier after a declaration
    if (str_is_in(g_current.lexeme, ASSIGNMENTS, sizeof(ASSIGNMENTS)/sizeof(char*))) {
        PARSE_INFO("parse_declaration assignment -> found '%s'\n", g_current.lexeme);
        advance();
        ASTNode* initExpr = parse_expression(0);
        declIdNode->right = initExpr;
        if (isDelimiter(g_current, ";")) {
            advance(); // consume ';'
        } else if (isDelimiter(g_current, "}")) {
        } else {
            PARSE_ERROR("Expected ';' or after variable declaration");
        }
    } else if (isDelimiter(g_current, "(")) { // Function declaration, parse the 
        PARSE_INFO("parse_decl function %s\n", g_current.lexeme);
        advance();
        ASTNode* func_args = parse_function_args();
        declIdNode->right = func_args;
        declIdNode->body = parse_block();
    }
    PARSE_INFO("parse_declaration -> end\n");
    return declNode;
}

// parse_assignment: "x = expr;"
static ASTNode* parse_assignment(void) {
    PARSE_INFO("parse_assignment -> start\n");
    Token ident = g_current;
    advance(); // consume identifier

    if (!isOperator(g_current, "=")) {
        PARSE_ERROR("Expected '=' in assignment");
    }
    advance(); // consume "="

    ASTNode* assignNode = create_node(AST_ASSIGN, &ident);
    ASTNode* rhs = parse_expression(0);
    assignNode->right = rhs;

    if (!isDelimiter(g_current, ";")) {
        PARSE_ERROR("Expected ';' after assignment");
    }
    advance(); // consume ";"
    PARSE_INFO("parse_assignment -> end\n");
    return assignNode;
}

/*
  6) parse_program
*/
static ASTNode* parse_program(void) {
    PARSE_INFO("parse_program -> start\n");
    Token dummy;
    memset(&dummy, 0, sizeof(Token));
    ASTNode* programNode = create_node(AST_PROGRAM, &dummy);

    ASTNode* head = NULL;
    ASTNode* tail = NULL;

    while (g_current.type != TOKEN_EOF) {
        PARSE_INFO("parse_program -> reading top-level, current='%s'\n", g_current.lexeme);
        ASTNode* node = NULL;
        if (str_is_in(g_current.lexeme, TYPES, sizeof(TYPES)/sizeof(char*))) {
            node = parse_declaration();
        } else if (str_is_in(g_current.lexeme, KEYWORDS, sizeof(KEYWORDS)/sizeof(char*))){
            node = parse_statement();
        } else {
            node = parse_statement();
        }
        if (!head) head = node;
        else       tail->next = node;
        tail = node;
        while (tail->next) {
            tail = tail->next;
        }
    }
    programNode->body = head;
    PARSE_INFO("parse_program -> end\n");
    return programNode;
}

/*
  parse_table (entry point)
*/
void parse_table(Token* table) {
    PARSE_INFO("parse_table -> start\n");
    g_tokens   = table;
    g_position = 0;
    advance(); 

    ASTNode* root = parse_program();
    PARSE_INFO("\n--- PARSED AST ---\n");
    print_ast(root);

    // Freed memory omitted for brevity
    PARSE_INFO("parse_table -> end\n");
}

#define MAXBUFLEN 1000000
int main(int argc, char* argv[]) {
    char* input = malloc(MAXBUFLEN * sizeof(char));
    if (argc == 2) {
        const char* file = argv[1]; 
        FILE *fp = fopen(file, "r");
        if (fp != NULL) {
            size_t new_len = fread(input, sizeof(char), MAXBUFLEN, fp);
            if ( ferror( fp ) != 0 ) {
                fputs("Error reading file", stderr);
            } else {
                input[new_len++] = '\0'; /* Just to be safe. */
            }
            fclose(fp);
        }
        int position = 0;
        PARSE_INFO("Analyzing input:\n%s\n\n", input);
        Token* tokens = make_table(input);
        if (tokens == NULL) {
            fprintf(stderr, "Failed to create token table\n");
            exit(0);
        }
        parse_table(tokens);
        free(tokens);
        free(input);
    } else {
        const char* testInputs[] = {
            "uint x = 5 + 2 * 3;",
            "int y = (2 + 3) * 4 - 1;",
            "int z = (10 - (3 + 2)) * 2;",
            "x = 10 - 3 - 2;",
            "x = 24 / 2 * 3;",
            "if (5 + 3 * 2 > 10) { print 1; }",
            "x = ((2 + 3) * (4 - 1)) / 5;",
            "print (1 + 2 * 3 - 4 / 2);",
            "x = (1 == x) * 4 && 1 + 45 / 5 % 6 + 1 * 2;",
            "while (0 == 1) { x += 1 };",
            "int main(int argc){ uint x = 0; float y = 5 };\nint foo(){ string y = 0; float z = 35 } ",
        };
        size_t NUM_TESTS = sizeof(testInputs) / sizeof(testInputs[0]);

        for (size_t i = 0; i < NUM_TESTS; i++) {
            PARSE_INFO("\n=== Test #%zu ===\nSource: %s\n", i+1, testInputs[i]);
            Token* tokens = make_table((char*)testInputs[i]);
            if (tokens == NULL) {
                fprintf(stderr, "Failed to create token table\n");
                exit(0);
            }
            parse_table(tokens);
            free(tokens);
        }
    }
    return 0;
}
