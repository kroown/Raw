# Operators

## Operator Precedence (highest to lowest)

| Precedence | Operators | Description |
|-----------|-----------|-------------|
| 1 | `()` `[]` `.` | Grouping, indexing, member access |
| 2 | `++` `--` `!` `-` `&` `*` | Unary operators (post/prefix) |
| 3 | `*` `/` `%` | Multiplicative |
| 4 | `+` `-` | Additive |
| 5 | `<` `<=` `>` `>=` | Comparison |
| 6 | `==` `!=` | Equality |
| 7 | `&&` | Logical AND |
| 8 | `\|\|` | Logical OR |
| 9 | `=` `+=` `-=` `*=` `/=` | Assignment |

## Arithmetic Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `x + y` |
| `-` | Subtraction | `x - y` |
| `*` | Multiplication | `x * y` |
| `/` | Division (signed) | `x / y` |
| `%` | Modulo (signed) | `x % y` |

```
let r: int = (2 + 3) * 4;   // 20
let m: int = 17 % 5;         // 2
```

## Comparison Operators

All comparison operators return `1` (true) or `0` (false).

| Operator | Description |
|----------|-------------|
| `==` | Equal |
| `!=` | Not equal |
| `<` | Less than |
| `<=` | Less than or equal |
| `>` | Greater than |
| `>=` | Greater than or equal |

```
if x == 1 && y == 1 {
    print("both are 1\n");
}
```

## Logical Operators

| Operator | Description |
|----------|-------------|
| `&&` | Logical AND, short circuit not guaranteed |
| `\|\|` | Logical OR, short circuit not guaranteed |
| `!` | Logical NOT |

Treats 0 as false and any non-zero as true.

```
if !flag {
    print("flag is false\n");
}

if a == 1 || b == 1 {
    print("at least one is 1\n");
}
```

## Assignment Operators

| Operator | Description |
|----------|-------------|
| `=` | Simple assignment |
| `+=` | Add and assign |
| `-=` | Subtract and assign |
| `*=` | Multiply and assign |
| `/=` | Divide and assign |

```
let x: int = 10;
x += 5;    // x is 15
x *= 2;    // x is 30
x -= 10;   // x is 20
x /= 4;    // x is 5
```

Assignment targets must be variables. You can't assign to expressions.

## Unary Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `-` | Negation | `-x` |
| `!` | Boolean NOT | `!flag` |
| `&` | Address of | `&x` |
| `*` | Dereference | `*ptr` |
| `++` | Increment, prefix only | `x++` |
| `--` | Decrement, prefix only | `x--` |

```
let x: int = 5;
let p: int* = &x;     // address of x
let val: int = *p;    // dereference: val is 5
x++;
print(x);              // 6
```

## Parentheses

Use parentheses to override precedence:

```
let a: int = 2 + 3 * 4;    // 14
let b: int = (2 + 3) * 4;  // 20
```
