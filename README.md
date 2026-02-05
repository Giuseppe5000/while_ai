# WHanalyzer - Static analyzer (While Language)

## Overview
The theory behind this static analyzer is called **Abstract Interpretation**.

> Abstract Interpretation is a theory of sound approximation of the semantics of computer programs, based on monotonic functions over ordered sets, especially lattices. It can be viewed as a partial execution of a computer program which gains information about its semantics (e.g., control-flow, data-flow) without performing all the calculations.
>
> Its main concrete application is formal static analysis, the automatic extraction of information about the possible executions of computer programs; such analyses have two main usages:
> - inside compilers, to analyse programs to decide whether certain optimizations or transformations are applicable;
> - for debugging or even the certification of programs against classes of bugs.
>
> Abstract interpretation was formalized by the French computer scientist working couple Patrick Cousot and Radhia Cousot in the late 1970s.
>
> Source: https://en.wikipedia.org/wiki/Abstract_interpretation.

This project was made as an assignment for the Software Verification course of the University of Padua.
It is written in C (C99) and has no external dependencies.

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

- Tutorial on Static Inference of Numeric Invariants by Abstract Interpretation (Antoine Miné):
    https://dl.acm.org/doi/10.1561/2500000034
- Software Verification course:
    https://unipd.coursecatalogue.cineca.it/corsi/2025/2634/insegnamenti/2025/52879_490656_65775/2025/52879
- Interproc static analyzer:
    https://pop-art.inrialpes.fr/people/bjeannet/bjeannet-forge/interproc/index.html
- Interproc on Docker:
    https://github.com/Edivad99/interproc-docker

## TODO

- [x] Test and check correctness of arithmetic ops in the domain.
- [x] Implement abstract tests in `abstract_interval_state_exec_command` (There is a TODO in the code).
- [x] Implement parentheses for bexp

Other things:
- [x] Check TODOs in the code.
- [ ] Check comments.
- [x] Styling and const correctness.
- [x] Add more examples.
- [ ] Adjust fuzz.py (make it delete the test_case.while if it finish correcly).
