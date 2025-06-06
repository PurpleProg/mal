#include "step2_eval.h"
#include "gc.h"
#include "linked_list.h"
#include "printer.h"
#include "reader.h"
#include <stdio.h>
#include <string.h>

MalType *READ(char *line);
MalType *EVAL(MalType *AST, const Operation repl_env[]);
char    *PRINT(MalType *AST);
char    *rep(char *line, const Operation repl_env[]);

MalType *EVAL_symbol(MalType *AST, const Operation *repl_env);

MalType *add(node_t *node) {
    // add a list of signed long
    if (node == NULL) {
        fprintf(stderr, "list is empty\n");
        return 0;
    }

    signed long result = 0;
    do {
        result += *(signed long *)(node->data);
        printf("add : %ld\n", *(signed long *)(node->data));
        node = node->next;
    } while (node != NULL);

    MalType *MalResult        = GC_MALLOC(sizeof(MalType));
    MalResult->type           = MAL_INT;
    MalResult->value.IntValue = GC_MALLOC(sizeof(MalInt));
    memcpy(MalResult->value.IntValue, &result, sizeof(result));
    return MalResult;
}
MalType *sub(node_t *node) {
    // sub a list of signed long
    if (node == NULL) {
        fprintf(stderr, "list is empty\n");
        return 0;
    }

    signed long result = *(signed long *)(node->data);
    printf("sub from : %ld\n", *(signed long *)(node->data));
    node = node->next;

    if (node == NULL) {
        fprintf(stderr, "at least two number pls\n");
        return 0;
    }

    do {
        result -= *(signed long *)(node->data);
        printf("sub : %ld\n", *(signed long *)(node->data));
        node = node->next;
    } while (node != NULL);

    MalType *MalResult        = GC_MALLOC(sizeof(MalType));
    MalResult->type           = MAL_INT;
    MalResult->value.IntValue = GC_MALLOC(sizeof(MalInt));

    memcpy(MalResult->value.IntValue, &result, sizeof(result));
    return MalResult;
}
MalType *mult(node_t *node) {
    // mult a list of signed long
    if (node == NULL) {
        fprintf(stderr, "list is empty\n");
        return 0;
    }

    signed long result = 1;
    do {
        result *= *(signed long *)(node->data);
        printf("mult by : %ld\n", *(signed long *)(node->data));
        node = node->next;
    } while (node != NULL);

    MalType *MalResult        = GC_MALLOC(sizeof(MalType));
    MalResult->type           = MAL_INT;
    MalResult->value.IntValue = GC_MALLOC(sizeof(MalInt));
    memcpy(MalResult->value.IntValue, &result, sizeof(result));
    return MalResult;
}
MalType *divide(node_t *node) {
    // divide two of signed long
    if (node == NULL) {
        fprintf(stderr, "list is empty\n");
        return 0;
    }

    signed long result = 0;

    if (node->next == NULL) {
        printf("divide must take two arg (not 0)");
        return 0;
    } else if (node->next == NULL) {
        printf("divide must take two arg (not 1)");
        return 0;
    } // else if (node->next->next->next != NULL) {

    signed long a = *(signed long *)(node->data);
    signed long b = *(signed long *)(node->next->data);

    printf("a : %ld, b : %ld\n", a, b);
    result = a / b;

    MalType *MalResult        = GC_MALLOC(sizeof(MalType));
    MalResult->type           = MAL_INT;
    MalResult->value.IntValue = GC_MALLOC(sizeof(MalInt));
    memcpy(MalResult->value.IntValue, &result, sizeof(result));
    return MalResult;
}

MalType *get_operation(const char *operator, const Operation * repl_env,
                       size_t repl_env_size) {
    // return a function pointer
    for (size_t i = 0; i < repl_env_size; i++) {
        if (*operator== repl_env[i].operator) {
            return repl_env[i].operation;
        }
    }
    fprintf(stderr, "operator '%c' not implemented\n", *operator);
    return NULL;
}

int main(void) {
    char   *line = NULL;
    size_t  len  = 0;
    ssize_t read;

    // make add a MalType
    MalType *Mal_add           = GC_MALLOC(sizeof(MalType));
    Mal_add->type              = MAL_CORE_FN;
    Mal_add->value.CoreFnValue = GC_MALLOC(sizeof(add));

    // make sub a MalType
    MalType *Mal_sub           = GC_MALLOC(sizeof(MalType));
    Mal_sub->type              = MAL_CORE_FN;
    Mal_sub->value.CoreFnValue = GC_MALLOC(sizeof(sub));

    // make mult a MalType
    MalType *Mal_mult           = GC_MALLOC(sizeof(MalType));
    Mal_mult->type              = MAL_CORE_FN;
    Mal_mult->value.CoreFnValue = GC_MALLOC(sizeof(mult));

    // make divide a MalType
    MalType *Mal_divide           = GC_MALLOC(sizeof(MalType));
    Mal_divide->type              = MAL_CORE_FN;
    Mal_divide->value.CoreFnValue = GC_MALLOC(sizeof(divide));

    Mal_add->value.CoreFnValue    = (MalCoreFn)&add;
    Mal_sub->value.CoreFnValue    = (MalCoreFn)&sub;
    Mal_mult->value.CoreFnValue   = (MalCoreFn)&mult;
    Mal_divide->value.CoreFnValue = (MalCoreFn)&divide;

    const Operation repl_env[] = {
        {'+', Mal_add},
        {'-', Mal_sub},
        {'*', Mal_mult},
        {'/', Mal_divide},
    };

    GC_INIT();

    printf("user> ");
    while ((read = getline(&line, &len, stdin)) != -1) {
        printf("%s\n", rep(line, repl_env));
        printf("user> ");
    }
}

MalType *READ(char *line) {
    return read_str(line);
}

MalType *EVAL(MalType *AST, const Operation *repl_env) {
    if (AST == NULL) {
        return AST;
    }
    switch (AST->type) {
    case MAL_SYMBOL: {
        // NOTE: return the function from the repl env
        return EVAL_symbol(AST, repl_env);
    case MAL_LIST: {
        // NOTE: call the function from the first element
        // and pass the rest of the list as args
        node_t *element = AST->value.ListValue;
        // if the list is empty
        if (element->data == NULL) {
            printf("list is empty\n");
            return AST;
        }

        // calling eval on the first element of the list should return a
        // MalCoreFn
        MalType *operation = EVAL(element->data, repl_env);
        if (operation == NULL) {
            printf("operator returned NULL ");
            return AST;
        }
        if (operation->type != MAL_CORE_FN) {
            fprintf(stderr, "first element of list is not operator\n");
            return AST;
        }
        // skip symbol
        if (element->next != NULL) {
            element = element->next;
        } else {
            printf("list only contain one symbol");
            return AST;
        }

        // print the rest of the element
        node_t *copy = element->next;
        while (copy != NULL) {
            MalType *thing = copy->data;
            printf("element : %ld\n", *(thing->value.IntValue));
            copy = copy->next;
        }

        // add the rest of the list to a list of evaluated things
        node_t *list = GC_MALLOC(sizeof(node_t));
        while (element != NULL) {
            MalType *evaluated_element = EVAL(element->data, repl_env);
            //
            if (evaluated_element->type == MAL_INT) {
                printf("element added to list : %ld\n",
                       *(evaluated_element->value.IntValue));
            } else {
                printf("evaluated elent is not a int \n");
            }
            append(list, (void *)evaluated_element->value.IntValue,
                   sizeof(*(evaluated_element->value.IntValue)));
            element = element->next;
        };

        MalType *result = (*(MalCoreFn)operation->value.CoreFnValue)(list);

        printf("result : %ld \n", *(result->value.IntValue));

        return result;

    case MAL_INT: return AST;
    case MAL_CORE_FN:
        // idk switch shall handle every posible enum soooo
        fprintf(stderr, "how this got tokenized ???");
    }
    }
    }

    fprintf(stderr, "unmatched AST");
    return NULL;
}

MalType *EVAL_symbol(MalType *AST, const Operation *repl_env) {
    // NOTE: return the function from the repl env
    const char *operator= AST->value.SymbolValue;
    MalType *operation = get_operation(operator, repl_env, 4);
    return operation;
}

char *PRINT(MalType *AST) {
    return pr_str(AST);
}

char *rep(char *line, const Operation *repl_env) {
    return PRINT(EVAL(READ(line), repl_env));
}
