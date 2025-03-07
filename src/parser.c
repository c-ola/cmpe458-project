#include "lexer.h"
#include "parser.h"

//parser
// returns a Token* which ends with a TOKEN_EOF
Token* make_table(char* in){
    Token* table = malloc(sizeof(Token) * MAX_TABLE_SIZE); // max 100000 lexemmes
    
    TokenType last = TOKEN_NONE;
    Token current;
    unsigned int position = 0;
    unsigned int lexemmes = 0;

    do{
        current = get_next_token(in, &position, last);
        last = current.type;
        print_token(current);

        table[lexemmes] = current;
        lexemmes++;
        
    }while (current.type != TOKEN_EOF);
    // will end up being EOF at the end

    Token* new_table = malloc(sizeof(Token) * lexemmes + 1);
    memcpy(table, new_table, (sizeof(Token) * lexemmes + 1));
    return new_table;
}

/* 
    Anything past this hasnt really been tested 
*/

int pos;
static int lookahead(TokenType l, Token* table){
    return l == table[pos+1].type;
}
static int match(TokenType type, Token t) {
    return t.type == type;
}

// use a lookahead LR 
void parse_table(Token* table){
    ASTN* tree = create_node(AST_PROGRAM, table);

    int count = sizeof(table) / sizeof(Token);
    int* stack = malloc(sizeof(Token) * count * 3);

    pos = 0;               // amount of tokens that have been processed
    int stack_pos = 0;              // pretty basic
    int complete = 0, error = 0;    // completion and if with errors
    int state = 0;
    TokenType t;
    Token current;

    // stack[stack_pos++] = table[curr_pos++]; // shift
    // stack_pos--;                            // reduce 
    // state = stack[--stack_pos];             // reduce part 2 state should equal what was put on the stack

    // figure out tree stuff TODO
    while(1){
        // either shift and modify state or reduce using some rule
        stack[stack_pos++] = state; // push state to stack
        current = table[pos];
        t = current.type;

        // since we are using enums, if the stack position is odd, it must be a goto so use other switch
        if (stack_pos/2 == 0)
        switch (state){
            case R_S:
                if (match(TOKEN_EOF, current))
                {
                    complete = 1;
                } 
                else
                {
                    stack[stack_pos++] = t;
                    state = 1;
                }
                break;
            case R_A:           
            
                break;
            case R_B:

                break;
            case R_C:
                if (lookahead(TOKEN_OPERATOR, table))
                {  // shift
                    stack[stack_pos++] = t;
                }
                else
                {                                  // pop
                    
                }
                break;
            case R_D:
                if (t == TOKEN_NUMBER || t == TOKEN_STRING || t == TOKEN_FLOAT || t == TOKEN_IDENTIFIER)
                    {
                        // reduce and add to tree
                        stack_pos--;
                        state = stack[--stack_pos];
                    }
                else ;  // error
                break;
                
            }
        else
        switch(state){
            case R_S:
                break;
            case R_A:
                break;
            case R_B:
                break;
            case R_C:
                break;
            case R_D:
                break;
        }

        if (complete) break;
    }
    // after the while loop post processing may happen for example to change ASTType and stuff.

}

/*
    CFG TYPES 

    Program Structure
    Variable Declarations
    Assignment Statements
    Arithmetic Expressions
    Boolean Expressions
    Conditional Statements (if-else)
    Loop Constructs (while, repeat-until)
    Function Calls (if applicable)
    Input/Output Operations (print, read)
    Unique Language Features (repeat-until, built-in factorial, runtime error detection)
*/

void print_ast(ASTN *node, int level) {
    if (!node) return;

    // Indent and print node details
    for (int i = 0; i < level; i++) printf("  ");
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            break;
        case AST_VARDECL:
            printf("VarDecl: %s\n", node->current.lexeme);
            break;
        // Add more node type printings
    }

    // Recursively print children
    print_ast(node->left, level + 1);
    print_ast(node->right, level + 1);
}

static ASTN *create_node(ASTType type, Token* t) {
    ASTN *node = malloc(sizeof(ASTN));
    if (node) {
        node->type = type;
        node->current = *t;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

int main(int argc, char* argv[]){
    int position = 0;
    Token token;

    char* input = "int x = 5;";

    printf("Analyzing input:\n%s\n\n", input);
    print_token_stream(input);
}
