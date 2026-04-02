#ifndef AST_H
#define AST_H

#define MAX_ARGS 10
#define MAX_BODY 10

// Fact struct
typedef struct {
    char* name;
    char* args[MAX_ARGS];
    int arg_count;
} Fact;

// Rule struct
typedef struct {
    char* name;
    char* args[MAX_ARGS];
    int arg_count;
    char* body[MAX_BODY];
    int body_count;
} Rule;

// Query struct
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