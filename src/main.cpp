#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"

static std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::INTEGER_LIT:  return "INTEGER_LIT";
        case TokenType::BOOL_LIT:     return "BOOL_LIT";
        case TokenType::IDENTIFIER:   return "IDENTIFIER";
        case TokenType::PLUS:         return "PLUS";
        case TokenType::MINUS:        return "MINUS";
        case TokenType::STAR:         return "STAR";
        case TokenType::SLASH:        return "SLASH";
        case TokenType::EQ_EQ:        return "EQ_EQ";
        case TokenType::BANG_EQ:      return "BANG_EQ";
        case TokenType::LESS:         return "LESS";
        case TokenType::LESS_EQ:      return "LESS_EQ";
        case TokenType::GREATER:      return "GREATER";
        case TokenType::GREATER_EQ:   return "GREATER_EQ";
        case TokenType::ASSIGN:       return "ASSIGN";
        case TokenType::BANG:         return "BANG";
        case TokenType::LPAREN:       return "LPAREN";
        case TokenType::RPAREN:       return "RPAREN";
        case TokenType::LBRACE:       return "LBRACE";
        case TokenType::RBRACE:       return "RBRACE";
        case TokenType::SEMICOLON:    return "SEMICOLON";
        case TokenType::LET:          return "LET";
        case TokenType::IF:           return "IF";
        case TokenType::ELSE:         return "ELSE";
        case TokenType::WHILE:        return "WHILE";
        case TokenType::PRINT:        return "PRINT";
        case TokenType::INPUT:        return "INPUT";
        case TokenType::EOF_TOK:      return "EOF";
        case TokenType::ERROR:        return "ERROR";
        default:                      return "UNKNOWN";
    }
}

int main() {
    std::string source = "let x = 10;\nlet y = 3;\nprint x + y * 2;";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    for (auto& tok : tokens)
        std::cout << "[line " << tok.line << "] "
                  << tokenTypeName(tok.type) << " '" << tok.lexeme << "'\n";
    return 0;
}