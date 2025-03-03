

States:
    Start Symbols
    S': ES' | EOF // S is EOF A is just anything else

    Expression Handling
    E: E DELIMITER | E OPERATOR E
    E': C | K
    C: LITERAL | IDENTIFIER // for constants

    Keyword Handling | Allows Nesting
    K: KEYWORD DELIMITER K DELIMITER | K'
    K': K' DELIMITER K' | E 

    

Terminals:
    EOF,
    LITERAL,
    OPERATOR,
    KEYWORD,
    IDENTIFIER,
    DELIMITER,