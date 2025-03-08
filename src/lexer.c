#include "lexer.h"
#include "tokens.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// For tracking line numbers during lexing
static int current_line = 1;


// checks if [str, str+len) is in the keywords[] array
int is_keyword(char* str, int len) {
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strncmp(str, keywords[i], len) == 0 && keywords[i][len] == '\0') {
            return 1;
        }
    }
    return 0;
}

int is_operator(char* str, int len) {
    for (int i = 0; i < NUM_OPERATORS; i++) {
        if (strncmp(str, operators[i], len) == 0) {
            return 1;
        }
    }
    return 0;
}
int is_operator_start(char c) {
    for (int i = 0; i < NUM_OPERATORS; i++) {
        if (c == operators[i][0]) {
            return 1;
        }
    }
    return 0;
}

int is_delimiter(char c) {
    for (int i = 0; i < NUM_DELIMITERS; i++) {
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
        case TOKEN_STRING:
            printf("STRING_LITERAL | Lexeme: \"");
            for (int j = 0; token.lexeme[j] != '\0'; j++) {
                if (token.lexeme[j] == '\n') printf("\\n");
                else if (token.lexeme[j] == '\t') printf("\\t");
                else if (token.lexeme[j] == '\"') printf("\\\"");
                else printf("%c", token.lexeme[j]);
            }
            printf("\" | Line: %d\n", token.line);
            return;
        case TOKEN_DELIMITER:
            printf("DELIMITER");
            break;
        case TOKEN_EOF:
            printf("EOF");
            break;
        default:
            printf("UNKNOWN %d", token.type);
    }
    printf(" | Lexeme: '%s' | Line: %d\n",
            token.lexeme, token.line);
}

void skip_whitespace(const char* input, int *pos, int *current_line) {
    char c;
    int found_comment = 0;
    while ((c = input[*pos]) != '\0' && (c == ' ' || c == '\n' || c == '\t' || c == '/') ) {
        if (c == '\n') {
            (*current_line)++;
            (*pos)++;
            continue;
        }

        // Skip line comments: "//"
        if ((input[*pos] == '/' && input[*pos + 1] == '/')) {
            found_comment = 1;
            (*pos) += 2;
            while (input[*pos] != '\0' && input[*pos] != '\n') {
                (*pos)++;
            };
            continue;
        }
        // Skip block comments: "/*...*/"
        else if (input[*pos] == '/' && input[*pos + 1] == '*') {
            (*pos) += 2;
            while (input[*pos] != '\0' &&
                   !(input[*pos] == '*' && input[*pos + 1] == '/'))
            {
                if (input[*pos] == '\n') {
                    (*current_line)++;
                }
                (*pos)++;
            }
            // skip the '*/'
            if (input[*pos] == '*') (*pos)++;
            if (input[*pos] == '/') (*pos)++;
            continue;
        } else if (input[*pos] == '/'){
            break;
        }
        (*pos)++;
    }
}

Token get_next_token(const char *input, int *pos, TokenType last_token_type) {
    Token token = {TOKEN_ERROR, "", current_line, ERROR_NONE};
    char c;

    // Skip whitespace + track line numbers
    skip_whitespace(input, pos, &current_line);
    c = input[*pos];
    // If end of input => TOKEN_EOF
    if (c == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    // If c is a delimiter => return TOKEN_DELIMITER
    if (is_delimiter(c)) {
        token.lexeme[0] = c;
        token.lexeme[1] = '\0';
        token.type = TOKEN_DELIMITER;
        (*pos)++;
        return token;
    }

    //  If c is a double-quote => parse string literal
    if (c == '"') {
        int i = 0;
        (*pos)++; // skip opening quote
        c = input[*pos];

        while (c != '"' && c != '\0' && i < (int)sizeof(token.lexeme) - 1) {
            if (c == '\n') {
                // unterminated string => error
                token.error = ERROR_INVALID_CHAR;
                snprintf(token.lexeme, sizeof(token.lexeme), "Unterminated string at line %d", current_line);
                // skip until next line
                while (input[*pos] != '\0' && input[*pos] != '\n') {
                    (*pos)++;
                }
                if (input[*pos] == '\n') {
                    current_line++;
                    (*pos)++;
                }
                return token;
            }
            // handle escape sequences
            if (c == '\\') {
                (*pos)++;
                c = input[*pos];
                switch (c) {
                    case 'n':  token.lexeme[i++] = '\\'; token.lexeme[i++] = 'n'; break;
                    case 't':  token.lexeme[i++] = '\\'; token.lexeme[i++] = 't'; break;
                    case '\\': token.lexeme[i++] = '\\'; break;
                    case '"':  token.lexeme[i++] = '\"'; break;
                    default:
                        token.error = ERROR_INVALID_CHAR;
                        snprintf(token.lexeme, sizeof(token.lexeme), "Invalid escape \\%c", c);
                        // skip rest of line
                        while (input[*pos] != '\0' && input[*pos] != '\n') {
                            (*pos)++;
                        }
                        if (input[*pos] == '\n') {
                            current_line++;
                            (*pos)++;
                        }
                        return token;
                }
            } else {
                token.lexeme[i++] = c;
            }
            (*pos)++;
            c = input[*pos];
        }

        // check if we ended properly
        if (c == '\0') {
            token.error = ERROR_INVALID_CHAR;
            snprintf(token.lexeme, sizeof(token.lexeme),
                     "Unterminated string at line %d", current_line);
            return token;
        }
        // else c == '"', so close the string
        (*pos)++; // skip closing quote
        token.lexeme[i] = '\0';
        token.type = TOKEN_STRING;
        return token;
    }

    // If c is a digit => parse number
    if (isdigit(c)) {
        int i = 0;
        int found_decimals = 0;
        while ((isdigit(c) || c == '.') && i < (int)sizeof(token.lexeme) - 1) {
            if (c == '.') {
                found_decimals++;
            }
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        }
        token.lexeme[i] = '\0';
        if (found_decimals > 1) {
            token.error = ERROR_INVALID_NUMBER;
            fprintf(stderr, "Invalid float literal with multiple decimals %s\n", token.lexeme);
        }
        token.type = TOKEN_NUMBER;
        return token;
    }

    // If c is a letter or underscore => begin parsing identifier/keyword
    if (isalpha(c) || c == '_') {
        int i = 0;
        while ((isdigit(c) || isalpha(c) || c == '_') && i < (int)sizeof(token.lexeme) - 1) {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        }
        token.lexeme[i] = '\0';

        // check if it's keyword
        if (is_keyword(token.lexeme, i)) {
            token.type = TOKEN_KEYWORD;
        }
        else {
            token.type = TOKEN_IDENTIFIER;
        }
        return token;
    }

    if (is_operator_start(c)) {
        int i = 0;
        while ((c != ' ' && c != '\n') && i < sizeof(token.lexeme)) {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        }
        if (is_operator(token.lexeme, i)) {
            token.type = TOKEN_OPERATOR;
            // check consecutive operators
            if (last_token_type == TOKEN_OPERATOR) {
                token.error = ERROR_CONSECUTIVE_OPERATORS;
            }
        }
        return token;
    }

    //  reach here => unknown or invalid character
    token.error = ERROR_INVALID_CHAR;
    token.lexeme[0] = c;
    token.lexeme[1] = '\0';
    (*pos)++;
    return token;
}

void print_token_stream(const char* input) {
    int position = 0;
    Token token;
    TokenType last = TOKEN_NONE;
    
    do {
        token = get_next_token(input, &position, last);
        last = token.type;
        print_token(token);
    } while (token.type != TOKEN_EOF);
}
