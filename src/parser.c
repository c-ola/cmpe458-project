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

static int lookahead(TokenType l, Token* table, int pos){
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

    int curr_pos = 0;               // amount of tokens that have been processed
    int stack_pos = 0;              // pretty basic
    int complete = 0, error = 0;    // completion and if with errors
    int state = 0;
    TokenType t;

    // stack[stack_pos++] = table[curr_pos++]; // shift
    // stack_pos--;                            // reduce

    // figure out tree stuff TODO
    while(1){
        // either shift and modify state or reduce using some rule
        stack[stack_pos++] = state; // push state to stack
        t = table[curr_pos].type;
        switch (state){
            case 0: // S
                if (!lookahead(TOKEN_EOF, table, curr_pos)){
                    stack[stack_pos++] = t;
                    state = 1;
                } else complete = 1;
                
                break;
            case 1: // K
                
            
                break;
            case 2: // K'
                break;
            case 3: // S
                break;
            case 4: // S'
                break;
            case 5: // E
                break;
            case 6: // E'
                if (t == TOKEN_NUMBER || t == TOKEN_STRING || t == TOKEN_FLOAT  || t == TOKEN_IDENTIFIER)
                    // reduce and add to tree
                    {

                    }
                break;
            }
        if (complete) break;
    }
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
