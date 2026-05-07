#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"

/* ===== Fact / rule / query stores ===================================== */

Fact *fact_store[MAX_FACTS];
int   fact_count = 0;

Rule *rule_store[MAX_RULES];
int   rule_count = 0;

static Query *query_queue[MAX_QUERIES];
static int    query_queue_count = 0;

void add_fact(Fact *f) {
    if (fact_count < MAX_FACTS) fact_store[fact_count++] = f;
}

void add_rule(Rule *r) {
    if (rule_count < MAX_RULES) rule_store[rule_count++] = r;
}

void eval_query(Query *q) {
    if (query_queue_count < MAX_QUERIES) query_queue[query_queue_count++] = q;
}

int query_count_pending(void) { return query_queue_count; }

/* ===== Substitution =================================================== */

#define MAX_BINDINGS 4096

typedef struct {
    char *var;
    char *val;
} Binding;

typedef struct {
    Binding b[MAX_BINDINGS];
    int     count;
} Subst;

static int g_fresh_id = 0;   /* monotonic counter for variable renaming */

static int is_variable(const char *t) {
    return t && (t[0] >= 'A' && t[0] <= 'Z');
}

/* Follow the substitution chain until we hit a non-variable or an
 * unbound variable. */
static const char *walk(const char *t, Subst *s) {
    while (is_variable(t)) {
        const char *next = NULL;
        for (int i = 0; i < s->count; i++) {
            if (strcmp(s->b[i].var, t) == 0) { next = s->b[i].val; break; }
        }
        if (!next || strcmp(next, t) == 0) return t;
        t = next;
    }
    return t;
}

static int bind_var(Subst *s, const char *var, const char *val) {
    if (s->count >= MAX_BINDINGS) return 0;
    s->b[s->count].var = strdup(var);
    s->b[s->count].val = strdup(val);
    s->count++;
    return 1;
}

static int unify(const char *a, const char *b, Subst *s) {
    a = walk(a, s);
    b = walk(b, s);
    if (strcmp(a, b) == 0) return 1;
    if (is_variable(a)) return bind_var(s, a, b);
    if (is_variable(b)) return bind_var(s, b, a);
    return 0;
}

static void undo(Subst *s, int mark) {
    while (s->count > mark) {
        s->count--;
        free(s->b[s->count].var);
        free(s->b[s->count].val);
    }
}

/* ===== Goal: parsed predicate ========================================= */

typedef struct {
    char *name;
    int   arg_count;
    char *args[MAX_ARGS];
} Goal;

static char *rename_term(const char *t, int sfx) {
    if (is_variable(t)) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%s#%d", t, sfx);
        return strdup(buf);
    }
    return strdup(t);
}

/* Parse "name(a, b, c)" into a Goal. Allocates name and args. */
static int parse_pred(const char *s, Goal *out) {
    const char *lp = strchr(s, '(');
    if (!lp) return 0;
    int nlen = (int)(lp - s);
    out->name = (char*)malloc(nlen + 1);
    memcpy(out->name, s, nlen);
    out->name[nlen] = '\0';
    out->arg_count = 0;
    const char *p = lp + 1;
    while (*p && *p != ')') {
        while (*p == ' ' || *p == '\t') p++;
        char buf[128];
        int  bi = 0;
        while (*p && *p != ',' && *p != ')' && *p != ' ' && *p != '\t' && bi < 127)
            buf[bi++] = *p++;
        buf[bi] = '\0';
        while (*p == ' ' || *p == '\t') p++;
        if (bi > 0 && out->arg_count < MAX_ARGS)
            out->args[out->arg_count++] = strdup(buf);
        if (*p == ',') p++;
    }
    return 1;
}

static void free_goal(Goal *g) {
    free(g->name);
    for (int i = 0; i < g->arg_count; i++) free(g->args[i]);
}

/* ===== Proof engine ==================================================== */

static int prove_conj(Goal *goals, int n, int idx, Subst *s);

static int prove_via_facts(Goal *g, Goal *goals, int n, int idx, Subst *s) {
    for (int i = 0; i < fact_count; i++) {
        Fact *f = fact_store[i];
        if (strcmp(f->name, g->name) != 0 || f->arg_count != g->arg_count) continue;
        int mark = s->count;
        int ok = 1;
        for (int j = 0; j < g->arg_count; j++) {
            if (!unify(f->args[j], g->args[j], s)) { ok = 0; break; }
        }
        if (ok && prove_conj(goals, n, idx + 1, s)) return 1;
        undo(s, mark);
    }
    return 0;
}

static int prove_via_rules(Goal *g, Goal *goals, int n, int idx, Subst *s) {
    for (int i = 0; i < rule_count; i++) {
        Rule *r = rule_store[i];
        if (strcmp(r->name, g->name) != 0 || r->arg_count != g->arg_count) continue;

        int sfx = ++g_fresh_id;

        char *fhead[MAX_ARGS];
        for (int j = 0; j < r->arg_count; j++) fhead[j] = rename_term(r->args[j], sfx);

        int   rest = n - idx - 1;
        int   new_n = r->body_count + rest;
        Goal *new_goals  = (new_n > 0)        ? (Goal*)malloc(sizeof(Goal) * new_n)        : NULL;
        Goal *body_owned = (r->body_count > 0)? (Goal*)malloc(sizeof(Goal) * r->body_count): NULL;

        int parse_ok = 1;
        for (int j = 0; j < r->body_count; j++) {
            Goal raw;
            if (!parse_pred(r->body[j], &raw)) { parse_ok = 0; break; }
            body_owned[j].name = strdup(raw.name);
            body_owned[j].arg_count = raw.arg_count;
            for (int k = 0; k < raw.arg_count; k++)
                body_owned[j].args[k] = rename_term(raw.args[k], sfx);
            free_goal(&raw);
            new_goals[j] = body_owned[j];      /* shallow ref; ownership in body_owned */
        }
        for (int j = 0; j < rest; j++) {
            new_goals[r->body_count + j] = goals[idx + 1 + j];   /* shallow ref */
        }

        int success = 0;
        if (parse_ok) {
            int mark = s->count;
            int head_ok = 1;
            for (int j = 0; j < g->arg_count; j++) {
                if (!unify(fhead[j], g->args[j], s)) { head_ok = 0; break; }
            }
            if (head_ok && prove_conj(new_goals, new_n, 0, s)) success = 1;
            if (!success) undo(s, mark);
        }

        for (int j = 0; j < r->arg_count; j++) free(fhead[j]);
        if (body_owned) {
            for (int j = 0; j < r->body_count; j++) free_goal(&body_owned[j]);
            free(body_owned);
        }
        free(new_goals);

        if (success) return 1;
    }
    return 0;
}

static int prove_conj(Goal *goals, int n, int idx, Subst *s) {
    if (idx >= n) return 1;
    Goal *g = &goals[idx];
    if (prove_via_facts(g, goals, n, idx, s)) return 1;
    if (prove_via_rules(g, goals, n, idx, s)) return 1;
    return 0;
}

/* ===== Query reporting ================================================ */

static const char *display_term(const char *walked) {
    if (!walked) return "_";
    if (is_variable(walked)) return "_";
    return walked;
}

static void print_query_head(const Query *q) {
    printf("?- %s(", q->name);
    for (int i = 0; i < q->arg_count; i++) {
        printf("%s", q->args[i]);
        if (i < q->arg_count - 1) printf(", ");
    }
    printf(").\n");
}

static void eval_one_query(Query *q) {
    Goal g;
    g.name = strdup(q->name);
    g.arg_count = q->arg_count;
    for (int i = 0; i < q->arg_count; i++) g.args[i] = strdup(q->args[i]);

    char *qvars[MAX_ARGS];
    int   qvar_count = 0;
    for (int i = 0; i < q->arg_count; i++) {
        if (!is_variable(q->args[i])) continue;
        int seen = 0;
        for (int j = 0; j < qvar_count; j++)
            if (strcmp(qvars[j], q->args[i]) == 0) { seen = 1; break; }
        if (!seen) qvars[qvar_count++] = q->args[i];
    }

    Subst s; s.count = 0;
    print_query_head(q);

    if (prove_conj(&g, 1, 0, &s)) {
        if (qvar_count == 0) {
            printf("  true.\n");
        } else {
            printf("  true");
            for (int i = 0; i < qvar_count; i++) {
                const char *v = walk(qvars[i], &s);
                printf(", %s = %s", qvars[i], display_term(v));
            }
            printf(".\n");
        }
    } else {
        printf("  false.\n");
    }
    undo(&s, 0);
    free_goal(&g);
}

void run_all_queries(void) {
    for (int i = 0; i < query_queue_count; i++) eval_one_query(query_queue[i]);
}

void dump_program(void) {
    printf("=== Facts (%d) ===\n", fact_count);
    for (int i = 0; i < fact_count; i++) {
        printf("  %s(", fact_store[i]->name);
        for (int j = 0; j < fact_store[i]->arg_count; j++) {
            printf("%s", fact_store[i]->args[j]);
            if (j < fact_store[i]->arg_count - 1) printf(", ");
        }
        printf(").\n");
    }
    printf("=== Rules (%d) ===\n", rule_count);
    for (int i = 0; i < rule_count; i++) {
        printf("  %s(", rule_store[i]->name);
        for (int j = 0; j < rule_store[i]->arg_count; j++) {
            printf("%s", rule_store[i]->args[j]);
            if (j < rule_store[i]->arg_count - 1) printf(", ");
        }
        printf(") :- ");
        for (int j = 0; j < rule_store[i]->body_count; j++) {
            printf("%s", rule_store[i]->body[j]);
            if (j < rule_store[i]->body_count - 1) printf(", ");
        }
        printf(".\n");
    }
    printf("=== Queries (%d) ===\n", query_queue_count);
    for (int i = 0; i < query_queue_count; i++) {
        printf("  ");
        print_query_head(query_queue[i]);
    }
}
