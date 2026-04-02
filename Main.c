#include <stdio.h>
#include "src/interpreter/interpreter.h"

int yyparse();
void run_interpreter();

int main() {
    printf("=== MyPL ===\n");
    printf("Parsing input...\n");

    if(yyparse()!=0) {
        printf("Parse error!\n");
        return 1;
    }

    printf("Running interpreter...\n");
    run_interpreter();
    return 0;
}