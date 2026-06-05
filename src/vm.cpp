#include "vm.h"
#include <iostream>
#include <stdexcept>

uint8_t  VM::readByte()  { return *m_ip++; }
uint16_t VM::readShort() {
    uint16_t hi = readByte(), lo = readByte();
    return static_cast<uint16_t>((hi << 8) | lo);
}
Value VM::readConstant() { return m_chunk->constants[readByte()]; }
std::string VM::readName() { return m_chunk->names[readByte()]; }

void  VM::push(Value v)  { m_stack.push_back(v); }
Value VM::pop()          { Value v = m_stack.back(); m_stack.pop_back(); return v; }
Value VM::peek(int d) const { return m_stack[m_stack.size() - 1 - d]; }

void VM::runtimeError(const std::string& msg) {
    std::cerr << "[runtime error] " << msg << "\n";
}

InterpretResult VM::run(Chunk& chunk) {
    m_chunk = &chunk;
    m_ip    = chunk.code.data();

    while (true) {
        auto instruction = static_cast<Opcode>(readByte());
        switch (instruction) {

        case Opcode::OP_CONSTANT: push(readConstant()); break;
        case Opcode::OP_TRUE:     push(Value{true});    break;
        case Opcode::OP_FALSE:    push(Value{false});   break;

        case Opcode::OP_ADD: { auto b=pop(); auto a=pop();
            if (!isInt(a)||!isInt(b)){ runtimeError("'+' requires integers"); return InterpretResult::RUNTIME_ERROR; }
            push(Value{asInt(a)+asInt(b)}); break; }
        case Opcode::OP_SUB: { auto b=pop(); auto a=pop();
            if (!isInt(a)||!isInt(b)){ runtimeError("'-' requires integers"); return InterpretResult::RUNTIME_ERROR; }
            push(Value{asInt(a)-asInt(b)}); break; }
        case Opcode::OP_MUL: { auto b=pop(); auto a=pop();
            if (!isInt(a)||!isInt(b)){ runtimeError("'*' requires integers"); return InterpretResult::RUNTIME_ERROR; }
            push(Value{asInt(a)*asInt(b)}); break; }
        case Opcode::OP_DIV: { auto b=pop(); auto a=pop();
            if (!isInt(a)||!isInt(b)){ runtimeError("'/' requires integers"); return InterpretResult::RUNTIME_ERROR; }
            if (asInt(b)==0){ runtimeError("Division by zero"); return InterpretResult::RUNTIME_ERROR; }
            push(Value{asInt(a)/asInt(b)}); break; }

        case Opcode::OP_EQUAL: { auto b=pop(); auto a=pop(); push(Value{a==b}); break; }
        case Opcode::OP_LESS:  { auto b=pop(); auto a=pop();
            if (!isInt(a)||!isInt(b)){ runtimeError("'<' requires integers"); return InterpretResult::RUNTIME_ERROR; }
            push(Value{asInt(a)<asInt(b)}); break; }
        case Opcode::OP_GREATER:{ auto b=pop(); auto a=pop();
            if (!isInt(a)||!isInt(b)){ runtimeError("'>' requires integers"); return InterpretResult::RUNTIME_ERROR; }
            push(Value{asInt(a)>asInt(b)}); break; }

        case Opcode::OP_NOT:    { auto v=pop(); if(!isBool(v)){ runtimeError("'!' requires bool"); return InterpretResult::RUNTIME_ERROR; } push(Value{!asBool(v)}); break; }
        case Opcode::OP_NEGATE: { auto v=pop(); if(!isInt(v)){ runtimeError("'-' requires int"); return InterpretResult::RUNTIME_ERROR; } push(Value{-asInt(v)}); break; }

        case Opcode::OP_DEFINE_GLOBAL: { std::string n=readName(); m_globals[n]=pop(); break; }
        case Opcode::OP_GET_GLOBAL:    { std::string n=readName(); auto it=m_globals.find(n);
            if(it==m_globals.end()){ runtimeError("Undefined variable '"+n+"'"); return InterpretResult::RUNTIME_ERROR; }
            push(it->second); break; }
        case Opcode::OP_SET_GLOBAL:    { std::string n=readName(); auto it=m_globals.find(n);
            if(it==m_globals.end()){ runtimeError("Undefined variable '"+n+"'"); return InterpretResult::RUNTIME_ERROR; }
            it->second = peek(0); break; }  // peek, not pop — assignment is an expression

        case Opcode::OP_PRINT: { std::cout << valueToString(pop()) << "\n"; break; }
        case Opcode::OP_INPUT: { int64_t n; std::cin >> n; push(Value{n}); break; }

        case Opcode::OP_JUMP_IF_FALSE: {
            uint16_t offset = readShort();
            if (isBool(peek()) && !asBool(peek())) m_ip += offset;
            break; }
        case Opcode::OP_JUMP: { uint16_t offset = readShort(); m_ip += offset; break; }
        case Opcode::OP_LOOP: { uint16_t offset = readShort(); m_ip -= offset; break; }

        case Opcode::OP_POP:    pop(); break;
        case Opcode::OP_RETURN: return InterpretResult::OK;

        default:
            runtimeError("Unknown opcode");
            return InterpretResult::RUNTIME_ERROR;
        }
    }
}