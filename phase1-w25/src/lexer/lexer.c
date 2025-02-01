/* lexer.c */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../../include/tokens.h"

// Line tracking
static int current_line = 1;
static TokenType last_token_type = TOKEN_NONE;

/* Print error messages for lexical errors */
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

/* Print token information
 *
 *  TODO Update your printing function accordingly
 */

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
        case TOKEN_EOF:
            printf("EOF");
            break;
        default:
            printf("UNKNOWN");
    }
    printf(" | Lexeme: '%s' | Line: %d\n",
           token.lexeme, token.line);
}


#define NUM_KEYWORDS 14

char* keywords[] = {
    "int",
    "uint",
    "float",
    "string",
    "char",
    "object",
    "if",
    "else",
    "for",
    "while",
    "loop",
    "break",
    "return",
    "fn",
};

int is_keyword(char* str, int len) {
    for(int i = 0; i < NUM_KEYWORDS; i++) {
        if (strncmp(str, keywords[i], len) == 0) {
            return 1;
        }
    }
    return 0;
}

#define NUM_OPERATORS 32

char* operators[] = {
    "==",
    "=",
    "!",
    "!=",
    ">",
    ">=",
    "<",
    "<=",
    "+",
    "-",
    "*",
    "/",
    "%",
    ">>",
    "<<",
    "&",
    "&&",
    "|",
    "||",
    "^",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    ">>=",
    "<<=",
    "&=",
    "&&=",
    "|=",
    "||=",
    "^=",
};

int is_operator(char* str, int len) {
    for(int i = 0; i < NUM_OPERATORS; i++) {
        if (strncmp(str, operators[i], len) == 0) {
            return 1;
        }
    }
    return 0;
}


#define NUM_DELIMITERS 9

char *delimiters[] = {
    '}',
    '{',
    ']',
    '[',
    ')',
    '(',
    '>', // TODO Check if you want these or not.
    '<',
    ',', // special cuz no closing bracket
};

int is_delimiter(char str){
    for(int i = 0; i < NUM_DELIMITERS; i++) {
        if (strncmp(str, delimiters[i], 1) == 0) return 1;
    }
    return 0;
}

/* Get next token from input */
Token get_next_token(const char *input, int *pos) {
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

    // TODO: Add keyword and identifier handling here
    // Hint: You'll have to add support for keywords and identifiers, and then string literals
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

// This is a basic lexer that handles numbers (e.g., "123", "456"), basic operators (+ and -), consecutive operator errors, whitespace and newlines, with simple line tracking for error reporting.

int main() {
    const char *input = 
"\
// this is a comment\n\
/* this is also\n\
 * a comment\n\
 */\n\
int x = 123 + 456 - 789;\n\
string x = \"hello\" + \"world\"\n\
foo/* this is allowed too*/haha\n\
"; // Test with multi-line input

    int position = 0;
    Token token;

    printf("Analyzing input:\n%s\n\n", input);

    do {
        token = get_next_token(input, &position);
        last_token_type = token.type;
        print_token(token);
    } while (token.type != TOKEN_EOF);

    return 0;
}
