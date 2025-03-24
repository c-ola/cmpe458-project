# Error Handling Strategy

Our parser does **single-pass** top-down parsing with immediate error stops. When a parse error is encountered, we print a message and exit.

## 1. Types of Errors

1. **Missing Semicolon**  
   - Example: `int x` instead of `int x;`.
   - We do: `PARSE_ERROR("Expected ';' after declaration")`.

2. **Unmatched Parentheses/Braces**  
   - Example: `if (x > 10 { ... }` => missing `)`.
   - We do: `PARSE_ERROR("Expected ')' after condition")`.

3. **Unknown Type**  
   - Example: `mystery foo;`.
   - `PARSE_ERROR("Expected a type, found something else")`.

4. **Unexpected Token**  
   - If we read a token that doesn't match any rule at that point, we throw `PARSE_ERROR("Unexpected token in parse_primary")` or similar.

5. **Consecutive Operators** (Lexer-level)  
   - If the lexer sees `++` or `--` as separate tokens incorrectly or something like `+` and `+` back to back, it can produce an error. The parser might also fail if the sequence is invalid.

## 2. Immediate Exit

Because we do single-pass top-down, when we see an error, we print the message and `exit(1)`. There's no advanced error recovery or synchronization. 

## 3. Coordination with the Lexer

- The lexer might also produce lexical errors (e.g., "Unterminated string"). 
- If the lexer error occurs, we typically bail out before the parser sees anything beyond that token.

## 4. Potential Future Enhancements

- Implementing error recovery to continue parsing after an error.
- Detailed line/column info from the lexer.
- Separate error codes for each parse error scenario.

