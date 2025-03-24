# AST Structure

## 1. Node Types

The `ASTType` enum includes:

- `AST_PROGRAM` for the top-level node
- `AST_BLOCK` for `{ ... }` blocks
- `AST_VARDECL` for variable declarations
- `AST_ASSIGN` for assignment statements
- `AST_IF`, `AST_WHILE`, `AST_REPEAT`, `AST_PRINT`
- `AST_FUNCTION_CALL` (for calls like `foo(2, 3)`)
- `AST_FUNCTION_ARGS` (if we store function parameters as a separate node, optional)
- `AST_BINOP` and `AST_UNARYOP` for expressions
- `AST_LITERAL` for numeric/string constants
- `AST_IDENTIFIER` for variable references
- `AST_FACTORIAL` for `factorial(expr)`

## 2. Node Fields

```c
typedef struct ASTNode {
    ASTType         type;
    Token           current; // the associated token (e.g. name, operator, literal)
    struct ASTNode* left;    // often used for subexpressions
    struct ASTNode* right;   // also subexpressions or block
    struct ASTNode* next;    // linking statements in a list
    struct ASTNode* body;    // for block contents or function param list
} ASTNode;
```
## 3. Common Usage
- `AST_BLOCK` nodes hold statements in body.
- For `AST_FUNCTION_CALL`, body is the head of the argument list.
- For `AST_IF`, left is condition, right is the then block, body is the optional else block.
- For `AST_BINOP`, left and right hold subexpressions.
- `AST_VARDECL` can store the initialization expression in right.
- `AST_ASSIGN` has current.lexeme = variable name, and right = expression being assigned.

## 4. Example Tree
For:
int x = 5 + 2;

We might get:
```sql
AST_VARDECL("x")
  ->right = AST_BINOP("+")
     left = AST_LITERAL("5")
     right= AST_LITERAL("2")
```

For:
```c
while (x < 10) { x += 1; }
```

We might see:
```sql
AST_WHILE
  left  -> AST_BINOP("<")
             left = AST_IDENTIFIER("x")
             right= AST_LITERAL("10")
  right -> AST_BLOCK
             body -> [ AST_ASSIGN("x") -> right= AST_BINOP("+=") left= x, right=1 ]
```