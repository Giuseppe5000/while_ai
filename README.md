# WHanalyzer - Static analyzer (While Language)

## Overview
TODO

## Requirements
- C99-compatible compiler.
- Make (optional, you can just put all *.c files into the compiler).

## How to build
Just run
``` bash
$ make
```
And you'll find an executable file named `cli` (it has an integrated help, just run `$ ./cli`).

## Abstract Domain
There is only one abstract domain: the **Parametric Interval** $\text{Int}_{m,n}$.

Given $m, n \in \mathbb{Z} \cup \lbrace -\infty, +\infty \rbrace$,\
the domain is defined as:

$\text{Int}_{m,n} \triangleq \lbrace\varnothing, \mathbb{Z} \rbrace \cup \lbrace [k, k] \mid k \in \mathbb{Z} \rbrace \cup \lbrace [a, b] \mid a < b, [a, b] \subseteq [m, n] \rbrace \cup \lbrace (-\infty, k] \mid k \in [m, n] \rbrace \cup \lbrace [k, +\infty) \mid k \in [m, n] \rbrace$

Some notes about $m, n$:
  - With $(m, n) = (-\infty, +\infty)$ the domain will become the standard interval domain (Int/Box).
  - With $m > n$ the domain will become the constant propagation domain.

## While Language grammar
```
numerals:
    n ∈ Num (integers)

variables:
    x ∈ Var

arithmetic expressions:
    a ∈ Aexp
    a ::= n | x | a1 + a2 | a1 - a2 | a1 * a2 | a1 / a2 | (a)

boolean expressions:
    b ∈ Bexp
    b ::= true | false | a1 = a2 | a1 <= a2 | !b | b1 & b2 | (b)

statements:
    S ∈ Stm
    S ::= x := a | skip | S1;S2 | if b then S1 else S2 fi | while b do S done

```

## References
TODO

## TODO

- [ ] Test and check correctness of arithmetic ops in the domain.
- [x] Implement abstract tests in `abstract_interval_state_exec_command` (There is a TODO in the code).
- [ ] Implement parentheses for bexp

Other things:
- [ ] Check TODOs in the code.
- [ ] Check comments.
- [ ] Styling and const correctness.
- [ ] Complete README.
