
#include "lex-pars.h"
//lexer part
int is_keyword(char* str, int len) {
    for(int i = 0; i < NUM_KEYWORDS; i++) {
        if (strncmp(str, keywords[i], len) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_operator(char* str, int len) {
    for(int i = 0; i < NUM_OPERATORS; i++) {
        if (strncmp(str, operators[i], len) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_delimiter(char c){
    for(int i = 0; i < NUM_DELIMITERS; i++) {
        if (c == delimiters[i]) return 1;
    }
    return 0;
}

void print_error(ErrorType error, int line, const char *lexeme) {
    printf("Lexical Error at line %d: ", line);
    switch (error) {
        case ERROR_INVALID_CHAR:
            printf("Invalid character '%s'\n", lexeme);
            break;
        case ERROR_INVALID_NUMBER:
            printf("Invalid number format\n");
            break;
        case ERROR_CONSECUTIVE_OPERATORS:
            printf("Consecutive operators not allowed\n");
            break;
        case ERROR_UNKNOWN_TYPE:
            printf("Unknown type for token %s\n", lexeme);
            break;
        default:
            printf("Unknown error\n");
    }
}

void print_token(Token token) {
    if (token.error != ERROR_NONE) {
        print_error(token.error, token.line, token.lexeme);
        return;
    }

    printf("Token: ");
    switch (token.type) {
        case TOKEN_NUMBER:
            printf("NUMBER");
            break;
        case TOKEN_OPERATOR:
            printf("OPERATOR");
            break;
        case TOKEN_KEYWORD:
            printf("KEYWORD");
            break;
        case TOKEN_IDENTIFIER:
            printf("IDENTIFIER");
            break;
        case TOKEN_STRING_LITERAL:
            printf("STRING_LITERAL | Lexeme: \"");
            for (int j = 0; token.lexeme[j] != '\0'; j++) {
                if (token.lexeme[j] == '\n') printf("\\n");
                else if (token.lexeme[j] == '\t') printf("\\t");
                else if (token.lexeme[j] == '\"') printf("\\\"");
                else printf("%c", token.lexeme[j]);
            }
            printf("\" | Line: %d\n", token.line);
            return;
        case TOKEN_EOF:
            printf("EOF");
            break;
        default:
            printf("UNKNOWN");
    }
    printf(" | Lexeme: '%s' | Line: %d\n",
            token.lexeme, token.line);
}

static int current_line = 1;
Token get_next_token(const char *input, int *pos, TokenType last_token_type) {
    Token token = {TOKEN_ERROR, "", current_line, ERROR_NONE};
    char c;

    // Skip whitespace and track line numbers
    while ((c = input[*pos]) != '\0' &&
           (c == ' ' ||
            c == '\n' ||
            c == '\t'))
    {
        if (c == '\n') current_line++;
        (*pos)++;
    }

    if (input[*pos] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    // skip over line comments
    while (input[*pos] == '/' && input[*pos+1] == '/') {
        current_line++;
        while (input[(*pos)++] != '\n');
    }

    // skip over block comments
    if (input[*pos] == '/' && input[*pos+1] == '*') {
        *pos += 2;
        while(!(input[*pos] == '*' && input[*pos + 1] == '/')) {
            *pos+=1;
        };
        (*pos)++;
        do { *pos+=1; } while (input[*pos] == '\n');
    }
    c = input[*pos];

    // Handle string literals
    if (c == '"') {
        int i = 0;
        (*pos)++;  // pass opening quote
        c = input[*pos];

        while (c != '"' && c != '\0' && i < sizeof(token.lexeme) - 1) {
            if (c == '\n' || c == '\0') {
                // unterminated string
                token.error = ERROR_INVALID_CHAR;
                snprintf(token.lexeme, sizeof(token.lexeme), "Unterminated string");

                // go to newline, skip processing
                while (input[*pos] != '\0' && input[*pos] != '\n') {
                    (*pos)++;
                }
                (*pos)++; // Move to the next line

                return token; 
            }

            // Handle escape sequences
            if (c == '\\') {
                (*pos)++;
                c = input[*pos];

                switch (c) {
                    case 'n': token.lexeme[i++] = '\\'; token.lexeme[i++] = 'n'; break;
                    case 't': token.lexeme[i++] = '\\'; token.lexeme[i++] = 't'; break;
                    case '\\': token.lexeme[i++] = '\\'; break;
                    case '"': token.lexeme[i++] = '\"'; break;
                    default:
                        // invalid escape seq
                        token.error = ERROR_INVALID_CHAR;
                        snprintf(token.lexeme, sizeof(token.lexeme), "Invalid escape: \\%c", c);

                        // skip line
                        while (input[*pos] != '\0' && input[*pos] != '\n') {
                            (*pos)++;
                        }
                        (*pos)++; // skip line

                        return token;
                }
            } else {
                token.lexeme[i++] = c;
            }
            (*pos)++;
            c = input[*pos];
        }

        if (c == '"') {
            token.lexeme[i] = '\0';
            token.type = TOKEN_STRING_LITERAL;
            (*pos)++;  // go past closing quote
        } else {
            //unterminated string
            token.error = ERROR_INVALID_CHAR;
            snprintf(token.lexeme, sizeof(token.lexeme), "Unterminated string");

            // skip line
            while (input[*pos] != '\0' && input[*pos] != '\n') {
                (*pos)++;
            }
            (*pos)++; // Move to the next line

            return token;
        }

        return token;
    }

    // Handle numbers
    if (isdigit(c)) {
        int i = 0;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        } while (isdigit(c) && i < sizeof(token.lexeme) - 1);

        token.lexeme[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    // handle end of expression
    if (c == ';') {
        token.lexeme[0] = ';';
        token.lexeme[1] = '\0';
        token.type = TOKEN_KEYWORD;
        (*pos)++;
        return token;
    }

    // Add keyword and identifier handling here
    if (!isdigit(c) || c == '_') {
        int i = 0;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        } while ((isalpha(c) || c == '_') && i < sizeof(token.lexeme) - 1);

        token.lexeme[i] = '\0';
        if (is_keyword(token.lexeme, i)) {
            token.type = TOKEN_KEYWORD;
        } 
        else if (is_operator(token.lexeme, i)) {
            token.type = TOKEN_OPERATOR;
            if (last_token_type == TOKEN_OPERATOR) {
                // Check for consecutive operators
                token.error = ERROR_CONSECUTIVE_OPERATORS;
                token.lexeme[0] = c;
                token.lexeme[1] = '\0';
                (*pos)++;
                return token;
            }
        } 
        else token.type = TOKEN_IDENTIFIER;
        return token;
    }

    // string literal handling
    if (input[*pos] == '"' || input[*pos] == '\'') {
        char toMatch = input[*pos];
        int i = 0;
        do {
            token.lexeme[i++] = c;
            
            // checking that the string will actually include a closing bracket
            if (i < (sizeof(token.lexeme) - 2)){ 
                token.lexeme[i++] = c;
                break;
            }

            (*pos)++;
            c = input[*pos];

        } while (c != toMatch);
        token.lexeme[i] = '\0';
        token.type = TOKEN_STRING;
        return token;
    }

    // TODO: Add delimiter handling here
    if (is_delimiter(c)){
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        token.type = TOKEN_DELIMITER;
        (*pos)++;
        return token;
    }


    // Handle invalid characters
    token.error = ERROR_INVALID_CHAR;
    token.lexeme[0] = c;
    token.lexeme[1] = '\0';
    (*pos)++;
    return token;
}

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

void init_states(State* ref){
    ref = malloc(sizeof(State)* STATES);
    
    // state 1
    ref[0].opt1[0] = S_2;
    ref[0].opt1[1] = S_1; // split
    ref[0].opt2[0] = S_EOF;
    
    // state 2
    ref[1].opt1[0] = S_3;
    ref[1].opt1[1] = S_DELIMITER; // split
    ref[1].opt2[0] = S_2;
    ref[1].opt2[1] = S_OPERATOR;
    ref[1].opt1[2] = S_2;

    // state 3
    ref[2].opt1[0] = S_4;
    ref[2].opt2[0] = S_5;
    
    // state 4
    ref[3].opt1[0] = S_LITERAL;
    ref[3].opt2[0] = S_IDENTIFIER;

    // state 5
    ref[4].opt1[0] = S_KEYWORD;
    ref[4].opt1[1] = S_DELIMITER;
    ref[4].opt1[2] = S_5;
    ref[4].opt1[3] = S_DELIMITER;
    ref[4].opt2[0] = S_6;

    // state 6
    ref[5].opt1[0] = S_6;
    ref[5].opt1[1] = S_DELIMITER; 
    ref[5].opt1[2] = S_6;
    ref[5].opt2[0] = S_2;

}

int token_type_to_state(TokenType tt){

}

// use rightmost derivation from left to right (MUST EXPORT A SYNTAX TREE)
void parse_table(Token* table){
    State* ref; // state table to jump around on
    init_states(&ref);
    int curr_state = S_1;

    int count = sizeof(table) / sizeof(Token);
    Token* stack = malloc(sizeof(Token) * count);

    // now follow context-free-grammar.md doc
    // transition on non terminal will be a GOTO and transition on terminal is a shift
    int curr_pos = 0;
    int stack_pos = 0;
    int complete = 0;

    do{
        if (table[curr_pos].type == TOKEN_EOF) complete = 1;

        stack[stack_pos++] = table[curr_pos]; // push
        stack_pos--; // pop
        if (stack_pos == 0) complete = 1; // start symbol is reached

    } while (!complete);

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

int main(int argc, char* argv[]){
    
}