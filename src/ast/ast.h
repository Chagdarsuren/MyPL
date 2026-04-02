#ifndef AST_H
#define AST_H

#define MAX_ARGS 10
#define MAX_BODY 10

// Fact (баримт)
typedef struct {
    char* name;            // parent
    char* args[MAX_ARGS];   // ["bat","bold"]
    int arg_count;
} Fact;

// Rule (дүрэм)
typedef struct {
    char* name;               // grandparent
    char* args[MAX_ARGS];      // ["X","Y"]
    int arg_count;
    char* body[MAX_BODY];      // ["parent(X,Z)", "parent(Z,Y)"]
    int body_count;
} Rule;

// Query (асуулт)
typedef struct {
    char* name;
    char* args[MAX_ARGS];
    int arg_count;
} Query;

// Function prototypes
Fact* create_fact(char* name, char* args[], int arg_count);
Rule* create_rule(char* name, char* args[], int arg_count, char* body[], int body_count);
Query* create_query(char* name, char* args[], int arg_count);

#endif