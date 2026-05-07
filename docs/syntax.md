# MyPL Syntax Reference

MyPL is a small Prolog-like logic programming language. A program is a
sequence of statements terminated by `.`. There are three kinds of
statement: **facts**, **rules**, and **queries**.

## Lexical structure

| Class      | Pattern                                | Examples                       |
|------------|----------------------------------------|--------------------------------|
| atom       | `[a-z_][A-Za-z0-9_]*` or `[0-9]+`      | `john`, `tom_jr`, `42`         |
| variable   | `[A-Z][A-Za-z0-9_]*`                   | `X`, `Ancestor`, `Y2`          |
| operator   | `:-`, `?-`                             |                                |
| punctuation| `(` `)` `,` `.`                        |                                |
| comment    | `# …` to end-of-line, or `% …`         |                                |

Whitespace is insignificant; the lexer tracks line numbers for error
messages.

The lexer treats integer literals as atoms — there are no numeric types
or arithmetic. They are simply ground symbols that unify by string
identity.

## Grammar (EBNF)

```ebnf
program     = { statement } ;
statement   = (fact | rule | query) "." ;

fact        = predicate ;
rule        = predicate ":-" body ;
query       = "?-" predicate ;

predicate   = IDENT "(" args ")" ;
args        = arg { "," arg } ;
arg         = IDENT ;                 (* atom OR variable *)

body        = predicate { "," predicate } ;
```

`IDENT` covers both atoms and variables; the distinction is purely
lexical (initial uppercase = variable).

## Examples

```prolog
# fact
parent(tom, bob).

# rule (single-clause body)
sibling(X, Y) :- parent(P, X), parent(P, Y).

# query
?- parent(tom, bob).
?- sibling(X, bob).
```

## What is *not* in the grammar

- No nested terms (`foo(bar(x))`) — arguments must be plain atoms or
  variables.
- No lists, no anonymous variables (`_`), no cut (`!`), no negation.
- No arithmetic — `1 + 2` is not an expression; `1` is just an atom.
- No infix operators beyond `:-` and `?-`.
- No string literals.

These omissions are deliberate; see `DESIGN.md` for the rationale.
