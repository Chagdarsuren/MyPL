# MyPL - Logic Programming Language Design Document

## 1. Language Purpose and Scope

**MyPL** is a simplified logic programming language designed to implement core concepts from Prolog and logic-based systems. The language is intended for educational purposes to demonstrate:

- Functional programming paradigms
- Pattern matching and unification
- Recursive rule evaluation
- Backtracking search (basic implementation)
- AST-based interpretation

### Target Use Cases:

- Representing facts and rules about relationships (family trees, social networks, etc.)
- Querying logical assertions
- Solving constraint satisfaction problems
- Educational demonstrations of logic programming concepts

---

## 2. Language Syntax

### Tokens

- **Identifiers**: Lowercase letters starting predicates and atoms (e.g., `parent`, `john`)
- **Variables**: Uppercase letters (e.g., `X`, `Y`, `Ancestor`)
- **Operators**: `:-` (rule definition), `?-` (query)
- **Punctuation**: `(` `)` `,` `.`

### Grammar

```
program       := statements
statements    := statement statements | ε
statement     := fact | rule | query
fact          := IDENT(args).
rule          := IDENT(args) :- body.
query         := ?- IDENT(args).
args          := arg | arg, args
arg           := IDENT
body          := predicate | predicate, body
predicate     := IDENT(args)
```

### Examples

```prolog
% Fact: John is the parent of Mary
parent(john, mary).

% Rule: X is an ancestor of Y if X is parent of Y
%       OR X is parent of Z and Z is ancestor of Y
ancestor(X, Y) :- parent(X, Y).
ancestor(X, Y) :- parent(X, Z), ancestor(Z, Y).

% Query: Is john an ancestor of mary?
?- ancestor(john, mary).
```

---

## 3. Semantic Model

### Core Concepts

#### 3.1 Facts

- Ground truths about the world
- No variables allowed in facts
- Stored in a fact database during execution
- Example: `parent(john, mary).`

#### 3.2 Rules

- Conditional statements defining new relationships
- Form: `head :- body`
- Head: single predicate (with possible variables)
- Body: one or more predicates (goals)
- Goal: predicate that must be satisfied
- Example: `ancestor(X, Y) :- parent(X, Y), ancestor(Z, Y).`

#### 3.3 Queries

- Test if a statement is true given known facts and rules
- Form: `?- predicate(args).`
- Answers: `true` or `false`
- Variables in queries are treated as existential (does there exist binding?)

#### 3.4 Unification

- Matching process between query and facts/rules
- Variables can bind to concrete values
- Example: Querying `parent(X, mary)` binds `X` to values of all parents of Mary (if implemented with backtracking)

#### 3.5 Pattern Matching

- Core operation: does query match a fact?
- Variables in queries/rules match any atom
- Atoms must match exactly
- Example: Query `parent(john, Y)` matches fact `parent(john, mary)` with binding `Y = mary`

---

## 4. Execution Model

### Interpretation Strategy

**MyPL** uses **depth-first search with goal proving**:

1. **Parsing Phase**
   - Lexer tokenizes input (flex/lexer.l)
   - Parser builds AST (bison/parser.y)
   - Facts and rules stored in memory

2. **Query Evaluation Phase**
   - For each query, invoke `prove(goal_name, goal_args)`
   - Attempt to match against facts or rules

3. **Proving a Goal**

   ```
   prove(G) =
     - Check if G matches any fact → return true
     - For each rule with head H matching G:
       - Unify query goals with rule head
       - Try to prove all body goals
       - If all succeed, return true
     - Return false if no fact/rule succeeds
   ```

4. **Variable Binding (Substitution)**
   - Maintain bindings table: Variable → Value
   - Apply substitutions before comparing terms
   - Fresh variable scope for each rule (avoid variable shadowing)

### Example Execution Trace

```
Query: ancestor(john, ann)

Step 1: Try rule 0: ancestor(X, Y) :- parent(X, Y)
  - Unify: john=X, ann=Y
  - Prove body: parent(john, ann) → false

Step 2: Try rule 1: ancestor(X, Y) :- parent(X, Z), ancestor(Z, Y)
  - Unify: john=X, ann=Y
  - Prove body goal 0: parent(john, Z) → true, Z=mary
  - Prove body goal 1: ancestor(mary, ann)
    - Recursively try rules...
    - ancestor(mary, ann) → true
  - All body goals proven → return true

Result: true
```

---

## 5. Design Choices and Rationale

### 5.1 Chosen Features

1. **Depth-First Search (DFS)**
   - Simpler to implement than breadth-first
   - Natural fit with recursive rule evaluation
   - Memory efficient for reasonably-sized problems

2. **Single-Answer Queries**
   - Queries return true/false, not variable bindings
   - Simplifies query output
   - Sufficient for demonstrating logic programming

3. **Facts and Rules Separation**
   - Clean distinction between ground facts and rules
   - Improves performance (facts checked before rules)
   - Aligns with Prolog conventions

4. **Fresh Variable Scoping**
   - Each rule gets fresh variables (no shadowing across rule instances)
   - Prevents variable conflicts in recursive calls
   - Critical for correctness of recursive rules

5. **Ground Atoms Only**
   - Atoms (lowercase identifiers) are constant symbols
   - No complex term structures (no lists, functors)
   - Keeps parsing simple

### 5.2 Intentionally Omitted Features

1. **Backtracking / Multiple Solutions**
   - **Why omitted**: Would require choice points and result enumeration
   - **Alternative**: Return only first solution (true/false)
   - **Impact**: Queries like "find all parents of john" return true/false, not bindings

2. **Cuts (!)**
   - **Why omitted**: Implementation complexity, requires choice point stack
   - **Alternative**: DFS naturally prunes alternatives after success

3. **Negation (\\+)**
   - **Why omitted**: Requires negation as failure, which conflicts with simple DFS
   - **Alternative**: Not supported in this version

4. **Built-in Predicates**
   - **Why omitted**: Would require parameter marshaling, type coercion
   - **Alternative**: Users must explicitly assert arithmetic results as facts

5. **List Support**
   - **Why omitted**: Requires complex pattern matching syntax and parsing
   - **Alternative**: Represent lists as nested facts or predicates

6. **Assert/Retract (dynamic predicates)**
   - **Why omitted**: Would require runtime fact/rule modification
   - **Alternative**: All facts and rules must be declared before queries

7. **DCG (Definite Clause Grammars)**
   - **Why omitted**: Advanced feature not needed for basic logic programming
   - **Alternative**: Manual predicate definitions for parsing

---

## 6. Type System

**MyPL does not have explicit types**. instead:

- **Atoms**: Untyped constants (lowercase identifiers)
- **Variables**: Can unify with any atom
- **Predicates**: No arity checking or type signatures required
- **Operations**: All operations are symbolic (no arithmetic evaluation)

### Type Safety Notes

- Mismatched arities in facts vs. queries result in false (not errors)
- Variables are implicitly typed as "any term"

---

## 7. Memory and Performance Considerations

### Memory Usage

- Facts stored as Fact structs in array (fixed MAX_FACTS=100)
- Rules stored as Rule structs in array (fixed MAX_RULES=100)
- Substitutions stored in array (fixed MAX_VARS=50 bindings per query)
- Parsed body predicates allocated/freed per evaluation

### Performance Characteristics

- Fact matching: O(n) linear scan
- Rule matching: O(n) \* O(rule body complexity)
- Recursive rules: Exponential worst case (e.g., ancestor chains)
- No indexing or compilation optimizations

---

## 8. AST Representation

The language compiles to an **Abstract Syntax Tree** before interpretation:

```c
// Fact: predicate name + arguments
typedef struct {
    char* name;
    char* args[MAX_ARGS];
    int arg_count;
} Fact;

// Rule: head + body goals (represented as strings)
typedef struct {
    char* name;
    char* args[MAX_ARGS];
    int arg_count;
    char* body[MAX_BODY];      // body[i] = "predicate(X,Y,...)"
    int body_count;
} Rule;

// Query: same structure as fact
typedef struct {
    char* name;
    char* args[MAX_ARGS];
    int arg_count;
} Query;
```

---

## 9. Implementation Notes

### Parser (Bison)

- Constructs AST during parsing via semantic actions
- Calls `create_fact`, `create_rule`, `create_query` helper functions
- Adds facts/rules to global store immediately

### Interpreter (C)

- **prove()**: Core recursive proof engine
- **unify()**: Unifies two terms with variable bindings
- **apply_substitution()**: Looks up variable bindings
- Main loop: iterate facts/rules, attempt unification

### Lexer (Flex)

- Tokenizes input: identifiers, operators, punctuation
- Skip whitespace and comments

---

## 10. Limitations and Future Work

### Limitations

- Fixed array sizes (no dynamic growth)
- No error recovery (parse errors terminate)
- No tail-call optimization (may stack overflow on deep recursion)
- Single fact per assertion (no compound terms)
- No module system or namespacing
- Limited debugging support

### Possible Future Extensions

1. **Backtracking with choice points** - enumerate all solutions
2. **List support** - Pattern matching on [Head|Tail] structures
3. **Cut operator (!)** - Control backtracking
4. **Arithmetic evaluation** - is/2, =:=/2, </2 predicates
5. **Assert/Retract** - Dynamic fact/rule modification
6. **Debugging** - Trace mode showing proof steps
7. **Performance** - Indexing on predicate names/first argument
8. **Module system** - Namespace separation
9. **Definite Clause Grammars** - For parsing domain-specific languages

---

## 11. Examples

### Example 1: Family Relations

```prolog
parent(tom, bob).
parent(bob, pat).

grandparent(X, Y) :- parent(X, Z), parent(Z, Y).

?- grandparent(tom, pat).    % true
?- parent(tom, pat).          % false
```

### Example 2: Social Network

```prolog
friend(alice, bob).
friend(bob, charlie).

knows(X, Y) :- friend(X, Y).
knows(X, Y) :- friend(X, Z), knows(Z, Y).

?- knows(alice, charlie).     % true
```

### Example 3: Reachability in Graph

```prolog
edge(a, b).
edge(b, c).
edge(c, d).

reach(X, Y) :- edge(X, Y).
reach(X, Y) :- edge(X, Z), reach(Z, Y).

?- reach(a, d).               % true
```

---

## 12. Conclusion

MyPL demonstrates core logic programming concepts through a simplified, AST-based interpreter. While lacking many advanced features of Prolog, it successfully implements:

- Fact and rule representation
- Unification and pattern matching
- Recursive rule evaluation
- Query answering through goal proving

The clean separation between parsing (using Bison/Flex) and interpretation (custom C code) makes the system educational and extensible.
