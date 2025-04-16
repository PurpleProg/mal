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
            strcat(string, "#<core function>");
            return string;
        case MAL_FN:
            strcat(string, "#<function>");
            return string;
        case MAL_NIL:
            strcat(string, "#<nil>");
            return string;
        case MAL_TRUE:
            strcat(string, "true");
            return string;
        case MAL_FALSE:
            strcat(string, "false");
            return string;
    }

    return string;
}


char * pr_env(env_t * env) {
    char * string;
    string = GC_malloc(1024); // TODO: dynamicly allocate this
    if (env == NULL) {
        return string;
    }

    strcat(string, "outer : ");

    // print the outer pointer
    char pointerStr[20];
    sprintf(pointerStr, "%p", (void *)env->outer);
    strcat(string, pointerStr);
    strcat(string, "\n");


    strcat(string, "{\n");
    node_t * current_node = env->data;
    if (current_node  == NULL) {
        printf("map is null\n");
        return NULL;
    }
    if (current_node ->data == NULL) {
        printf("map is empty\n");
        return NULL;
    }
    while (current_node != NULL){
        // key
        strcat(string, current_node->data);
        strcat(string, " : ");
        // value
        strcat(string, pr_str(current_node->next->data));
        strcat(string, "\n");

        current_node = current_node->next->next;
    }
    strcat(string, "}\n");
    return string;
}
