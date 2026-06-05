#pragma once
#include "ast.h"
#include "chunk.h"

class Compiler : public ExprVisitor, public StmtVisitor {
public:
    Chunk compile(const std::vector<StmtPtr>& stmts);

private:
    Chunk m_chunk;
    int   m_currentLine{0};
    bool  m_hadError{false};

    void emit(uint8_t byte);
    void emit(Opcode op);
    void emit(Opcode op, uint8_t operand);
    void emitConstant(Value val);

    // Jump helpers
    size_t emitJump(Opcode jumpOp);
    void   patchJump(size_t offset);
    void   emitLoop(size_t loopStart);

    // Visitors
    void visit(IntLiteralExpr&)  override;
    void visit(BoolLiteralExpr&) override;
    void visit(VariableExpr&)    override;
    void visit(BinaryOpExpr&)    override;
    void visit(UnaryOpExpr&)     override;
    void visit(AssignExpr&)      override;
    void visit(InputExpr&)       override;

    void visit(ExprStmt&)  override;
    void visit(PrintStmt&) override;
    void visit(LetStmt&)   override;
    void visit(BlockStmt&) override;
    void visit(IfStmt&)    override;
    void visit(WhileStmt&) override;

    void compileExpr(Expr& e);
    void compileStmt(Stmt& s);
    uint8_t nameConstant(const std::string& name);
};