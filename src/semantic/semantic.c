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
Symbol* add_symbol(SymbolTable* table, const char* name, int type, int line) {
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
    return symbol;
}

// Look up a symbol in the table
// Searches for a variable by name across all accessible scopes
// Returns the symbol if found, NULL otherwise
Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current) {
        if (strcmp(current->name, name) == 0 && current->scope_level <= table->current_scope) {
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
            // free(to_free->name); // this fixed an error
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
    printf("Starting semantic analysis...\n");
    SymbolTable* table = init_symbol_table();
    int result = check_program(ast, table);
    
    // Print symbol table contents
    printf("\nSymbol Table Contents:\n");
    print_symbol_table(table);
    
    // Print final result
    if (result) {
        printf("\nSemantic analysis completed successfully.\n");
    } else {
        printf("\nSemantic analysis failed. Errors detected.\n");
    }
    
    free_symbol_table(table);
    return result;
}

DataType check_type(char* lexemme){
    if (strcmp(lexemme, "int") == 0) return TYPE_INT;
    else if (strcmp(lexemme, "uint") == 0) return TYPE_UINT;
    else if (strcmp(lexemme, "float") == 0) return TYPE_FLOAT;
    else if (strcmp(lexemme, "string") == 0) return TYPE_STRING;
    else if (strcmp(lexemme, "char") == 0) return TYPE_CHAR;
    else return TYPE_UNKNOWN;
}


int check_args(ASTNode *node, SymbolTable *table) {
    ASTNode* cur = node;
    int result = 1;
    while (cur) {
        result = check_declaration(cur, table) && result;
        cur = cur->next;
    }
    return result;
}

// Check a variable declaration
int check_declaration(ASTNode* node, SymbolTable* table) {
    if (node->type != AST_VARDECLTYPE) return 0;

    printf("Checking\n");

    if (node->body->type == AST_VARDECL) {
        printf("vardecl %s\n", node->body->current.lexeme);
        const char* name = node->body->current.lexeme;
        Symbol* existing = lookup_symbol_current_scope(table, name);
        if (existing) {
            semantic_error(SEM_ERROR_REDECLARED_VARIABLE, name, node->current.line);
            return 0;
        }

        // When we add a symbol, mark it as initialized immediately
        // This fixes the issue with 'int x;' being considered uninitialized
        const DataType t = check_type(node->current.lexeme);

        add_symbol(table, name, t, node->current.line);
        printf("Symbol declared %s of type %d\n", node->body->current.lexeme, t);

        Symbol* symbol = lookup_symbol_current_scope(table, name);
        if (symbol) symbol->is_initialized = 1;  // Mark as initialized upon declaration
        ASTNode* func_block = node->body->body;
        ASTNode* func_args = node->body->right;
        if (func_block && func_args) {
            printf("Found Function Declaration Args and Block\n");
            enter_scope(table);
            int result = check_args(func_args, table) && check_block(func_block, table);
            //remove_symbols_in_current_scope(table);
            exit_scope(table);
            return result;
        }
    } else if (node->body->type == AST_ASSIGN) {
        ASTNode* assignment = node->body;
        const char* name = assignment->left->current.lexeme;
        printf("vardecl assign %s\n", name);
        Symbol* existing = lookup_symbol_current_scope(table, name);
        if (existing) {
            semantic_error(SEM_ERROR_REDECLARED_VARIABLE, name, assignment->left->current.line);
            return 0;
        }

        // When we add a symbol, mark it as initialized immediately
        // This fixes the issue with 'int x;' being considered uninitialized
        const DataType lhs_type = check_type(node->current.lexeme);
        Symbol* symbol = add_symbol(table, name, lhs_type, assignment->left->current.line);
        printf("Symbol declared %s of type %d\n", name, lhs_type);
        if (symbol) symbol->is_initialized = 1;  // Mark as initialized upon declaration
        if (!check_expression(assignment->right, table)) {
            return 0;
        }
        const DataType rhs_type = get_expression_type(assignment->right, table);
        
        if (check_type_compatibility(lhs_type, rhs_type) == TYPE_COMPAT_ERROR) {
            semantic_error(SEM_ERROR_TYPE_MISMATCH, name, assignment->left->current.line);
            return 0;
        }
    }

    return 1;
}

// Check program node
int check_program(ASTNode* node, SymbolTable* table) {
    if (!node) return 1;

    int result = 1;
    if (node->body) {
        ASTNode* cur = node->body;
        while (cur) {
            result = check_statement(cur, table) && result;
            cur = cur->next;
        }
    }

    return result;
}

// Check a variable assignment
int check_assignment(ASTNode* node, SymbolTable* table) {
    if (!node->left || !node->right) {
        return 0;
    }

    const char* name = node->left->current.lexeme;
    printf("Checking assignment to variable '%s'\n", name);

    Symbol* symbol = lookup_symbol(table, name);
    if (!symbol) {
        semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, name, node->left->current.line);
        return 0;
    }

    if (node->type == AST_ASSIGN) {
        DataType left_type = symbol->type;
        DataType right_type = get_expression_type(node->right, table);

        if ((left_type == TYPE_INT || left_type == TYPE_UINT || left_type == TYPE_CHAR) && 
            (right_type == TYPE_INT || right_type == TYPE_UINT || right_type == TYPE_CHAR)) {
            return 1;
        }

        semantic_error(SEM_ERROR_TYPE_MISMATCH, symbol->name, node->current.line);
        return 0;
    }

    // Regular assignment
    return check_expression(node->right, table);
}

DataType get_expression_type(ASTNode* node, SymbolTable* table) {
    if (!node) return TYPE_UNKNOWN;

    switch (node->type) {
        case AST_LITERAL:
            switch (node->current.type) {
                case TOKEN_NUMBER: return TYPE_INT;
                case TOKEN_FLOAT: return TYPE_FLOAT;
                case TOKEN_STRING: return TYPE_STRING;
                case TOKEN_IDENTIFIER: 
                    // Look up the actual type from symbol table instead of assuming CHAR
                    {
                        Symbol* sym = lookup_symbol(table, node->current.lexeme);
                        return sym ? sym->type : TYPE_UNKNOWN;
                    }
                    
                default: return TYPE_UNKNOWN;
            }
        
        case AST_IDENTIFIER: {
            Symbol* sym = lookup_symbol(table, node->current.lexeme);
            if (!sym) {
                semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, node->current.lexeme, node->current.line);
                return TYPE_UNKNOWN;
            }
            return sym->type;
        }

        case AST_BINOP: {
            DataType left_type = get_expression_type(node->left, table);
            DataType right_type = get_expression_type(node->right, table);
            return get_result_type(left_type, right_type, node->current.lexeme);
        }

        case AST_FUNCTION_CALL: {
            DataType operand_type = get_expression_type(node->body, table);
            if (operand_type != TYPE_INT && operand_type != TYPE_UINT) {
                semantic_error(SEM_ERROR_TYPE_MISMATCH, "factorial", node->current.line);
                return TYPE_UNKNOWN;
            }
            return TYPE_INT;
        }

        default:
            return TYPE_UNKNOWN;
    }
}

TypeCompatibility check_type_compatibility(DataType left, DataType right) {
    if (left == right) return TYPE_COMPAT_OK;
    
    // Handle numeric type conversions - both integers and floats are compatible
    if ((left == TYPE_INT || left == TYPE_UINT || left == TYPE_FLOAT) &&
        (right == TYPE_INT || right == TYPE_UINT || right == TYPE_FLOAT)) {
        return TYPE_COMPAT_OK;  // Changed from TYPE_COMPAT_CONVERT to TYPE_COMPAT_OK
    }
    
    return TYPE_COMPAT_ERROR;
}

DataType get_result_type(DataType left, DataType right, const char* operator) {
    // Handle arithmetic operators
    if (strcmp(operator, "+") == 0 || 
        strcmp(operator, "-") == 0 || 
        strcmp(operator, "*") == 0 || 
        strcmp(operator, "/") == 0) {
        
        // If either operand is float, result is float
        if (left == TYPE_FLOAT || right == TYPE_FLOAT) {
            return TYPE_FLOAT;
        }
        // If both are integers (signed or unsigned), result is int
        return TYPE_INT;
    }
    
    // Handle comparison operators
    if (strcmp(operator, "<") == 0 || 
        strcmp(operator, ">") == 0 || 
        strcmp(operator, "<=") == 0 || 
        strcmp(operator, ">=") == 0 || 
        strcmp(operator, "==") == 0 || 
        strcmp(operator, "!=") == 0) {
        return TYPE_INT;  // Boolean result
    }
    
    return TYPE_UNKNOWN;
}

// Update the check_expression function
int check_expression(ASTNode* node, SymbolTable* table) {
    if (!node) return 0;

    printf("Checking expression: ");
    switch (node->type) {
        case AST_BINOP:
            printf("Binary operation '%s'\n", node->current.lexeme);
            break;
        case AST_IDENTIFIER:
            printf("Identifier '%s'\n", node->current.lexeme);
            break;
        case AST_LITERAL:
            printf("Literal '%s'\n", node->current.lexeme);
            break;
        case AST_FUNCTION_CALL:
            printf("Function Call '%s'\n", node->current.lexeme);
            break;
        default:
            printf("Unknown expression type\n");
    }
    
    switch (node->type) {
        case AST_BINOP: {
            // Check left and right operands recursively
            if (!check_expression(node->left, table) || 
                !check_expression(node->right, table)) {
                return 0;
            }

            DataType left_type = get_expression_type(node->left, table);
            DataType right_type = get_expression_type(node->right, table);
            TypeCompatibility compat = check_type_compatibility(left_type, right_type);
            if (compat == TYPE_COMPAT_ERROR) {
                semantic_error(SEM_ERROR_TYPE_MISMATCH, node->current.lexeme, node->current.line);
                return 0;
            }
            // Get the result type and store it for future use
            // THIS IS NOT ALLOWED
            //node->type = get_result_type(left_type, right_type, node->current.lexeme);

            // Add division by zero check
            if (strcmp(node->current.lexeme, "/") == 0) {
                // If right operand is a literal number
                if (node->right->type == AST_LITERAL && 
                    node->right->current.type == TOKEN_NUMBER) {
                    int value = atoi(node->right->current.lexeme);
                    if (value == 0) {
                        semantic_error(SEM_ERROR_INVALID_OPERATION, "division by zero", node->current.line);
                        return 0;
                    }
                }
            }
            
            return 1;
        }
        
        case AST_UNARYOP: {
            if (!check_expression(node->right, table)) return 0;
            
            DataType operand_type = get_expression_type(node->right, table);
            // Validate unary operator compatibility
            if (strcmp(node->current.lexeme, "!") == 0) {
                // Logical NOT - result is always boolean (int)
                // THIS IS NOT ALLOWED
                //node->type = TYPE_INT;
            } else if (strcmp(node->current.lexeme, "-") == 0) {
                // Numeric negation - preserve operand type
                // THIS IS NOT ALLOWED
                //node->type = operand_type;
            }
            return 1;
        }
        
        case AST_LITERAL:
        case AST_IDENTIFIER:
            // These are already type-checked in get_expression_type
            return 1;
            
        case AST_FUNCTION_CALL: {
            const char* func_name = node->current.lexeme;
            
            // Special handling for factorial
            if (strcmp(func_name, "factorial") == 0) {
                //print_symbol_table(table);
                // Factorial requires exactly one argument
                if (!node->body || node->body->next) {
                    semantic_error(SEM_ERROR_INVALID_OPERATION, "factorial requires exactly one argument", node->current.line);
                    return 0;
                }
                
                // Check argument type (must be int or uint)
                DataType arg_type = get_expression_type(node->body, table);
                if (arg_type != TYPE_INT && arg_type != TYPE_UINT) {
                    semantic_error(SEM_ERROR_TYPE_MISMATCH, "factorial argument must be integer", node->current.line);
                    return 0;
                }
                
                // this doesnt make sense, can't make the node type a DataType
                // it's supposed to be a ASTType
                // it also doesn't work
                // Factorial returns int
                //node->type = TYPE_INT;
                return 1;
            }
            
            // Add other function validations here if needed
            Symbol* symbol = lookup_symbol(table, func_name);
            //print_symbol_table(table);
            if (!symbol)
                semantic_error(SEM_ERROR_INVALID_OPERATION, "unknown function", node->current.line);
            return 0;
        }
        
        default:
            return 0;
    }
}

// Check statement
int check_statement(ASTNode* node, SymbolTable* table) {
    int result = 1;
    switch (node->type) {
        case AST_BLOCK:
            result = check_block(node->body, table) && result;
            break;
        case AST_VARDECLTYPE:
        case AST_VARDECLFUNC:
        // case AST_VARDECL:
            result = check_declaration(node, table) && result;
            break;
        case AST_ASSIGN:
            result = check_assignment(node, table) && result;
            break;
        case AST_IF:
            // Validate condition
            result = check_condition(node->left, table) && result;
            // Validate 'then' block
            if (node->body) {
                result = check_block(node->body, table) && result;
            }
            // Validate 'else' block if it exists
            if (node->right) {
                result = check_block(node->right, table) && result;
            }
            break;
        case AST_WHILE:
            // Validate condition
            result = check_condition(node->left, table) && result;
            // Validate loop body
            if (node->body) {
                result = check_block(node->body, table) && result;
            } else {
                semantic_error(SEM_ERROR_INVALID_OPERATION, "while", node->current.line);
                result = 0;
            }
            break;
        case AST_REPEAT:
            // Validate body first (since it executes at least once)
            if (node->body) {
                result = check_block(node->body, table) && result;
            } else {
                semantic_error(SEM_ERROR_INVALID_OPERATION, "repeat", node->current.line);
                result = 0;
            }
            // Validate condition
            if (node->left) {
                result = check_condition(node->left, table) && result;
            } else {
                semantic_error(SEM_ERROR_INVALID_OPERATION, "until", node->current.line);
                result = 0;
            }
            break;
        case AST_PRINT:
            // Print statement must have an expression to print
            if (!node->body) {
                semantic_error(SEM_ERROR_INVALID_OPERATION, "print statement requires an expression", node->current.line);
                return 0;
            }
            
            // Check that the expression is valid
            result = check_expression(node->body, table);
            
            // All types are printable, so no need for type checking
            return result;
        case AST_FUNCTION_CALL:
            // Validate function call as a statement
            return check_expression(node, table);
        default:
            break;


    }
    return result;
}

// Check a block of statements, handling scope
int check_block(ASTNode* node, SymbolTable* table) {
    enter_scope(table);
    int result = 1;
    
    ASTNode* temp = node;
    while (temp) {
        result = check_statement(temp, table) && result;
        temp = temp->next;
    }
    //print_symbol_table(table);
    //remove_symbols_in_current_scope(table);
    exit_scope(table);
    return result;
}

// Check a condition (e.g., in if statements)
int check_condition(ASTNode* node, SymbolTable* table) {
    if (!node) {
        semantic_error(SEM_ERROR_INVALID_OPERATION, "condition", 0);
        return 0;
    }

    switch (node->type) {
        case AST_BINOP: {
            // First check if it's a comparison operator
            const char* op = node->current.lexeme;
            int is_comparison = (strcmp(op, "<") == 0 || 
                               strcmp(op, ">") == 0 || 
                               strcmp(op, "<=") == 0 || 
                               strcmp(op, ">=") == 0 || 
                               strcmp(op, "==") == 0 || 
                               strcmp(op, "!=") == 0 ||
                               strcmp(op, "&&") == 0 || 
                               strcmp(op, "||") == 0);

            if (!is_comparison && strcmp(op, "!") != 0) {
                semantic_error(SEM_ERROR_INVALID_OPERATION, "Invalid condition operator", node->current.line);
                return 0;
            }

            // Validate both operands
            if (!node->left || !node->right) {
                semantic_error(SEM_ERROR_INVALID_OPERATION, op, node->current.line);
                return 0;
            }

            // Check types are compatible
            DataType left_type = get_expression_type(node->left, table);
            DataType right_type = get_expression_type(node->right, table);
            
            TypeCompatibility compat = check_type_compatibility(left_type, right_type);
            if (compat == TYPE_COMPAT_ERROR) {
                semantic_error(SEM_ERROR_TYPE_MISMATCH, op, node->current.line);
                return 0;
            }

            return 1;
        }

        case AST_UNARYOP: {
            // Only allow logical NOT in conditions
            if (strcmp(node->current.lexeme, "!") != 0) {
                semantic_error(SEM_ERROR_INVALID_OPERATION, "Invalid unary operator in condition", node->current.line);
                return 0;
            }
            return check_condition(node->right, table);
        }

        case AST_IDENTIFIER:
        case AST_LITERAL: {
            // Allow boolean/numeric values in conditions
            DataType type = get_expression_type(node, table);
            if (type != TYPE_INT && type != TYPE_UINT && type != TYPE_FLOAT) {
                semantic_error(SEM_ERROR_TYPE_MISMATCH, "Condition must be numeric or boolean", node->current.line);
                return 0;
            }
            return 1;
        }

        default:
            semantic_error(SEM_ERROR_INVALID_OPERATION, "Invalid condition expression", node->current.line);
            return 0;
    }
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

// Let's also add a helper function to validate function arguments:
int validate_function_args(ASTNode* args, SymbolTable* table, const char* func_name) {
    if (strcmp(func_name, "factorial") == 0) {
        // Count arguments
        int arg_count = 0;
        ASTNode* current = args;
        while (current) {
            arg_count++;
            current = current->next;
        }
        
        if (arg_count != 1) {
            semantic_error(SEM_ERROR_INVALID_OPERATION, 
                         "factorial requires exactly one argument", 
                         args ? args->current.line : 0);
            return 0;
        }
        
        // Check argument type
        DataType arg_type = get_expression_type(args, table);
        if (arg_type != TYPE_INT && arg_type != TYPE_UINT) {
            semantic_error(SEM_ERROR_TYPE_MISMATCH, 
                         "factorial argument must be integer", 
                         args->current.line);
            return 0;
        }
        
        return 1;
    }
    
    // Add validation for other special functions here
    return 0;
}

// Add this function to print symbol table contents
void print_symbol_table(SymbolTable* table) {
    printf("== SYMBOL TABLE DUMP ==\n");
    
    // Count total symbols
    int total = 0;
    Symbol* current = table->head;
    while (current) {
        total++;
        current = current->next;
    }
    printf("Total symbols: %d\n\n", total);
    
    // Print each symbol's details
    current = table->head;
    int index = 0;
    while (current) {
        printf("Symbol[%d]:\n", index++);
        printf("  Name: %s\n", current->name);
        printf("  Type: %d\n", current->type);
        printf("  Scope Level: %d\n", current->scope_level);
        printf("  Line Declared: %d\n", current->line_declared);
        printf("  Initialized: %s\n\n", current->is_initialized ? "Yes" : "No");
        current = current->next;
    }
    printf("===================\n");
}
