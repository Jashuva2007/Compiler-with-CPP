#pragma once
#include <vector>
#include <stdexcept>
#include "lexer.h"
#include "ast.h"

struct ParseError : std::runtime_error {
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::vector<StmtPtr> parse();

private:
    std::vector<Token> m_tokens;
    size_t             m_pos{0};

    // Navigation
    Token& peek();
    Token& previous();
    bool   check(TokenType type);
    bool   isAtEnd();
    Token& advance();
    bool   match(std::initializer_list<TokenType> types);
    Token& consume(TokenType type, const std::string& msg);

    // Statement rules
    StmtPtr         statement();
    StmtPtr         letStmt();
    StmtPtr         ifStmt();
    StmtPtr         whileStmt();
    StmtPtr         printStmt();
    StmtPtr         exprStmt();
    std::unique_ptr<BlockStmt> block();

    // Expression rules (precedence climbing)
    ExprPtr expr();
    ExprPtr assignment();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr primary();
};