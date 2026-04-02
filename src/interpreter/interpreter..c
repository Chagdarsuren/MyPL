#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "interpreter.h"

Fact* fact_store[MAX_FACTS];
int fact_count = 0;

Rule* rule_store[MAX_RULES];
int rule_count = 0;

void add_fact(Fact* f) {
    if(fact_count < MAX_FACTS) fact_store[fact_count++] = f;
}

void add_rule(Rule* r) {
    if(rule_count < MAX_RULES) rule_store[rule_count++] = r;
}

// simple pattern match: variable X,Y,Z (uppercase) treated as wildcard
int match(Fact* f, char* name, char* args[], int arg_count) {
    if(strcmp(f->name,name)!=0 || f->arg_count!=arg_count) return 0;
    for(int i=0;i<arg_count;i++){
        if(args[i][0]>='A' && args[i][0]<='Z') continue; // variable
        if(strcmp(f->args[i],args[i])!=0) return 0;
    }
    return 1;
}

// Query evaluate (simple: only search facts, no recursion yet)
void eval_query(Query* q){
    printf("Query: %s(", q->name);
    for(int i=0;i<q->arg_count;i++){
        printf("%s",q->args[i]);
        if(i<q->arg_count-1) printf(",");
    }
    printf(")\nResults:\n");

    int found = 0;
    for(int i=0;i<fact_count;i++){
        if(match(fact_store[i], q->name, q->args, q->arg_count)){
            printf(" - ");
            for(int j=0;j<q->arg_count;j++) printf("%s%s", fact_store[i]->args[j], j<q->arg_count-1?",":"");
            printf("\n");
            found=1;
        }
    }
    if(!found) printf(" No matching facts found.\n");
}

void run_interpreter(){
    printf("=== Interpreter ===\n");

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