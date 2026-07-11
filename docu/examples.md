# Examples

All examples are in the `examples/` directory.

## hello.raw

```
fn main() -> int {
    print("hello world\n");
    return 0;
}
```

The simplest program. `print()` takes a string literal. `\n` is a newline. `main` returns `0` for success.

## vars.raw

```
fn main() -> int {
    let x: int = 42;
    let y: int = 10;
    let z: int = x + y;
    print("z = ");
    print("\n");
    return z;
}
```

Declares three variables with `let`. The return value of `main` is the process exit code, so this program exits with code `52` (42 + 10).

## ifelse.raw

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

`if` without parentheses around the condition. The `else` branch is optional.

## while.raw

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

Prints 5 asterisks, one per line. Variables must be updated manually. Use `i = i + 1` or `i++` as a statement.

## for.raw

```
fn main() -> int {
    for let i: int = 0; i < 5; i = i + 1 {
        print("loop\n");
    }
    return 0;
}
```

C style for loop. The init section can declare a new variable with `let`. The condition and increment are expressions.

## expr.raw

```
fn main() -> int {
    let r: int = (2 + 3) * 4;
    print("done\n");
    return r;
}
```

Shows how parentheses control precedence. Returns exit code `20`.

## printint.raw

```
fn main() -> int {
    let x: int = 42;
    print(x);
    print("\n");
    return 0;
}
```

`print()` also takes integer arguments. It uses `printf("%d")` under the hood.

## compound.raw

```
fn main() -> int {
    let x: int = 10;
    x += 5;
    x *= 2;
    print("done\n");
    return x;
}
```

`x` becomes `10 + 5 = 15`, then `15 * 2 = 30`. Returns exit code `30`.

## new_features.raw

```
fn main() -> int {
    let a: int = 1;
    let b: int = 0;
    let c: int = 1;

    if a == 1 && b == 1 {
        print("AND fail\n");
    } else {
        print("AND ok\n");
    }

    if a == 1 || b == 1 {
        print("OR ok\n");
    }

    let x: int = 17;
    let m: int = x % 5;
    print(m);
    print("\n");

    return 0;
}
```

Shows `&&` (logical AND), `||` (logical OR), and `%` (modulo). `17 % 5` is `2`.

## sdl_demo.raw

```
extern {
    fn SDL_Init(flags: int) -> int;
    fn SDL_CreateWindow(title: char*, x: int, y: int, w: int, h: int, flags: int) -> void*;
    fn SDL_GetWindowSurface(window: void*) -> void*;
    fn SDL_FillRect(surface: void*, rect: void*, color: int) -> int;
    fn SDL_UpdateWindowSurface(window: void*) -> int;
    fn SDL_PollEvent(event: void*) -> int;
    fn SDL_Delay(ms: int);
    fn SDL_Quit();
}

fn main() -> int {
    let res: int = SDL_Init(32);
    if res < 0 { return 1; }

    let win: void* = SDL_CreateWindow("Raw UI Demo", 100, 100, 640, 480, 4);
    if win == 0 { SDL_Quit(); return 1; }

    let surf: void* = SDL_GetWindowSurface(win);
    SDL_FillRect(surf, 0, 0xFF0000);
    SDL_UpdateWindowSurface(win);

    let event_buf: int[8];
    let running: int = 1;
    let frames: int = 0;
    while running {
        frames = frames + 1;
        if frames > 200 { running = 0; }
        let got: int = SDL_PollEvent(&event_buf);
        if got == 1 {
            let event_type: int = event_buf;
            if event_type == 256 { running = 0; }
        }
        SDL_Delay(16);
    }

    SDL_Quit();
    return 0;
}
```

A full SDL2 app written in Raw. Shows off:

* `extern` blocks for C FFI
* Pointer types (`void*`, `char*`)
* Array declarations (`int[8]`)
* Address of operator (`&event_buf`)
* Nested control flow
* Calling into a shared library
