# Language Basics

## Comments

```
// single line comment

/*
   block comment
*/
```

## Variables

Variables are declared with `let`, a name, a type annotation, and an optional initializer.

```
let x: int = 42;
let name: str = "raw";
let flag: bool = true;
```

Uninitialized variables default to zero or false.

```
let x: int;   // x is 0
```

Variables are reassigned with `=`:

```
let x: int = 1;
x = 5;
```

## Types

| Type | Description | Size |
|------|-------------|------|
| `int` | 64 bit signed integer | 8 bytes |
| `char` | Single character, stored as int | 8 bytes |
| `bool` | Boolean, `true` or `false` | 8 bytes |
| `str` | String literal, pointer to null terminated bytes | 8 bytes (pointer) |
| `void` | No value, used for function return types | 0 bytes |

### Pointer Types

Append `*` to any type to get a pointer type:

```
let p: int* = &x;      // pointer to int
let q: void* = ...;    // generic pointer
let pp: int** = ...;   // pointer to pointer to int
```

### Array Types

Append `[size]` to any type:

```
let buf: int[10];           // array of 10 ints
let event: int[8];          // array of 8 ints
```

Arrays are accessed with bracket indexing:

```
buf[0] = 42;
let val: int = buf[3];
```

## String Literals

Strings are wrapped in double quotes. Escape sequences are supported:

| Escape | Meaning |
|--------|---------|
| `\n` | Newline |
| `\t` | Tab |
| `\0` | Null byte |
| `\"` | Double quote |
| `\\` | Backslash |

```
print("hello world\n");
print("tab\there\n");
print("she said \"hi\"\n");
```

## Integer Literals

Decimal and hex both work:

```
let x: int = 42;
let h: int = 0xFF;
```

## Print Builtin

`print()` is the only builtin. It takes either a string or an integer:

```
print("hello\n");   // prints a string
print(42);          // prints an integer
```

## Scope

Variables are scoped to the block (`{ }`) they are declared in:

```
fn main() -> int {
    let x: int = 1;
    {
        let y: int = 2;
        print(y);   // ok
    }
    // print(y);    // error: y is not defined here
    return 0;
}
```
