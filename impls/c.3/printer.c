#include "stdio.h"
#include "string.h"
#include "types.h"
#include "linked_list.h"
#include "gc.h"
#include "printer.h"


char * pr_str(MalType * AST) {
    char * string;

    switch (AST->type) {
        case MAL_INT:
            string = GC_malloc(sizeof(MalInt));
            // snprintf(string, sizeof(), NULL);
            sprintf(string, "%ld", *AST->value.IntValue);
            return string;
        case MAL_SYMBOL:
            return AST->value.SymbolValue;
        case MAL_LIST:
            string = GC_malloc(1024); // TODO: dynamicly allocate this
            node_t * node = AST->value.ListValue;

            strcat(string, "(");
            while (node != NULL) {
                strcat(string, pr_str((MalType *)node->data));

                strcat(string, " ");
                node = node->next;
            }
            strcat(string, ")");
    }

    return string;
}
