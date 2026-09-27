# Chad

Chad is an esoteric programming language based on interaction networks aimed to be chad.

Chad is cool. Chad is great. Chad needs no dependencies. Chad works alone. All is good.

## Example

```
ngl show n then pass the World to the next Count
Count(w, next) vs n
  if n < 0 => next ~ w
  else => n ~ Show(w, Say['\n'](w2)), n - 1 ~ Count(w2, next)

main
  9 ~ Count(world, Ghost)
```

More in `examples/`.

## How it works

A program is a net of Chads connected by wires. Each Chad has one face and some arms. When two Chads meet face to face, the rule for their pair replaces both of them. The program runs until no two faces meet.

- `A(x, y)` is a Chad of breed `A` with arms `x` and `y`.
- `A[1, 2](x)` also carries the values `1` and `2`.
- `a ~ b` connects `a` and `b` with a wire.
- A number like `42` is a Chad too. A name like `w` is a wire.
- `A(x) vs B(y) => ...` is the rule for when `A` meets `B`. `A(x) vs n` matches any number `n`.
- A rule can have cases: `if cond => ...` and a final `else => ...`. The first case that fits is used.
- `main` is the net the program starts with. It comes after the rules and runs to the end of the file. `world` is the wire to the World, and it must be used exactly once.
- `ngl` starts a comment until the end of the line.

Values are 64-bit integers. They support `+ - * / %`, `== != < <= > >=` and `and`, `or`, `not`. `'a'` is the character's number. `"hi"` is `Cons('h', Cons('i', Nil))`.

## Built-in breeds

| Breed | Does |
| --- | --- |
| `Say[c](next)` | when it meets the World, prints `c` as a UTF-8 character and gives the World to `next` |
| `Hear(next, out)` | when it meets the World, reads one UTF-8 character into `out` (`Silence` at the end) and gives the World to `next` |
| `Ghost` | erases whatever it meets |
| `Rep(a, b)` | copies whatever it meets into `a` and `b` |
| `Fn(in, out)` | a function. Two `Fn`s meeting join their arms |
| `Cons(head, rest)`, `Nil` | lists. The head can be any Chad |
| `Silence` | end of input |

## Prelude

Every program includes these integrated Chad functionalities (directly written in Chad):

| Breed | Does |
| --- | --- |
| `Print(w, next)` | prints a string |
| `Show(w, next)` | prints a number |
| `HearNum(w, next, out)` | reads a number into `out` (`Silence` at the end) |
| `Add(b, out)`, `Sub`, `Mul`, `Div`, `Mod` | `a ~ Add(b, out)` puts `a + b` on `out` |
| `Eq(b, out)`, `Lt`, `Gt` | same, but compares |

## Build

Chad only needs a C++17 compiler and CMake. Nothing else.

```
cmake -B build
cmake --build build
```

## Run

```
chad <file.chad>
chad -v    print the version
chad -h    print help
```

## Test

```
ctest --test-dir build
```

Test cases: [tests/cases/](tests/cases/)

## License

[LICENSE](LICENSE)