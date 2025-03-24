#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include "semantic.h"
#include "parser.h"
#include "lexer.h"

/*

Step 2

*/

// Initialize a new symbol table
// Creates an empty symbol table structure with scope level set to 0
SymbolTable* init_symbol_table() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    if (table) {
        table->head = NULL;
        table->current_scope = 0;
    }
    return table;
}

// Add a symbol to the table
// Inserts a new variable with given name, type, and line number into the current scope
void add_symbol(SymbolTable* table, const char* name, int type, int line) {
    Symbol* symbol = malloc(sizeof(Symbol));
    if (symbol) {
        strcpy(symbol->name, name);
        symbol->type = type;
        symbol->scope_level = table->current_scope;
        symbol->line_declared = line;
        symbol->is_initialized = 0;
        
        symbol->next = table->head;
        table->head = symbol;
    }
}

// Look up a symbol in the table
// Searches for a variable by name across all accessible scopes
// Returns the symbol if found, NULL otherwise
Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Look up symbol in current scope only
Symbol* lookup_symbol_current_scope(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0 && 
            current->scope_level == table->current_scope) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Enter a new scope level
// Increments the current scope level when entering a block (e.g., if, while)
void enter_scope(SymbolTable* table) {
    table->current_scope++;
}

// Exit the current scope
// Decrements the current scope level when leaving a block
// Optionally removes symbols that are no longer in scope
void exit_scope(SymbolTable* table) {
    table->current_scope--;
}

// Remove symbols from the current scope
// Cleans up symbols that are no longer accessible after leaving a scope
void remove_symbols_in_current_scope(SymbolTable* table) {
    Symbol* current = table->head;
    Symbol* prev = NULL;

    while (current) {
        if (current->scope_level == table->current_scope) {
            // Remove this symbol
            if (prev) {
                prev->next = current->next;
            } else {
                table->head = current->next;
            }
            Symbol* to_free = current;
            current = current->next;
            free(to_free->name);
            free(to_free);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

// Free the symbol table memory
// Releases all allocated memory when the symbol table is no longer needed
void free_symbol_table(SymbolTable* table) {
    Symbol* current = table->head;
    while (current) {
        Symbol* next = current->next;
        free(current);
        current = next;
    }
    free(table);
}

/*

Step 3

*/

// Main semantic analysis function
int analyze_semantics(ASTNode* ast) {
    SymbolTable* table = init_symbol_table();
    int result = check_program(ast, table);
    free_symbol_table(table);
    return result;
}

// Check a variable declaration
int check_declaration(ASTNode* node, SymbolTable* table) {
    if (node->type != AST_VARDECL) {
        return 0;
    }

    const char* name = node->current.lexeme;
    Symbol* existing = lookup_symbol_current_scope(table, name);
    if (existing) {
        semantic_error(SEM_ERROR_REDECLARED_VARIABLE, name, node->current.line);
        return 0;
    }

    add_symbol(table, name, node->type, node->current.line);
    return 1;
}

// Check program node
int check_program(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;

    if (node->body) printf("Body: %d\n", node->body->type); else printf("Null body\n");
    if (node->left) printf("Left: %d\n", node->left->type); else printf("Null left\n");
    if (node->right) printf("Right: %d\n", node->right->type); else printf("Null right\n");
    if (node->next) printf("Next: %d\n", node->next->type); else printf("Null next\n");

    int result = 1;
    switch (node->type){
        case AST_PROGRAM:
            if (node->body) {
                result = check_statement(node->body, table) && result;
            }
            
            // Check right child (rest of program)
            if (node->right) {
                result = check_program(node->right, table) && result;
            }
            break;

        default:
            break;
    }

    return result;
}

// Check a variable assignment
int check_assignment(ASTNode* node, SymbolTable* table) {
    if (node->type != AST_ASSIGN || !node->left || !node->right) {
        return 0;
    }

    const char* name = node->left->current.lexeme;
    Symbol* symbol = lookup_symbol(table,name);
    if (!symbol) {
        semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, name, node->left->current.line);
        return 0;
    }
    
    int expr_valid = check_expression(node->right, table);
    if (expr_valid) {
        symbol->is_initialized = 1;
    }
    return expr_valid;
}

// Check an expression for type correctness
int check_expression(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;

    switch (node->type){
        case AST_BINOP:
            break;
        
        case AST_UNARYOP:
            break;
    }

    return 0;
}

// Check statement
int check_statement(ASTNode* node, SymbolTable* table) {
    int result = 1;
    switch (node->type){
        case AST_BLOCK:
            result = check_block(node, table) && result;
            break;
        case AST_VARDECLTYPE:
            break;    
        case AST_VARDECLFUNC:
            break;
        case AST_VARDECL:
            break;
        case AST_ASSIGN:
            result = check_assignment(node, table) && result;
            break;
        case AST_IF:
            break;
        case AST_WHILE:
            break;
        case AST_REPEAT:
            break;
        case AST_PRINT:
            break;
        case AST_FUNCTION_CALL:
            break;
        
    }
    return 0;
}

// Check a block of statements, handling scope
int check_block(ASTNode* node, SymbolTable* table) {
    return 0;
}

// Check a condition (e.g., in if statements)
int check_condition(ASTNode* node, SymbolTable* table) {
    return 0;
}


/*

Step 4

*/

// Report semantic errors
void semantic_error(SemanticErrorType error, const char* name, int line) {
    printf("Semantic Error at line %d: ", line);
    
    switch (error) {
        case SEM_ERROR_UNDECLARED_VARIABLE:
            printf("Undeclared variable '%s'\n", name);
            break;
        case SEM_ERROR_REDECLARED_VARIABLE:
            printf("Variable '%s' already declared in this scope\n", name);
            break;
        case SEM_ERROR_TYPE_MISMATCH:
            printf("Type mismatch involving '%s'\n", name);
            break;
        case SEM_ERROR_UNINITIALIZED_VARIABLE:
            printf("Variable '%s' may be used uninitialized\n", name);
            break;
        case SEM_ERROR_INVALID_OPERATION:
            printf("Invalid operation involving '%s'\n", name);
            break;
        default:
            printf("Unknown semantic error with '%s'\n", name);
    }
}
