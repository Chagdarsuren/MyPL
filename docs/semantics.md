# MyPL Semantics

## 1. Execution model

A MyPL program is read top-to-bottom. Facts and rules populate two
global stores; queries are queued. Once parsing finishes, every queued
query is evaluated in declaration order against the fact / rule store.

There is no mutable state, no I/O, and no order-dependent control flow
between queries — each query is independent.

## 2. Terms and substitutions

A **term** is either an atom (lowercase or numeric symbol) or a
variable (uppercase symbol). Internally the proof engine extends the
variable namespace with a "freshness suffix": `X#7`, `Y#7`, etc. The
suffix is incremented every time a rule is activated so that two
activations of the same rule cannot see each other's bindings.

A **substitution** is a list of (variable → term) bindings. The
substitution chain is followed by a `walk` operation: given any term,
follow bindings until reaching either a non-variable or an unbound
variable.

## 3. Unification

`unify(a, b, σ)` is the standard syntactic unifier:

```
unify(a, b, σ):
  a ← walk(a, σ)
  b ← walk(b, σ)
  if a == b      : succeed (no binding added)
  if a is var    : add a → b to σ; succeed
  if b is var    : add b → a to σ; succeed
  otherwise      : fail
```

(There is no occurs check — see §6.)

## 4. Proof procedure (SLD resolution with backtracking)

The engine is goal-directed depth-first search with chronological
backtracking, expressed as a single function `prove_conj(goals, idx, σ)`
that proves a conjunction of goals starting at index `idx`:

```
prove_conj(G[], idx, σ):
  if idx == |G|: succeed
  let g = G[idx]
  for each fact F with name(F) == name(g) and arity(F) == arity(g):
      σ' = σ
      if unify all args of F with g under σ' AND
         prove_conj(G[], idx+1, σ'):
          succeed (commit σ')
      else:
          undo to σ
  for each rule R with name(R) == name(g) and arity(R) == arity(g):
      let n = fresh suffix
      let R' = rename_variables(R, n)
      let G' = R'.body ++ G[idx+1..]            (prepend body, keep continuation)
      σ' = σ
      if unify all args of R'.head with g under σ' AND
         prove_conj(G', 0, σ'):
          succeed
      else:
          undo
  fail
```

Two important details:

1. **Variable renaming per activation.** Every time a rule is selected,
   its variables are renamed to a fresh namespace. Without this,
   recursive rules like `ancestor` clobber the outer binding of `X`.
2. **Backtracking restores the substitution.** Each candidate fact /
   rule starts from the same substitution checkpoint. If a
   continuation fails, all bindings introduced after the checkpoint
   are undone before the next candidate is tried.

## 5. Query reporting

A query is a single goal `?- p(t₁, …, tₙ)`. The engine collects the
distinct variables appearing in the query, then calls `prove_conj`. On
success, each query variable is `walk`-ed through the final
substitution and printed. Unbound variables print as `_`. The engine
finds the **first** solution and stops — multi-solution enumeration is
intentionally omitted (see `DESIGN.md` §5.2).

## 6. Soundness and incompleteness

- **Sound** with respect to a model where atoms are uninterpreted
  ground constants and predicates are the smallest set closed under
  the declared rules.
- **Incomplete** because:
  - There is no occurs check, so `?- p(X, f(X)).` would loop on
    cyclic terms — but MyPL has no nested terms, so this is moot.
  - Left-recursive rules can loop forever, e.g.
    `p(X) :- p(X), q(X).`. There is no iterative deepening.
  - Only the first solution is reported; a query that has solutions
    via a *later* rule clause but reaches a non-terminating earlier
    clause will diverge.

## 7. Worked example

```
parent(tom, bob).
ancestor(X, Y) :- parent(X, Y).
ancestor(X, Y) :- parent(X, Z), ancestor(Z, Y).

?- ancestor(tom, jim).
```

1. `prove_conj([ancestor(tom, jim)], 0, ∅)`
2. No matching fact. Try rule clause 1 with suffix `#1`:
   - head `ancestor(X#1, Y#1)`; body `parent(X#1, Y#1)`.
   - unify head: σ = {X#1 → tom, Y#1 → jim}
   - prove `parent(tom, jim)` — no fact matches, no rules → fail.
   - undo σ.
3. Try rule clause 2 with suffix `#2`:
   - body `parent(X#2, Z#2), ancestor(Z#2, Y#2)`.
   - unify head: σ = {X#2 → tom, Y#2 → jim}
   - prove `parent(tom, Z#2)` — fact `parent(tom, bob)` unifies,
     binds `Z#2 → bob`.
   - prove `ancestor(bob, jim)` — recursive call, eventually
     succeeds via further rule activations…

The recursion terminates because each step strictly traverses the
parent chain.
