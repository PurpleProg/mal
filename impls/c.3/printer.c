#include "printer.h"
#include "gc.h"
#include "linked_list.h"
#include "stdio.h"
#include "string.h"
#include "types.h"

char *pr_str(MalType *AST, int print_readably) {
    char *string;
    string = GC_malloc(1024); // TODO: dynamicly allocate this

    if (AST == NULL) {
        return string;
    }

    if (AST->value.IntValue == NULL) {
        printf("/!\\ AST.value is NULL but type is allocated\n");
        return string;
    }

    switch (AST->type) {
    case MAL_INT: {
        string = GC_malloc(sizeof(MalInt));
        sprintf(string, "%ld", *AST->value.IntValue);
        return string;
    }
    case MAL_KEYWORD: {
        strcat(string, ":");
        strcat(string, AST->value.SymbolValue);
        return string;
    }
    case MAL_SYMBOL: {
        return AST->value.SymbolValue;
    }
    case MAL_STRING: {
        if (print_readably == 0) {
            strcat(string, AST->value.StringValue);
            return string;
        }
        // max new size
        char *token      = AST->value.StringValue;
        char *new_string = GC_MALLOC(2 * strlen(token));

        // transform  " -> \"
        int i_new_string = 0;
        // strcat(new_string, "\"");
        for (unsigned long i_token = 0; i_token < strlen(token); i_token++) {
            if (token[i_token] == '\\') {
                new_string[i_new_string] = token[i_token];
                strcat(new_string, "\\");
                i_new_string++;
            } else if (token[i_token] == '\n') {
                strcat(new_string, "\\n");
                i_new_string++;
            } else if (token[i_token] == '\"') {
                strcat(new_string, "\\\"");
                i_new_string++;
            } else {
                new_string[i_new_string] = token[i_token];
            }
            i_new_string++;
        }

        // wrap string in ""
        char *wrap_string = GC_MALLOC(strlen(new_string) + 2);
        strcat(wrap_string, "\"");
        strcat(wrap_string, new_string);
        strcat(wrap_string, "\"");

        return wrap_string;
    }
    case MAL_LIST: {
        strcat(string, "(");
        node_t *node = AST->value.ListValue;

        while (node != NULL && node->data != NULL) {
            strcat(string, pr_str((MalType *)node->data, print_readably));

            if (node->next != NULL) {
                strcat(string, " ");
            }
            node = node->next;
        }
        strcat(string, ")");
        return string;
    }
    case MAL_VECTOR: {
        strcat(string, "[");
        node_t *node = AST->value.ListValue;

        while (node != NULL && node->data != NULL) {
            strcat(string, pr_str((MalType *)node->data, print_readably));

            if (node->next != NULL) {
                strcat(string, " ");
            }
            node = node->next;
        }
        strcat(string, "]");
        return string;
    }
    case MAL_CORE_FN: {
        strcat(string, "#<core function>");
        return string;
    }
    case MAL_FN: {
        strcat(string, "#<function>");
        return string;
    }
    case MAL_NIL: {
        strcat(string, "nil");
        return string;
    }
    case MAL_TRUE: {
        strcat(string, "true");
        return string;
    }
    case MAL_FALSE: {
        strcat(string, "false");
        return string;
    }
    case MAL_FN_WRAPER: {
        strcat(string, "#<function wraper>");
        return string;
    }
    case MAL_ATOM: {
        strcat(string, "(atom ");
        strcat(string, pr_str(AST->value.AtomValue, print_readably));
        strcat(string, ")");

        return string;
    }
    case MAL_HASHMAP: {
        strcat(string, "{");
        map_t *node = AST->value.HashmapValue;

        while (node != NULL && node->data != NULL && node->next != NULL &&
               node->next->data != NULL) {
            // key
            strcat(string, "\"");
            strcat(string, (char *)node->data);
            strcat(string, "\" ");

            // value
            strcat(string, pr_str((MalType *)node->next->data, print_readably));
            if (node->next->next != NULL) {
                strcat(string, " ");
            }

            node = node->next->next;
        }
        strcat(string, "}");
        return string;
    }
    }

    return string;
}

char *pr_env(env_t *env) {
    char *string;
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
    node_t *current_node = env->data;
    if (current_node == NULL) {
        printf("map is null\n");
        return NULL;
    }
    if (current_node->data == NULL) {
        printf("map is empty\n");
        return NULL;
    }
    while (current_node != NULL) {
        // key
        strcat(string, current_node->data);
        strcat(string, " : ");
        // value
        if (current_node->next->data == NULL) {
            strcat(string, "NULL");
        }
        strcat(string, pr_str(current_node->next->data, 0));
        strcat(string, "\n");

        current_node = current_node->next->next;
    }
    strcat(string, "}\n");
    return string;
}
