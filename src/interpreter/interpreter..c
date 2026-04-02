#include <stdio.h>
#include "interpreter.h"

Fact* fact_store[MAX_FACTS];
int fact_count = 0;
Rule* rule_store[MAX_RULES];
int rule_count = 0;

void add_fact(Fact* f) {
    if(fact_count<MAX_FACTS) fact_store[fact_count++] = f;
}

void add_rule(Rule* r) {
    if(rule_count<MAX_RULES) rule_store[rule_count++] = r;
}

void run_interpreter() {
    printf("=== Interpreter running ===\n");

    printf("Facts:\n");
    for(int i=0;i<fact_count;i++){
        printf(" %s(", fact_store[i]->name);
        for(int j=0;j<fact_store[i]->arg_count;j++){
            printf("%s", fact_store[i]->args[j]);
            if(j<fact_store[i]->arg_count-1) printf(", ");
        }
        printf(")\n");
    }

    printf("Rules:\n");
    for(int i=0;i<rule_count;i++){
        printf(" %s(", rule_store[i]->name);
        for(int j=0;j<rule_store[i]->arg_count;j++){
            printf("%s", rule_store[i]->args[j]);
            if(j<rule_store[i]->arg_count-1) printf(", ");
        }
        printf(") :- ");
        for(int j=0;j<rule_store[i]->body_count;j++){
            printf("%s", rule_store[i]->body[j]);
            if(j<rule_store[i]->body_count-1) printf(", ");
        }
        printf("\n");
    }
    printf("=== Done ===\n");
}