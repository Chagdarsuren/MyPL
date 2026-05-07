# MyPL — Team 3 (Rule / Logic-like language)

Course: F.CSB305 *Programming Language Principles*
Topic: Design and implementation of a small programming language.

MyPL is a miniature Prolog. Programs are made of **facts**, **rules**, and
**queries**; the proof engine answers queries by SLD resolution with
chronological backtracking and per-activation variable renaming.

## Build

```bash
make            # produces ./mypl
make clean      # remove build artefacts
make test       # build + run the full positive / negative test suite
```

Requires `flex`, `bison`, and a C99 compiler.

## CLI

```
mypl run   <file>     parse and evaluate queries
mypl check <file>     parse only; report syntax errors and program stats
mypl ast   <file>     parse and dump the AST
mypl help             show this help
mypl                  read source from stdin and run
```

Example:

```
$ ./mypl run examples/family.logic
?- parent(bat, bold).
  true.
?- ancestor(bat, dorj).
  true.
```

## Language at a glance

```prolog
# facts
parent(tom, bob).
parent(bob, ann).

# rules
ancestor(X, Y) :- parent(X, Y).
ancestor(X, Y) :- parent(X, Z), ancestor(Z, Y).

# queries (true/false; queries with variables also report bindings)
?- ancestor(tom, ann).      # → true
?- ancestor(X, ann).        # → true, X = tom
```

See `docs/syntax.md` for the EBNF grammar and `docs/semantics.md` for
the proof procedure.

## Repository layout

```
MyPL/
├── Main.c                       # CLI entry point
├── Makefile
├── README.md                    # this file
├── DESIGN.md                    # formal language spec
├── IMPLEMENTATION.md            # implementation status / report
├── docs/
│   ├── syntax.md                # grammar reference
│   ├── semantics.md             # proof procedure
│   └── design.md                # implementation design notes
├── src/
│   ├── ast/         { ast.h, ast.c }
│   ├── parser/      { lexer.l, parser.y }
│   └── interpreter/ { interpreter.h, interpreter.c }
├── examples/        { family, social, food, graph, animals }.logic
└── tests/
    ├── *.logic                  # 20 positive tests
    ├── negative/*.logic         # negative tests (parse + runtime)
    └── run_tests.sh             # full suite runner
```

## Test status

Run `make test`. Latest run:

```
=== Positive tests (tests/*.logic) ===
=== Examples (examples/*.logic) ===
=== Negative tests (tests/negative/*.logic) ===

passed: 31
failed: 0
```

## Required concepts (Team 3 brief)

| Concept           | Where                                                        |
|-------------------|--------------------------------------------------------------|
| Fact              | `parent(tom, bob).` — see `examples/family.logic`            |
| Rule              | `ancestor(X,Y) :- parent(X,Z), ancestor(Z,Y).`               |
| Query             | `?- ancestor(tom, ann).`                                     |
| Pattern matching  | Unification with variable renaming — `docs/semantics.md` §3 |
| Search            | DFS with chronological backtracking — `docs/semantics.md` §4|
