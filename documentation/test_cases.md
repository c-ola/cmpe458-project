# Test Cases Documentation (parser only)

We provide **two** main `.txt` files:

1. `test_valid.txt`: Contains example inputs that **should** parse successfully, demonstrating:
   - Variable declarations (with or without initialization)
   - Assignments and arithmetic
   - If-else statements
   - While loops
   - Repeat-until
   - Factorial usage
   - Function definitions and calls
   - Logical operators (`&&`, `||`)
   - Nested parentheses
   - Print statements

2. `test_invalid.txt`: Contains intentionally incorrect inputs:
   - Missing semicolons
   - Unmatched parentheses/braces
   - Unknown type
   - Function missing closing brace
   - Strange/extra operators (`#`)
   - Consecutive operators that are invalid
   - Unterminated string

## Running Tests

1. **Compile** your parser with `make`.
2. **Run** the parser executable and redirect or feed `test_valid.txt` line by line. Confirm no errors occur.
3. **Run** with `test_invalid.txt`; confirm the parser prints error messages.

## Additional Tests

We also have an integrated test runner in `main()` that uses a `testInputs[]` array of strings. Each item is fed to the parser. This checks how your code handles each scenario from ephemeral input strings. You can add or remove items in `testInputs[]` to adapt to your needs.

## Expanding Coverage

You can create advanced tests involving:
- Deeply nested function calls
- Float literals with multiple digits
- Edge cases for repeating loops
- Larger function bodies with many statements
- Or any additional language features you add.