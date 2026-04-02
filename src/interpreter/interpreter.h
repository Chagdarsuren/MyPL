#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "../ast/ast.h"

#define MAX_FACTS 100
#define MAX_RULES 100

extern Fact* fact_store[MAX_FACTS];
extern int fact_count;

extern Rule* rule_store[MAX_RULES];
extern int rule_count;

void add_fact(Fact* f);
void add_rule(Rule* r);
void run_interpreter();

// Шинэ нэмэлт
void eval_query(Query* q);

#endif