

States:
    Start Symbols
    S': K DELIMITER S' | EOF                            // S is EOF A is just anything else
    
    K: KEYWORD K | E | S | NULL   // covers loop constructs
    S: DELIMITER E DELIMITER                            // covers statement after loops or ifs

    E: V | V OPERATOR E                   // covers assignment, arithmetic, boolean.
    V: LITERAL | IDENTIFIER                // for constants

Terminals:
    EOF,
    LITERAL,
    OPERATOR,
    KEYWORD,
    IDENTIFIER,
    DELIMITER,