#include <stdio.h>
#include <string.h>
#include "gc.h"
#include "reader.h"
#include "printer.h"
#include "types.h"
#include "env.h"
#include "core.h"


MalType * add(node_t * node) {
	// add a list of signed long
	if (node->data == NULL) {
       return 0;
       }

       signed long result = 0;
       do {
           result += *((MalType *)node->data)->value.IntValue;
       node = node->next;
       } while (node != NULL);

       MalType * MalResult = GC_MALLOC(sizeof(MalType));
       MalResult->type = MAL_INT;
       MalResult->value.IntValue = GC_MALLOC(sizeof(MalInt));
       memcpy(MalResult->value.IntValue, &result, sizeof(result));
       return MalResult;
}
MalType * sub(node_t * node) {
	// sub a list of signed long
	if (node->data == NULL) {
       return 0;
       }

       signed long result = 0;

       if (node == NULL) {
           fprintf(stderr, "at least two number pls\n");
       return 0;
       }

       signed long a = *((MalType *)node->data)->value.IntValue;
       signed long b = *((MalType *)node->next->data)->value.IntValue;
       result = a - b;

       MalType * MalResult = GC_MALLOC(sizeof(MalType));
       MalResult->type = MAL_INT;
       MalResult->value.IntValue = GC_MALLOC(sizeof(MalInt));

       memcpy(MalResult->value.IntValue, &result, sizeof(result));
       return MalResult;
}
MalType * mult(node_t * node) {
	// mult a list of signed long
    if (node->data == NULL) {
       return 0;
       }

	   signed long result = 1;
	   do {
		   result *= *((MalType *)node->data)->value.IntValue;
	   node = node->next;
	   } while (node != NULL);

	   MalType * MalResult = GC_MALLOC(sizeof(MalType));
	   MalResult->type = MAL_INT;
	   MalResult->value.IntValue = GC_MALLOC(sizeof(MalInt));
	   memcpy(MalResult->value.IntValue, &result, sizeof(result));
	   return MalResult;
}
MalType * divide(node_t * node) {
	// divide two of signed long
	if (node->data == NULL) {
        return 0;
    }

    signed long result = 0;

    if (node->next == NULL) {
        printf("divide must take two arg (not 0)");
        return 0;
    } else if (node->next == NULL){
        printf("divide must take two arg (not 1)");
        return 0;
    } //else if (node->next->next->next != NULL) {

    signed long a = *((MalType *)node->data)->value.IntValue;
    signed long b = *((MalType *)node->next->data)->value.IntValue;

    // printf("a : %ld, b : %ld\n", a, b);
    result = a / b;

    MalType * MalResult = GC_MALLOC(sizeof(MalType));
    MalResult->type = MAL_INT;
    MalResult->value.IntValue = (MalInt *)GC_MALLOC(sizeof(MalInt));
    memcpy(MalResult->value.IntValue, &result, sizeof(result));
    return MalResult;
}

MalType * prstr(node_t * node) {
    size_t buffer_size = 256;
    char * string = GC_MALLOC(buffer_size);
    string[0] = '\0';

    while (node != NULL) {
        char * new_string = pr_str(node->data, 1);

        // +1 for a space
        size_t new_size = strlen(string) + strlen(new_string) + 1;
        if (new_size > buffer_size) {
            char *temp = GC_MALLOC(new_size);
            strcpy(temp, string);
            buffer_size = new_size;
            string = temp;
        }

        strcat(string, new_string);
        if (node->next != NULL) {strcat(string, " ");}
        node = node->next;
    }

    MalType * mal_string_ret = GC_MALLOC(sizeof(MalType));
    mal_string_ret->type = MAL_STRING;
    mal_string_ret->value.StringValue = string;

    return mal_string_ret;
}
MalType * str(node_t * node) {
    size_t buffer_size = 256;
    char * string = GC_MALLOC(buffer_size);
    string[0] = '\0';

    while (node != NULL) {
        char * new_string = pr_str(node->data, 0);

        size_t new_size = strlen(string) + strlen(new_string);
        if (new_size > buffer_size) {
            char *temp = GC_MALLOC(new_size);
            strcpy(temp, string);
            buffer_size = new_size;
            string = temp;
        }

        strcat(string, new_string);
        node = node->next;
    }

    MalType * mal_string_ret = GC_MALLOC(sizeof(MalType));
    mal_string_ret->type = MAL_STRING;
    mal_string_ret->value.StringValue = string;

    return mal_string_ret;
}
MalType * prn(node_t * node) {
    size_t buffer_size = 256;
    char * string = GC_MALLOC(buffer_size);
    string[0] = '\0';

    while (node != NULL) {
        char * new_string = pr_str(node->data, 1);

        // +1 for a space
        size_t new_size = strlen(string) + strlen(new_string) + 1;
        if (new_size > buffer_size) {
            char *temp = GC_MALLOC(new_size);
            strcpy(temp, string);
            buffer_size = new_size;
            string = temp;
        }

        strcat(string, new_string);
        if (node->next != NULL) {strcat(string, " ");}
        node = node->next;
    }

    printf("%s\n", string);

    MalType * nil = GC_MALLOC(sizeof(MalType));
    nil->type = MAL_NIL;
    nil->value.NilValue = NULL;

    return nil;
}
MalType * println(node_t * node) {
    size_t buffer_size = 256;
    char * string = GC_MALLOC(buffer_size);
    string[0] = '\0';

    while (node != NULL) {
        char * new_string = pr_str(node->data, 0);

        // +1 for a space
        size_t new_size = strlen(string) + strlen(new_string) + 1;
        if (new_size > buffer_size) {
            char *temp = GC_MALLOC(new_size);
            strcpy(temp, string);
            buffer_size = new_size;
            string = temp;
        }

        strcat(string, new_string);
        if (node->next != NULL) {strcat(string, " ");}
        node = node->next;
    }

    printf("%s\n", string);

    MalType * nil = GC_MALLOC(sizeof(MalType));
    nil->type = MAL_NIL;
    nil->value.NilValue = NULL;

    return nil;
}

MalType * list(node_t * node) {
    if (node->data == NULL) {
        // list is empty, allocate a new one
        node_t * new_list = GC_MALLOC(sizeof(node_t));
        new_list->next = NULL;
        new_list->data = NULL;
        node = new_list;
    }

    MalType * ret = GC_MALLOC(sizeof(MalType));
    ret->type = MAL_LIST;
    ret->value.ListValue = node;
    return ret;
}
MalType * list_question_mark(node_t * node) {
    MalType * bool = GC_MALLOC(sizeof(MalType));
	if (node->data == NULL) {
        bool->type = MAL_FALSE;
        bool->value.FalseValue = NULL;
        return bool;
    }
    if (((MalType *)node->data)->type == MAL_LIST) {
        bool->type = MAL_TRUE;
        bool->value.TrueValue = NULL;
    } else {
        bool->type = MAL_FALSE;
        bool->value.FalseValue = NULL;
    }
    return bool;
}
MalType * empty_question_mark(node_t * node) {
    MalType * bool = GC_MALLOC(sizeof(MalType));
	if (node->data == NULL) {
        bool->type = MAL_FALSE;
        bool->value.FalseValue = NULL;
        return bool;
    }
    if ( !( ((MalType *)node->data)->type == MAL_LIST || ((MalType *)node->data)->type == MAL_VECTOR)) {
        printf("arg is not a list\n");
        bool->type = MAL_FALSE;
        bool->value.FalseValue = NULL;
        return bool;
    }
    MalList * list = ((MalType *)node->data)->value.ListValue;
    if (list->data == NULL) {
        bool->type = MAL_TRUE;
        bool->value.TrueValue = NULL;
    } else {
        bool->type = MAL_FALSE;
        bool->value.FalseValue = NULL;
    }
    return bool;

}
MalType * count(node_t * node) {
    MalType * ret = GC_MALLOC(sizeof(MalType));
    ret->type = MAL_INT;
    ret->value.IntValue = GC_MALLOC(sizeof(MalInt));
    *(ret->value.IntValue) = 0;

    node_t * new_node = GC_MALLOC(sizeof(node_t));
    if (node->data == NULL) {
        // arg is empty
        *(ret->value.IntValue) = -1;
        return ret;
    }
    if ( ! (((MalType *)node->data)->type == MAL_LIST || ((MalType *)node->data)->type == MAL_VECTOR)) {
        *(ret->value.IntValue) = 0;
        return ret;
    }

    new_node = ((MalType *)node->data)->value.ListValue;

    if (new_node->data == NULL) {
        // list is empty
        *(ret->value.IntValue) = 0;
        return ret;
    }

    signed long counter = 0;
    while (new_node != NULL) {
        counter += 1;
        new_node = new_node->next;
    }
    *(ret->value.IntValue) = counter;
    return ret;
}

MalType * equal(node_t * node) {
    MalType * false = GC_MALLOC(sizeof(MalType));
    false->type = MAL_FALSE;
    false->value.FalseValue = GC_MALLOC(sizeof(MalFalse));
    *false->value.FalseValue = 0;

    MalType * true = GC_MALLOC(sizeof(MalType));
    true->type = MAL_TRUE;
    true->value.TrueValue = GC_MALLOC(sizeof(MalTrue));
    *true->value.TrueValue = 0;

    if (node == NULL) {
        printf("= with no arg \n");
        return NULL;
    }
    if (node->data == NULL) {
        printf("= arg1 is NULL \n");
        return NULL;
    }
    if (node->next == NULL) {
        printf("= with only one arg\n");
        return NULL;
    }
    if (node->next->data == NULL) {
        printf("= arg2 is NULL \n");
        return NULL;
    }

    MalType * arg1 = GC_MALLOC(sizeof(MalType));
    MalType * arg2 = GC_MALLOC(sizeof(MalType));
    arg1 = node->data;
    arg2 = node->next->data;

    if (arg1->type != arg2->type) {
        if (! ((arg1->type == MAL_LIST || arg1->type == MAL_VECTOR) && (arg2->type == MAL_LIST || arg2->type == MAL_VECTOR))) {
            return false;
        }
    }
    switch (arg1->type) {
        // types are the same, so singletones (true false nil) can return true directly
        case MAL_VECTOR: {
            node_t * node1 = arg1->value.ListValue;
            node_t * node2 = arg2->value.ListValue;
            while (node1 != NULL && node2 != NULL) {
                // if two elements are empty:
                if (node1->data == NULL && node2->data == NULL) {
                    return true;
                } else if (node1->data == NULL || node2->data == NULL) {
                    // lenght are different
                    return false;
                }

                node_t * zipped = GC_MALLOC(sizeof(node_t));
                zipped->data = NULL;
                zipped->next = NULL;
                append(zipped, node1->data, sizeof(MalType));
                append(zipped, node2->data, sizeof(MalType));



                if (equal(zipped)->type == MAL_FALSE) {
                    return false;
                }
                node1 = node1->next;
                node2 = node2->next;
            }
            // if lenght are different
            if ((node1 == NULL) ^ (node2 == NULL)) {
                return false;
            }
            return true;
        }
        case MAL_LIST: {
            node_t * node1 = arg1->value.ListValue;
            node_t * node2 = arg2->value.ListValue;
            while (node1 != NULL && node2 != NULL) {
                // if two elements are empty:
                if (node1->data == NULL && node2->data == NULL) {
                    return true;
                } else if (node1->data == NULL || node2->data == NULL) {
                    // lenght are different
                    return false;
                }

                node_t * zipped = GC_MALLOC(sizeof(node_t));
                zipped->data = NULL;
                zipped->next = NULL;
                append(zipped, node1->data, sizeof(MalType));
                append(zipped, node2->data, sizeof(MalType));



                if (equal(zipped)->type == MAL_FALSE) {
                    return false;
                }
                node1 = node1->next;
                node2 = node2->next;
            }
            // if lenght are different
            if ((node1 == NULL) ^ (node2 == NULL)) {
                return false;
            }
            return true;
        }
        case MAL_INT: {
            signed long int1 = *arg1->value.IntValue;
            signed long int2 = *arg2->value.IntValue;
            if (int1 == int2) {
                return true;
            }
            // false
            return false;
        }
        case MAL_TRUE: {return true;}
        case MAL_FALSE: {return true;}
        case MAL_NIL: {return true;}
        case MAL_STRING: {
            char * string1 = arg1->value.StringValue;
            char * string2 = arg2->value.StringValue;
            if (strcmp(string1, string2) == 0) {
                return true;
            }
            return false;

        }
        case MAL_KEYWORD: {
            char * symbol1 = arg1->value.SymbolValue;
            char * symbol2 = arg2->value.SymbolValue;
            if (strcmp(symbol1, symbol2) == 0) {
                return true;
            }
            return false;
        }
        case MAL_SYMBOL: {
            char * symbol1 = arg1->value.SymbolValue;
            char * symbol2 = arg2->value.SymbolValue;
            if (strcmp(symbol1, symbol2) == 0) {
                return true;
            }
            return false;
        }
        case MAL_FN: {
            // two function with same body and same
            MalFn * fn1 = arg1->value.FnValue;
            MalFn * fn2 = arg2->value.FnValue;

            // build a new body_list for recursive equal call
            node_t * body_list = GC_MALLOC(sizeof(node_t));
            append(body_list, fn1->body, sizeof(MalType));
            append(body_list, fn2->body, sizeof(MalType));
            if (! equal(body_list)) {
                return false;
            }

            // param is a list of char *
            // to fix TODO:
            node_t * symbol_list_1 = fn1->param;
            node_t * symbol_list_2 = fn2->param;
            while (symbol_list_1 != NULL && symbol_list_2 != NULL) {
                if (strcmp(symbol_list_1->data, symbol_list_2->data) != 0) {
                    return false;
                }
                symbol_list_1 = symbol_list_1->next;
                symbol_list_2 = symbol_list_2->next;
            }
            // if lenght are different
            if ((symbol_list_1 == NULL) ^ (symbol_list_2 == NULL)) {
                return false;
            }

            // check if outer env is the same
            env_t * env1 = fn1->env->outer;
            env_t * env2 = fn2->env->outer;
            if (env1 != env2) {
                return false;
            }

            // i should check if the env itself is the same but im lazy rn
            // TODO: check (map_t *)env->data

            // if all of that was not false, then fn1 must be = to fn2
            return true;
        }
        case MAL_CORE_FN: {
            // check if pointers are the same
            if (arg1->value.CoreFnValue == arg2->value.CoreFnValue) {
                return true;
            }
            return false;
        }
    }

    printf("= fallback\n");
    // manual breakpoint
    char * a = NULL;
    *(int *) a = 1;
    return false;
}

MalType * less(node_t * node) {
    MalType * false = GC_MALLOC(sizeof(MalType));
    false->type = MAL_FALSE;
    false->value.FalseValue = GC_MALLOC(sizeof(MalFalse));
    *false->value.FalseValue = 0;

    MalType * true = GC_MALLOC(sizeof(MalType));
    true->type = MAL_TRUE;
    true->value.TrueValue = GC_MALLOC(sizeof(MalTrue));
    *true->value.TrueValue = 0;

    if (node == NULL) {
        printf("< with no arg \n");
        return NULL;
    }
    if (node->data == NULL) {
        printf("< arg1 is NULL \n");
        return NULL;
    }
    if (node->next == NULL) {
        printf("< with only one arg\n");
        return NULL;
    }
    if (node->next->data == NULL) {
        printf("< arg2 is NULL \n");
        return NULL;
    }

    // maybe i dont need to allocate new memory here ?
    MalType * arg1 = GC_MALLOC(sizeof(MalType));
    MalType * arg2 = GC_MALLOC(sizeof(MalType));
    arg1 = node->data;
    arg2 = node->next->data;

    if (arg1->type != MAL_INT || arg2->type != MAL_INT) {
        printf("< with non int parameters\n");
        return NULL;
    }
    if (*arg1->value.IntValue < *arg2->value.IntValue) {
        return true;
    }
    return false;
}
MalType * more(node_t * node) {
    MalType * false = GC_MALLOC(sizeof(MalType));
    false->type = MAL_FALSE;
    false->value.FalseValue = GC_MALLOC(sizeof(MalFalse));
    *false->value.FalseValue = 0;

    MalType * true = GC_MALLOC(sizeof(MalType));
    true->type = MAL_TRUE;
    true->value.TrueValue = GC_MALLOC(sizeof(MalTrue));
    *true->value.TrueValue = 0;

    if (node == NULL) {
        printf("> with no arg \n");
        return NULL;
    }
    if (node->data == NULL) {
        printf("> arg1 is NULL \n");
        return NULL;
    }
    if (node->next == NULL) {
        printf("> with only one arg\n");
        return NULL;
    }
    if (node->next->data == NULL) {
        printf("> arg2 is NULL \n");
        return NULL;
    }

    // maybe i dont need to allocate new memory here ?
    MalType * arg1 = GC_MALLOC(sizeof(MalType));
    MalType * arg2 = GC_MALLOC(sizeof(MalType));
    arg1 = node->data;
    arg2 = node->next->data;

    if (arg1->type != MAL_INT || arg2->type != MAL_INT) {
        printf("> with non int parameters\n");
        return NULL;
    }
    if (*arg1->value.IntValue > *arg2->value.IntValue) {
        return true;
    }
    return false;
}
MalType * equal_less(node_t * node) {
    MalType * false = GC_MALLOC(sizeof(MalType));
    false->type = MAL_FALSE;
    false->value.FalseValue = GC_MALLOC(sizeof(MalFalse));
    *false->value.FalseValue = 0;

    MalType * true = GC_MALLOC(sizeof(MalType));
    true->type = MAL_TRUE;
    true->value.TrueValue = GC_MALLOC(sizeof(MalTrue));
    *true->value.TrueValue = 0;

    if (node == NULL) {
        printf("<= with no arg \n");
        return NULL;
    }
    if (node->data == NULL) {
        printf("<= arg1 is NULL \n");
        return NULL;
    }
    if (node->next == NULL) {
        printf("<= with only one arg\n");
        return NULL;
    }
    if (node->next->data == NULL) {
        printf("<= arg2 is NULL \n");
        return NULL;
    }

    // maybe i dont need to allocate new memory here ?
    MalType * arg1 = GC_MALLOC(sizeof(MalType));
    MalType * arg2 = GC_MALLOC(sizeof(MalType));
    arg1 = node->data;
    arg2 = node->next->data;

    if (arg1->type != MAL_INT || arg2->type != MAL_INT) {
        printf("<= with non int parameters\n");
        return NULL;
    }
    if (*arg1->value.IntValue <= *arg2->value.IntValue) {
        return true;
    }
    return false;
}
MalType * equal_more(node_t * node) {
    MalType * false = GC_MALLOC(sizeof(MalType));
    false->type = MAL_FALSE;
    false->value.FalseValue = GC_MALLOC(sizeof(MalFalse));
    *false->value.FalseValue = 0;

    MalType * true = GC_MALLOC(sizeof(MalType));
    true->type = MAL_TRUE;
    true->value.TrueValue = GC_MALLOC(sizeof(MalTrue));
    *true->value.TrueValue = 0;

    if (node == NULL) {
        printf(">= with no arg \n");
        return NULL;
    }
    if (node->data == NULL) {
        printf(">= arg1 is NULL \n");
        return NULL;
    }
    if (node->next == NULL) {
        printf(">= with only one arg\n");
        return NULL;
    }
    if (node->next->data == NULL) {
        printf(">= arg2 is NULL \n");
        return NULL;
    }

    // maybe i dont need to allocate new memory here ?
    MalType * arg1 = GC_MALLOC(sizeof(MalType));
    MalType * arg2 = GC_MALLOC(sizeof(MalType));
    arg1 = node->data;
    arg2 = node->next->data;

    if (arg1->type != MAL_INT || arg2->type != MAL_INT) {
        printf(">= with non int parameters\n");
        return NULL;
    }
    if (*arg1->value.IntValue >= *arg2->value.IntValue) {
        return true;
    }
    return false;
}

MalType * wrap_function(MalType * (*func)(node_t * node)) {
    // make func a MalType
    MalType * Mal_func = GC_MALLOC(sizeof(MalType));
       Mal_func->type = MAL_CORE_FN;
       Mal_func->value.CoreFnValue = (MalCoreFn)func;
       return Mal_func;
}

env_t * create_repl(){

    env_t * env = create_env(NULL, NULL, NULL);

    node_t * function_pointers_list = GC_MALLOC(sizeof(node_t));
    function_pointers_list->next = NULL;
    function_pointers_list->data = NULL;

    node_t * symbol_list = GC_MALLOC(sizeof(node_t));
    symbol_list->next = NULL;
    symbol_list->data = NULL;

    // yes, hard coded.
    append(function_pointers_list, wrap_function(println), sizeof(MalType));
    append(symbol_list, "println", 7);

    append(function_pointers_list, wrap_function(str), sizeof(MalType));
    append(symbol_list, "str", 3);

    append(function_pointers_list, wrap_function(prn), sizeof(MalType));
    append(symbol_list, "prn", 3);

    append(function_pointers_list, wrap_function(prstr), sizeof(MalType));
    append(symbol_list, "pr-str", 6);

    append(function_pointers_list, wrap_function(add), sizeof(MalType));
    append(symbol_list, "+", 1);

    append(function_pointers_list, wrap_function(sub), sizeof(MalType));
    append(symbol_list, "-", 1);

    append(function_pointers_list, wrap_function(mult), sizeof(MalType));
    append(symbol_list, "*", 1);

    append(function_pointers_list, wrap_function(divide), sizeof(MalType));
    append(symbol_list, "/", 1);

    append(function_pointers_list, wrap_function(prn), sizeof(MalType));
    append(symbol_list, "prn", 3);

    append(function_pointers_list, wrap_function(list), sizeof(MalType));
    append(symbol_list, "list", 4);

    append(function_pointers_list, wrap_function(list_question_mark), sizeof(MalType));
    append(symbol_list, "list?", 5);

    append(function_pointers_list, wrap_function(empty_question_mark), sizeof(MalType));
    append(symbol_list, "empty?", 6);

    append(function_pointers_list, wrap_function(count), sizeof(MalType));
    append(symbol_list, "count", 5);

    append(function_pointers_list, wrap_function(equal), sizeof(MalType));
    append(symbol_list, "=", 1);

    append(function_pointers_list, wrap_function(less), sizeof(MalType));
    append(symbol_list, "<", 1);

    append(function_pointers_list, wrap_function(more), sizeof(MalType));
    append(symbol_list, ">", 1);

    append(function_pointers_list, wrap_function(equal_less), sizeof(MalType));
    append(symbol_list, "<=", 2);

    append(function_pointers_list, wrap_function(equal_more), sizeof(MalType));
    append(symbol_list, ">=", 2);




    while (symbol_list != NULL && function_pointers_list != NULL) {
        set(env, symbol_list->data, function_pointers_list->data);

        symbol_list = symbol_list->next;
        function_pointers_list = function_pointers_list->next;
    }
    if ((symbol_list == NULL) ^ (function_pointers_list == NULL)) {
        fprintf(stderr, "symbol_list and function_poiters_list are different lenght, some have been discarded\n");
    }

    return env;
}
