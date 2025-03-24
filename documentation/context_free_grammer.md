

States:
    Start Symbols
    S: A DELIMITER S | EOF          // S is EOF A is just anything else
    
    A: KEYWORD A | C | B | NULL     // covers loop constructs
    B: DELIMITER C DELIMITER        // covers statement after loops or ifs

    C: D | D OPERATOR C             // covers assignment, arithmetic, boolean.
    D: LITERAL | IDENTIFIER         // for constants

Terminals:
    EOF,
    LITERAL,
    OPERATOR,
    KEYWORD,
    IDENTIFIER,
    DELIMITER,