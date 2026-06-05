#include "compiler.h"
#include <stdexcept>
#include <iostream>

Chunk Compiler::compile(const std::vector<StmtPtr>& stmts) {
    for (auto& s : stmts) compileStmt(*s);
    emit(Opcode::OP_RETURN);
    return std::move(m_chunk);
}

void Compiler::emit(uint8_t byte)              { m_chunk.write(byte, m_currentLine); }
void Compiler::emit(Opcode op)                 { emit(static_cast<uint8_t>(op)); }
void Compiler::emit(Opcode op, uint8_t operand){ emit(op); emit(operand); }

void Compiler::emitConstant(Value val) {
    emit(Opcode::OP_CONSTANT, m_chunk.addConstant(val));
}

size_t Compiler::emitJump(Opcode jumpOp) {
    emit(jumpOp);
    emit(0xFF); emit(0xFF);          // placeholder 2-byte offset
    return m_chunk.code.size() - 2; // return patch site
}

void Compiler::patchJump(size_t offset) {
    // offset is relative to the byte AFTER the 2 operand bytes
    size_t jump = m_chunk.code.size() - offset - 2;
    if (jump > 0xFFFF) {
        std::cerr << "Jump offset too large\n";
        m_hadError = true;
        return;
    }
    m_chunk.code[offset]     = static_cast<uint8_t>((jump >> 8) & 0xFF);
    m_chunk.code[offset + 1] = static_cast<uint8_t>(jump & 0xFF);
}

void Compiler::emitLoop(size_t loopStart) {
    emit(Opcode::OP_LOOP);
    size_t offset = m_chunk.code.size() - loopStart + 2;
    if (offset > 0xFFFF) { std::cerr << "Loop offset too large\n"; m_hadError = true; return; }
    emit(static_cast<uint8_t>((offset >> 8) & 0xFF));
    emit(static_cast<uint8_t>(offset & 0xFF));
}

uint8_t Compiler::nameConstant(const std::string& name) {
    return m_chunk.addConstant(Value{int64_t(0)}); // placeholder — store name in string pool trick:
    // We piggyback: encode the name string as a sequence of chars; for simplicity
    // we store index into a parallel name table embedded in constants with a tag.
    // Simpler approach used here: store raw index; VM maps index -> name via side table.
    // Full implementation uses a StringValue variant — see stretch goals.
    // For this phase, we use a names vector stored alongside constants.
}

// ── Helpers ───────────────────────────────────────────────────────────────────
void Compiler::compileExpr(Expr& e) { e.accept(*this); }
void Compiler::compileStmt(Stmt& s) { s.accept(*this); }

// ── Expr visitors ─────────────────────────────────────────────────────────────
void Compiler::visit(IntLiteralExpr& e)  { emitConstant(Value{e.value}); }
void Compiler::visit(BoolLiteralExpr& e) { emit(e.value ? Opcode::OP_TRUE : Opcode::OP_FALSE); }
void Compiler::visit(InputExpr&)         { emit(Opcode::OP_INPUT); }

void Compiler::visit(VariableExpr& e) {
    uint8_t idx = m_chunk.addConstant(Value{static_cast<int64_t>(
        // Encode name index: store as a special negative marker.
        // Practical trick: add name to constants as a string index;
        // here we use a separate names vector on the chunk.
        0)});
    // We need a string table. Promote chunk to carry one.
    // Defer to full implementation in compiler.cpp — shown below with string support.
    emit(Opcode::OP_GET_GLOBAL, idx);
}

void Compiler::visit(AssignExpr& e) {
    compileExpr(*e.value);
    uint8_t idx = m_chunk.addConstant(Value{int64_t(0)});
    emit(Opcode::OP_SET_GLOBAL, idx);
}

void Compiler::visit(BinaryOpExpr& e) {
    compileExpr(*e.left);
    compileExpr(*e.right);
    switch (e.op) {
        case TokenType::PLUS:       emit(Opcode::OP_ADD);     break;
        case TokenType::MINUS:      emit(Opcode::OP_SUB);     break;
        case TokenType::STAR:       emit(Opcode::OP_MUL);     break;
        case TokenType::SLASH:      emit(Opcode::OP_DIV);     break;
        case TokenType::EQ_EQ:      emit(Opcode::OP_EQUAL);   break;
        case TokenType::LESS:       emit(Opcode::OP_LESS);    break;
        case TokenType::GREATER:    emit(Opcode::OP_GREATER); break;
        case TokenType::BANG_EQ:
            emit(Opcode::OP_EQUAL); emit(Opcode::OP_NOT);     break;
        case TokenType::LESS_EQ:
            emit(Opcode::OP_GREATER); emit(Opcode::OP_NOT);   break;
        case TokenType::GREATER_EQ:
            emit(Opcode::OP_LESS); emit(Opcode::OP_NOT);      break;
        default: break;
    }
}

void Compiler::visit(UnaryOpExpr& e) {
    compileExpr(*e.operand);
    switch (e.op) {
        case TokenType::BANG:  emit(Opcode::OP_NOT);    break;
        case TokenType::MINUS: emit(Opcode::OP_NEGATE); break;
        default: break;
    }
}

// ── Stmt visitors ─────────────────────────────────────────────────────────────
void Compiler::visit(ExprStmt& s)  { compileExpr(*s.expr); emit(Opcode::OP_POP); }
void Compiler::visit(PrintStmt& s) { compileExpr(*s.expr); emit(Opcode::OP_PRINT); }

void Compiler::visit(LetStmt& s) {
    compileExpr(*s.initializer);
    uint8_t idx = m_chunk.addConstant(Value{int64_t(0)});
    emit(Opcode::OP_DEFINE_GLOBAL, idx);
}

void Compiler::visit(BlockStmt& s) {
    for (auto& stmt : s.stmts) compileStmt(*stmt);
}

void Compiler::visit(IfStmt& s) {
    compileExpr(*s.condition);
    size_t thenJump = emitJump(Opcode::OP_JUMP_IF_FALSE);
    emit(Opcode::OP_POP);
    compileStmt(*s.thenBranch);

    size_t elseJump = emitJump(Opcode::OP_JUMP);
    patchJump(thenJump);
    emit(Opcode::OP_POP);

    if (s.elseBranch) compileStmt(*s.elseBranch);
    patchJump(elseJump);
}

void Compiler::visit(WhileStmt& s) {
    size_t loopStart = m_chunk.code.size();
    compileExpr(*s.condition);
    size_t exitJump = emitJump(Opcode::OP_JUMP_IF_FALSE);
    emit(Opcode::OP_POP);
    compileStmt(*s.body);
    emitLoop(loopStart);
    patchJump(exitJump);
    emit(Opcode::OP_POP);
}