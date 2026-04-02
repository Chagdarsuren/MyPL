%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"
#include "../interpreter/interpreter.h"

extern int yylex();
void yyerror(const char* s);

// Temporary storage for parser actions
char* args_array[10];
int arg_count = 0;
char* body_array[10];
int body_count = 0;
%}

%union { char* str; }

%token <str> IDENT
%token LPAREN RPAREN COMMA DOT COLON_DASH QUERY

%type <str> argument body_item

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
  | query DOT { Query* q=create_query($2,args_array,arg_count); eval_query(q); arg_count=0; }
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

// Arguments list
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
    IDENT LPAREN arguments RPAREN { body_array[body_count++] = strdup(yytext); }
;

%%

void yyerror(const char* s) { fprintf(stderr,"Parse error: %s\n",s); }