/* parser.c */
#include "lexer.h"
#include "tokens.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations to stop code from yelling at me */
static ASTNode* parse_program(void);
static ASTNode* parse_expression(int min_precedence);
static ASTNode* parse_declaration(void);
static ASTNode* parse_assignment(void);
static ASTNode* parse_block(void);
static ASTNode* parse_if_statement(void);
static ASTNode* parse_while_statement(void);
static ASTNode* parse_repeat_until(void);
static ASTNode* parse_print_statement(void);
static ASTNode* parse_statement(void);
static ASTNode* parse_primary(void);
static ASTNode* parse_function(void);
static ASTNode* parse_factorial(void);
static void combine_float_literal(ASTNode* intNode);

static int scope_level = 0; /* Track block nesting level */

const char* TYPES[]       = {"int", "uint", "string", "float", "char"};
const char* KEYWORDS[]    = {"while", "repeat", "for"};
const char* ASSIGNMENTS[] = {"=", "+=", "-=", "/=", "*=", "%=", "&=", "|=", "<<=", ">>="};

const char* UNARY[]     = { "++", "--", "~", "!" };
const char* FACTOR[]    = { "*", "/", "%"};
const char* ADD_SUB[]   = { "+", "-"};
const char* BITSHIFTS[] = { "<<", ">>"};
const char* COMPARISON[]= { "<=", "=>", "<", ">"};
const char* EQUALITY[]  = { "!=", "==" };
const char* BITAND[]    = { "&" };
const char* BITXOR[]    = { "^" };
const char* BITOR[]     = { "|" };
const char* LOGAND[]    = { "&&" };
const char* LOGOR[]     = { "||" };

static Token* g_tokens   = NULL;
static int    g_position = 0;
static Token  g_current;

#define PARSE_ERROR(msg, ...) \
    fprintf(stderr, "\n[PARSER ERROR] near token '%s': " msg "\n", g_current.lexeme, ##__VA_ARGS__), exit(1)

#define PARSE_INFO(msg, ...) \
    fprintf(stdout, "[PARSER DEBUG] " msg, ##__VA_ARGS__)

/* check if token matches certain conditions */
static int isKeyword(const Token t, const char *kw) {
    return (t.type == TOKEN_KEYWORD && !strcmp(t.lexeme, kw));
}
static int isOperator(const Token t, const char* op) {
    return (t.type == TOKEN_OPERATOR && !strcmp(t.lexeme, op));
}
static int isDelimiter(const Token t, const char* delim) {
    return (t.type == TOKEN_DELIMITER && !strcmp(t.lexeme, delim));
}

/* str_is_in checks if str is in arr[] of length num */
static int str_is_in(const char* str, const char* arr[], int num) {
    for (int i=0; i<num; i++) {
        if (!strcmp(str, arr[i])) {
            return 1;
        }
    }
    return 0;
}

/* get_precedence used by parse_expression */
static int get_precedence(const char* op) {
    if (str_is_in(op, UNARY, (int)(sizeof(UNARY)/sizeof(UNARY[0]))))        return 1;
    if (str_is_in(op, FACTOR, (int)(sizeof(FACTOR)/sizeof(FACTOR[0]))))     return 2;
    if (str_is_in(op, ADD_SUB, (int)(sizeof(ADD_SUB)/sizeof(ADD_SUB[0]))))  return 3;
    if (str_is_in(op, BITSHIFTS, (int)(sizeof(BITSHIFTS)/sizeof(BITSHIFTS[0])))) return 4;
    if (str_is_in(op, COMPARISON,(int)(sizeof(COMPARISON)/sizeof(COMPARISON[0])))) return 5;
    if (str_is_in(op, EQUALITY, (int)(sizeof(EQUALITY)/sizeof(EQUALITY[0]))))     return 6;
    if (str_is_in(op, BITAND, (int)(sizeof(BITAND)/sizeof(BITAND[0]))))           return 7;
    if (str_is_in(op, BITXOR, (int)(sizeof(BITXOR)/sizeof(BITXOR[0]))))           return 8;
    if (str_is_in(op, BITOR,  (int)(sizeof(BITOR)/sizeof(BITOR[0]))))            return 9;
    if (str_is_in(op, LOGAND, (int)(sizeof(LOGAND)/sizeof(LOGAND[0]))))          return 10;
    if (str_is_in(op, LOGOR,  (int)(sizeof(LOGOR)/sizeof(LOGOR[0]))))            return 11;
    return 99; /* not recognized => lowest binding */
}

/* Advance to next token */
static void advance() {
    g_current = g_tokens[g_position];
    if (g_current.type != TOKEN_EOF) {
        g_position++;
    }
    PARSE_INFO("advance() -> token='%s' (type=%d)\n", g_current.lexeme, g_current.type);
}

/* Create AST node */
ASTNode* create_node(ASTType type, const Token* tk) {
    PARSE_INFO("create_node(type=%d, token='%s')\n", (int)type, tk->lexeme);
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

/* Print AST for debugging */
static void print_ast_recursive(ASTNode* node,int level){
    if(!node)return;
    for(int i=0;i<level;i++) PARSE_INFO("  ");
    switch(node->type){
      case AST_PROGRAM:       PARSE_INFO("Program\n"); break;
      case AST_BLOCK:         PARSE_INFO("Block\n");   break;
      case AST_VARDECL:       PARSE_INFO("VarDecl: %s\n", node->current.lexeme); break;
      case AST_ASSIGN:        PARSE_INFO("Assignment to: %s\n", node->current.lexeme); break;
      case AST_IF:            PARSE_INFO("If\n"); break;
      case AST_WHILE:         PARSE_INFO("While\n"); break;
      case AST_REPEAT:        PARSE_INFO("RepeatUntil\n"); break;
      case AST_PRINT:         PARSE_INFO("Print\n"); break;
      case AST_FUNCTION_CALL: PARSE_INFO("FunctionCall: %s\n", node->current.lexeme); break;
      case AST_FUNCTION_ARGS: PARSE_INFO("Function Args: %s\n", node->current.lexeme); break;
      case AST_BINOP:         PARSE_INFO("BinOp: %s\n", node->current.lexeme); break;
      case AST_UNARYOP:       PARSE_INFO("UnaryOp: %s\n", node->current.lexeme); break;
      case AST_LITERAL:       PARSE_INFO("Literal: %s\n", node->current.lexeme); break;
      case AST_IDENTIFIER:    PARSE_INFO("Identifier: %s\n", node->current.lexeme); break;
      case AST_FACTORIAL:     PARSE_INFO("Factorial\n"); break;
      default:                PARSE_INFO("Unknown AST Node\n"); break;
    }
    if(node->left)  print_ast_recursive(node->left,level+1);
    if(node->right) print_ast_recursive(node->right,level+1);

    if(node->body){
        for(ASTNode* s=node->body;s;s=s->next){
            print_ast_recursive(s,level+1);
        }
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
    Token* table=(Token*)malloc(sizeof(Token)*MAX_TABLE_SIZE);
    TokenType last=TOKEN_NONE;
    Token cur; 
    int position=0, lexemmes=0;
    do{
        cur=get_next_token(in,&position,last);
        last=cur.type;
        table[lexemmes++]=cur;
    } while(cur.type!=TOKEN_EOF);
    Token* new_table=(Token*)malloc(sizeof(Token)*(lexemmes+1));
    memcpy(new_table,table,sizeof(Token)*(lexemmes+1));
    free(table);
    PARSE_INFO("make_table -> generated %d tokens\n",lexemmes);
    return new_table;
}

/* combine_float_literal merges e.g. "3" + "." + "14" => "3.14" in one AST literal node. */
static void combine_float_literal(ASTNode* intNode){
    // skip '.' delimiter
    advance();
    if(g_current.type!=TOKEN_NUMBER){
        PARSE_ERROR("Expected digits after '.' for float literal");
    }
    char buf[256];
    snprintf(buf,sizeof(buf),"%s.%s", intNode->current.lexeme, g_current.lexeme);
    strncpy(intNode->current.lexeme,buf,sizeof(intNode->current.lexeme)-1);
    // skip fractional digits
    advance();
}

/* parse_factorial if keyword is "factorial(...)" */
static ASTNode* parse_factorial(){
    ASTNode* fact=create_node(AST_FACTORIAL,&g_current);
    advance(); // skip "factorial"
    if(!isDelimiter(g_current,"(")){
        PARSE_ERROR("Expected '(' after factorial");
    }
    advance();
    ASTNode* expr=parse_expression(0);
    if(!isDelimiter(g_current,")")){
        PARSE_ERROR("Expected ')' after factorial expr");
    }
    advance();
    fact->left=expr;
    return fact;
}

/* parse_primary: numbers, strings, ids, parentheses, function calls, factorial, plus float logic. */
static ASTNode* parse_primary(){
    PARSE_INFO("parse_primary -> current='%s'\n", g_current.lexeme);

    // check factorial
    if(g_current.type==TOKEN_KEYWORD && !strcmp(g_current.lexeme,"factorial")){
        return parse_factorial();
    }
    // parenthesized expression
    if(isDelimiter(g_current,"(")){
        advance();
        ASTNode* expr=parse_expression(0);
        if(!isDelimiter(g_current,")")){
            PARSE_ERROR("Expected ')' after expression");
        }
        advance();
        return expr;
    }
    // numeric literal => also check next token => '.' => combine float
    if(g_current.type==TOKEN_NUMBER){
        PARSE_INFO("parse_primary -> NUMBER '%s'\n",g_current.lexeme);
        ASTNode* node=create_node(AST_LITERAL,&g_current);
        advance(); // skip the number
        // check if next is '.' delimiter => float
        if(isDelimiter(g_current,".")){
            combine_float_literal(node);
        }
        return node;
    }
    // string literal
    if(g_current.type==TOKEN_STRING){
        PARSE_INFO("parse_primary -> STRING '%s'\n", g_current.lexeme);
        ASTNode* node=create_node(AST_LITERAL,&g_current);
        advance();
        return node;
    }
    // identifier => function call or variable reference
    if(g_current.type==TOKEN_IDENTIFIER){
        Token id=g_current;
        advance();
        if(isDelimiter(g_current,"(")){
            // function call
            PARSE_INFO("parse_primary->function call\n");
            ASTNode* call=create_node(AST_FUNCTION_CALL,&id);
            advance(); // skip '('
            ASTNode* argHead=NULL; 
            ASTNode* argTail=NULL;
            while(!isDelimiter(g_current,")") && g_current.type!=TOKEN_EOF){
                ASTNode* arg=parse_expression(0);
                // link
                if(!argHead) argHead=arg; else argTail->next=arg;
                argTail=arg;
                // if we see ',' => skip
                if(isDelimiter(g_current,",")){
                    advance(); // skip comma
                } else {
                    break;
                }
            }
            if(!isDelimiter(g_current,")")){
                PARSE_ERROR("Expected ')' after function call args");
            }
            advance();
            call->body=argHead;
            return call;
        } else {
            // plain identifier
            PARSE_INFO("parse_primary->identifier '%s'\n", id.lexeme);
            ASTNode* idNode=create_node(AST_IDENTIFIER,&id);
            return idNode;
        }
    }

    PARSE_ERROR("Unexpected token in parse_primary");
    return NULL; // unreachable
}

/* Pratt parse_expression with min_precedence */
static ASTNode* parse_expression(int min_precedence){
    PARSE_INFO("parse_expression-> start, current='%s'\n",g_current.lexeme);
    ASTNode* left=parse_primary();
    while(g_current.type==TOKEN_OPERATOR){
        Token op=g_current;
        int prec=get_precedence(op.lexeme);
        // if operator precedence is < min_precedence => break
        if(prec<min_precedence) break;
        advance();
        ASTNode* right=parse_expression(prec+1);
        ASTNode* binNode=create_node(AST_BINOP,&op);
        binNode->left=left;
        binNode->right=right;
        left=binNode;
    }
    PARSE_INFO("parse_expression-> end %s\n", g_current.lexeme);
    return left;
}

/* parse_block => increment scope, parse statements or declarations, decrement scope */
static ASTNode* parse_block(){
    PARSE_INFO("parse_block->start, current='%s'\n", g_current.lexeme);
    if(!isDelimiter(g_current,"{")){
        PARSE_ERROR("Expected '{' at start of block");
    }
    Token braceTok=g_current;
    scope_level++;
    advance(); 
    ASTNode* blockNode=create_node(AST_BLOCK,&braceTok);

    ASTNode* head=NULL;
    ASTNode* tail=NULL;
    while(!isDelimiter(g_current,"}") && g_current.type!=TOKEN_EOF){
        PARSE_INFO("parse_block-> reading stmt, current='%s'\n", g_current.lexeme);
        int isAType=0;
        for(int i=0; i<(int)(sizeof(TYPES)/sizeof(TYPES[0]));i++){
            if(!strcmp(g_current.lexeme,TYPES[i]) && g_current.type==TOKEN_KEYWORD){
                isAType=1;break;
            }
        }
        ASTNode* stmt=NULL;
        if(isAType){
            stmt=parse_declaration();
        } else {
            stmt=parse_statement();
        }
        // link
        if(!head) head=stmt; else tail->next=stmt;
        tail=stmt;
        while(tail->next) tail=tail->next;
    }
    if(!isDelimiter(g_current,"}")){
        PARSE_ERROR("Expected '}' at end of block");
    }
    advance();
    scope_level--;
    blockNode->body=head;
    PARSE_INFO("parse_block->end\n");
    return blockNode;
}

/* if-statement => if(expr) block [else block] */
static ASTNode* parse_if_statement(){
    PARSE_INFO("parse_if_statement->start\n");
    Token ifTok=g_current;
    advance(); // skip "if"
    if(!isDelimiter(g_current,"(")){
        PARSE_ERROR("Expected '(' after 'if'");
    }
    advance();
    ASTNode* cond=parse_expression(0);
    if(!isDelimiter(g_current,")")){
        PARSE_ERROR("Expected ')' after if condition");
    }
    advance();
    ASTNode* ifNode=create_node(AST_IF,&ifTok);
    ASTNode* thenBlk=parse_block();
    ifNode->left=cond;
    ifNode->right=thenBlk;

    if(isKeyword(g_current,"else")){
        PARSE_INFO("parse_if_statement-> found 'else'\n");
        advance();
        ASTNode* elseBlk=parse_block();
        ifNode->body=elseBlk;
    }
    PARSE_INFO("parse_if_statement->end\n");
    return ifNode;
}

/* while-statement => while(expr) block */
static ASTNode* parse_while_statement(){
    PARSE_INFO("parse_while_statement->start\n");
    Token wTok=g_current; 
    advance();
    if(!isDelimiter(g_current,"(")){
        PARSE_ERROR("Expected '(' after while");
    }
    advance();
    ASTNode* cond=parse_expression(0);
    if(!isDelimiter(g_current,")")){
        PARSE_ERROR("Expected ')' after while cond");
    }
    advance();
    ASTNode* wNode=create_node(AST_WHILE,&wTok);
    wNode->left=cond;
    wNode->right=parse_block();
    PARSE_INFO("parse_while_statement->end\n");
    return wNode;
}

/* repeat-until => repeat{...} until(expr) */
static ASTNode* parse_repeat_until(){
    PARSE_INFO("parse_repeat_until->start\n");
    Token rpt=g_current;
    advance(); // skip "repeat"
    ASTNode* blk=parse_block();
    if(!isKeyword(g_current,"until")){
        PARSE_ERROR("Expected 'until' after repeat block");
    }
    advance();
    if(!isDelimiter(g_current,"(")){
        PARSE_ERROR("Expected '(' after 'until'");
    }
    advance();
    ASTNode* cond=parse_expression(0);
    if(!isDelimiter(g_current,")")){
        PARSE_ERROR("Expected ')' after repeat-until cond");
    }
    advance();
    ASTNode* rNode=create_node(AST_REPEAT,&rpt);
    rNode->left=blk;
    rNode->right=cond;
    PARSE_INFO("parse_repeat_until->end\n");
    return rNode;
}

/* print-statement => print expr ; */
static ASTNode* parse_print_statement(){
    PARSE_INFO("parse_print_statement->start\n");
    Token pr=g_current;
    advance(); // skip "print"
    ASTNode* prNode=create_node(AST_PRINT,&pr);
    ASTNode* ex=parse_expression(0);
    prNode->right=ex;
    if(!isDelimiter(g_current,";")){
        PARSE_ERROR("Expected ';' after print statement");
    }
    advance();
    PARSE_INFO("parse_print_statement->end\n");
    return prNode;
}

/* parse_statement => if, while, repeat, print, block, or expr statement. */
static ASTNode* parse_statement(){
    PARSE_INFO("parse_statement->start, current='%s'\n", g_current.lexeme);
    if(isKeyword(g_current,"if"))    return parse_if_statement();
    if(isKeyword(g_current,"while")) return parse_while_statement();
    if(isKeyword(g_current,"repeat"))return parse_repeat_until();
    if(isKeyword(g_current,"print")) return parse_print_statement();
    if(isDelimiter(g_current,"{"))   return parse_block();

    // handle assignment if next is an assignment operator
    if(g_current.type==TOKEN_IDENTIFIER){
        Token nxt=g_tokens[g_position];
        if(nxt.type==TOKEN_OPERATOR && str_is_in(nxt.lexeme,ASSIGNMENTS,(int)(sizeof(ASSIGNMENTS)/sizeof(ASSIGNMENTS[0])))){
            return parse_assignment();
        } else {
            // expression statement
            ASTNode* expr=parse_expression(0);
            if(!isDelimiter(g_current,";")){
                PARSE_ERROR("Expected ';' after expression statement");
            }
            advance();
            return expr;
        }
    }

    // fallback => expression statement
    ASTNode* e=parse_expression(0);
    if(!isDelimiter(g_current,";")){
        PARSE_ERROR("Expected ';' after expression statement");
    }
    advance();
    return e;
}

/* parse_function => minimal param logic ( int a, float b ) */
static ASTNode* parse_function(){
    PARSE_INFO("parse_function->start\n");
    ASTNode* head=NULL;
    ASTNode* tail=NULL;
    while(!isDelimiter(g_current,")") && g_current.type!=TOKEN_EOF){
        // check if next token is a type
        int isAType=0;
        for(int i=0;i<(int)(sizeof(TYPES)/sizeof(TYPES[0]));i++){
            if(!strcmp(g_current.lexeme,TYPES[i]) && g_current.type==TOKEN_KEYWORD){
                isAType=1;break;
            }
        }
        if(!isAType){
            // no more parameters
            break;
        }
        advance(); // skip the type
        if(g_current.type!=TOKEN_IDENTIFIER){
            PARSE_ERROR("Expected param name after type in function param list");
        }
        Token paramName=g_current;
        advance();
        ASTNode* paramNode=create_node(AST_VARDECL,&paramName);
        // link
        if(!head) head=paramNode; else tail->next=paramNode;
        tail=paramNode;

        // if we see ',' => skip
        if(isDelimiter(g_current,",")){
            advance();
        } else {
            // no more parameters
            break;
        }
    }
    return head;
}

/* parse_declaration => type ident [= expr] ; or function definition */
static ASTNode* parse_declaration(){
    PARSE_INFO("parse_declaration->start\n");
    // confirm a type
    int matchedType=0;
    for(int i=0; i<(int)(sizeof(TYPES)/sizeof(TYPES[0])); i++){
        if(g_current.type==TOKEN_KEYWORD && !strcmp(g_current.lexeme,TYPES[i])){
            matchedType=1;break;
        }
    }
    if(!matchedType){
        PARSE_ERROR("Expected a type, found something else");
    }
    advance(); // skip the type

    if(g_current.type!=TOKEN_IDENTIFIER){
        PARSE_ERROR("Expected identifier after type");
    }
    Token idTok=g_current;
    advance(); 
    ASTNode* declNode=create_node(AST_VARDECL,&idTok);

    /* if operator is in ASSIGNMENTS => var init
       else if '(' => function
       else expect ';'
    */
    if(str_is_in(g_current.lexeme, ASSIGNMENTS, (int)(sizeof(ASSIGNMENTS)/sizeof(ASSIGNMENTS[0])))){
        // e.g. int x = expr;
        PARSE_INFO("parse_declaration assignment -> found '%s'\n", g_current.lexeme);
        advance(); // skip = or += ...
        ASTNode* initExpr=parse_expression(0);
        declNode->right=initExpr;
        if(!isDelimiter(g_current,";")){
            PARSE_ERROR("Expected ';' after variable declaration");
        }
        advance();
        PARSE_INFO("parse_declaration-> end(var)\n");
        return declNode;
    }
    else if(isDelimiter(g_current,"(")){
        /* function declaration: e.g. int foo(...) { ... } */
        PARSE_INFO("parse_decl function\n");
        advance();
        ASTNode* funcParams=parse_function();
        // expect ')'
        if(!isDelimiter(g_current,")")){
            PARSE_ERROR("Expected ')' after function params");
        }
        advance();
        // then either '{' => function body or ';' => forward decl
        if(isDelimiter(g_current,"{")){
            ASTNode* body=parse_block();
            declNode->body=body;
        } else if(isDelimiter(g_current,";")){
            advance();
        } else {
            PARSE_ERROR("Expected '{' or ';' after function declaration");
        }
        PARSE_INFO("parse_declaration-> end(func)\n");
        return declNode;
    }
    else {
        // normal var w/o init, e.g. int x;
        if(!isDelimiter(g_current,";")){
            PARSE_ERROR("Expected ';' after variable declaration");
        }
        advance();
        PARSE_INFO("parse_declaration-> end\n");
        return declNode;
    }
}

/* parse_assignment => x [op]= expr ; */
static ASTNode* parse_assignment(){
    PARSE_INFO("parse_assignment->start\n");
    Token ident=g_current;
    advance();
    if(!str_is_in(g_current.lexeme, ASSIGNMENTS, (int)(sizeof(ASSIGNMENTS)/sizeof(ASSIGNMENTS[0])))){
        PARSE_ERROR("Expected an assignment operator");
    }
    advance();
    ASTNode* assign=create_node(AST_ASSIGN,&ident);
    ASTNode* rhs=parse_expression(0);
    assign->right=rhs;
    if(!isDelimiter(g_current,";")){
        PARSE_ERROR("Expected ';' after assignment");
    }
    advance();
    PARSE_INFO("parse_assignment->end\n");
    return assign;
}

/* parse_program => top-level parse loop */
static ASTNode* parse_program(){
    PARSE_INFO("parse_program-> start\n");
    Token dummy; memset(&dummy,0,sizeof(Token));
    ASTNode* prog=create_node(AST_PROGRAM,&dummy);
    ASTNode* head=NULL; 
    ASTNode* tail=NULL;

    while(g_current.type!=TOKEN_EOF){
        PARSE_INFO("parse_program-> reading top-level, current='%s'\n",g_current.lexeme);
        int isAType=0;
        for(int i=0;i<(int)(sizeof(TYPES)/sizeof(TYPES[0]));i++){
            if(!strcmp(g_current.lexeme,TYPES[i]) && g_current.type==TOKEN_KEYWORD){
                isAType=1;break;
            }
        }
        int isAKeyword=0;
        for(int i=0;i<(int)(sizeof(KEYWORDS)/sizeof(KEYWORDS[0]));i++){
            if(!strcmp(g_current.lexeme,KEYWORDS[i]) && g_current.type==TOKEN_KEYWORD){
                isAKeyword=1;break;
            }
        }

        ASTNode* node=NULL;
        if(isAType){
            node=parse_declaration();
        } else if(isAKeyword){
            node=parse_statement();
        } else {
            node=parse_statement();
        }
        if(!head) head=node; else tail->next=node;
        tail=node;
        while(tail->next) tail=tail->next;
    }
    prog->body=head;
    PARSE_INFO("parse_program->end\n");
    return prog;
}

void parse_table(Token* table){
    PARSE_INFO("parse_table-> start\n");
    g_tokens=table;
    g_position=0;
    advance();

    ASTNode* root=parse_program();
    PARSE_INFO("\n--- PARSED AST ---\n");
    print_ast(root);

    PARSE_INFO("parse_table-> end\n");
}

/* main: testing float, commas in function declarations/calls, etc. */
int main(void){
    const char* testInputs[]={
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

    size_t NUM_TESTS = sizeof(testInputs)/sizeof(testInputs[0]);
    for (size_t i=0; i<NUM_TESTS; i++){
        PARSE_INFO("\n=== Test #%zu ===\nSource: %s\n", i+1, testInputs[i]);
        Token* tokens=make_table((char*)testInputs[i]);
        parse_table(tokens);
        free(tokens);
    }
    return 0;
}
