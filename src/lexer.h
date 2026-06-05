#pragma once
#include <string>
#include <vector>
#include <string_view>

enum class TokenType {
    // Literals
    INTEGER_LIT, BOOL_LIT, IDENTIFIER,
    // Operators
    PLUS, MINUS, STAR, SLASH,
    EQ_EQ, BANG_EQ, LESS, LESS_EQ, GREATER, GREATER_EQ,
    ASSIGN, BANG,
    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON,
    // Keywords
    LET, IF, ELSE, WHILE, PRINT, INPUT,
    // Meta
    EOF_TOK, ERROR
};

struct Token {
    TokenType   type;
    std::string lexeme;   // owned copy — safe after source is gone
    int         line{1};
};

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string m_source;
    size_t      m_start{0};
    size_t      m_current{0};
    int         m_line{1};

    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    bool isAtEnd() const;
    void skipWhitespaceAndComments();
    Token makeToken(TokenType type) const;
    Token errorToken(const std::string& msg) const;
    Token scanToken();
    Token scanNumber();
    Token scanIdentifierOrKeyword();
};