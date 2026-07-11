# Control Flow

## if / else

```
if condition {
    // then branch
} else {
    // else branch
}
```

`else if` is done by nesting:

```
if x > 10 {
    print("big\n");
} else {
    if x > 5 {
        print("medium\n");
    } else {
        print("small\n");
    }
}
```

The condition does **not** need parentheses. Any expression that evaluates to an integer works as a condition. 0 is false, non-zero is true.

## while

```
while condition {
    // body
}
```

Counting to 5:

```
let i: int = 0;
while i < 5 {
    print("*\n");
    i = i + 1;
}
```

## for

C style for loop with init, condition, and increment:

```
for INIT; CONDITION; INCREMENT {
    // body
}
```

Example:

```
for let i: int = 0; i < 5; i = i + 1 {
    print("loop\n");
}
```

The init section can declare a variable with `let`:

```
for let i: int = 10; i > 0; i = i - 1 {
    print(i);
    print("\n");
}
```

## break

Exits the innermost loop right away:

```
while true {
    let x: int = get_input();
    if x == 0 {
        break;
    }
    print(x);
    print("\n");
}
```

## continue

Skips to the next iteration of the innermost loop:

```
for let i: int = 0; i < 10; i = i + 1 {
    if i % 2 == 0 {
        continue;
    }
    print(i);   // prints odd numbers only
    print("\n");
}
```

## Loops within Loops

`break` and `continue` always refer to the **innermost** loop. Nested loops work great:

```
for let i: int = 0; i < 3; i = i + 1 {
    for let j: int = 0; j < 3; j = j + 1 {
        if j == 1 { continue; }   // skips inner loop iteration
        print(i);
        print("\n");
    }
}
```
