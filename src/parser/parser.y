%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/ast/ast.h"
#include "src/interpreter/interpreter.h"

extern int yylex();
extern int yylineno;
void yyerror(const char* s);

int parse_error_count = 0;

/* Parser action scratch buffers */
static char* args_array[MAX_ARGS];
static int   arg_count = 0;

static char* temp_args[MAX_ARGS];
static int   temp_arg_count = 0;

static char* body_array[MAX_BODY];
static int   body_count = 0;

/* Helper: serialise a body item back to "name(a,b,c)" form. */
static char* construct_body_item(const char* name, char* args[], int count) {
    char buf[512];
    int  pos = snprintf(buf, sizeof(buf), "%s(", name);
    for (int i = 0; i < count; i++) {
        pos += snprintf(buf + pos, sizeof(buf) - pos, "%s", args[i]);
        if (i < count - 1)
            pos += snprintf(buf + pos, sizeof(buf) - pos, ",");
    }
    snprintf(buf + pos, sizeof(buf) - pos, ")");
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
      fact  DOT { Fact*  f = create_fact($1, args_array, arg_count);
                  add_fact(f);
                  arg_count = 0; }
    | rule  DOT { Rule*  r = create_rule($1, args_array, arg_count, body_array, body_count);
                  add_rule(r);
                  arg_count = body_count = 0; }
    | query DOT { Query* q = create_query($1, args_array, arg_count);
                  eval_query(q);          /* enqueues; does not execute */
                  arg_count = 0; }
    ;

fact:
      IDENT LPAREN arguments RPAREN { $$ = $1; }
    ;

rule:
      IDENT LPAREN arguments RPAREN COLON_DASH body_items { $$ = $1; }
    ;

query:
      QUERY IDENT LPAREN arguments RPAREN { $$ = $2; }
    ;

arguments:
      argument
    | argument COMMA arguments
    ;

argument:
      IDENT { args_array[arg_count++] = $1; }
    ;

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

body_arguments:
      body_argument
    | body_argument COMMA body_arguments
    ;

body_argument:
      IDENT { temp_args[temp_arg_count++] = $1; }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
    parse_error_count++;
}
