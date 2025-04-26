#include <stdio.h>
#include <string.h>
#include "gc.h"
#include "reader.h"
#include "printer.h"
#include "types.h"
#include "env.h"
#include "core.h"
#include "EVAL.h"


MalType * add(node_t * node) {
    // add a list of signed long
    if (node->data == NULL) {
        return 0;
    }

    signed long result = 0;
    while (node != NULL) {
        MalType * integer = node->data;
        if (integer->type != MAL_INT) {
            printf("add arg is not int\n");
            break;
        }
        result += *integer->value.IntValue;
        node = node->next;
    };

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
    mal_string_ret->value.StringValue = GC_MALLOC(strlen(string));
    memcpy(mal_string_ret->value.StringValue, string, strlen(string));

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
    mal_string_ret->value.StringValue = GC_MALLOC(strlen(string));
    memcpy(mal_string_ret->value.StringValue, string, strlen(string));

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
MalType * readstring(node_t * node) {
    if ( ((MalType *)node->data)->type != MAL_STRING) {
        printf("read-string arg is not a string\n");
        return NULL;
    }
    char * string = ((MalType *)node->data)->value.StringValue;

    return read_str(string);
}
MalType * slurp(node_t * node) {
    // chatGPT wrote this one

    // unpack the filename
    if ( ((MalType *)node->data)->type != MAL_STRING) {
        printf("slurp arg should be a string\n");
        return NULL;
    }
    char * filename = ((MalType *)node->data)->value.StringValue;

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL; // Return NULL if the file cannot be opened
    }

    // Move the file pointer to the end to determine the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET); // Move back to the beginning of the file

    // Allocate memory for the contents (+1 for the null terminator)
    char* buffer = (char*)GC_MALLOC(file_size + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL; // Return NULL if memory allocation fails
    }

    // Read the file contents into the buffer
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0'; // Null-terminate the string

    fclose(file); // Close the file

    // wrap buffer in a MAL_SRING
    MalType * string = GC_MALLOC(sizeof(MalType));
    string->type = MAL_STRING;
    string->value.StringValue = buffer;

    return string; // Return the contents of the file
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
        case MAL_FN_WRAPER: {
            // unpack and recursive call
            MalFn * fn1 = arg1->value.FnWraperValue->fn;
            MalFn * fn2 = arg2->value.FnWraperValue->fn;

            // wrap in a list
            node_t * list = GC_MALLOC(sizeof(node_t));
            append(list, fn1, sizeof(MalFn));
            append(list, fn2, sizeof(MalFn));

            return equal(list);
        }
        case MAL_FN: {
            // two function with same body and same params
            MalFn * fn1 = arg1->value.FnValue;
            MalFn * fn2 = arg2->value.FnValue;

            // build a new body_list for recursive equal call
            node_t * body_list = GC_MALLOC(sizeof(node_t));
            append(body_list, fn1->body, sizeof(MalType));
            append(body_list, fn2->body, sizeof(MalType));
            if (! equal(body_list)) {
                return false;
            }

            // param is a MalType->list of MalType->Symbol
            if ( ((MalType *)fn1->param)->type != MAL_LIST &&  ((MalType *)fn1->param)->type != MAL_VECTOR) {
                printf("equal : fn1->param is not a list\n");
            }
            if ( ((MalType *)fn2->param)->type != MAL_LIST &&  ((MalType *)fn2->param)->type != MAL_VECTOR) {
                printf("equal : fn2->param is not a list\n");
            }
            node_t * symbol_list_1 = ((MalType *)fn1->param)->value.ListValue;
            node_t * symbol_list_2 = ((MalType *)fn2->param)->value.ListValue;

            while (symbol_list_1 != NULL && symbol_list_2 != NULL) {
                // should check that every symbol is indeed a MAL_SYMBOL
                char * symbol1 = ((MalType *)symbol_list_1->data)->value.SymbolValue;
                char * symbol2 = ((MalType *)symbol_list_2->data)->value.SymbolValue;

                if (strcmp(symbol1, symbol2) != 0) {
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

            // TODO: check if the env themself are the same
            // (map_t *)env->data
            // when hashmap implemented, recuscive equal call

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
        // types are the same, so singletones (true false nil) can return true directly
        default: {return true;}
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

MalType * atom(node_t * node) {
    if (node->data == NULL) {
        printf("atom must take an arg \n");
        return node->data;
    }
    MalType * ret = GC_MALLOC(sizeof(MalType));
    ret->type = MAL_ATOM;
    ret->value.AtomValue = node->data;
    return ret;

}
MalType * atom_question_mark(node_t * node) {
    MalType * false = GC_MALLOC(sizeof(MalType));
    false->type = MAL_FALSE;
    false->value.FalseValue = NULL;
    MalType * true = GC_MALLOC(sizeof(MalType));
    true->type = MAL_TRUE;
    true->value.TrueValue = NULL;

    if (node->data == NULL) {
        return false;
    }

    if (((MalType *)node->data)->type == MAL_ATOM) {
        return true;
    } else {
        return false;
    }
}
MalType * deref(node_t * node) {
    MalType * atom = node->data;
    if (atom->type != MAL_ATOM) {
        printf("deref arg is not an atom\n");
        return node->data;
    }
    return atom->value.AtomValue;
}
MalType * reset(node_t * node) {
    MalType * atom = node->data;
    if (atom->type != MAL_ATOM) {
        printf("reset arg1 is not an atom\n");
        return node->data;
    }
    if (node->next == NULL) {
        printf("reset take two arg\n");
        return NULL;
    }
    memcpy(atom->value.AtomValue, node->next->data, sizeof(MalType));
    return atom->value.AtomValue;
}
MalType * swap(node_t * node) {
    MalType * atom = node->data;
    if (atom->type != MAL_ATOM) {
        printf("swap arg1 is not an atom\n");
        return node->data;
    }
    if (node->next == NULL) {
        printf("swap take two arg\n");
        return NULL;
    }
    MalType * fn = node->next->data;

    // wrap the fn, atom and args into a list to call EVAL
    // EVAL is not included
    // manual fn apply ?
    // create a EVAL.h that expose EVAL from step6.c ?
    // even now how can i get the env ?
    // from the function maybe...
    MalType * wraper = GC_MALLOC(sizeof(MalType));
    wraper->type = MAL_LIST;
    wraper->value.ListValue = GC_MALLOC(sizeof(node_t));

    // list (fn, atom->value, arg1, arg2, ...)
    append(wraper->value.ListValue, fn, sizeof(MalType));
    append(wraper->value.ListValue, atom->value.AtomValue, sizeof(MalType));
    // add args if any
    node_t * arg = node->next->next;
    while (arg != NULL) {
        append(wraper->value.ListValue, arg->data, sizeof(MalType));
        arg = arg->next;
    }

    MalType * ret = EVAL(wraper, fn->value.FnWraperValue->env);

    // reset
    memcpy(atom->value.AtomValue, ret, sizeof(MalType));

    return atom->value.AtomValue;
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
    append(function_pointers_list, wrap_function(swap), sizeof(MalType));
    append(symbol_list, "swap!", 5);

    append(function_pointers_list, wrap_function(reset), sizeof(MalType));
    append(symbol_list, "reset!", 6);

    append(function_pointers_list, wrap_function(deref), sizeof(MalType));
    append(symbol_list, "deref", 5);

    append(function_pointers_list, wrap_function(atom_question_mark), sizeof(MalType));
    append(symbol_list, "atom?", 5);

    append(function_pointers_list, wrap_function(atom), sizeof(MalType));
    append(symbol_list, "atom", 4);

    append(function_pointers_list, wrap_function(slurp), sizeof(MalType));
    append(symbol_list, "slurp", 5);

    append(function_pointers_list, wrap_function(readstring), sizeof(MalType));
    append(symbol_list, "read-string", 11);

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
