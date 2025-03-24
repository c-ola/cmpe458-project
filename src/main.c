#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

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
        ASTNode *programNode = parse_program(&parser);

        // parse(&parser);
        free_parser(parser);
        free(input);
    } else {
        const char* testInputs[] = {
        "int main(){ int x = 0; }",
        "x = 5;",
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
        "int main(){ string x = \"hey\"; }",
        "if ((0 + 1 == 1) && (1 + 0 == 1)) { print 1; }",
        //"float f = 3.14;", // FLOATS DO NOT WORK
        "int foo(int a, float b){ b = b + a; }",
        "foo(2, 15);",
        };
        size_t NUM_TESTS = sizeof(testInputs) / sizeof(testInputs[0]);
        NUM_TESTS = 1;

        for (size_t i = 0; i < NUM_TESTS; i++) {
            PARSE_INFO("\n=== Test #%zu ===\nSource: %s\n", i+1, testInputs[i]);
            Parser parser = new_parser((char*)testInputs[i]);
#ifdef DEBUG
            print_token_stream(testInputs[i]);
#endif
            // using this so that the parse 
            parse(&parser);
            
            analyze_semantics(parser.root);

            free_parser(parser);
        }
    }
    return 0;
}
