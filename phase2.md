# Phase 1: Lexical Analysis

## Introduction
Welcome to the first phase of your compiler construction journey! This initial phase focuses on building a lexical analyzer, commonly known as a lexer - the fundamental component that transforms raw source code into meaningful tokens. Your lexer will scan the input character by character, identifying patterns and converting them into tokens representing language constructs like keywords, identifiers, and operators.

## Project Overview
While we provide a suggested project structure below, you can organize your code as you see fit. The key components remain the same:

```plaintext
my-mini-compiler/
├── CMakeLists.txt              # Build configuration
└── phase1-w25/                 # Phase 1 implementation
    ├── include/
    │   └── tokens.h           # Token definitions and types
    └── src/
        └── lexer/
            └── lexer.c        # Core lexer implementation
```

The `tokens.h` header file contains the definitions for different token types your lexer will recognize, while `lexer.c` houses the main implementation of your lexical analyzer. As you progress, you can extend this foundation to support all the features your target programming language requires.

## Understanding the Starter Lexer Code

### tokens.h
The header file contains basic definitions you'll need to extend:

- Token type definitions (currently handles numbers and operators)
- Error type definitions
- Token structure definition

**Think about what additional token types you'll need for your language features.**

### lexer.c
The source file provides:

- Basic number recognition
- Simple operator handling
- Error reporting structure
- Line number tracking

**Study how the existing token recognition works before adding new features.**

## Implementation Strategy

### Planning Your Extensions
Consider how you'll handle each feature:

- Keywords (`if`, `repeat`, `until`)
- Identifiers (variable names)
- String literals
- Additional operators
- Delimiters
- Comments

### Development Approach
1. Study the existing number recognition implementation
2. Plan each new feature before coding
3. Test thoroughly after each addition
4. Consider error cases for each feature

## Testing Strategy
Your lexer should handle various input scenarios. Here are some examples to get you started:

### Test Case 1: Simple Arithmetic

```plaintext
123 + 456 - 789
```

### Test Case 2: Variable Operations

```plaintext
int x = 42;
y = x + 10;
```

### Test Case 3: Error Handling

```plaintext
123 ++ 456
x@ = 10
```

### Testing Tips
- Start with simple cases
- Test each feature individually
- Include error cases
- Try multi-line inputs

## Implementation Guidelines

### Adding New Features
For each new feature:
1. Plan the addition
2. Update token types if needed
3. Implement recognition logic
4. Add error handling
5. Test thoroughly

### String Literals
When implementing string literal handling, consider these key aspects:

#### Basic Recognition:
- Track opening and closing quotes (`"`)
- Handle escape sequences (`\"`, `\n`, `\t`)
- Process character by character until the closing quote

#### Error Handling:
- Check for unterminated strings
- Validate escape sequences
- Handle buffer overflow for long strings

#### Test Cases:

```plaintext
"Basic string"
"String with \"quotes\""
"Unterminated string    // Error case
```

**Remember to update your token types and error codes in `tokens.h` to support string literals and their associated errors.**

### Error Handling
Consider:
- What can go wrong?
- How to detect errors?
- What error messages to display?
- How to recover from errors?

## Key Considerations

### Design Questions
Think about:

- How will you distinguish between identifiers and keywords?
- What makes a valid identifier in your language?
- How will you handle multi-character operators?
- What error messages will be most helpful to users?

### Common Pitfalls
Watch out for:
- Position tracking errors
- Incorrect error handling
- Missing edge cases
- Incomplete token recognition

## Deliverables Checklist

- [ ] Extended token types in `tokens.h`
- [ ] Implemented token recognition in `lexer.c`
- [ ] Added comment handling
- [ ] Added string literal handling
- [ ] Comprehensive error handling
- [ ] Test cases with expected outputs
- [ ] Documentation of grammar, features, `error_codes`, and design decisions

## Building and Testing

### Using CLion
1. Load CMake project
2. Build (`Ctrl+F9`)
3. Run (`Shift+F10`)
4. Test with various inputs
5. Verify output matches expectations

### Visual Studio Code
1. Install VSCode
2. Add C/C++ extension
3. Install MinGW or GCC
4. Compile using terminal:

```bash
gcc lexer.c -o lexer
./lexer
```

**Or do it however you like as long as we get a lexer that works!**

## Tips for Success

- Start small and build incrementally
- Test each feature thoroughly
- Document your changes
- Consider all possible error cases
- Keep your code organized and well-commented
- Add your documentation/ report under the documentation directory

## Final Notes
Remember:

- A good lexer is fundamental to your compiler
- Take time to plan before implementing and work as a team. DO NOT PROCRASTINATE!
- Test thoroughly
- Ask your TAs for help if needed
- Document your decisions

Good luck with your implementation!
