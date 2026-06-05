#pragma once
#include <memory>
#include <string>
#include <vector>
#include "lexer.h"

// Forward declarations
struct IntLiteralExpr;
struct BoolLiteralExpr;
struct VariableExpr;
struct BinaryOpExpr;
struct UnaryOpExpr;
struct AssignExpr;
struct InputExpr;

struct ExprStmt;
struct PrintStmt;
struct LetStmt;
struct BlockStmt;
struct IfStmt;
struct WhileStmt;

// ── Visitor interfaces ────────────────────────────────────────────────────────
struct ExprVisitor {
    virtual ~ExprVisitor() = default;
    virtual void visit(IntLiteralExpr&)  = 0;
    virtual void visit(BoolLiteralExpr&) = 0;
    virtual void visit(VariableExpr&)    = 0;
    virtual void visit(BinaryOpExpr&)    = 0;
    virtual void visit(UnaryOpExpr&)     = 0;
    virtual void visit(AssignExpr&)      = 0;
    virtual void visit(InputExpr&)       = 0;
};

struct StmtVisitor {
    virtual ~StmtVisitor() = default;
    virtual void visit(ExprStmt&)  = 0;
    virtual void visit(PrintStmt&) = 0;
    virtual void visit(LetStmt&)   = 0;
    virtual void visit(BlockStmt&) = 0;
    virtual void visit(IfStmt&)    = 0;
    virtual void visit(WhileStmt&) = 0;
};

// ── Expr nodes ────────────────────────────────────────────────────────────────
struct Expr {
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor& v) = 0;
};
using ExprPtr = std::unique_ptr<Expr>;

struct IntLiteralExpr : Expr {
    int64_t value;
    explicit IntLiteralExpr(int64_t v) : value(v) {}
    void accept(ExprVisitor& v) override { v.visit(*this); }
};

struct BoolLiteralExpr : Expr {
    bool value;
    explicit BoolLiteralExpr(bool v) : value(v) {}
    void accept(ExprVisitor& v) override { v.visit(*this); }
};

struct VariableExpr : Expr {
    std::string name;
    explicit VariableExpr(std::string n) : name(std::move(n)) {}
    void accept(ExprVisitor& v) override { v.visit(*this); }
};

struct BinaryOpExpr : Expr {
    TokenType op;
    ExprPtr   left, right;
    BinaryOpExpr(TokenType op, ExprPtr l, ExprPtr r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
    void accept(ExprVisitor& v) override { v.visit(*this); }
};

struct UnaryOpExpr : Expr {
    TokenType op;
    ExprPtr   operand;
    UnaryOpExpr(TokenType op, ExprPtr operand)
        : op(op), operand(std::move(operand)) {}
    void accept(ExprVisitor& v) override { v.visit(*this); }
};

struct AssignExpr : Expr {
    std::string name;
    ExprPtr     value;
    AssignExpr(std::string n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}
    void accept(ExprVisitor& v) override { v.visit(*this); }
};

struct InputExpr : Expr {
    void accept(ExprVisitor& v) override { v.visit(*this); }
};

// ── Stmt nodes ────────────────────────────────────────────────────────────────
struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& v) = 0;
};
using StmtPtr = std::unique_ptr<Stmt>;

struct ExprStmt : Stmt {
    ExprPtr expr;
    explicit ExprStmt(ExprPtr e) : expr(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

struct PrintStmt : Stmt {
    ExprPtr expr;
    explicit PrintStmt(ExprPtr e) : expr(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

struct LetStmt : Stmt {
    std::string name;
    ExprPtr     initializer;
    LetStmt(std::string n, ExprPtr init) : name(std::move(n)), initializer(std::move(init)) {}
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> stmts;
    explicit BlockStmt(std::vector<StmtPtr> s) : stmts(std::move(s)) {}
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

struct IfStmt : Stmt {
    ExprPtr  condition;
    StmtPtr  thenBranch;
    StmtPtr  elseBranch; // may be nullptr
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
    WhileStmt(ExprPtr c, StmtPtr b) : condition(std::move(c)), body(std::move(b)) {}
    void accept(StmtVisitor& v) override { v.visit(*this); }
};