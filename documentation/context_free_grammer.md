

States:
    Start Symbols
    S': K DELIMITER S' | EOF // S is EOF A is just anything else

    Keyword Handling | Allows Nesting
    K: KEYWORD DELIMITER K DELIMITER K | K' // covers block statements 
    K': E | S // covers loop constructs 

    S: DELIMITER S DELIMITER | S'
    S': K' DELIMITER K' | E

    Expression Handling
    E: E' | E' OPERATOR E           // covers assignment, arithmetic, boolean.
    E': LITERAL | IDENTIFIER        // for constants

Terminals:
    EOF,
    LITERAL,
    OPERATOR,
    KEYWORD,
    IDENTIFIER,
    DELIMITER,