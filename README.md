# CVM++

A from-scratch bytecode virtual machine written in C++17. Source text flows through a lexer, recursive-descent parser, single-pass compiler, and stack-based execution engine — with zero external dependencies.

> Reference architecture: [Crafting Interpreters](https://craftinginterpreters.com) by Robert Nystrom (Part III — clox), ported and simplified into C++17 with CMake.

---

## Pipeline architecture

```
source text
    │
    ▼
┌─────────┐    vector<Token>    ┌─────────┐    AST nodes    ┌──────────┐    Chunk    ┌────────┐
│  Lexer  │ ─────────────────▶ │ Parser  │ ──────────────▶ │ Compiler │ ──────────▶ │   VM   │
└─────────┘                    └─────────┘                  └──────────┘             └────────┘
lexer.h/cpp                   parser.h/cpp                compiler.h/cpp            vm.h/cpp
                               ast.h                       chunk.h
                                                           value.h
```

Each stage is a self-contained header/source pair. The VM executes a flat `uint8_t` bytecode array in a single `switch` dispatch loop.

---

## Prerequisites

| Tool | Minimum version |
|------|----------------|
| C++ compiler | GCC 8 / Clang 7 / MSVC 2019 (C++17) |
| CMake | 3.16 |

No third-party libraries required — standard library only.

---

## Quick start

```bash
git clone https://github.com/your-username/cvm.git
cd cvm
cmake -S . -B build
cmake --build build
./build/cvm examples/hello.cvm
```

Expected output:
```
16
```

---

## Build

```bash
# Configure (debug symbols)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Configure (optimised)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Clean rebuild (required after changing the Opcode enum)
cmake --build build --clean-first
```

The single CMake target is `cvm`. All `.cpp` files under `src/` are picked up automatically via `file(GLOB_RECURSE)`.

---

## Usage

### Run a `.cvm` file

```bash
./build/cvm path/to/program.cvm
```

### Interactive REPL

```bash
./build/cvm
```

```
CVM++ v0.1  (ctrl-d to exit)
cvm> let x = 6;
cvm> print x * 7;
42
cvm> 
```

Each line is compiled and executed independently. Variables defined in one line persist for the session.

### Disassemble bytecode

```bash
./build/cvm --debug examples/counter.cvm
```

Prints a human-readable opcode listing before executing:

```
=== disassembly ===
0    OP_CONSTANT 0 (0)
2    OP_DEFINE_GLOBAL 'i'
...
===================
0
1
2
3
4
```

---

## Language reference

### Types

| Type | Literals | Example |
|------|----------|---------|
| Integer | decimal digits | `42`, `-7`, `0` |
| Boolean | keywords | `true`, `false` |

### Operators

| Category | Operators | Notes |
|----------|-----------|-------|
| Arithmetic | `+` `-` `*` `/` | integers only; `/` truncates toward zero |
| Comparison | `<` `<=` `>` `>=` | integers only; result is bool |
| Equality | `==` `!=` | any two values of the same type |
| Logical | `!` | boolean operand; prefix unary |
| Assignment | `=` | updates an existing variable; expression |

Operator precedence (highest to lowest): `!` / unary `-` → `*` `/` → `+` `-` → `<` `<=` `>` `>=` → `==` `!=` → `=`

### Statements

#### Variable declaration

```
let name = expression;
```

Declares a new global variable. Re-declaring an existing name creates a new binding.

#### Assignment

```
name = expression;
```

Updates an existing variable. Assigning to an undeclared name is a runtime error.

#### Print

```
print expression;
```

Evaluates the expression, converts the result to a string, and writes it to stdout followed by a newline.

#### If / else

```
if (condition) {
    // then branch
}

if (condition) {
    // then branch
} else {
    // else branch
}
```

The condition must evaluate to a boolean. Braces are required.

#### While loop

```
while (condition) {
    // body
}
```

Executes the body repeatedly as long as the condition is true.

#### Input

```
let n = input;
```

Reads one integer from stdin and produces it as a value. Can appear anywhere an expression is valid.

### Comments

```
// This is a line comment — everything after // is ignored
```

Block comments are not supported.

---

## Example programs

### Arithmetic and variables

```
let x = 10;
let y = 3;
print x + y * 2;    // prints 16
```

### If / else

```
let a = 5;
if (a < 10) {
    print true;
} else {
    print false;
}
// prints true
```

### While loop

```
let i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
// prints 0 1 2 3 4
```

### Read from stdin

```
let n = input;
print n * n;
// reads an integer, prints its square
```

### Iterative Fibonacci

```
let a = 0;
let b = 1;
let i = 0;
while (i < 10) {
    print a;
    let tmp = a + b;
    a = b;
    b = tmp;
    i = i + 1;
}
// prints first 10 Fibonacci numbers
```

---

## Project structure

```
cvm/
├── CMakeLists.txt          single target: cvm
├── README.md
├── .gitignore
├── src/
│   ├── main.cpp            REPL loop + file runner + --debug flag
│   ├── lexer.h             Token types, Lexer class declaration
│   ├── lexer.cpp           Character-by-character scanner
│   ├── ast.h               Expr/Stmt node types, Visitor interfaces (header-only)
│   ├── parser.h            Parser class declaration
│   ├── parser.cpp          Recursive-descent parser
│   ├── value.h             Value = std::variant<int64_t, bool> (header-only)
│   ├── chunk.h             Chunk struct, Opcode enum (header-only)
│   ├── compiler.h          Compiler class declaration
│   ├── compiler.cpp        AST → bytecode visitor
│   ├── vm.h                VM struct declaration
│   └── vm.cpp              Dispatch loop execution engine
└── examples/
    ├── hello.cvm           Arithmetic and variables smoke test
    ├── counter.cvm         While loop 0–4
    └── fib.cvm             Iterative Fibonacci, first 10 terms
```

---

## Build phases

The project was assembled in 7 incremental commits, each leaving the repo in a clean, compilable state:

| Phase | Commit message | Gate test |
|-------|---------------|-----------|
| 0 | `chore: project scaffold, CMake, .gitignore` | `cmake --build build` succeeds |
| 1 | `feat(lexer): tokenize source → flat Token vector` | `let x = 10;` → correct token list |
| 2 | `feat(parser): recursive-descent parser, full AST` | `print 2 + 3;` → correct AST dump |
| 3 | `feat(compiler,vm): value types, chunk, dispatch loop` | `print 2 + 3 * 4;` → `14` |
| 4 | `feat(vars): OP_DEFINE/GET/SET_GLOBAL, globals map` | `let x=10; x=x+1; print x;` → `11` |
| 5 | `feat(control-flow): if/else, while, jump patching` | while counter loop prints `0 1 2 3 4` |
| 6 | `feat(io): OP_PRINT/OP_INPUT, file runner, --debug` | `./build/cvm examples/hello.cvm` → `16` |
| 7 | `feat(repl): interactive REPL, argc-based dispatch` | `./build/cvm` opens prompt |

---

## Opcodes

| Opcode | Operands | Stack effect | Description |
|--------|----------|-------------|-------------|
| `OP_CONSTANT` | `idx` (1 byte) | +1 | Push `constants[idx]` |
| `OP_TRUE` / `OP_FALSE` | — | +1 | Push boolean literal |
| `OP_ADD` / `OP_SUB` / `OP_MUL` / `OP_DIV` | — | −1 | Pop two integers, push result |
| `OP_EQUAL` / `OP_LESS` / `OP_GREATER` | — | −1 | Pop two values, push bool |
| `OP_NOT` | — | 0 | Negate boolean on top of stack |
| `OP_NEGATE` | — | 0 | Negate integer on top of stack |
| `OP_DEFINE_GLOBAL` | `idx` (1 byte) | −1 | Pop value, define `names[idx]` in globals |
| `OP_GET_GLOBAL` | `idx` (1 byte) | +1 | Push value of `names[idx]` from globals |
| `OP_SET_GLOBAL` | `idx` (1 byte) | 0 | Peek top, store to `names[idx]` in globals |
| `OP_PRINT` | — | −1 | Pop and print to stdout |
| `OP_INPUT` | — | +1 | Read integer from stdin, push |
| `OP_JUMP_IF_FALSE` | `offset` (2 bytes) | 0 | Skip `offset` bytes if top of stack is false |
| `OP_JUMP` | `offset` (2 bytes) | 0 | Unconditional forward skip |
| `OP_LOOP` | `offset` (2 bytes) | 0 | Unconditional backward jump (while) |
| `OP_POP` | — | −1 | Discard top of stack |
| `OP_RETURN` | — | — | Halt execution |

Jump offsets are big-endian 16-bit unsigned integers. They are relative to the byte immediately after the two operand bytes.

---

## Stretch goals

The following extensions slot cleanly onto the existing architecture:

- **Float support** — extend `Value = std::variant<int64_t, double, bool>`
- **String literals** — add `StringValue`, string `+` concatenation, `print` formatting
- **Disassembler** — walk `chunk.code` and print human-readable opcode names (partial implementation already in `main.cpp --debug`)
- **Local variables** — replace the globals hashmap with stack-slot locals (clox Chapter 22 style)
- **User-defined functions** — `OP_CALL` / `OP_RETURN` with a `CallFrame` stack

---

## Common pitfalls

**Off-by-one in jump offsets.** The offset stored by `OP_JUMP_IF_FALSE` and `OP_JUMP` is relative to the byte *after* the two operand bytes, not the opcode itself. Sketch the byte layout before modifying `patchJump()`.

**Stack imbalance.** Every expression must leave exactly `+1` on the stack; every statement must leave `±0`. Enable `DEBUG_TRACE_EXECUTION` in `vm.cpp` to print the stack after each instruction during development.

**Global variable lifetime.** The `globals` map uses `std::string` keys — not `string_view`. Lexemes are views into the source buffer which may be deallocated before the VM runs.

**`std::variant` access.** Always check `std::holds_alternative` before `std::get`. A type mismatch throws `std::bad_variant_access` which terminates the process with no useful message.

**Stale object files.** After changing the `Opcode` enum values or order, always do a clean rebuild: `cmake --build build --clean-first`. Stale `.o` files produce confusing linker errors.

---

## Contributing

1. Fork the repository and create a feature branch.
2. Follow the existing code style — C++17, no macros except `BINARY_OP`, no external dependencies.
3. Each commit should leave the project in a compilable state with all example programs producing correct output.
4. Open a pull request with a short description of the change and the gate test you verified.

---
