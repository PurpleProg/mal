#include "stdio.h"
#include "string.h"
#include "types.h"
#include "linked_list.h"
#include "gc.h"
#include "printer.h"


char * pr_str(MalType * AST) {
    char * string;
    string = GC_malloc(1024); // TODO: dynamicly allocate this

    if (AST == NULL) {
        return string;
    }

    switch (AST->type) {
        case MAL_INT:
            string = GC_malloc(sizeof(MalInt));
            // snprintf(string, sizeof(), NULL);
            sprintf(string, "%ld", *AST->value.IntValue);
            return string;
        case MAL_SYMBOL:
            return AST->value.SymbolValue;
        case MAL_LIST:
            strcat(string, "(");
            node_t * node = AST->value.ListValue;

            while (node != NULL && node->data != NULL) {
                strcat(string, pr_str((MalType *)node->data));

                if (node->next != NULL) {
                    strcat(string, " ");
                }
                node = node->next;
            }
            strcat(string, ")");
            return string;
        case MAL_CORE_FN:
            printf("WTF core function in AST in pr_str ???");
    }

    return string;
}
