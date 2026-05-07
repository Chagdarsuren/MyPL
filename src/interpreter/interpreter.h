#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "../ast/ast.h"

#define MAX_FACTS 1000
#define MAX_RULES 1000
#define MAX_QUERIES 1000

extern Fact* fact_store[MAX_FACTS];
extern int fact_count;

extern Rule* rule_store[MAX_RULES];
extern int rule_count;

void add_fact(Fact* f);
void add_rule(Rule* r);

/* Called from parser action: enqueues for later, does NOT execute. */
void eval_query(Query* q);

/* Run after parsing completes. */
void run_all_queries(void);
void dump_program(void);
int  query_count_pending(void);

#endif
