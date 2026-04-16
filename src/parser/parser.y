%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/ast/ast.h"
#include "src/interpreter/interpreter.h"

extern int yylex();
void yyerror(const char* s);

// Temporary storage for parser actions
char* args_array[10];
int arg_count = 0;

char* temp_args[10];
int temp_arg_count = 0;

char* body_array[10];
int body_count = 0;

// Helper function to construct body item string
char* construct_body_item(char* name, char* args[], int count) {
    char buf[256];
    int pos = sprintf(buf, "%s(", name);
    for (int i = 0; i < count; i++) {
        pos += sprintf(buf + pos, "%s", args[i]);
        if (i < count - 1) pos += sprintf(buf + pos, ",");
    }
    pos += sprintf(buf + pos, ")");
    return strdup(buf);
}
%}

%union { char* str; }

%token <str> IDENT
%token LPAREN RPAREN COMMA DOT COLON_DASH QUERY

%type <str> argument body_argument body_item fact rule query

%%

program:
    statements
;

statements:
      statement statements
    | /* empty */
;

statement:
    fact DOT { Fact* f=create_fact($1,args_array,arg_count); add_fact(f); arg_count=0; }
  | rule DOT { Rule* r=create_rule($1,args_array,arg_count,body_array,body_count); add_rule(r); arg_count=body_count=0; }
  | query DOT { Query* q=create_query($1,args_array,arg_count); eval_query(q); arg_count=0; }
;

// Fact: IDENT(args)
fact:
    IDENT LPAREN arguments RPAREN { $$=$1; }
;

// Rule: IDENT(args) :- body_items
rule:
    IDENT LPAREN arguments RPAREN COLON_DASH body_items { $$=$1; }
;

// Query: ?- IDENT(args)
query:
    QUERY IDENT LPAREN arguments RPAREN { $$=$2; }
;

// Arguments list for heads
arguments:
      argument
    | argument COMMA arguments
;

argument:
    IDENT { args_array[arg_count++]=$1; }
;

// Body items for rule
body_items:
      body_item
    | body_item COMMA body_items
;

body_item:
    IDENT LPAREN body_arguments RPAREN { 
        body_array[body_count++] = construct_body_item($1, temp_args, temp_arg_count);
        temp_arg_count = 0;
    }
;

// Separate arguments for body items
body_arguments:
      body_argument
    | body_argument COMMA body_arguments
;

body_argument:
    IDENT { temp_args[temp_arg_count++]=$1; }
;

%%

void yyerror(const char* s) { fprintf(stderr,"Parse error: %s\n",s); }