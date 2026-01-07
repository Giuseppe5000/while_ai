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
- [x] Implement `abstract_interval_state_widening` with thresholds (just passing a list of numbers containing -INF and +INF, maybe in the ctx).
- [x] Setup the widening in the analysis, selecting the widening points (the threshold will be taken from contants in the program and constants after constant propagation).
- [x] Check if the way of counting the steps in the exec is correct, if widen_delay = 5, then the widen happens at the sixth iteration.

Narrowing:
- [x] Setup the narrowing in the analysis, applied a finite number of times.

Exec:
- [ ] Implement abstract tests in `abstract_interval_state_exec_command`.
- [x] Let the `While_Analyzer_Exec_Opt` take a string that represents the initial abstract state for the entry program point. Then each domain must have a function that takes that configuration and create a abstract state with that values. (examples of configuration in the examples).

Other things:
- [x] Current variable darray stores only vars that are lvalue, so if a var is used only as rvalue (undefined variable) then this causes UB.
- [ ] Check TODOs in the code.



