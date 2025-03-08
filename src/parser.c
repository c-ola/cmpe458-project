/* parser.c */
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int isKeyword(const Token t, const char *kw);
static int isOperator(const Token t, const char *op);
static int isDelimiter(const Token t, const char *delim);

/* Forward declarations to stop code from yelling at me */
static ASTNode* parse_program(void);
static ASTNode* parse_expression(void);
static ASTNode* parse_declaration(void);
static ASTNode* parse_assignment(void);
static ASTNode* parse_block(void);
static ASTNode* parse_if_statement(void);
static ASTNode* parse_while_statement(void);
static ASTNode* parse_repeat_until(void);
static ASTNode* parse_print_statement(void);
static ASTNode* parse_statement(void);
static ASTNode* parse_primary(void);
static ASTNode* parse_unary(void);
static ASTNode* parse_factor(void);
static ASTNode* parse_term(void);
static ASTNode* parse_comparison(void);
static ASTNode* parse_equality(void);
static void parse_error(const char* message);
static void expectKeyword(const char* kw);

/*
  Global parser state
*/
static Token* g_tokens  = NULL; 
static int    g_position= 0;
static Token  g_current;

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
    printf("[DEBUG] advance() -> token='%s' (type=%d)\n",
           g_current.lexeme, g_current.type);
}

static void parse_error(const char* message) {
    fprintf(stderr, "\n[DEBUG] Parse Error near token '%s': %s\n",
            g_current.lexeme, message);
    exit(1);
}

/*
 AST Node Creation & Print
*/
ASTNode* create_node(ASTType type, const Token* tk) {
    printf("[DEBUG] create_node(type=%d, token='%s')\n",
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
    if (node->left)  print_ast_recursive(node->left,  level+1);
    if (node->right) print_ast_recursive(node->right, level+1);

    if (node->body) {
        for (ASTNode* s = node->body; s; s = s->next) {
            print_ast_recursive(s, level+1);
        }
    }
    if (node->next) {
        print_ast_recursive(node->next, level);
    }
}

void print_ast(ASTNode* root) {
    print_ast_recursive(root, 0);
}

/*
 Build Token Table
*/
Token* make_table(char* in) {
    printf("[DEBUG] make_table()\n");
    Token* table = malloc(sizeof(Token) * MAX_TABLE_SIZE);
    TokenType last = TOKEN_NONE;
    Token current;
    int position = 0;
    int lexemmes = 0;

    do {
        current = get_next_token(in, &position, last);
        last = current.type;
        table[lexemmes++] = current;
    } while (current.type != TOKEN_EOF);

    Token* new_table = malloc(sizeof(Token) * (lexemmes + 1));
    memcpy(new_table, table, sizeof(Token) * (lexemmes + 1));
    free(table);

    printf("[DEBUG] make_table -> generated %d tokens\n", lexemmes);
    return new_table;
}

/*
 Expression Parsing (Operator Precedence)
*/

static ASTNode* parse_primary() {
    printf("[DEBUG] parse_primary -> current='%s'\n", g_current.lexeme);

    if (isDelimiter(g_current, "(")) {
        printf("[DEBUG] parse_primary -> '(' found\n");
        advance();
        ASTNode* expr = parse_expression();
        if (!isDelimiter(g_current, ")")) {
            parse_error("Expected ')' after expression in parse_primary");
        }
        advance(); // consume ")"
        return expr;
    }
    if (g_current.type == TOKEN_NUMBER) {
        printf("[DEBUG] parse_primary -> NUMBER '%s'\n", g_current.lexeme);
        ASTNode* node = create_node(AST_LITERAL, &g_current);
        advance();
        return node;
    }
    if (g_current.type == TOKEN_STRING) {
        printf("[DEBUG] parse_primary -> STRING '%s'\n", g_current.lexeme);
        ASTNode* node = create_node(AST_LITERAL, &g_current);
        advance();
        return node;
    }
    if (g_current.type == TOKEN_IDENTIFIER) {
        // Could be function call or plain identifier
        Token id = g_current;
        advance();
        if (isDelimiter(g_current, "(")) {
            printf("[DEBUG] parse_primary -> function call\n");
            ASTNode* callNode = create_node(AST_FUNCTION_CALL, &id);
            advance(); // consume "("
            ASTNode* argHead = NULL;
            ASTNode* argTail = NULL;
            while (!isDelimiter(g_current, ")") &&
                   g_current.type != TOKEN_EOF)
            {
                ASTNode* arg = parse_expression();
                if (!argHead) argHead = arg;
                else          argTail->next = arg;
                argTail = arg;

                if (isOperator(g_current, ",")) {
                    advance(); // consume comma
                } else {
                    break;
                }
            }
            if (!isDelimiter(g_current, ")")) {
                parse_error("Expected ')' after function arguments in parse_primary");
            }
            advance();
            callNode->body = argHead;
            return callNode;
        } else {
            printf("[DEBUG] parse_primary -> identifier '%s'\n", id.lexeme);
            ASTNode* idNode = create_node(AST_IDENTIFIER, &id);
            return idNode;
        }
    }
    // Error if we get here
    parse_error("Unexpected token in parse_primary");
    return NULL;
}

static ASTNode* parse_unary() {
    printf("[DEBUG] parse_unary -> current='%s'\n", g_current.lexeme);
    if (g_current.type == TOKEN_OPERATOR) {
        if (!strcmp(g_current.lexeme, "+") ||
            !strcmp(g_current.lexeme, "-") ||
            !strcmp(g_current.lexeme, "!"))
        {
            Token op = g_current;
            printf("[DEBUG] parse_unary -> unary operator '%s'\n", op.lexeme);
            advance();
            ASTNode* right = parse_unary();
            ASTNode* node = create_node(AST_UNARYOP, &op);
            node->right = right;
            return node;
        }
    }
    return parse_primary();
}

static ASTNode* parse_factor() {
    printf("[DEBUG] parse_factor -> start, current='%s'\n", g_current.lexeme);
    ASTNode* left = parse_unary();
    while (g_current.type == TOKEN_OPERATOR &&
           (!strcmp(g_current.lexeme, "*") || !strcmp(g_current.lexeme, "/")))
    {
        printf("[DEBUG] parse_factor -> saw '%s'\n", g_current.lexeme);
        Token op = g_current;
        advance();
        ASTNode* right = parse_unary();
        ASTNode* binNode = create_node(AST_BINOP, &op);
        binNode->left  = left;
        binNode->right = right;
        left = binNode;
    }
    return left;
}

static ASTNode* parse_term() {
    printf("[DEBUG] parse_term -> start, current='%s'\n", g_current.lexeme);
    ASTNode* left = parse_factor();
    while (g_current.type == TOKEN_OPERATOR &&
           (!strcmp(g_current.lexeme, "+") || !strcmp(g_current.lexeme, "-")))
    {
        printf("[DEBUG] parse_term -> saw '%s'\n", g_current.lexeme);
        Token op = g_current;
        advance();
        ASTNode* right = parse_factor();
        ASTNode* binNode = create_node(AST_BINOP, &op);
        binNode->left  = left;
        binNode->right = right;
        left = binNode;
    }
    return left;
}

static ASTNode* parse_comparison() {
    printf("[DEBUG] parse_comparison -> start, current='%s'\n", g_current.lexeme);
    ASTNode* left = parse_term();
    while (g_current.type == TOKEN_OPERATOR &&
           (!strcmp(g_current.lexeme, "<")  || !strcmp(g_current.lexeme, ">")  ||
            !strcmp(g_current.lexeme, "<=") || !strcmp(g_current.lexeme, ">=")))
    {
        printf("[DEBUG] parse_comparison -> saw '%s'\n", g_current.lexeme);
        Token op = g_current;
        advance();
        ASTNode* right = parse_term();
        ASTNode* binNode = create_node(AST_BINOP, &op);
        binNode->left  = left;
        binNode->right = right;
        left = binNode;
    }
    return left;
}

static ASTNode* parse_equality() {
    printf("[DEBUG] parse_equality -> start, current='%s'\n", g_current.lexeme);
    ASTNode* left = parse_comparison();
    while (g_current.type == TOKEN_OPERATOR &&
           (!strcmp(g_current.lexeme, "==") || !strcmp(g_current.lexeme, "!=")))
    {
        printf("[DEBUG] parse_equality -> saw '%s'\n", g_current.lexeme);
        Token op = g_current;
        advance();
        ASTNode* right = parse_comparison();
        ASTNode* binNode = create_node(AST_BINOP, &op);
        binNode->left  = left;
        binNode->right = right;
        left = binNode;
    }
    return left;
}

static ASTNode* parse_expression() {
    printf("[DEBUG] parse_expression -> start, current='%s'\n", g_current.lexeme);
    ASTNode* node = parse_equality();
    printf("[DEBUG] parse_expression -> end\n");
    return node;
}

/*
 Statement Parsing
*/

// parse_block: "{" { ... } "}"
static ASTNode* parse_block(void) {
    printf("[DEBUG] parse_block -> start, current='%s'\n", g_current.lexeme);
    if (!isDelimiter(g_current, "{")) {
        parse_error("Expected '{' at start of block");
    }
    Token braceTok = g_current;
    advance(); // consume "{"

    ASTNode* blockNode = create_node(AST_BLOCK, &braceTok);
    ASTNode* head = NULL;
    ASTNode* tail = NULL;

    while (!isDelimiter(g_current, "}") && g_current.type != TOKEN_EOF) {
        printf("[DEBUG] parse_block -> reading stmt, current='%s'\n", g_current.lexeme);
        ASTNode* stmt = NULL;

        if (isKeyword(g_current, "int")) {
            stmt = parse_declaration();
        } else {
            stmt = parse_statement();
        }

        if (!head) head = stmt;
        else       tail->next = stmt;

        tail = stmt;
        while (tail->next) {
            tail = tail->next;
        }
    }

    if (!isDelimiter(g_current, "}")) {
        parse_error("Expected '}' at end of block");
    }
    advance(); // consume "}"

    blockNode->body = head;
    printf("[DEBUG] parse_block -> end\n");
    return blockNode;
}

static ASTNode* parse_if_statement(void) {
    printf("[DEBUG] parse_if_statement -> start, current='%s'\n", g_current.lexeme);
    Token ifTok = g_current; 
    advance(); // consume "if"

    if (!isDelimiter(g_current, "(")) {
        parse_error("Expected '(' after 'if'");
    }
    advance(); // consume "("

    ASTNode* cond = parse_expression();

    if (!isDelimiter(g_current, ")")) {
        parse_error("Expected ')' after if condition");
    }
    advance(); // consume ")"

    ASTNode* ifNode = create_node(AST_IF, &ifTok);
    ASTNode* thenBlock = parse_block();
    ifNode->left  = cond;
    ifNode->right = thenBlock;

    if (isKeyword(g_current, "else")) {
        printf("[DEBUG] parse_if_statement -> found 'else'\n");
        advance(); // consume "else"
        ASTNode* elseBlock = parse_block();
        ifNode->body = elseBlock;
    }
    printf("[DEBUG] parse_if_statement -> end\n");
    return ifNode;
}

static ASTNode* parse_while_statement(void) {
    printf("[DEBUG] parse_while_statement -> start\n");
    Token whTok = g_current;
    advance(); // consume "while"

    if (!isDelimiter(g_current, "(")) {
        parse_error("Expected '(' after 'while'");
    }
    advance(); // consume "("

    ASTNode* cond = parse_expression();

    if (!isDelimiter(g_current, ")")) {
        parse_error("Expected ')' after while condition");
    }
    advance(); // consume ")"

    ASTNode* whNode = create_node(AST_WHILE, &whTok);
    ASTNode* bodyBlock = parse_block();
    whNode->left  = cond;
    whNode->right = bodyBlock;
    printf("[DEBUG] parse_while_statement -> end\n");
    return whNode;
}

static ASTNode* parse_repeat_until(void) {
    printf("[DEBUG] parse_repeat_until -> start\n");
    Token rptTok = g_current;
    advance(); // consume "repeat"

    ASTNode* blockNode = parse_block();

    if (!isKeyword(g_current, "until")) {
        parse_error("Expected 'until' after repeat block");
    }
    advance(); // consume "until"

    if (!isDelimiter(g_current, "(")) {
        parse_error("Expected '(' after 'until'");
    }
    advance(); // consume "("

    ASTNode* cond = parse_expression();

    if (!isDelimiter(g_current, ")")) {
        parse_error("Expected ')' after repeat-until condition");
    }
    advance(); // consume ")"

    ASTNode* rptNode = create_node(AST_REPEAT, &rptTok);
    rptNode->left  = blockNode;
    rptNode->right = cond;
    printf("[DEBUG] parse_repeat_until -> end\n");
    return rptNode;
}

static ASTNode* parse_print_statement(void) {
    printf("[DEBUG] parse_print_statement -> start\n");
    Token prTok = g_current;
    advance(); // consume "print"

    ASTNode* prNode = create_node(AST_PRINT, &prTok);
    ASTNode* expr = parse_expression();
    prNode->right = expr;

    if (!isDelimiter(g_current, ";")) {
        parse_error("Expected ';' after print statement");
    }
    advance(); // consume ";"
    printf("[DEBUG] parse_print_statement -> end\n");
    return prNode;
}

static ASTNode* parse_statement(void) {
    printf("[DEBUG] parse_statement -> start, current='%s'\n", g_current.lexeme);

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
            ASTNode* expr = parse_expression();
            if (!isDelimiter(g_current, ";")) {
                parse_error("Expected ';' after expression statement");
            }
            advance(); // consume ";"
            return expr;
        }
    }

    // fallback: expression statement
    ASTNode* expr = parse_expression();
    if (!isDelimiter(g_current, ";")) {
        parse_error("Expected ';' after expression statement");
    }
    advance(); // consume ";"
    printf("[DEBUG] parse_statement -> end (expression stmt)\n");
    return expr;
}

static void expectKeyword(const char* kw) {
    printf("[DEBUG] expectKeyword -> looking for '%s', current='%s'\n", kw, g_current.lexeme);
    if (!(g_current.type == TOKEN_KEYWORD && strcmp(g_current.lexeme, kw) == 0)) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Expected keyword '%s', got '%s'", kw, g_current.lexeme);
        parse_error(msg);
    }
    advance();
}

// parse_declaration: "int x [= expr];"
static ASTNode* parse_declaration(void) {
    printf("[DEBUG] parse_declaration -> start\n");
    expectKeyword("int");

    if (g_current.type != TOKEN_IDENTIFIER) {
        parse_error("Expected identifier after 'int'");
    }
    Token idToken = g_current;
    advance(); // consume identifier

    ASTNode* declNode = create_node(AST_VARDECL, &idToken);

    if (isOperator(g_current, "=")) {
        printf("[DEBUG] parse_declaration -> found '='\n");
        advance();
        ASTNode* initExpr = parse_expression();
        declNode->right = initExpr;
    }

    if (!isDelimiter(g_current, ";")) {
        parse_error("Expected ';' after variable declaration");
    }
    advance(); // consume ';'
    printf("[DEBUG] parse_declaration -> end\n");
    return declNode;
}

// parse_assignment: "x = expr;"
static ASTNode* parse_assignment(void) {
    printf("[DEBUG] parse_assignment -> start\n");
    Token ident = g_current;
    advance(); // consume identifier

    if (!isOperator(g_current, "=")) {
        parse_error("Expected '=' in assignment");
    }
    advance(); // consume "="

    ASTNode* assignNode = create_node(AST_ASSIGN, &ident);
    ASTNode* rhs = parse_expression();
    assignNode->right = rhs;

    if (!isDelimiter(g_current, ";")) {
        parse_error("Expected ';' after assignment");
    }
    advance(); // consume ";"
    printf("[DEBUG] parse_assignment -> end\n");
    return assignNode;
}

/*
  6) parse_program
*/
static ASTNode* parse_program(void) {
    printf("[DEBUG] parse_program -> start\n");
    Token dummy;
    memset(&dummy, 0, sizeof(Token));
    ASTNode* programNode = create_node(AST_PROGRAM, &dummy);

    ASTNode* head = NULL;
    ASTNode* tail = NULL;

    while (g_current.type != TOKEN_EOF) {
        printf("[DEBUG] parse_program -> reading top-level, current='%s'\n", g_current.lexeme);
        ASTNode* node = NULL;
        if (isKeyword(g_current, "int")) {
            node = parse_declaration();
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
    printf("[DEBUG] parse_program -> end\n");
    return programNode;
}

/*
  parse_table (entry point)
*/
void parse_table(Token* table) {
    printf("[DEBUG] parse_table -> start\n");
    g_tokens   = table;
    g_position = 0;
    advance(); 

    ASTNode* root = parse_program();
    printf("\n--- PARSED AST ---\n");
    print_ast(root);

    // Freed memory omitted for brevity
    printf("[DEBUG] parse_table -> end\n");
}

/*
test operator precedence
*/
int main(void) {
    const char* testInputs[] = {
        "int x = 5 + 2 * 3;",
        "int y = (2 + 3) * 4 - 1;",
        "int z = (10 - (3 + 2)) * 2;",
        "x = 10 - 3 - 2;",
        "x = 24 / 2 * 3;",
        "if (5 + 3 * 2 > 10) { print 1; }",
        "x = ((2 + 3) * (4 - 1)) / 5;",
        "print (1 + 2 * 3 - 4 / 2);"
    };
    size_t NUM_TESTS = sizeof(testInputs) / sizeof(testInputs[0]);

    for (size_t i = 0; i < NUM_TESTS; i++) {
        printf("\n=== Test #%zu ===\nSource: %s\n", i+1, testInputs[i]);

        Token* tokens = make_table((char*)testInputs[i]);
        parse_table(tokens);
        free(tokens);
    }

    return 0;
}
