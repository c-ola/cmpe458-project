#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"

// Define DataType enum first
typedef enum {
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_UNKNOWN
} DataType;

// Type compatibility result
typedef enum {
    TYPE_COMPAT_OK,
    TYPE_COMPAT_CONVERT,
    TYPE_COMPAT_ERROR
} TypeCompatibility;

// Symbol table structures
typedef struct Symbol {
    char name[100];
    DataType type;  // Now DataType is defined before use
    int scope_level;
    int line_declared;
    int is_initialized;
    struct Symbol* next;
} Symbol;

typedef struct {
    Symbol* head;
    int current_scope;
} SymbolTable;

// Semantic error types
typedef enum {
    SEM_ERROR_NONE,
    SEM_ERROR_UNDECLARED_VARIABLE,
    SEM_ERROR_REDECLARED_VARIABLE, 
    SEM_ERROR_TYPE_MISMATCH,
    SEM_ERROR_UNINITIALIZED_VARIABLE,
    SEM_ERROR_INVALID_OPERATION,
    SEM_ERROR_SEMANTIC_ERROR
} SemanticErrorType;

// Symbol table functions
SymbolTable* init_symbol_table(void);
void add_symbol(SymbolTable* table, const char* name, int type, int line);
Symbol* lookup_symbol(SymbolTable* table, const char* name);
Symbol* lookup_symbol_current_scope(SymbolTable* table, const char* name);
void enter_scope(SymbolTable* table);
void exit_scope(SymbolTable* table);
void remove_symbols_in_current_scope(SymbolTable* table);
void free_symbol_table(SymbolTable* table);

// Semantic analysis functions
int analyze_semantics(ASTNode* ast);
int check_declaration(ASTNode* node, SymbolTable* table);
int check_program(ASTNode* node, SymbolTable* table);
int check_assignment(ASTNode* node, SymbolTable* table);
int check_expression(ASTNode* node, SymbolTable* table);
int check_statement(ASTNode* node, SymbolTable* table);
int check_block(ASTNode* node, SymbolTable* table);
int check_condition(ASTNode* node, SymbolTable* table);

// Error reporting
void semantic_error(SemanticErrorType error, const char* name, int line);

// Type checking utility functions
TypeCompatibility check_type_compatibility(DataType left, DataType right);
DataType get_result_type(DataType left, DataType right, const char* operator);
DataType get_expression_type(ASTNode* node, SymbolTable* table);

#endif // SEMANTIC_H
