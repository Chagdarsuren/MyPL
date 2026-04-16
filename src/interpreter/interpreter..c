#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "interpreter.h"

Fact *fact_store[MAX_FACTS];
int fact_count = 0;

Rule *rule_store[MAX_RULES];
int rule_count = 0;

// Substitution/bindings: maps variable names to values
#define MAX_VARS 50
typedef struct
{
    char *var;
    char *val;
} Binding;

typedef struct
{
    Binding bindings[MAX_VARS];
    int count;
} Substitution;

void add_fact(Fact *f)
{
    if (fact_count < MAX_FACTS)
        fact_store[fact_count++] = f;
}

void add_rule(Rule *r)
{
    if (rule_count < MAX_RULES)
        rule_store[rule_count++] = r;
}

// Check if a term is a variable (uppercase first letter)
int is_variable(const char *term)
{
    return term[0] >= 'A' && term[0] <= 'Z';
}

// Apply substitution to a term
char *apply_substitution(const char *term, Substitution *subst)
{
    if (!is_variable(term))
        return (char *)term;
    for (int i = 0; i < subst->count; i++)
    {
        if (strcmp(subst->bindings[i].var, term) == 0)
        {
            return subst->bindings[i].val;
        }
    }
    return (char *)term;
}

// Unify two terms with substitution
int unify(const char *t1, const char *t2, Substitution *subst)
{
    t1 = apply_substitution(t1, subst);
    t2 = apply_substitution(t2, subst);

    if (strcmp(t1, t2) == 0)
    {
        return 1;
    }

    if (is_variable(t1))
    {
        if (subst->count >= MAX_VARS)
            return 0;
        subst->bindings[subst->count].var = strdup(t1); // strdup to preserve the string
        subst->bindings[subst->count].val = strdup(t2); // strdup to preserve the string
        subst->count++;
        return 1;
    }

    if (is_variable(t2))
    {
        if (subst->count >= MAX_VARS)
            return 0;
        subst->bindings[subst->count].var = strdup(t2); // strdup to preserve the string
        subst->bindings[subst->count].val = strdup(t1); // strdup to preserve the string
        subst->count++;
        return 1;
    }

    return 0;
}

// Check if fact matches predicate with substitution
int unify_fact(Fact *f, const char *name, char *args[], int arg_count, Substitution *subst)
{
    if (strcmp(f->name, name) != 0 || f->arg_count != arg_count)
        return 0;

    Substitution temp = *subst;
    for (int i = 0; i < arg_count; i++)
    {
        if (!unify(f->args[i], args[i], &temp))
            return 0;
    }
    *subst = temp;
    return 1;
}

// Parse predicate string like "parent(X,Y)" into name and args
// Returns newly allocated strings, caller must free
typedef struct
{
    char name[50];
    char **args;
    int arg_count;
} ParsedPredicate;

ParsedPredicate *parse_predicate_new(const char *pred_str)
{
    ParsedPredicate *pp = malloc(sizeof(ParsedPredicate));
    pp->args = malloc(sizeof(char *) * MAX_ARGS);
    pp->arg_count = 0;

    const char *paren = strchr(pred_str, '(');
    if (!paren)
    {
        free(pp->args);
        free(pp);
        return NULL;
    }

    int name_len = paren - pred_str;
    strncpy(pp->name, pred_str, name_len);
    pp->name[name_len] = '\0';

    const char *curr = paren + 1;
    while (*curr && *curr != ')' && pp->arg_count < MAX_ARGS)
    {
        char arg[50] = {0};
        int arg_len = 0;
        // Skip whitespace
        while (*curr && (*curr == ' ' || *curr == '\t'))
            curr++;

        while (*curr && *curr != ',' && *curr != ')' && *curr != ' ' && *curr != '\t')
        {
            arg[arg_len++] = *curr++;
        }

        // Skip trailing whitespace
        while (*curr && (*curr == ' ' || *curr == '\t'))
            curr++;

        if (arg_len > 0)
        {
            pp->args[pp->arg_count] = strdup(arg);
            pp->arg_count++;
        }
        if (*curr == ',')
            curr++;
    }
    return pp;
}

void free_parsed_predicate(ParsedPredicate *pp)
{
    if (!pp)
        return;
    for (int i = 0; i < pp->arg_count; i++)
    {
        free(pp->args[i]);
    }
    free(pp->args);
    free(pp);
}

// Forward declaration
int prove(const char *query_name, char *query_args[], int query_arg_count, Substitution *subst);

// Prove a goal using facts and rules
int prove(const char *goal_name, char *goal_args[], int goal_arg_count, Substitution *subst)
{
    // Try to unify with facts
    for (int i = 0; i < fact_count; i++)
    {
        Substitution temp_subst = *subst;
        if (unify_fact(fact_store[i], goal_name, goal_args, goal_arg_count, &temp_subst))
        {
            *subst = temp_subst;
            return 1;
        }
    }

    // Try to unify with rules and prove body
    for (int i = 0; i < rule_count; i++)
    {
        Rule *rule = rule_store[i];
        if (strcmp(rule->name, goal_name) != 0 || rule->arg_count != goal_arg_count)
            continue;

        // Start with fresh substitution for this rule's variables
        // (goal_args are already substituted from parent context)
        Substitution rule_subst = {0};

        // Unify goal with rule head
        int head_unified = 1;
        for (int j = 0; j < goal_arg_count; j++)
        {
            if (!unify(goal_args[j], rule->args[j], &rule_subst))
            {
                head_unified = 0;
                break;
            }
        }

        if (!head_unified)
            continue;

        // Try to prove all body goals
        int all_proved = 1;
        for (int j = 0; j < rule->body_count; j++)
        {
            ParsedPredicate *body_pred = parse_predicate_new(rule->body[j]);
            if (!body_pred)
            {
                all_proved = 0;
                break;
            }

            // Apply current substitution to body goal arguments
            char *subst_args[MAX_ARGS];
            for (int k = 0; k < body_pred->arg_count; k++)
            {
                subst_args[k] = apply_substitution(body_pred->args[k], &rule_subst);
            }

            if (!prove(body_pred->name, subst_args, body_pred->arg_count, &rule_subst))
            {
                free_parsed_predicate(body_pred);
                all_proved = 0;
                break;
            }

            free_parsed_predicate(body_pred);
        }

        if (all_proved)
        {
            *subst = rule_subst;
            return 1;
        }
    }

    return 0;
}

// Query evaluate with rule-based inference
void eval_query(Query *q)
{
    printf("Query: %s(", q->name);
    for (int i = 0; i < q->arg_count; i++)
    {
        printf("%s", q->args[i]);
        if (i < q->arg_count - 1)
            printf(",");
    }
    printf(")\nResults:\n");

    Substitution subst = {0};
    if (prove(q->name, q->args, q->arg_count, &subst))
    {
        printf(" true\n");
    }
    else
    {
        printf(" false\n");
    }
}

void run_interpreter()
{
    printf("=== Interpreter ===\n");

    printf("Facts:\n");
    for (int i = 0; i < fact_count; i++)
    {
        printf(" %s(", fact_store[i]->name);
        for (int j = 0; j < fact_store[i]->arg_count; j++)
        {
            printf("%s", fact_store[i]->args[j]);
            if (j < fact_store[i]->arg_count - 1)
                printf(", ");
        }
        printf(")\n");
    }

    printf("Rules:\n");
    for (int i = 0; i < rule_count; i++)
    {
        printf(" %s(", rule_store[i]->name);
        for (int j = 0; j < rule_store[i]->arg_count; j++)
        {
            printf("%s", rule_store[i]->args[j]);
            if (j < rule_store[i]->arg_count - 1)
                printf(", ");
        }
        printf(") :- ");
        for (int j = 0; j < rule_store[i]->body_count; j++)
        {
            printf("%s", rule_store[i]->body[j]);
            if (j < rule_store[i]->body_count - 1)
                printf(", ");
        }
        printf("\n");
    }
    printf("=== Done ===\n");
}