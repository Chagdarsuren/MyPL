#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/interpreter/interpreter.h"

extern int   yyparse(void);
extern FILE* yyin;
extern int   parse_error_count;

static void usage(const char* prog) {
    fprintf(stderr,
        "MyPL — a small logic programming language\n"
        "\n"
        "Usage:\n"
        "  %s run   <file>     Parse and execute queries (default if a file is given)\n"
        "  %s check <file>     Parse only; report syntax errors and program stats\n"
        "  %s ast   <file>     Parse and dump the AST (facts, rules, queries)\n"
        "  %s help              Show this help\n"
        "  %s                   (no args) read source from stdin and run\n",
        prog, prog, prog, prog, prog);
}

typedef enum { MODE_RUN, MODE_CHECK, MODE_AST } Mode;

int main(int argc, char** argv) {
    Mode  mode = MODE_RUN;
    const char* path = NULL;

    if (argc == 1) {
        /* stdin run mode */
    } else if (argc == 2) {
        if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0 ||
            strcmp(argv[1], "--help") == 0) {
            usage(argv[0]);
            return 0;
        }
        /* `mypl <file>` — assume run */
        path = argv[1];
    } else if (argc == 3) {
        if      (strcmp(argv[1], "run")   == 0) mode = MODE_RUN;
        else if (strcmp(argv[1], "check") == 0) mode = MODE_CHECK;
        else if (strcmp(argv[1], "ast")   == 0) mode = MODE_AST;
        else { usage(argv[0]); return 2; }
        path = argv[2];
    } else {
        usage(argv[0]);
        return 2;
    }

    if (path) {
        yyin = fopen(path, "r");
        if (!yyin) {
            fprintf(stderr, "mypl: cannot open '%s'\n", path);
            return 1;
        }
    }

    int rc = yyparse();
    if (path) fclose(yyin);

    if (rc != 0 || parse_error_count > 0) {
        fprintf(stderr, "mypl: %d parse error(s)\n",
                parse_error_count > 0 ? parse_error_count : 1);
        return 1;
    }

    switch (mode) {
        case MODE_CHECK:
            printf("OK: %d facts, %d rules, %d queries.\n",
                   fact_count, rule_count, query_count_pending());
            break;
        case MODE_AST:
            dump_program();
            break;
        case MODE_RUN:
            run_all_queries();
            break;
    }
    return 0;
}
