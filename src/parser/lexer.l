%{
#include "parser.tab.h"
extern int yylineno;
%}

%option yylineno

%%

"#"[^\n]*               ;                          /* line comment */
"%"[^\n]*               ;                          /* prolog-style comment */
":-"                    { return COLON_DASH; }
"?-"                    { return QUERY; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
","                     { return COMMA; }
"."                     { return DOT; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = strdup(yytext); return IDENT; }
[0-9]+                  { yylval.str = strdup(yytext); return IDENT; }
[ \t\r\n]+              ;
.                       { fprintf(stderr, "Lexical error at line %d: unexpected character '%c'\n", yylineno, yytext[0]); }

%%

int yywrap() { return 1; }
