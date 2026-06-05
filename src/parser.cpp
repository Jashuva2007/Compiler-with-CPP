#include "parser.h"
#include <stdexcept>
#include <sstream>

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

Token& Parser::peek()     { return m_tokens[m_pos]; }
Token& Parser::previous() { return m_tokens[m_pos - 1]; }
bool   Parser::isAtEnd()  { return peek().type == TokenType::EOF_TOK; }
bool   Parser::check(TokenType t) { return !isAtEnd() && peek().type == t; }

Token& Parser::advance() {
    if (!isAtEnd()) ++m_pos;
    return previous();
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto t : types) { if (check(t)) { advance(); return true; } }
    return false;
}

Token& Parser::consume(TokenType type, const std::string& msg) {
    if (check(type)) return advance();
    throw ParseError("Line " + std::to_string(peek().line) + ": " + msg +
                     " (got '" + peek().lexeme + "')");
}

// ── Statements ────────────────────────────────────────────────────────────────
std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> stmts;
    while (!isAtEnd()) stmts.push_back(statement());
    return stmts;
}

StmtPtr Parser::statement() {
    if (match({TokenType::LET}))   return letStmt();
    if (match({TokenType::IF}))    return ifStmt();
    if (match({TokenType::WHILE})) return whileStmt();
    if (match({TokenType::PRINT})) return printStmt();
    return exprStmt();
}

StmtPtr Parser::letStmt() {
    Token& name = consume(TokenType::IDENTIFIER, "Expected variable name after 'let'");
    consume(TokenType::ASSIGN, "Expected '=' after variable name");
    ExprPtr init = expr();
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<LetStmt>(name.lexeme, std::move(init));
}

StmtPtr Parser::ifStmt() {
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    ExprPtr cond = expr();
    consume(TokenType::RPAREN, "Expected ')' after if condition");
    StmtPtr thenBranch = std::make_unique<BlockStmt>(std::vector<StmtPtr>());
    // allow single-stmt or block
    if (check(TokenType::LBRACE)) thenBranch = block();
    else thenBranch = statement();

    StmtPtr elseBranch = nullptr;
    if (match({TokenType::ELSE})) {
        if (check(TokenType::LBRACE)) elseBranch = block();
        else elseBranch = statement();
    }
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch));
}

StmtPtr Parser::whileStmt() {
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    ExprPtr cond = expr();
    consume(TokenType::RPAREN, "Expected ')' after while condition");
    StmtPtr body = check(TokenType::LBRACE) ? block() : statement();
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

StmtPtr Parser::printStmt() {
    ExprPtr value = expr();
    consume(TokenType::SEMICOLON, "Expected ';' after print expression");
    return std::make_unique<PrintStmt>(std::move(value));
}

StmtPtr Parser::exprStmt() {
    ExprPtr e = expr();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<ExprStmt>(std::move(e));
}

std::unique_ptr<BlockStmt> Parser::block() {
    consume(TokenType::LBRACE, "Expected '{'");
    std::vector<StmtPtr> stmts;
    while (!check(TokenType::RBRACE) && !isAtEnd()) stmts.push_back(statement());
    consume(TokenType::RBRACE, "Expected '}'");
    return std::make_unique<BlockStmt>(std::move(stmts));
}

// ── Expressions ───────────────────────────────────────────────────────────────
ExprPtr Parser::expr()       { return assignment(); }

ExprPtr Parser::assignment() {
    ExprPtr left = equality();
    if (match({TokenType::ASSIGN})) {
        auto* var = dynamic_cast<VariableExpr*>(left.get());
        if (!var) throw ParseError("Invalid assignment target");
        return std::make_unique<AssignExpr>(var->name, expr());
    }
    return left;
}

ExprPtr Parser::equality() {
    ExprPtr left = comparison();
    while (match({TokenType::EQ_EQ, TokenType::BANG_EQ})) {
        TokenType op = previous().type;
        left = std::make_unique<BinaryOpExpr>(op, std::move(left), comparison());
    }
    return left;
}

ExprPtr Parser::comparison() {
    ExprPtr left = term();
    while (match({TokenType::LESS, TokenType::LESS_EQ, TokenType::GREATER, TokenType::GREATER_EQ})) {
        TokenType op = previous().type;
        left = std::make_unique<BinaryOpExpr>(op, std::move(left), term());
    }
    return left;
}

ExprPtr Parser::term() {
    ExprPtr left = factor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        TokenType op = previous().type;
        left = std::make_unique<BinaryOpExpr>(op, std::move(left), factor());
    }
    return left;
}

ExprPtr Parser::factor() {
    ExprPtr left = unary();
    while (match({TokenType::STAR, TokenType::SLASH})) {
        TokenType op = previous().type;
        left = std::make_unique<BinaryOpExpr>(op, std::move(left), unary());
    }
    return left;
}

ExprPtr Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        TokenType op = previous().type;
        return std::make_unique<UnaryOpExpr>(op, unary());
    }
    return primary();
}

ExprPtr Parser::primary() {
    if (match({TokenType::INTEGER_LIT}))
        return std::make_unique<IntLiteralExpr>(std::stoll(previous().lexeme));
    if (match({TokenType::BOOL_LIT}))
        return std::make_unique<BoolLiteralExpr>(previous().lexeme == "true");
    if (match({TokenType::IDENTIFIER}))
        return std::make_unique<VariableExpr>(previous().lexeme);
    if (match({TokenType::INPUT}))
        return std::make_unique<InputExpr>();
    if (match({TokenType::LPAREN})) {
        ExprPtr e = expr();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return e;
    }
    throw ParseError("Line " + std::to_string(peek().line) +
                     ": Unexpected token '" + peek().lexeme + "'");
}