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
- [ ] Lexer doesn't tokenize negative numbers (-x).
- [ ] Current variable darray stores only vars that are lvalue, so if a var is used only as rvalue (undefined variable) then this causes UB.
- [ ] Implement the arithmetic ops in the domain (doing also testing).
- [ ] Implement `abstract_interval_state_exec_command` (three possible commands).
- [ ] Create the worklist (a queue) in `abstract_analyzer.c` and compute the fixpoint (without widening/narrowing).
- [ ] Implement `abstract_interval_state_widening` and `abstract_interval_state_narrowing` and use it in the worklist algorithm.
