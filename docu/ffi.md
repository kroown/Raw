# FFI (Foreign Function Interface)

Raw can call C functions using `extern` blocks. This lets you tap into any C library.

## Syntax

```
extern {
    fn function_name(param1: type1, param2: type2) -> return_type;
}
```

Each extern function declaration ends with a semicolon and has **no body**.

## Example: SDL2

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
    while running {
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

## How It Works

1. The `extern` block tells the compiler these functions exist somewhere outside your code.
2. The compiler generates `call` instructions for them.
3. You have to link against the correct library when building. For SDL2:

```bash
rawc sdl_demo.raw -o sdl_demo
gcc sdl_demo.s -o sdl_demo -lSDL2
```

That's on Linux. On Windows, make sure the DLL is available.

## Supported Types in Extern

| Raw Type | C Equivalent |
|----------|-------------|
| `int` | `int` / `long` (64 bit) |
| `char` | `char` |
| `bool` | `_Bool` |
| `str` / `char*` | `const char*` |
| `void*` | `void*` |
| `T*` | `T*` (pointer to T) |
| `T[n]` | `T[n]` (array, passed by pointer) |

## Tips

* Use `void*` for opaque handles like windows and surfaces.
* Pass arrays with `&` to get a pointer.
* The compiler uses the platform calling convention (System V AMD64 on Linux, Microsoft x64 on Windows).
