#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Create Fact
Fact* create_fact(char* name, char* args[], int arg_count) {
    Fact* f = malloc(sizeof(Fact));
    f->name = strdup(name);
    f->arg_count = arg_count;
    for(int i=0;i<arg_count;i++) f->args[i] = strdup(args[i]);
    return f;
}

// Create Rule
Rule* create_rule(char* name, char* args[], int arg_count, char* body[], int body_count) {
    Rule* r = malloc(sizeof(Rule));
    r->name = strdup(name);
    r->arg_count = arg_count;
    for(int i=0;i<arg_count;i++) r->args[i] = strdup(args[i]);
    r->body_count = body_count;
    for(int i=0;i<body_count;i++) r->body[i] = strdup(body[i]);
    return r;
}

// Create Query
Query* create_query(char* name, char* args[], int arg_count) {
    Query* q = malloc(sizeof(Query));
    q->name = strdup(name);
    q->arg_count = arg_count;
    for(int i=0;i<arg_count;i++) q->args[i] = strdup(args[i]);
    return q;
}