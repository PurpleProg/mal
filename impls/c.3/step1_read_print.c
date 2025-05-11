#include "gc.h"
#include "linked_list.h"
#include "printer.h"
#include "reader.h"
#include <stdio.h>
#include <string.h>

MalType *READ(char *line);
MalType *EVAL(MalType *AST);
char    *PRINT(MalType *AST);
char    *rep(char *line);

int main(void) {
    char   *line = NULL;
    size_t  len  = 0;
    ssize_t read;

    GC_INIT();

    printf("user> ");
    while ((read = getline(&line, &len, stdin)) != -1) {
        printf("%s\n", rep(line));
        printf("user> ");
    }
}

MalType *READ(char *line) {
    return read_str(line);
}

MalType *EVAL(MalType *AST) {
    return AST;
}

char *PRINT(MalType *AST) {
    return pr_str(AST);
}

char *rep(char *line) {
    return PRINT(EVAL(READ(line)));
}
