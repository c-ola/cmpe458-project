# Grammar Rules

This document describes the high-level grammar rules for our language, reflecting the features in the parser.

## 1. Declarations

A declaration is:
<TYPE> <IDENTIFIER> [= <expression>] ;
or
<TYPE> <IDENTIFIER> ( <parameters> ) [ { <block> } | ; ]
Where `<TYPE>` can be `int`, `uint`, `string`, `float`, or `char`.

### Function Declarations
int foo(int a, float b) { // statements... }
Parameters follow the same `<TYPE> <IDENTIFIER>` pattern, separated by commas.

## 2. Statements

We allow:
1. **Block**: `{ <statementlist> }`
2. **Assignment**: `<IDENTIFIER> [op]= <expression> ;`
3. **If-else**:
if ( <expression> ) { <block> } [ else { <block> } ]
4. **While**:
while ( <expression> ) { <block> }
5. **Repeat-Until**:
repeat { <block> } until ( <expression> )
6. **Print**:
print <expression> ;
7. **Expression statement**:
<expression> ;
e.g. `foo(2, 3.14);`

## 3. Expressions

Expressions are parsed with operator precedence. The parser uses a Pratt or precedence-based approach. Operators:
- `* / %` (factor)
- `+ -` (add_sub)
- `<< >>` (bitshifts)
- `< <= > >= =>` (comparisons)
- `== !=` (equalities)
- `& ^ | && ||` (bitwise/logical ops)
- `=` `+=` `-=` `*=` `/=` etc. are handled in assignment statements.

We also allow parentheses:
( <expression> )
and built-in `factorial(<expr>)`.

## 4. Factorial
We treat `factorial(<expression>)` as a built-in function returning a numeric result.

## 5. Types
- `int`, `uint`, `string`, `float`, `char`

## 6. Comments
- `//` line comment
- `/* ... */` block comment

## 7. Examples

int main(){ int x = 5; if (x > 0) { print "hello"; } return x; }

Copy
Edit
float f = 3.14; x = factorial(5) + 2;

Copy
Edit
int foo(int a, float b){ return a * b; } foo(2, 3.5);

Copy
Edit
while (x < 10) { x += 1; }

vbnet
Copy
Edit
