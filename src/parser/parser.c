/* parser.c */
#include "lexer.h"
#include "tokens.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int str_is_in(const char* str, const char* arr[], int num) {
    for (int i = 0; i < num; i++) {
        if (!strcmp(str, arr[i])) {
            return 1;
        }
    }
    return 0;
}

/* get_precedence used by parse_expression */
// idk why but this only works when inverted
int get_precedence(const char* op) {
    int result = 0;
    if (str_is_in(op, UNARY, sizeof(UNARY)/sizeof(char*))) result = 1;
    if (str_is_in(op, FACTOR, sizeof(FACTOR)/sizeof(char*))) result = 2;
    if (str_is_in(op, ADD_SUB, sizeof(ADD_SUB)/sizeof(char*))) result = 3;
    if (str_is_in(op, BITSHIFTS, sizeof(BITSHIFTS)/sizeof(char*))) result = 4;
    if (str_is_in(op, COMPARISON, sizeof(COMPARISON)/sizeof(char*))) result = 5;
    if (str_is_in(op, EQUALITY, sizeof(EQUALITY)/sizeof(char*))) result = 6;
    if (str_is_in(op, BITAND, sizeof(BITAND)/sizeof(char*))) result = 7;
    if (str_is_in(op, BITXOR, sizeof(BITXOR)/sizeof(char*))) result = 8;
    if (str_is_in(op, BITOR, sizeof(BITOR)/sizeof(char*))) result = 9;
    if (str_is_in(op, LOGAND, sizeof(LOGAND)/sizeof(char*))) result = 10;
    if (str_is_in(op, LOGOR, sizeof(LOGOR)/sizeof(char*))) result = 11;
    return 11 - result;
}


/*
Some Helper Functions
-*/
int isKeyword(const Token t, const char *kw) {
    return (t.type == TOKEN_KEYWORD && strcmp(t.lexeme, kw) == 0);
}
int isOperator(const Token t, const char *op) {
    return (t.type == TOKEN_OPERATOR && strcmp(t.lexeme, op) == 0);
}
int isDelimiter(const Token t, const char *delim) {
    return (t.type == TOKEN_DELIMITER && strcmp(t.lexeme, delim) == 0);
}

void advance(Parser* parser) {
    parser->current = parser->tokens[parser->position];
    if (parser->current.type != TOKEN_EOF) {
        parser->position++;
    }
    PARSE_INFO("advance() -> token='%s' (type=%d)\n", parser->current.lexeme, parser->current.type);
}

/* Create AST node */
ASTNode* create_node(ASTType type, const Token* tk) {
    const char* name = ast_type_to_string(type);
    PARSE_INFO("create_node(type=%s, token='%s')\n", name, tk->lexeme);
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if(!node){
        fprintf(stderr,"No memory for ASTNode\n");
        exit(1);
    }
    node->type=type;
    node->current= *tk;
    node->left=node->right=node->next=node->body=NULL;
    return node;
}
ASTNode* create_node_simple(ASTType type) {
    Token empty; memset(&empty,0,sizeof(Token));
    return create_node(type,&empty);
}

const char* ast_type_to_string(ASTType type) {
    switch (type) {
        case AST_PROGRAM:
            return "AST_PROGRAM";
        case AST_BLOCK:
            return "AST_BLOCK";
        case AST_VARDECL:
            return "AST_VARDECL";
        case AST_VARDECLTYPE:
            return "AST_VARDECLTYPE";
        case AST_ASSIGN:
            return "AST_ASSIGN";
        case AST_IF:
            return "AST_IF";
        case AST_WHILE:
            return "AST_WHILE";
        case AST_REPEAT:
            return "AST_REPEAT";
        case AST_PRINT:
            return "AST_PRINT";
        case AST_FUNCTION_CALL:
            return "AST_FUNCTION_CALL";
        case AST_FUNCTION_ARGS:
            return "AST_FUNCTION_ARGS";
        case AST_BINOP:
            return "AST_BINOP";
        case AST_UNARYOP:
            return "AST_UNARYOP";
        case AST_LITERAL:
            return "AST_LITERAL";
        case AST_IDENTIFIER:
            return "AST_IDENTIFIER";
        default:
            return "UNKNOWN AST";
    }
    return "UNKNOWN_AST";
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
            printf("Assignment: %s\n", node->current.lexeme); break;
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
        print_ast_recursive(node->body, level+1);
    }
    if(node->next){
        print_ast_recursive(node->next,level);
    }
}
void print_ast(ASTNode* root){
    print_ast_recursive(root,0);
}

/* Build Token Table */
Token* make_table(char* in){
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
            //free(table);
            //fprintf(stderr, "Failed to create token table\n");
            //return NULL;
        }
        last = current.type;
        table[lexemmes++] = current;
    } while (current.type != TOKEN_EOF);

    Token* new_table = malloc(sizeof(Token) * (lexemmes + 1));
    memcpy(new_table, table, sizeof(Token) * (lexemmes + 1));
    free(table);
    PARSE_INFO("make_table -> generated %d tokens\n",lexemmes);
    return new_table;
}

/* parse_factorial if keyword is "factorial(...)" */
ASTNode* parse_factorial(Parser* parser){
    ASTNode* fact = create_node(AST_FACTORIAL,&parser->current);
    advance(parser); // skip "factorial"
    if(!isDelimiter(parser->current, "(")) {
        PARSE_ERROR(parser, EXPECTED, "'(' after factorial");
    }
    advance(parser);
    ASTNode* expr=parse_expression(parser, 0);
    if(!isDelimiter(parser->current,")")){
        PARSE_ERROR(parser, EXPECTED, "'(' after expression");
    }
    advance(parser);
    fact->left = expr;
    return fact;
}

/* parse_primary: numbers, strings, ids, parentheses, function calls, factorial. */
ASTNode* parse_primary(Parser* parser) {
    PARSE_INFO("parse_primary -> current='%s'\n", parser->current.lexeme);

    // check factorial
    if(parser->current.type==TOKEN_KEYWORD && !strcmp(parser->current.lexeme, "factorial")){
        return parse_factorial(parser);
    }

    if (isDelimiter(parser->current, "(")) {
        PARSE_INFO("parse_primary -> '(' found\n");
        advance(parser);
        ASTNode* expr = parse_expression(parser, 0);
        if (!isDelimiter(parser->current, ")")) {
            PARSE_ERROR(parser, EXPECTED_DELIMITER, "')' to match '('")
        }
        advance(parser); // consume ")"
        return expr;
    }
    if (parser->current.type == TOKEN_NUMBER) {
        PARSE_INFO("parse_primary -> NUMBER '%s'\n", parser->current.lexeme);
        ASTNode* node = create_node(AST_LITERAL, &parser->current);
        advance(parser);
        return node;
    }
    if (parser->current.type == TOKEN_STRING) {
        PARSE_INFO("parse_primary -> STRING '%s'\n", parser->current.lexeme);
        ASTNode* node = create_node(AST_LITERAL, &parser->current);
        advance(parser);
        return node;
    }
    if (parser->current.type == TOKEN_IDENTIFIER) {
        // Could be function call or plain identifier
        Token id = parser->current;
        advance(parser);
        if (id.type == parser->current.type) {
            PARSE_ERROR(parser, UNEXPECTED, "Back to back identifiers %s", id.lexeme);
        }
        if (isDelimiter(parser->current, "(")) {
            PARSE_INFO("parse_primary -> function call\n");
            ASTNode* callNode = create_node(AST_FUNCTION_CALL, &id);
            advance(parser); // consume "("
            ASTNode* argHead = NULL;
            ASTNode* argTail = NULL;
            while (!isDelimiter(parser->current, ")") && parser->current.type != TOKEN_EOF) {
                ASTNode* arg = parse_expression(parser, 0);

                if (arg == NULL) {
                    PARSE_ERROR(parser, EXPECTED, "expression after identifier");
                    advance(parser);
                    continue;
                }
                if (!argHead) argHead = arg;
                else          argTail->next = arg;
                argTail = arg;

                if (isDelimiter(parser->current, ",")) {
                    advance(parser); // consume comma
                } else {
                    break;
                }
            }
            if (!isDelimiter(parser->current, ")")) {
                PARSE_ERROR(parser, EXPECTED_DELIMITER, "')' in function call")
            }
            advance(parser);
            callNode->body = argHead;
            return callNode;
        } else {
            // plain identifier
            PARSE_INFO("parse_primary->identifier '%s'\n", id.lexeme);
            ASTNode* idNode=create_node(AST_IDENTIFIER,&id);
            return idNode;
        }
    }

    // Error if we get here
    PARSE_ERROR_S(parser, UNEXPECTED)
    return NULL;
}

// Parse expression based on operator precedence
ASTNode* parse_expression(Parser* parser, int min_precedence) {
    PARSE_INFO("parse_expression -> start, current='%s'\n", parser->current.lexeme);
    ASTNode* left = parse_primary(parser);

    while (parser->current.type == TOKEN_OPERATOR) {
        Token op = parser->current; 
        int prec = get_precedence(parser->current.lexeme);
        if (prec < min_precedence) {
            break;
        }
        advance(parser);
        ASTNode* right = parse_expression(parser, prec + 1);
        ASTNode* binNode = create_node(AST_BINOP, &op);
        binNode->left = left;
        binNode->right = right;
        left = binNode;
    }
    PARSE_INFO("parse_expression -> end %s\n", parser->current.lexeme);
    return left;
}

/*
   Statement Parsing
   */

// parse_block: "{" { ... } "}"
ASTNode* parse_block(Parser* parser) {
    PARSE_INFO("parse_block -> start, current='%s'\n", parser->current.lexeme);
    if (!isDelimiter(parser->current, "{")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, "{ in block");
    }
    Token braceTok = parser->current;
    advance(parser); // consume "{"

    ASTNode* blockNode = create_node(AST_BLOCK, &braceTok);
    ASTNode* head = NULL;
    ASTNode* tail = NULL;
    while (!isDelimiter(parser->current, "}") && parser->current.type != TOKEN_EOF) {
        PARSE_INFO("parse_block -> reading stmt, current='%s'\n", parser->current.lexeme);
        ASTNode* stmt = NULL;

        if (CONTAINS_STR(TYPES, parser->current.lexeme)) {
            stmt = parse_declaration(parser);
        } else {
            stmt = parse_statement(parser);
        }
        if (stmt == NULL) {
            PARSE_ERROR(parser, EXPECTED, "statement or declaration, found invalid");
            advance(parser);
            continue;
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

    if (!isDelimiter(parser->current, "}")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, "'}' to match '{' in a block");
    }
    advance(parser); // consume "}"

    // If you get a ;, just consume it to ignore it
    if (isDelimiter(parser->current, ";")) {
        advance(parser); // consume ";"
    }
    parser->scope_level--;
    blockNode->body = head;
    PARSE_INFO("parse_block -> end\n");
    return blockNode;
}

ASTNode* parse_if_statement(Parser* parser) {
    PARSE_INFO("parse_if_statement -> start, current='%s'\n", parser->current.lexeme);
    Token ifTok = parser->current; 
    advance(parser); // consume "if"

    if (!isDelimiter(parser->current, "(")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, "( after if");
    }
    advance(parser); // consume "("

    ASTNode* cond = parse_expression(parser, 0);

    if (!isDelimiter(parser->current, ")")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, ") after if condition");
    }
    advance(parser); // consume ")"

    ASTNode* ifNode = create_node(AST_IF, &ifTok);
    ASTNode* thenBlock = parse_block(parser);
    ifNode->left  = cond;
    ifNode->right = thenBlock;

    if (isKeyword(parser->current, "else")) {
        PARSE_INFO("parse_if_statement -> found 'else'\n");
        advance(parser); // consume "else"
        ASTNode* elseBlock = parse_block(parser);
        ifNode->body = elseBlock;
    }
    PARSE_INFO("parse_if_statement->end\n");
    return ifNode;
}

ASTNode* parse_while_statement(Parser* parser) {
    PARSE_INFO("parse_while_statement -> start\n");
    Token whTok = parser->current;
    advance(parser); // consume "while"

    if (!isDelimiter(parser->current, "(")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, "( after while");
    }
    advance(parser); // consume "("

    ASTNode* cond = parse_expression(parser, 0);

    if (!isDelimiter(parser->current, ")")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, ") after while condition");
    }
    advance(parser); // consume ")"

    ASTNode* whNode = create_node(AST_WHILE, &whTok);
    ASTNode* bodyBlock = parse_block(parser);
    whNode->left  = cond;
    whNode->right = bodyBlock;
    PARSE_INFO("parse_while_statement -> end\n");
    return whNode;
}

ASTNode* parse_repeat_until(Parser* parser) {
    PARSE_INFO("parse_repeat_until -> start\n");
    Token rptTok = parser->current;
    advance(parser); // consume "repeat"

    ASTNode* blockNode = parse_block(parser);

    if (!isKeyword(parser->current, "until")) {
        PARSE_ERROR(parser, EXPECTED, "'until' after 'repeat'");
    }
    advance(parser); // consume "until"

    if (!isDelimiter(parser->current, "(")) {
        PARSE_ERROR(parser, EXPECTED, "'(' after 'until'");
    }
    advance(parser); // consume "("

    ASTNode* cond = parse_expression(parser, 0);

    if (!isDelimiter(parser->current, ")")) {
        PARSE_ERROR(parser, EXPECTED, "')' after 'until' condition")
    }
    advance(parser); // consume ")"

    ASTNode* rptNode = create_node(AST_REPEAT, &rptTok);
    rptNode->left  = blockNode;
    rptNode->right = cond;
    PARSE_INFO("parse_repeat_until -> end\n");
    return rptNode;
}

ASTNode* parse_print_statement(Parser* parser) {
    PARSE_INFO("parse_print_statement -> start\n");
    Token prTok = parser->current;
    advance(parser); // consume "print"

    ASTNode* prNode = create_node(AST_PRINT, &prTok);
    ASTNode* expr = parse_expression(parser, 0);
    prNode->right = expr;

    if (!isDelimiter(parser->current, ";")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, "; after print");
    }
    advance(parser); // consume ";"
    PARSE_INFO("parse_print_statement -> end\n");
    return prNode;
}

ASTNode* parse_statement(Parser* parser) {
    PARSE_INFO("parse_statement -> start, current='%s'\n", parser->current.lexeme);

    if (isKeyword(parser->current, "if"))      return parse_if_statement(parser);
    if (isKeyword(parser->current, "while"))   return parse_while_statement(parser);
    if (isKeyword(parser->current, "repeat"))  return parse_repeat_until(parser);
    if (isKeyword(parser->current, "print"))   return parse_print_statement(parser);
    if (isDelimiter(parser->current, "{"))     return parse_block(parser);
    ASTNode* statement = NULL;
    if (parser->current.type == TOKEN_IDENTIFIER) {
        // Peek next token
        Token nextTok = parser->tokens[parser->position];
        if (CONTAINS_STR(ASSIGNMENTS, nextTok.lexeme)) {
            ASTNode* lhs = create_node(AST_IDENTIFIER, &parser->current);
            advance(parser);
            statement = parse_assignment(parser, lhs);
        } else {
            // expression statement
            ASTNode* expr = parse_expression(parser, 0);
            statement = expr;
        }
    }
    // If you get a ;, just consume it to ignore it
    if (isDelimiter(parser->current, ";")) {
        advance(parser); // consume ";"
    } else {
        //PARSE_ERROR(parser, EXPECTED_DELIMITER, "; after statement");
    }

    PARSE_INFO("parse_statement -> end (expression stmt)\n");
    return statement;
}


// parse args for function main(int argc, char* argv[])
ASTNode* parse_function_args(Parser* parser) {
    PARSE_INFO("parse_function_args -> start\n");
    ASTNode* func_args = NULL;
    if (isDelimiter(parser->current, ")")) {
        advance(parser);
        return NULL;
    }
    while (1) {
        if (!CONTAINS_STR(TYPES, parser->current.lexeme)) {
            PARSE_ERROR_S(parser, EXPECTED_TYPE_IN_FUNC_DECL);
        }
        Token type = parser->current;
        // Could be strict here to make semantics easier
        // Or lax, making parser easier but semantics harder
        ASTNode* arg_type = create_node(AST_VARDECLTYPE, &parser->current);
        if (func_args == NULL) {
            func_args = arg_type;
        }
        advance(parser);
        if (parser->current.type != TOKEN_IDENTIFIER) {
            PARSE_ERROR(parser, EXPECTED_IDENTIFIER, "with type %s", type.lexeme);
        }
        arg_type->body = create_node(AST_VARDECL, &parser->current);
        if (arg_type != func_args)
            func_args->next = arg_type;
        advance(parser);
        if (!isDelimiter(parser->current, ",")) {
            break;
        }
        advance(parser);
    }
    if (!isDelimiter(parser->current, ")")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, ") after function args")
    }
    advance(parser);

    // Now get the body of the function
    if (!isDelimiter(parser->current, "{")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, "{ after function declaration")
    }

    return func_args;
}

// parse_declaration: "int x"
ASTNode* parse_declaration(Parser* parser) {
    PARSE_INFO("parse_declaration -> start\n");
    if (parser->current.type != TOKEN_KEYWORD && !CONTAINS_STR(TYPES, parser->current.lexeme)) {
        PARSE_ERROR(parser, EXPECTED_TYPE, "in declaration");
    }
    ASTNode* declNode = create_node(AST_VARDECLTYPE, &parser->current);
    advance(parser);

    if (parser->current.type != TOKEN_IDENTIFIER) {
        PARSE_ERROR(parser, EXPECTED_IDENTIFIER, "in declaration after type %s", declNode->current.lexeme);
    }
    ASTNode* lhs = create_node(AST_VARDECL, &parser->current);
    advance(parser);

    // Parse assignment to an identifier after a declaration
    if (CONTAINS_STR(ASSIGNMENTS, parser->current.lexeme)) {
        PARSE_INFO("parse_declaration assignment -> found '%s'\n", parser->current.lexeme);
        ASTNode* assignmnent = parse_assignment(parser, lhs);
        declNode->next = assignmnent;
        if (isDelimiter(parser->current, ";")) {
            advance(parser); // consume ';'
        } else if (isDelimiter(parser->current, "}")) {
        }
    } else if (isDelimiter(parser->current, "(")) { // Function declaration, parse the 
        PARSE_INFO("parse_decl function %s\n", parser->current.lexeme);
        advance(parser);
        ASTNode* func_args = parse_function_args(parser);
        lhs->right = func_args;
        lhs->body = parse_block(parser);
        declNode->body = lhs;
    } else if (isDelimiter(parser->current, ";")) {
        declNode->body = lhs;
        advance(parser);
    }
    PARSE_INFO("parse_declaration -> end\n");
    return declNode;
}

// parse_assignment: "x = expr;"
ASTNode* parse_assignment(Parser* parser, ASTNode* lhs) {
    PARSE_INFO("parse_assignment -> start\n");
    if (!CONTAINS_STR(ASSIGNMENTS, parser->current.lexeme)) {
        PARSE_ERROR(parser, EXPECTED_ASSIGNMENT, "}");
    }
    ASTNode* assignNode = create_node(AST_ASSIGN, &parser->current);
    advance(parser); // consume operator
    ASTNode* rhs = parse_expression(parser, 0);
    assignNode->right = rhs;
    assignNode->left = lhs;

    if (!isDelimiter(parser->current, ";")) {
        PARSE_ERROR(parser, EXPECTED_DELIMITER, "; after assignment");
    }
    advance(parser); // consume ";"
    PARSE_INFO("parse_assignment -> end\n");
    return assignNode;
}

/*
   6) parse_program
   */
ASTNode* parse_program(Parser* parser) {
    PARSE_INFO("parse_program -> start\n");
    Token dummy;
    memset(&dummy, 0, sizeof(Token));
    ASTNode* programNode = create_node(AST_PROGRAM, &dummy);
    ASTNode* head = NULL;
    ASTNode* tail = NULL;

    while (parser->current.type != TOKEN_EOF) {
        PARSE_INFO("parse_program -> reading top-level, current='%s'\n", parser->current.lexeme);
        ASTNode* node = NULL;
        if (str_is_in(parser->current.lexeme, TYPES, sizeof(TYPES)/sizeof(char*))) {
            node = parse_declaration(parser);
        } else if (str_is_in(parser->current.lexeme, KEYWORDS, sizeof(KEYWORDS)/sizeof(char*))){
            node = parse_statement(parser);
        } else {
            node = parse_statement(parser);
        }
        if (!head) { head = node; } else { tail->next = node; }
        tail = node;
        while (tail->next) { tail = tail->next; }
    }

    programNode->body = head;
    PARSE_INFO("parse_program->end\n");
    return programNode;
}


Parser new_parser(char *input) {
    Parser parser = {};
    memset(&parser, 0, sizeof(Parser));
    parser.tokens = make_table(input);
    if (parser.tokens == NULL) {
        fprintf(stderr, "Failed to create token table\n");
        exit(0);
    }
    return parser;
}

int parse(Parser* parser) {
    PARSE_INFO("parse -> start\n");
    advance(parser); 

    parser->root = parse_program(parser);
    PARSE_INFO("\n--- PARSED AST ---\n");
    print_ast(parser->root);

    PARSE_INFO("parse -> end\n");
    return 0;
}

void free_parser(Parser parser) {
    free(parser.tokens);
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
        PARSE_INFO("Analyzing input:\n%s\n\n", input);
        Parser parser = new_parser(input);
        parse(&parser);
        free_parser(parser);
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
        "while (0 == 1) { x += 1; }",
        "int main(){ int x = 0; }",
        "int main(){ string x = \"hey\"; }",
        "if ((0 + 1 == 1) && (1 + 0 == 1)) { print 1; }",
        //"float f = 3.14;", // FLOATS DO NOT WORK
        "int foo(int a, float b){ b = b + a; }",
        "foo(2, 15);",
        };
        size_t NUM_TESTS = sizeof(testInputs) / sizeof(testInputs[0]);

        for (size_t i = 0; i < NUM_TESTS; i++) {
            PARSE_INFO("\n=== Test #%zu ===\nSource: %s\n", i+1, testInputs[i]);
            Parser parser = new_parser((char*)testInputs[i]);
#ifdef DEBUG
            print_token_stream(testInputs[i]);
#endif
            parse(&parser);
            free_parser(parser);
        }
    }
    return 0;
}
