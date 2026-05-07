# MyPL Design Notes

This file complements `DESIGN.md` (the formal spec) with notes on the
design choices that aren't strictly part of the language definition.
For language purpose, scope, syntax, and intentionally-omitted features
see `DESIGN.md`. For the proof-engine algorithm see
`docs/semantics.md`.

## 1. Why a logic language?

The team-3 brief in the syllabus asks for a *rule / logic-like*
language built around facts, rules, queries, and pattern matching.
A miniature Prolog cleanly satisfies all four bullets and forces the
implementation to confront unification, substitution, and search —
the exact concepts the course is testing.

## 2. AST is the boundary

Parser actions construct AST nodes (`Fact`, `Rule`, `Query`) and never
execute logic. Queries are *enqueued* during parse; the proof engine
runs over the AST after `yyparse` returns. This means:

- The parser is reusable for tools that don't want to execute (e.g.
  `mypl check` and `mypl ast`).
- The engine never sees a half-parsed file.
- Future passes (a type / arity checker, dead-code analysis, etc.) can
  slot in between parse and execute.

## 3. Why per-activation variable renaming?

Without it, recursive rules collide. Concretely:

```prolog
ancestor(X, Y) :- parent(X, Z), ancestor(Z, Y).
?- ancestor(tom, jim).
```

When the recursive call reaches `ancestor(bob, jim)` via the same
rule, both activations share `X` and `Y`. Binding `X → bob` in the
inner call would corrupt `X → tom` in the outer one. Each rule
activation must therefore use fresh variables `X#k`, `Y#k`. We
implement this by allocating a monotonic suffix and copying the rule's
head and body with renamed terms.

## 4. Why a goal-stack instead of nested calls?

The first version of the interpreter recursed `prove(body_i)` for each
body goal in sequence and merged substitutions on success. This *looks*
right, but it cannot backtrack across body goals: if the first body
goal succeeds with one fact and the second fails, we cannot retry the
first goal with the next fact.

The corrected version keeps a single goal list and recurses on
"prove goal `idx`, then continue with `idx+1`". When activating a
rule, the rule's body goals are *prepended* to the remaining
continuation, giving a single linear sequence. Backtracking is
automatic: each candidate is tried in turn, and on failure the
substitution is rolled back to the checkpoint taken before that
candidate was tried.

## 5. Why store body items as strings?

The AST stores rule bodies as serialised predicate strings
(`"parent(X,Y)"`) rather than parsed structures. The proof engine
re-parses on demand. This is slower but keeps the parser actions
trivial — they don't have to allocate a separate `Goal[]` per rule.
For a teaching language with rules of arity ≤ 10 and bodies of length
≤ 10, the cost is invisible.

## 6. Why not show all solutions?

Multi-solution enumeration would require either:

- Generators / coroutines (not idiomatic in C), or
- Continuations stored on a heap-allocated choice-point stack.

The brief asks for true/false answering with optional bindings, and
`DESIGN.md` §5.2 already calls this out as an intentional omission.
We surface the *first* binding for each query variable — enough for
worked examples, not enough for a real Prolog.

## 7. Trade-offs we are explicitly *not* fixing

- **No occurs check.** Cannot bite us because there are no nested
  terms.
- **No iterative deepening.** Left-recursive rules can spin forever.
  The teaching examples don't trigger this.
- **Fixed array sizes.** `MAX_FACTS = MAX_RULES = MAX_QUERIES = 1000`,
  `MAX_BINDINGS = 4096`. Real programs in scope are far smaller.
- **No module system.** Single global namespace. Fine for one-file
  programs.

## 8. CLI design

```
mypl run   <file>   parse and evaluate queries
mypl check <file>   parse only; print fact/rule/query counts
mypl ast   <file>   parse and dump the AST
mypl                read source from stdin and run
```

The `check` mode is the cheapest way to integrate MyPL into a CI
pipeline or editor: it returns a non-zero exit on any parse error and
otherwise prints a one-line summary. `ast` is for debugging the parser
itself.
