# Abstract interpreter for the abstract equational semantics X# of While Language

## Overview
TODO

## How to build
Just
``` bash
$ make
```
And you'll find an executable file named `cli` (it has an integrated help, just run `$ ./cli`).

## Abstract Domain
There is only one abstract domain: the **Parametric Interval** $\text{Int}_{m,n}$.

Given $m, n \in \mathbb{Z} \cup \{ -\infty, +\infty \}$,\
the domain is defined as:

$\text{Int}_{m,n} \triangleq \{\varnothing, \mathbb{Z} \} \cup \{ [k, k] \mid k \in \mathbb{Z} \} \cup \{ [a, b] \mid a < b, [a, b] \subseteq [m, n] \} \cup \{ (-\infty, k] \mid k \in [m, n] \} \cup \{ [k, +\infty) \mid k \in [m, n] \}$

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
    a ::= n | x | a1 + a2 | a1 - a2 | a1 * a2 | a1 / a2

boolean expressions:
    b ∈ Bexp
    b ::= true | false | a1 = a2 | a1 <= a2 | !b | b1 & b2

statements:
    S ∈ Stm
    S ::= x := a | skip | S1;S2 | if b then S1 else S2 fi | while b do S done

```

## TODO
- [ ] Test and check correctness of arithmetic ops in the domain (especially division).

Widening:
- [ ] Implement `abstract_interval_state_widening` with thresholds (just passing a list of numbers containing -INF and +INF, maybe in the ctx).
- [ ] Setup the widening in the analysis, selecting the widening points (the threshold will be taken from contants in the program and constants after constant propagation).

Narrowing:
- [ ] Implement `abstract_interval_state_narrowing` and use it in the worklist algorithm.
- [ ] Setup the narrowing in the analysis, applied a finite number of times.

Exec:
- [ ] Implement abstract tests in `abstract_interval_state_exec_command`.

Other things:
- [ ] Lexer doesn't tokenize negative numbers (-x).
- [ ] Current variable darray stores only vars that are lvalue, so if a var is used only as rvalue (undefined variable) then this causes UB.
- [ ] Check TODOs in the code.



