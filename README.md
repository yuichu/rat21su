# About rat21su

A lexer and a parser for the programming language RAT21SU.

# How it works

This RAT21SU lexer and parser will:

- read RAT21SU language source code from an input file then separate the source code into five different type of tokens and matching lexemes by using a Deterministic Finite State Machine (DFSM).

- run each token through the top-down parser built to recognize the structure of the RAT21SU source code using a set of production rules that defines the grammar of the RAT21SU programming language.

- convert the RAT21SU source code into assembly instructions and generate a symbol table that displays all identifiers declared in the source code and the memory address in which it is stored.

### Created by
Vincent Nguyen, 
Yujin Chung, 
Wilson Tu.
