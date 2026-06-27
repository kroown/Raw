# raw

language · linux · raw

<p align="center">
  <img src="https://skillicons.dev/icons?i=linux,cpp">
</p>

raw is a c-like systems programming language with a hand-written compiler in c++. targets x86_64 linux.

## build

```bash
git clone https://github.com/kroown/raw.git
cd raw
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

## usage

```bash
rawc input.raw -o output
./output
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

## language

- c-style syntax with `fn` for functions
- static types: `int`, `char`, `bool`, `str`, `void`
- `let` for variable declarations
- `print()` builtin for output
- if/else, while loops
- arithmetic, comparison, and boolean operators
- recursive functions (pending proper stack management)

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

the compiler pipeline: source → lexer → tokens → parser → ast → codegen → assembly → `as` → object → `ld` → executable.


