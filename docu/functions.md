# Functions

## Declaration

Functions are declared with `fn`, a name, parameters, an optional return type, and a body:

```
fn greet(name: str) -> void {
    print("hello ");
    print(name);
    print("\n");
}
```

### No Return Type

If a function returns nothing, just leave off the return type or use `-> void`:

```
fn do_nothing() {
    // same as -> void
}
```

### Parameters

Parameters are comma separated with type annotations:

```
fn add(a: int, b: int) -> int {
    return a + b;
}
```

Every parameter needs an explicit type. No default arguments right now.

### Return

Use `return` to return a value:

```
fn square(x: int) -> int {
    return x * x;
}
```

A function without an explicit `return` returns void. If a non-void function reaches the end of its body without returning, that's undefined behavior.

## Calling Functions

```
greet("world");
let result: int = add(3, 4);
print(result);
```

## The `main` Function

Every Raw program needs a `main` function that returns `int`:

```
fn main() -> int {
    return 0;
}
```

The return value of `main` becomes the process exit code.

## Recursion

Functions can call themselves:

```
fn factorial(n: int) -> int {
    if n <= 1 {
        return 1;
    }
    return n * factorial(n - 1);
}

fn main() -> int {
    let result: int = factorial(5);
    print(result);    // 120
    print("\n");
    return 0;
}
```

## Calling Order

A function can call any other function in the same file. No forward declarations needed. The compiler processes all function declarations before generating code.

## Example: Multiple Functions

```
fn abs(x: int) -> int {
    if x < 0 {
        return -x;
    }
    return x;
}

fn max(a: int, b: int) -> int {
    if a > b {
        return a;
    }
    return b;
}

fn main() -> int {
    print(abs(-5));       // 5
    print("\n");
    print(max(10, 20));   // 20
    print("\n");
    return 0;
}
```
