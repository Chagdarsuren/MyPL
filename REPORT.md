# MyPL — Тайлан

**Хичээл:** F.CSB305 «Программчлалын хэлний зарчмууд»
**Сэдэв:** Жижиг программчлалын хэлний дизайн ба хэрэгжилт
**Баг 3:** Дүрэмд суурилсан хэл (Rule / Logic-like language)

---

## Агуулга

1. [Хэлний дизайн](#1-хэлний-дизайн)
2. [Синтакс](#2-синтакс)
3. [AST (Abstract Syntax Tree)](#3-ast-abstract-syntax-tree)
4. [Семантик](#4-семантик)
5. [Хэрэглээний документаци](#5-хэрэглээний-документаци)
6. [Демо: жишээ программууд](#6-демо-жишээ-программууд)
7. [Хавсралт: ажилласан тестүүд](#7-хавсралт-ажилласан-тестүүд)

---

## 1. Хэлний дизайн

### 1.1 Зорилго ба хэрэглээний хүрээ

**MyPL** нь Prolog-ийн нэг хэсэгт суурилсан, баг 3-ын даалгаврын дагуу
**дүрэмд суурилсан жижиг программчлалын хэл**. MyPL-ийн зорилго:

- Баримт (fact), дүрэм (rule), асуулт (query) гэсэн гурван үндсэн ойлголтыг
  ашиглан үнэн худлыг логикийн аргаар тогтоох.
- Pattern matching ба unification-ыг боловсрол, судалгааны зорилгоор
  харуулах.
- Гэр бүлийн харилцаа, нийгмийн сүлжээ, граф traversal зэрэг тогтсон
  хамаарал бүхий жижиг бодлогуудыг шийдвэрлэх.

### 1.2 «Программ» гэж юу вэ?

MyPL-д программ гэдэг бол:

```
программ ::= баримтууд + дүрмүүд + асуултууд
```

Программын гүйцэтгэл нь асуулт бүрд `true` эсвэл `false` (хэрэв query
хувьсагч агуулсан бол bind хийгдсэн утга) хариулахаас тогтоно.
**Side effect, төлөв, оролт/гаралт байхгүй.**

### 1.3 Гол дизайн сонголтууд

| Сонголт                       | MyPL-ийн шийдэл                                     |
|-------------------------------|-----------------------------------------------------|
| State (төлөв)                 | **Байхгүй** — программ нэг удаа уншигдаж, бүх асуулт declarative-аар бодогдоно |
| Control flow (хяналт)         | **Байхгүй** — `if`, `while`, циклгүй; зөвхөн дүрмийн рекурс  |
| Typing (типчлэл)              | **Байхгүй** (uninterpreted symbols)                  |
| Execution model               | **SLD resolution + chronological backtracking**     |
| Multi-solution enumeration    | **Байхгүй** — зөвхөн эхний шийд                      |
| Cut (`!`), negation (`\+`)    | **Байхгүй**                                          |
| Compound terms, lists         | **Байхгүй** — зөвхөн atom + variable                |
| Arithmetic                    | **Байхгүй** — тоо нь зүгээр л atom (тэмдэгт)        |

### 1.4 Санаатайгаар оруулаагүй элементүүд ба шалтгаан

| Элемент              | Яагаад оруулаагүй                                                   |
|----------------------|---------------------------------------------------------------------|
| Backtracking-аар бүх шийдийг тоолох | C-д continuation/coroutine хэрэгжүүлэхэд хүндрэлтэй; teaching хэлэнд эхний шийд хангалттай |
| Cut (`!`)            | Choice-point stack хэрэгтэй болно; цөм энгийн байх ёстой           |
| Negation as failure  | Энгийн DFS-тэй нийцэхгүй; өгөгдөл монотонж байхын тулд орхив       |
| Built-in predicates  | Type marshalling, type coercion-ыг бичих ёстой болно                |
| Lists (`[H|T]`)      | Pattern matching syntax + complex term parsing хэрэгтэй            |
| Assert/retract       | Дүрэм болон баримт runtime-д өөрчлөгдөх ёстой болно                  |
| DCG                  | Үндсэн logic programming-д хэрэггүй                                  |
| Occurs check         | Compound term байхгүй учир хэрэггүй                                   |

### 1.5 Дизайны зарчим

1. **AST бол хил.** Parser зөвхөн AST барина — гүйцэтгэл хийхгүй.
2. **Семантик AST дээр тулгуурлана.** Proof engine нь parser-аас бүрэн
   тусдаа ажилладаг.
3. **Минимализм.** Үндсэн ойлголт дөрөв (fact, rule, query, unification)
   эдгээрийг л харуулна.
4. **Тодорхойлогдсон бүтэлгүйтэл.** Үл мэдэгдэх предикат, arity таарахгүй
   гэх мэт нь crash биш `false` хариу өгнө.

---

## 2. Синтакс

### 2.1 Lexical (токен) дүрмүүд

Эх кодыг token-уудад хувааж, Flex (`src/parser/lexer.l`) ашигладаг.

| Ангилал           | Загвар                                           | Жишээ                |
|-------------------|--------------------------------------------------|----------------------|
| atom              | `[a-z_][A-Za-z0-9_]*` эсвэл `[0-9]+`            | `john`, `tom_jr`, `42` |
| variable          | `[A-Z][A-Za-z0-9_]*`                            | `X`, `Ancestor`, `Y2` |
| operator          | `:-`, `?-`                                       |                      |
| punctuation       | `(`, `)`, `,`, `.`                              |                      |
| comment           | `# ...`, `% ...` (мөрийн төгсгөлд)              |                      |
| whitespace        | хоосон зай / tab / newline (умартагдана)        |                      |

**Чухал:** atom ба variable-ийн ялгаа цэвэр lexical: эхний үсэг нь
**том үсэг бол variable**, **бусад тохиолдолд atom**. Тоонууд atom
гэж тооцогдоно.

### 2.2 Grammar (EBNF)

Bison (`src/parser/parser.y`) дараах грамматикийг хэрэгжүүлдэг:

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

`IDENT` нь atom ба variable-ыг хоёуланг нь хамардаг — ялгаа нь зөвхөн
эхний үсэгээр.

### 2.3 Precedence ба ambiguity

MyPL-д precedence-ын асуудал **байхгүй**, учир нь:

- Infix operator байхгүй (`:-` ба `?-` нь зөвхөн statement эхэнд тохиолдоно).
- Бүх argument list нь `,`-ээр зөвхөн right-recursive байдлаар бичигдэнэ.
- `body_items` болон `arguments` дүрмүүд нь нэг л хэлбэртэй.

Тиймээс грамматик нь shift/reduce конфликтгүй (Bison-аар бүтээхэд
warning гарахгүй).

### 2.4 Жишээнүүд

```prolog
# баримт
parent(tom, bob).

# дүрэм (нэг clause body)
sibling(X, Y) :- parent(P, X), parent(P, Y).

# асуулт
?- parent(tom, bob).
?- sibling(X, bob).

# тоон atom
number(1).
even(2).
?- even(2).
```

### 2.5 Грамматикт **байхгүй** зүйлс

- Nested term (`foo(bar(x))`) — argument нь зөвхөн atom/variable.
- List (`[H | T]`).
- Анонимат хувьсагч (`_`).
- Cut (`!`), negation (`\+`).
- Арифметик (`1 + 2` биш — `1` нь зүгээр atom).
- Infix operator (`+`, `-`, `=`, `<` гм).
- String literal.

---

## 3. AST (Abstract Syntax Tree)

### 3.1 AST node-ууд

`src/ast/ast.h` дотор гурван бүтэцтэй:

```c
// Баримт: predicate name + arguments
typedef struct {
    char* name;
    char* args[MAX_ARGS];
    int   arg_count;
} Fact;

// Дүрэм: head + body goals (string-ээр хадгалагдана)
typedef struct {
    char* name;
    char* args[MAX_ARGS];
    int   arg_count;
    char* body[MAX_BODY];     // body[i] = "predicate(X,Y,...)"
    int   body_count;
} Rule;

// Асуулт: ижил бүтэцтэй (Fact-той ижил)
typedef struct {
    char* name;
    char* args[MAX_ARGS];
    int   arg_count;
} Query;
```

### 3.2 AST-г parser яаж барина

Bison-ы semantic action бүр AST node үүсгэдэг, гэхдээ **гүйцэтгэхгүй**:

```c
statement:
    fact  DOT { Fact*  f = create_fact($1, args_array, arg_count);
                add_fact(f);
                arg_count = 0; }
  | rule  DOT { Rule*  r = create_rule($1, args_array, arg_count,
                                       body_array, body_count);
                add_rule(r);
                arg_count = body_count = 0; }
  | query DOT { Query* q = create_query($1, args_array, arg_count);
                eval_query(q);          // queue хийгдэнэ — гүйцэтгэхгүй
                arg_count = 0; }
;
```

Гурван глобал хадгалалт:

- `fact_store[]` — бүх Fact AST-ууд
- `rule_store[]` — бүх Rule AST-ууд
- `query_queue[]` — бүх Query AST-ууд (гүйцэтгэлийг хойшлуулж байгаа)

### 3.3 Дүрмийн body яагаад string-ээр хадгалагдсан

`Rule.body[i]` нь `"parent(X,Z)"` гэсэн string юм. Proof engine энэ
string-ийг хэрэгцээтэй үед `parse_pred()`-ээр задалдаг. Энэ нь:

- Parser action-ыг энгийн байлгана (тусдаа Goal[] хуваарилахгүй).
- Memory layout-ыг зөв хадгална.
- Жишээний хэмжээнд (arity ≤ 10, body length ≤ 10) гүйцэтгэлд нөлөөгүй.

### 3.4 AST дамп жишээ

```bash
$ ./mypl ast examples/family.logic

=== Facts (2) ===
  parent(bat, bold).
  parent(bold, dorj).
=== Rules (2) ===
  ancestor(X, Y) :- parent(X,Y).
  ancestor(X, Y) :- parent(X,Z), ancestor(Z,Y).
=== Queries (2) ===
  ?- parent(bat, bold).
  ?- ancestor(bat, dorj).
```

---

## 4. Семантик

### 4.1 Гүйцэтгэлийн загвар

```
source.logic
   │   Flex (lexer.l)
   ▼
tokens
   │   Bison (parser.y) — semantic action нь зөвхөн AST үүсгэнэ
   ▼
AST + query_queue
   │   interpreter.c — yyparse() дууссаны дараа ажиллана
   ▼
true / false (+ binding)
```

Програм дээрээс доош уншигдана. Баримт ба дүрэм нь хадгалалтад нэмэгдэж,
асуултууд queue-д хадгалагдана. Parsing дууссаны дараа queue-ийн асуулт
бүр **тунхагласан дарааллаар** үнэлэгдэнэ.

### 4.2 Term ба substitution

- **Term** — atom (жишээ нь `john`, `42`) эсвэл хувьсагч (жишээ нь `X`).
- **Substitution σ** — хувьсагч → term гэсэн bind-ын жагсаалт.
- **Walk** үйлдэл — substitution-ы chain-ыг дагах:
  ```
  walk(X, {X→Y, Y→bob})  =  bob
  walk(X, {X→Y})         =  Y    (Y unbound)
  walk(alice, {})        =  alice (variable биш)
  ```

### 4.3 Unification (tunifikatsi)

`unify(a, b, σ)` — стандарт syntactic unifier:

```
unify(a, b, σ):
    a ← walk(a, σ)
    b ← walk(b, σ)
    if a == b           : succeed (binding нэмэхгүй)
    if a is variable    : σ-д a → b нэмж succeed
    if b is variable    : σ-д b → a нэмж succeed
    otherwise           : fail
```

Хоёр variable хоорондоо unify хийгдвэл нэг нь нөгөөдөө bind хийгдэнэ
(жишээ нь `X#1 → Z#3`).

**Occurs check байхгүй** — компаунд term байхгүй учир хэрэггүй.

### 4.4 Proof procedure (SLD resolution + backtracking)

Үндсэн функц нь `prove_conj(goals, idx, σ)` — `goals[idx..n-1]` дарааллыг
`σ`-н хүрээнд proof хийдэг:

```
prove_conj(G, idx, σ):
  if idx == |G|: succeed     // бүх goal-уудыг proof хийсэн

  let g = G[idx]

  // Эхлээд бүх таарах БАРИМТ-уудыг турших
  for each fact F where name(F)==name(g) and arity(F)==arity(g):
      mark = σ.count                       // checkpoint авах
      if unify(F.args, g.args, σ) AND prove_conj(G, idx+1, σ):
          return success
      undo σ to mark                       // rollback, дараагийн F-г турших

  // Дараа нь бүх таарах ДҮРЭМ-үүдийг турших
  for each rule R where name(R)==name(g) and arity(R)==arity(g):
      sfx = ++fresh_id                     // тус бүрд нь фреш ID
      R' = rename(R, sfx)                  // X → X#sfx, Y → Y#sfx
      G' = R'.body ++ G[idx+1..]           // body-г үлдсэн дээр prepend
      mark = σ.count
      if unify(R'.head, g.args, σ) AND prove_conj(G', 0, σ):
          return success
      undo σ to mark

  return failure
```

**Хоёр гол санаа:**

#### 4.4.1 Тус бүрийн дүрмийн variable-уудыг шинэчлэх (renaming)

Дүрэм activate болох бүрд бүх variable-д шинэ suffix өгнө:

```
дүрэм:        knows(X, Y) :- friend(X, Y)
activation 1: knows(X#1, Y#1) :- friend(X#1, Y#1)
activation 2: knows(X#2, Y#2) :- friend(X#2, Y#2)
```

Үүнгүй бол recursive дүрмийн activation-ууд бие биенийхээ binding-ыг
устгана. Жишээ нь:

```
ancestor(X, Y) :- parent(X, Z), ancestor(Z, Y).
```

`ancestor(tom, jim)` proof хийхэд `Z → bob` гэж bind болно. Дотор
recursive call `ancestor(bob, jim)` нь өөрийн `Z`-ийг ашиглах ёстой —
гадны `Z → bob`-ыг даран бичиж болохгүй. Suffix `#k` нь activation
бүрд тусдаа namespace өгнө.

#### 4.4.2 Goal stack ба backtracking

Дүрэм activate болоход body goal-уудыг **үлдсэн continuation-д
prepend** хийдэг. Жишээ нь `[g0, g1, g2]`-ийн idx=0 дээр g0 нь
`body=[b0, b1]` дүрмээр шийдэгдвэл шинэ жагсаалт `[b0, b1, g1, g2]`,
idx=0 болно.

Энэ нь **backtracking body goal-ууд хооронд алхана** гэсэн үг. Хэрэв
b0 нэг fact-аар success болоод b1 fail болвол engine хатаагдахгүй —
σ-г rollback хийж, b0-ыг дараагийн fact-аар оролдоно.

### 4.5 Query reporting

`?- p(t1, ..., tn)` гэсэн асуулт нэг goal:

1. Query-ийн distinct хувьсагч нэрсийг цуглуулна.
2. `prove_conj(&g, 1, 0, &σ)` дуудна.
3. Success болвол:
   - Хэрэв query-д хувьсагч байхгүй: `true.` хэвлэнэ.
   - Хэрэв байгаа: query хувьсагч бүрд `walk()` ашиглан утгыг олж,
     `true, X = bob.` гэж хэвлэнэ.
4. Fail болвол: `false.`

Unbound үлдсэн хувьсагчийг `_` гэж хэвлэдэг.

### 4.6 Soundness ба incompleteness

- **Sound** — atom бол uninterpreted ground constant, predicate-ууд бол
  тунхагласан дүрмийн хамгийн жижиг закрытий гэж тооцох model-ийн дор.
- **Incomplete** учир нь:
  - Зөвхөн эхний шийдийг буцаана.
  - Left-recursive дүрэм мөнхийн эргэлдэж болно (жишээ
    `p(X) :- p(X), q(X).`). Iterative deepening байхгүй.
  - Олон шийд агуулсан query-д хариулт өгөхөд эхний clause нь хэт
    удаан тооцогдвол дараагийн clause хүрэхгүй.

### 4.7 Бодит жишээгээр trace

`?- ancestor(tom, jim)` (баримт `parent(tom, bob)`, `parent(bob, jim)`):

| Алхам | Үйлдэл                                                    | σ                                |
|------:|-----------------------------------------------------------|----------------------------------|
|     1 | `prove_conj([ancestor(tom,jim)], 0, ∅)`                   | ∅                                |
|     2 | Баримт байхгүй. Дүрэм 1: `ancestor(X,Y) :- parent(X,Y)`. sfx=1. | ∅                          |
|     3 | Head unify: `X#1 → tom, Y#1 → jim`                        | `{X#1→tom, Y#1→jim}`             |
|     4 | Body `parent(X#1, Y#1)` proof. Баримт `parent(tom, bob)`: `(tom,tom)` ✓, `(bob,jim)` ✗. Дараагийн `parent(bob, jim)`: `(bob,tom)` ✗. **fail.** |  |
|     5 | Undo. Дүрэм 2: `ancestor(X,Y) :- parent(X,Z), ancestor(Z,Y)`. sfx=2. | ∅                          |
|     6 | Head unify: `X#2 → tom, Y#2 → jim`                        | `{X#2→tom, Y#2→jim}`             |
|     7 | Body `[parent(X#2,Z#2), ancestor(Z#2,Y#2)]`, idx=0.       |                                  |
|     8 | `parent(tom, Z#2)` — `parent(tom, bob)` unify-аар `Z#2 → bob`. ✓ | `…, Z#2→bob`               |
|     9 | idx=1: `ancestor(Z#2, Y#2)` ≡ `ancestor(bob, jim)` (walk). |                                  |
|    10 | Дүрэм 1, sfx=3. Head unify: `X#3 → bob, Y#3 → jim`. Body: `parent(bob, jim)` — баримтад тохирно. ✓ |    |
|    11 | Бүх goal proof хийгдлээ. **Success → true.**             |                                  |

---

## 5. Хэрэглээний документаци

### 5.1 Эх кодын бүтэц

```
MyPL/
├── Main.c                       # CLI entry point
├── Makefile                     # build систем
├── README.md                    # хурдан гарын авлага
├── DESIGN.md                    # формаль спец (англиар)
├── REPORT.md                    # энэ тайлан
├── IMPLEMENTATION.md            # хэрэгжилтийн төлөв
├── docs/
│   ├── syntax.md                # грамматик reference
│   ├── semantics.md             # proof procedure
│   └── design.md                # design notes
├── src/
│   ├── ast/
│   │   ├── ast.h                # AST бүтцийн тодорхойлолт
│   │   └── ast.c                # AST бүтээх функцууд
│   ├── parser/
│   │   ├── lexer.l              # Flex lexer
│   │   └── parser.y             # Bison parser
│   └── interpreter/
│       ├── interpreter.h        # public API
│       └── interpreter.c        # proof engine
├── examples/                    # 5 нэгдсэн жишээ программ
│   ├── family.logic
│   ├── social.logic
│   ├── food.logic
│   ├── graph.logic
│   └── animals.logic
└── tests/                       # 20 positive + 6 negative тест
    ├── *.logic
    ├── negative/*.logic
    └── run_tests.sh
```

### 5.2 Build

Шаардлага: `gcc` (эсвэл `clang`), `flex`, `bison`, `make`.

```bash
make           # ./mypl исполняемый үүсгэнэ
make clean     # бүх build артефакт устгана
make test      # build хийж бүх тестийг ажиллуулна
```

### 5.3 CLI

```
mypl run   <file>     эх кодыг parse хийж асуултуудыг гүйцэтгэнэ
mypl check <file>     зөвхөн parse хийж syntax error болон тоонуудыг хэвлэнэ
mypl ast   <file>     AST-ыг dump хийнэ
mypl help             help-г хэвлэнэ
mypl                  stdin-аас унших ба run горимоор ажиллана
```

#### Жишээ:

```bash
$ ./mypl run examples/family.logic
?- parent(bat, bold).
  true.
?- ancestor(bat, dorj).
  true.

$ ./mypl check examples/family.logic
OK: 2 facts, 2 rules, 2 queries.

$ ./mypl run tests/negative/err_missing_dot.logic
Parse error at line 3: syntax error
mypl: 1 parse error(s)
```

### 5.4 Хэрэглээний дараалал

1. `.logic` өргөтгөлтэй файл бичих.
2. Эхлээд **бүх баримт ба дүрмийг тунхаглах**, дараа нь асуултууд.
3. `mypl run yourfile.logic` ажиллуулах.

### 5.5 Хязгаарлалтууд

| Парметр                           | Хязгаар |
|-----------------------------------|---------|
| `MAX_FACTS`                       | 1000    |
| `MAX_RULES`                       | 1000    |
| `MAX_QUERIES`                     | 1000    |
| `MAX_ARGS` (predicate-д)          | 10      |
| `MAX_BODY` (rule body length)     | 10      |
| `MAX_BINDINGS` (substitution-д)   | 4096    |

Хэрэв хэт том программ зохиох бол `interpreter.h` болон `ast.h`-д
эдгээр макро-уудыг ихэсгэх боломжтой.

### 5.6 Алдааны мессеж

Parse алдаа line number-ийн хамт:

```
Lexical error at line 2: unexpected character '@'
Parse error at line 2: syntax error
mypl: 1 parse error(s)
```

Runtime алдаа (тодорхойлогдоогүй predicate, arity таарахгүй):

```
?- parent(bat).
  false.        # crash биш — proof engine "тохирох баримт/дүрэм
                # байхгүй" гэсэн утгаар false буцаана
```

---

## 6. Демо: жишээ программууд

5 жишээний бүгд `examples/` фолдерт байна.

### 6.1 `family.logic` — Гэр бүлийн харилцаа

```prolog
parent(bat, bold).
parent(bold, dorj).

ancestor(X, Y) :- parent(X, Y).
ancestor(X, Y) :- parent(X, Z), ancestor(Z, Y).

?- parent(bat, bold).
?- ancestor(bat, dorj).
```

**Гүйцэтгэл:**
```
?- parent(bat, bold).
  true.
?- ancestor(bat, dorj).
  true.
```

Recursive `ancestor` дүрмийг харуулна. `bat → bold → dorj` chain-ыг
хоёр алхмаар проф хийнэ.

### 6.2 `social.logic` — Нийгмийн сүлжээ

```prolog
person(alice). person(bob). person(charlie). person(diana). person(eve).

friend(alice, bob).
friend(bob, charlie).
friend(charlie, diana).
friend(diana, eve).

knows(X, Y) :- friend(X, Y).
knows(X, Y) :- friend(Y, X).        % симметрик

connection(X, Y) :- knows(X, Y).
connection(X, Y) :- knows(X, Z), knows(Z, Y).  % 2-hop

?- friend(alice, bob).
?- knows(alice, bob).
?- knows(bob, alice).               % симметрик дүрмээр true
?- connection(alice, charlie).      % alice→bob→charlie
?- connection(alice, diana).        % 3 hop хэрэгтэй — false
```

**Гүйцэтгэл:**
```
?- friend(alice, bob).         → true.
?- knows(alice, bob).          → true.
?- knows(bob, alice).          → true.
?- connection(alice, charlie). → true.
?- connection(alice, diana).   → false.
```

Симметрик харилцаа ба 2-hop transitive closure-ыг харуулна.

### 6.3 `graph.logic` — Граф traversal

```prolog
node(a). node(b). node(c). node(d). node(e).

edge(a, b). edge(b, c). edge(c, d). edge(d, e). edge(a, d).

connected(X, Y) :- edge(X, Y).

reachable(X, Y) :- connected(X, Y).
reachable(X, Y) :- connected(X, Z), reachable(Z, Y).

?- reachable(a, c).
?- reachable(a, e).
?- reachable(b, d).
?- reachable(e, a).
```

**Гүйцэтгэл:**
```
?- reachable(a, c). → true.   (a→b→c)
?- reachable(a, e). → true.   (a→b→c→d→e эсвэл a→d→e)
?- reachable(b, d). → true.
?- reachable(e, a). → false.  (нэг чигийн edge)
```

Multi-hop recursive дүрмийг харуулна. Энэ нь хуучны bug-ийн жишээ —
`reachable(a, c)` нь анх `false` буцааж байсныг proof engine-ийг
дахин бичиж засварласан.

### 6.4 `food.logic` — Хүнсний ангилал

```prolog
fruit(apple). fruit(banana). fruit(orange).
vegetable(carrot). vegetable(broccoli).
healthy(apple). healthy(carrot).

edible(X) :- fruit(X).
edible(X) :- vegetable(X).
good_for_health(X) :- healthy(X).

?- edible(apple).
?- edible(pizza).
?- good_for_health(carrot).
```

**Гүйцэтгэл:**
```
?- edible(apple).         → true.
?- edible(pizza).         → false.
?- good_for_health(carrot). → true.
```

`edible` дүрэм нь хоёр clause-аар (fruit OR vegetable) ажилладаг
union-of-categories загвар.

### 6.5 `animals.logic` — Амьтны таксономи + хувьсагчтай query

```prolog
mammal(dog). mammal(cat). mammal(whale). mammal(bat).
bird(eagle). bird(sparrow).
can_fly(eagle). can_fly(sparrow). can_fly(bat).
lives_in_water(whale).

animal(X) :- mammal(X).
animal(X) :- bird(X).

warm_blooded(X) :- mammal(X).
warm_blooded(X) :- bird(X).

flying_mammal(X) :- mammal(X), can_fly(X).
aquatic_warm_blooded(X) :- warm_blooded(X), lives_in_water(X).

?- flying_mammal(bat).
?- flying_mammal(eagle).
?- aquatic_warm_blooded(whale).
?- flying_mammal(X).         % хувьсагчтай query
```

**Гүйцэтгэл:**
```
?- flying_mammal(bat).             → true.
?- flying_mammal(eagle).           → false. (eagle нь bird, mammal биш)
?- aquatic_warm_blooded(whale).    → true.
?- flying_mammal(X).               → true, X = bat.    # ← variable binding
```

Олон шатлалт ангилал ба **хувьсагчийн bind хэвлэх** боломжийг харуулна.

---

## 7. Хавсралт: ажилласан тестүүд

### 7.1 Positive тестүүд (`tests/*.logic`)

| Файл                              | Зорилго                                  |
|-----------------------------------|------------------------------------------|
| `test_facts.logic`                | Зөвхөн баримт                            |
| `test_simple_rule.logic`          | Энгийн рекурсгүй дүрэм                   |
| `test_recursive_ancestor.logic`   | Рекурсив `ancestor`                      |
| `test_multiple_args.logic`        | Олон аргументтай predicate              |
| `test_multiple_body.logic`        | Олон body goal                           |
| `test_numbers.logic`              | Тоон atom                                |
| `test_deep_recursion.logic`       | Гүн рекурс                                |
| `test_multiple_rules.logic`       | Нэг predicate-д олон clause              |
| `test_chain.logic`                | Дүрмийн chain                            |
| `test_variable_binding.logic`     | Хувьсагчийн bind                          |
| `test_transitivity.logic`         | Transitive харилцаа                      |
| `test_relations.logic`            | Хоёрдмол харилцаа                        |
| `test_complex_rules.logic`        | Олон body                                |
| `test_colors.logic`               | Олон утгат                                |
| `test_database.logic`             | Database query                           |
| `test_indirect.logic`             | Шууд бус inference                       |
| `test_relations2.logic`           | Grandparent                              |
| `test_ownership.logic`            | Property                                  |
| `test_symmetric.logic`            | Симметрик харилцаа                       |
| `test_composite.logic`            | Composite дүрэм                          |

### 7.2 Negative тестүүд (`tests/negative/*.logic`)

| Файл                              | Алдааны төрөл                                  |
|-----------------------------------|-----------------------------------------------|
| `err_missing_dot.logic`           | Statement-ын төгсгөл `.` дутуу — parse error |
| `err_unbalanced_paren.logic`      | `(` хаагаагүй — parse error                  |
| `err_bad_token.logic`             | `@` тэмдэгт — lexical error                  |
| `err_query_no_args.logic`         | Аргументгүй query — parse error              |
| `err_undefined_predicate.logic`   | Тодорхойлогдоогүй predicate — runtime false  |
| `err_arity_mismatch.logic`        | Arity таарахгүй — runtime false              |

### 7.3 Тест ажиллуулах

```bash
$ make test
=== Positive tests (tests/*.logic) ===
=== Examples (examples/*.logic) ===
=== Negative tests (tests/negative/*.logic) ===

passed: 31
failed: 0
```

---

## 8. Хамгаалалтын асуултын жишээ ба хариулт

**А. Яагаад per-activation variable renaming хэрэгтэй вэ?**

Recursive дүрмийн хоёр activation нэг `X` нэртэй variable хуваалцвал бие
биенийхээ binding-ыг устгана. Тиймээс activation бүрд `X#1`, `X#2`
гэх мэт unique suffix өгч fresh namespace үүсгэх ёстой.

**Б. Яагаад proof engine рекурсив дуудлагад нэг substitution-ыг
overwrite хийж болохгүй вэ?**

`ancestor(X,Y) :- parent(X,Z), ancestor(Z,Y)` — body[0] нь `Z → bob`
гэж bind хийнэ. Дараагийн body[1] нь энэ Z-ийг ашиглах ёстой. Хэрэв
рекурсив дуудлага σ-г бүхэлд нь даран бичвэл `Z → bob` алдагдаж,
`ancestor(bob, jim)` proof хийгдэхгүй болно. Тиймээс σ-г checkpoint
+ rollback загвараар ашиглана.

**В. AST-гүйгээр шууд parser action-д proof хийж болох уу?**

Болохгүй. Удирдамжид «Parser-ын semantic action дотор шууд гүйцэтгэх
логик бичихийг хориглоно» гэж заасан. Бид eval_query-г queue хийдэг
бөгөөд yyparse дууссаны дараа л proof engine-ийг ажиллуулдаг.

**Г. Яагаад negation, cut, list зэрэг feature-уудыг оруулаагүй вэ?**

Эдгээр нь хэрэгжилтийн нарийн ширийн ойлголтуудтай, цөмийн ойлголт
(fact/rule/query/unification)-ийг харуулахад зайлшгүй биш. DESIGN.md-д
энэ сонголтыг тус бүрд нь үндэслэлжүүлсэн.

**Д. Backtracking хэрхэн ажилладаг вэ?**

Substitution-г стек шиг ашиглана: бid candidate турших бүрд
`mark = σ.count` гэж checkpoint авна, амжилтгүй болвол σ-г mark-руу
буцаана. Goal stack нь body goal-уудыг continuation-д prepend хийдэг
учир backtracking body goal хооронд ч алхаж чадна.

---

## Дүгнэлт

MyPL нь **20 positive + 6 negative тест + 5 нэгдсэн жишээ программтай**,
бүрэн функциональ logic programming хэл бөгөөд:

- **Синтакс** — Flex/Bison-аар тодорхойлогдсон, EBNF-д бичигдсэн
- **AST** — гурван бүтцэд (Fact, Rule, Query) хадгалагдана
- **Семантик** — SLD resolution, per-activation variable renaming,
  chronological backtracking
- **CLI** — run / check / ast subcommand-уудтай
- **Документаци** — syntax, semantics, design нь docs/-д бүрэн бичигдсэн

Бүх шаардлагыг хангаж, **31/31 тест амжилттай ажиллаж байна**.
