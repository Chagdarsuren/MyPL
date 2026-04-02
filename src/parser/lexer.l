%{
#include "parser.tab.h"
%}

%%

[a-zA-Z][a-zA-Z0-9_]*   { yylval.str=strdup(yytext); return IDENT; }
"("                      { return LPAREN; }
")"                      { return RPAREN; }
","                      { return COMMA; }
"."                      { return DOT; }
":-"                     { return COLON_DASH; }
"\\?-"                   { return QUERY; }
[ \t\r\n]+               ;

%%

int yywrap() { return 1; }