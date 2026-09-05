# Chad

Chad is a stack based esoteric programming language aimed to be chad.

Chad is cool. Chad is great. Chad needs no dependencies. Chad works alone. All is good.

## Example

```
ngl Counts 9 down to 0
9
[ rep sayn 1 - ]
sayn
```

More in `examples/`.

## Instructions

Everything is a 64-bit integer on one stack. Tokens are separated by whitespace. Words are case-insensitive.

In the stack column, `a b` means `b` is on top.

| Instruction | Stack | Does |
| --- | --- | --- |
| `42`, `-7` | → `n` | push the number |
| `rep` | `a` → `a a` | duplicate top |
| `ghost` | `a` → | drop top |
| `pivot` | `a b` → `b a` | swap top two |
| `+` | `a b` → `a+b` | add (wraps on overflow) |
| `-` | `a b` → `a-b` | subtract (wraps) |
| `*` | `a b` → `a*b` | multiply (wraps) |
| `/` | `a b` → `a/b` | divide, rounds toward zero; error if `b` is 0 |
| `%` | `a b` → `a%b` | remainder; error if `b` is 0 |
| `mogs` | `a b` → `a>b` | 1 if `a` is greater than `b`, else 0 |
| `cope` | `a` → `!a` | 1 if `a` is 0, else 0 |
| `[` | `a` → `a` | if top is 0, jump past the matching `]` |
| `]` | `a` → `a` | if top is not 0, jump back past the matching `[` |
| `sayc` | `a` → | print `a` as a UTF-8 character |
| `sayn` | `a` → | print `a` as a number |
| `readc` | → `c` | read one UTF-8 character from stdin |
| `readn` | → `n` | read a number from stdin |
| `ngl` | | comment until end of line |

`[` and `]` only look at the top of the stack. They don't pop it. Running an instruction with too few values on the stack is an error.

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