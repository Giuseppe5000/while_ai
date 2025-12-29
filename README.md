# Abstract interpreter for the abstract equational semantics X# of While Language

## Overview
TODO

## Abstract Domain

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

- [ ] Implement abstract tests in `abstract_interval_state_exec_command`.

Other things:
- [ ] Lexer doesn't tokenize negative numbers (-x).
- [ ] Current variable darray stores only vars that are lvalue, so if a var is used only as rvalue (undefined variable) then this causes UB.
- [ ] Check TODOs in the code.



