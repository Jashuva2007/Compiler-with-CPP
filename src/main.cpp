#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "Cannot open file: " << path << "\n"; std::exit(1); }
    return std::string(std::istreambuf_iterator<char>(f), {});
}

static InterpretResult runSource(const std::string& source, bool debug = false) {
    Lexer  lexer(source);
    auto   tokens = lexer.tokenize();

    // Check for lex errors
    for (auto& t : tokens) {
        if (t.type == TokenType::ERROR) {
            std::cerr << "[lex error] line " << t.line << ": " << t.lexeme << "\n";
            return InterpretResult::COMPILE_ERROR;
        }
    }

    Parser parser(std::move(tokens));
    std::vector<StmtPtr> stmts;
    try {
        stmts = parser.parse();
    } catch (const ParseError& e) {
        std::cerr << "[parse error] " << e.what() << "\n";
        return InterpretResult::COMPILE_ERROR;
    }

    Compiler compiler;
    Chunk    chunk = compiler.compile(stmts);

    if (debug) {
        std::cout << "=== disassembly ===\n";
        for (size_t i = 0; i < chunk.code.size(); ) {
            auto op = static_cast<Opcode>(chunk.code[i]);
            std::cout << i << "\t";
            switch (op) {
                case Opcode::OP_CONSTANT:
                    std::cout << "OP_CONSTANT " << (int)chunk.code[i+1]
                              << " (" << valueToString(chunk.constants[chunk.code[i+1]]) << ")";
                    i += 2; break;
                case Opcode::OP_DEFINE_GLOBAL:
                    std::cout << "OP_DEFINE_GLOBAL '" << chunk.names[chunk.code[i+1]] << "'";
                    i += 2; break;
                case Opcode::OP_GET_GLOBAL:
                    std::cout << "OP_GET_GLOBAL '" << chunk.names[chunk.code[i+1]] << "'";
                    i += 2; break;
                case Opcode::OP_SET_GLOBAL:
                    std::cout << "OP_SET_GLOBAL '" << chunk.names[chunk.code[i+1]] << "'";
                    i += 2; break;
                case Opcode::OP_JUMP_IF_FALSE:
                case Opcode::OP_JUMP:
                case Opcode::OP_LOOP: {
                    uint16_t off = static_cast<uint16_t>((chunk.code[i+1]<<8)|chunk.code[i+2]);
                    std::cout << (op==Opcode::OP_LOOP?"OP_LOOP":op==Opcode::OP_JUMP?"OP_JUMP":"OP_JUMP_IF_FALSE")
                              << " " << off;
                    i += 3; break;
                }
                default:
                    std::cout << "OP_" << (int)chunk.code[i];
                    ++i; break;
            }
            std::cout << "\n";
        }
        std::cout << "===================\n";
    }

    VM vm;
    return vm.run(chunk);
}

int main(int argc, char* argv[]) {
    bool debug = false;
    std::string filepath;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") debug = true;
        else filepath = arg;
    }

    if (filepath.empty()) {
        // Phase 7 will add full REPL here — for now, simple inline test
        std::string src = "let n = 7;\nprint n * n;";
        runSource(src, debug);
    } else {
        runSource(readFile(filepath), debug);
    }
    return 0;
}