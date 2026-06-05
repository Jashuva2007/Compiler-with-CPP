#pragma once
#include <vector>
#include <cstdint>
#include "value.h"

enum class Opcode : uint8_t {
    OP_CONSTANT,
    OP_TRUE, OP_FALSE,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_EQUAL, OP_LESS, OP_GREATER,
    OP_NOT, OP_NEGATE,
    OP_DEFINE_GLOBAL, OP_GET_GLOBAL, OP_SET_GLOBAL,
    OP_PRINT, OP_INPUT,
    OP_JUMP_IF_FALSE, OP_JUMP, OP_LOOP,
    OP_POP, OP_RETURN
};

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value>   constants;
    std::vector<int>     lines;

    std::vector<std::string> names;  // parallel to constants for global var names

    uint8_t addName(const std::string& name) {
        names.push_back(name);
        return static_cast<uint8_t>(names.size() - 1);
    }
    void write(uint8_t byte, int line) {
        code.push_back(byte);
        lines.push_back(line);
    }

    uint8_t addConstant(Value val) {
        constants.push_back(val);
        return static_cast<uint8_t>(constants.size() - 1);
    }
};