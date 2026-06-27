# raw

language · cross-platform · raw

raw is a c-like systems programming language with a hand-written compiler in c++. targets x86_64 linux and windows.

## build

```bash
git clone https://github.com/kroown/raw.git
cd raw
cmake -B build
cmake --build build
sudo cmake --install build   # linux
# or add build/Debug/rawc.exe to PATH on windows
```

**windows note:** requires [MSYS2](https://www.msys2.org) or [MinGW-w64](https://www.mingw-w64.org) for assembling/linking (rawc generates GAS assembly and uses `gcc`).

## usage

```bash
rawc input.raw -o output
./output        # linux
output.exe      # windows
```

## examples

hello world:

```
fn main() -> int {
    print("hello world\n");
    return 0;
}
```

variables and math:

```
fn main() -> int {
    let x: int = 42;
    let y: int = 10;
    let z: int = x + y;
    return z;
}
```

if/else:

```
fn main() -> int {
    let x: int = 10;
    if x > 5 {
        print("big\n");
    } else {
        print("small\n");
    }
    return 0;
}
```

while loop:

```
fn main() -> int {
    let i: int = 0;
    while i < 5 {
        print("*\n");
        i = i + 1;
    }
    return 0;
}
```

for loop and compound assignment:

```
fn main() -> int {
    for let i: int = 0; i < 5; i = i + 1 {
        print("loop\n");
    }
    let x: int = 10;
    x += 5;
    x *= 2;
    print("done\n");
    return x;
}
```

## language

- c-style syntax with `fn` for functions
- static types: `int`, `char`, `bool`, `str`, `void`
- `let` for variable declarations
- `print()` builtin for output (integers and strings)
- if/else, while, for loops
- arithmetic (`+`, `-`, `*`, `/`, `%`), comparison, boolean, and logical (`&&`, `||`) operators
- pointers, arrays, compound assignment (`+=`, `-=`, `*=`, `/=`)
- `extern` blocks for FFI (e.g. SDL2)
- recursive functions

## architecture

```
src/
  main.cpp    — entry, file i/o, assemble & link
  lexer.cpp   — tokenizer
  parser.cpp  — recursive descent parser
  codegen.cpp — x86_64 assembly generation
  token.hpp   — token type definitions
  ast.hpp     — ast node definitions
```

the compiler pipeline: source → lexer → tokens → parser → ast → codegen → assembly → `gcc` → executable.
