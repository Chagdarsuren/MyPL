# MyPL Implementation Report

## Status

| Requirement (per syllabus rubric)                        | Status |
|----------------------------------------------------------|--------|
| Mini PL with syntax, AST, and semantics                  |   ✓    |
| AST built by parser; semantics walks AST                 |   ✓    |
| No execution inside parser semantic actions              |   ✓    |
| Parser generator (Flex + Bison)                          |   ✓    |
| 20+ positive tests                                       |   ✓ 20 |
| 5+ integrated example programs                           |   ✓ 5  |
| Negative tests with line-numbered error messages         |   ✓ 6  |
| CLI tooling (`run`, `check`, `ast`)                      |   ✓    |
| Spec + grammar + AST + semantics docs                    |   ✓    |

`make test` runs the entire suite. Latest: **31/31 passing**.

## Layout

```
src/
  ast/         AST node types (Fact, Rule, Query)
  parser/      lexer.l (Flex) + parser.y (Bison), build the AST only
  interpreter/ proof engine: unification, substitution, SLD with
               chronological backtracking, per-activation variable
               renaming
Main.c         argv parsing, mode dispatch (run / check / ast)
```

## Pipeline

```
source.logic
   │  Flex (lexer.l)
   ▼
tokens
   │  Bison (parser.y) — semantic actions ONLY build AST and enqueue queries
   ▼
AST  + query queue
   │  interpreter.c — runs after yyparse returns
   ▼
true/false answers (with bindings for variable queries)
```

## Examples

| File                       | Demonstrates                          |
|----------------------------|---------------------------------------|
| `examples/family.logic`    | recursive `ancestor`                  |
| `examples/social.logic`    | symmetric + transitive friendship     |
| `examples/graph.logic`     | reachability via two-rule recursion   |
| `examples/food.logic`      | union-of-categories                   |
| `examples/animals.logic`   | multi-level inheritance + var queries |

## Test suite

- **20 positive tests** in `tests/*.logic`: cover facts only, single
  rules, multi-body rules, recursion (linear, deep, two-rule),
  symmetric and transitive relations, multi-arg predicates,
  numeric-atom unification, etc.
- **6 negative tests** in `tests/negative/`:
  - `err_missing_dot` — parse error: missing `.` terminator
  - `err_unbalanced_paren` — parse error: unmatched `(`
  - `err_bad_token` — lexical error on `@`
  - `err_query_no_args` — parse error: empty argument list
  - `err_undefined_predicate` — runtime: query of an undeclared
    predicate yields `false`
  - `err_arity_mismatch` — runtime: arity mismatch yields `false`
- `tests/run_tests.sh` runs all of them and asserts the expected
  exit code / output.

## Design highlights

- **Per-activation variable renaming.** Each rule activation gets a
  fresh suffix (`X#7`, `Y#7`) so recursive activations don't collide.
- **Goal-stack proof.** Rule bodies are *prepended* to the
  continuation, so backtracking spans body goals: if `goal[i]`'s first
  match commits an inconsistent binding for `goal[i+1]`, the engine
  retries `goal[i]` with the next match.
- **Substitution checkpoints.** Each candidate fact / rule starts from
  a saved binding count; failure rolls back exactly to that point.
- **Deferred query evaluation.** Parser actions only enqueue queries;
  evaluation runs after `yyparse` returns. This keeps the parser
  reusable (`mypl check`, `mypl ast`) and means the engine never sees
  a half-parsed file.

See `docs/semantics.md` for the proof procedure and `docs/design.md`
for rationale on these choices.

## What is intentionally *not* there

(Documented in `DESIGN.md` §5.2 with reasoning.)

- No multi-solution enumeration (only the first answer is reported).
- No cut, no negation-as-failure.
- No nested compound terms or lists.
- No arithmetic — integers are atoms.
- No assert/retract or any other dynamic predicates.

## How to defend / demo

1. `make test` — show 31/31 green.
2. `./mypl run examples/family.logic` — basic recursion.
3. `./mypl run examples/graph.logic` — multi-hop reachability
   (the bug fixed during week 11 — see commit history).
4. `./mypl run examples/animals.logic` — variable bindings:
   `?- flying_mammal(X). → true, X = bat`.
5. `./mypl check tests/negative/err_missing_dot.logic` — line-numbered
   parse error.
6. `./mypl ast examples/family.logic` — show that the parser builds an
   AST without executing.
