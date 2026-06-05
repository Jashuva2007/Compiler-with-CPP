#include "lexer.h"
#include <unordered_map>
#include <cctype>

static const std::unordered_map<std::string, TokenType> kKeywords = {
    {"let",   TokenType::LET},
    {"if",    TokenType::IF},
    {"else",  TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"print", TokenType::PRINT},
    {"input", TokenType::INPUT},
    {"true",  TokenType::BOOL_LIT},
    {"false", TokenType::BOOL_LIT},
};

Lexer::Lexer(std::string source) : m_source(std::move(source)) {}

char Lexer::advance()        { return m_source[m_current++]; }
char Lexer::peek()    const  { return isAtEnd() ? '\0' : m_source[m_current]; }
char Lexer::peekNext() const { return (m_current + 1 >= m_source.size()) ? '\0' : m_source[m_current + 1]; }
bool Lexer::isAtEnd() const  { return m_current >= m_source.size(); }

bool Lexer::match(char expected) {
    if (isAtEnd() || m_source[m_current] != expected) return false;
    ++m_current;
    return true;
}

Token Lexer::makeToken(TokenType type) const {
    return {type, m_source.substr(m_start, m_current - m_start), m_line};
}

Token Lexer::errorToken(const std::string& msg) const {
    return {TokenType::ERROR, msg, m_line};
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == '\n') { ++m_line; advance(); }
        else if (std::isspace(static_cast<unsigned char>(c))) { advance(); }
        else if (c == '/' && peekNext() == '/') {
            while (!isAtEnd() && peek() != '\n') advance();
        } else break;
    }
}

Token Lexer::scanNumber() {
    while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
    return makeToken(TokenType::INTEGER_LIT);
}

Token Lexer::scanIdentifierOrKeyword() {
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') advance();
    std::string word = m_source.substr(m_start, m_current - m_start);
    auto it = kKeywords.find(word);
    return makeToken(it != kKeywords.end() ? it->second : TokenType::IDENTIFIER);
}

Token Lexer::scanToken() {
    skipWhitespaceAndComments();
    m_start = m_current;
    if (isAtEnd()) return makeToken(TokenType::EOF_TOK);

    char c = advance();
    if (std::isdigit(static_cast<unsigned char>(c))) return scanNumber();
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') return scanIdentifierOrKeyword();

    switch (c) {
        case '+': return makeToken(TokenType::PLUS);
        case '-': return makeToken(TokenType::MINUS);
        case '*': return makeToken(TokenType::STAR);
        case '/': return makeToken(TokenType::SLASH);
        case '(': return makeToken(TokenType::LPAREN);
        case ')': return makeToken(TokenType::RPAREN);
        case '{': return makeToken(TokenType::LBRACE);
        case '}': return makeToken(TokenType::RBRACE);
        case ';': return makeToken(TokenType::SEMICOLON);
        case '=': return makeToken(match('=') ? TokenType::EQ_EQ   : TokenType::ASSIGN);
        case '!': return makeToken(match('=') ? TokenType::BANG_EQ  : TokenType::BANG);
        case '<': return makeToken(match('=') ? TokenType::LESS_EQ  : TokenType::LESS);
        case '>': return makeToken(match('=') ? TokenType::GREATER_EQ : TokenType::GREATER);
        default:  return errorToken(std::string("Unknown character '") + c + "'");
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token t = scanToken();
        tokens.push_back(t);
        if (t.type == TokenType::EOF_TOK || t.type == TokenType::ERROR) break;
    }
    return tokens;
}