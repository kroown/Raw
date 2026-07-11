# Raw Programming Language

Raw is a statically typed, C-like systems programming language with a hand-written compiler in C++. It targets x86_64 on Linux and Windows.

## Quick Start

```bash
rawc hello.raw -o hello
./hello        # Linux
hello.exe      # Windows
```

## Hello World

```
fn main() -> int {
    print("hello world\n");
    return 0;
}
```

## Documentation

* [Language Basics](language_basics.md) syntax, variables, types
* [Control Flow](control_flow.md) if/else, while, for, break/continue
* [Functions](functions.md) declarations, parameters, return values, recursion
* [Operators](operators.md) arithmetic, comparison, logical, assignment, unary
* [FFI (extern)](ffi.md) calling C functions from Raw
* [Examples](examples.md) walkthrough of all example programs

## Language Overview

* C style syntax using `fn` for function declarations
* Static types: `int`, `char`, `bool`, `str`, `void`
* `let` for variable declarations
* `print()` builtin for output (strings and integers)
* Control flow: `if`/`else`, `while`, `for`
* Arithmetic: `+`, `-`, `*`, `/`, `%`
* Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
* Logical: `&&`, `||`, `!`
* Pointers: `&` (address of), `*` (dereference), pointer types like `int*`, `void*`
* Arrays: `int[10]`, indexed with `arr[i]`
* Compound assignment: `+=`, `-=`, `*=`, `/=`
* Increment/decrement: `++`, `--`
* FFI: `extern` blocks for calling C libraries like SDL2
* Recursion fully supported

## Architecture

```
source (.raw) -> Lexer -> Tokens -> Parser -> AST -> Codegen -> Assembly (.s) -> gcc -> executable
```

The compiler (`rawc`) is a single pass pipeline. It lexes, parses, and generates x86_64 GAS assembly, then shells out to `gcc` to assemble and link.

### File Layout

| File | Purpose |
|------|---------|
| `main.cpp` | Entry point, file I/O, assemble and link |
| `lexer.cpp/hpp` | Tokenizer, turns source into tokens |
| `parser.cpp/hpp` | Recursive descent parser, tokens into AST |
| `codegen.cpp/hpp` | x86_64 assembly generation, AST into `.s` |
| `token.hpp` | Token type definitions |
| `ast.hpp` | AST node definitions |
