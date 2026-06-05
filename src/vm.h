#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "chunk.h"
#include "value.h"

enum class InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

class VM {
public:
    InterpretResult run(Chunk& chunk);

private:
    Chunk*           m_chunk{nullptr};
    const uint8_t*   m_ip{nullptr};
    std::vector<Value>                   m_stack;
    std::unordered_map<std::string,Value> m_globals;

    uint8_t  readByte();
    uint16_t readShort();
    Value    readConstant();
    std::string readName();

    void push(Value v);
    Value pop();
    Value peek(int distance = 0) const;

    void runtimeError(const std::string& msg);
};