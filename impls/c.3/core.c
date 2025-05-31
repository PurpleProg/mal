#include "core.h"
#include "EVAL.h"
#include "env.h"
#include "gc.h"
#include "linked_list.h"
#include "printer.h"
#include "reader.h"
#include "types.h"
#include <gc/gc.h>
#include <stdio.h>
#include <string.h>

// math
MalType *add(node_t *node) {
    // add a list of MalInt
    if (node->data == NULL) {
        return 0;
    }

    MalInt result = 0;
    while (node != NULL) {
        MalType *integer = node->data;
        if (!IsInt(integer)) {
            printf("add arg is not int\n");
            global_error = NewMalList(node);
            break;
        }
        result += *GetInt(integer);
        node = node->next;
    };

    return NewMalInt(result);
}
MalType *sub(node_t *node) {
    // sub a list of MalInt
    if (node->data == NULL) {
        return 0;
    }

    MalInt result = 0;

    if (is_empty(node) || is_empty(node->next)) {
        fprintf(stderr, "at least two number pls\n");
        global_error = NewMalList(node);
        return 0;
    }

    MalInt a = *GetInt(node->data);
    MalInt b = *GetInt(node->next->data);
    result   = a - b;

    return NewMalInt(result);
}
MalType *mult(node_t *node) {
    // mult a list of MalInt
    if (node->data == NULL) {
        return 0;
    }

    MalInt result = 1;
    do {
        result *= *GetInt(node->data);
        node = node->next;
    } while (node != NULL);

    return NewMalInt(result);
}
MalType *divide(node_t *node) {
    // divide two of MalInt
    if (node->data == NULL) {
        return 0;
    }

    MalInt result = 0;

    if (node->next == NULL) {
        printf("divide must take two arg (not 0)");
        global_error = NewMalList(node);
        return 0;
    } else if (node->next == NULL) {
        printf("divide must take two arg (not 1)");
        global_error = NewMalList(node);
        return 0;
    } // else if (node->next->next->next != NULL) {

    MalInt a = *GetInt(node->data);
    MalInt b = *GetInt(node->next->data);

    result = a / b;

    return NewMalInt(result);
}

// strings
MalType *prstr(node_t *node) {
    size_t buffer_size = 256;
    char  *string      = GC_MALLOC(buffer_size);
    string[0]          = '\0';

    while (node != NULL) {
        char *new_string = pr_str(node->data, 1);

        // +1 for a space
        size_t new_size = strlen(string) + strlen(new_string) + 1;
        if (new_size > buffer_size) {
            char *temp = GC_MALLOC(new_size);
            strcpy(temp, string);
            buffer_size = new_size;
            string      = temp;
        }

        strcat(string, new_string);
        if (node->next != NULL) {
            strcat(string, " ");
        }
        node = node->next;
    }

    return NewMalString(string);
}
MalType *str(node_t *node) {
    size_t buffer_size = 256;
    char  *string      = GC_MALLOC(buffer_size);
    string[0]          = '\0';

    while (node != NULL) {
        char *new_string = pr_str(node->data, 0);

        size_t new_size = strlen(string) + strlen(new_string);
        if (new_size > buffer_size) {
            char *temp = GC_MALLOC(new_size);
            strcpy(temp, string);
            buffer_size = new_size;
            string      = temp;
        }

        strcat(string, new_string);
        node = node->next;
    }

    return NewMalString(string);
}
MalType *prn(node_t *node) {
    size_t buffer_size = 256;
    char  *string      = GC_MALLOC(buffer_size);
    string[0]          = '\0';

    while (node != NULL) {
        char *new_string = pr_str(node->data, 1);

        // +1 for a space
        size_t new_size = strlen(string) + strlen(new_string) + 1;
        if (new_size > buffer_size) {
            char *temp = GC_MALLOC(new_size);
            strcpy(temp, string);
            buffer_size = new_size;
            string      = temp;
        }

        strcat(string, new_string);
        if (node->next != NULL) {
            strcat(string, " ");
        }
        node = node->next;
    }

    printf("%s\n", string);

    return NewMalNIL();
}
MalType *println(node_t *node) {
    size_t buffer_size = 256;
    char  *string      = GC_MALLOC(buffer_size);
    string[0]          = '\0';

    while (node != NULL) {
        char *new_string = pr_str(node->data, 0);

        // +1 for a space
        size_t new_size = strlen(string) + strlen(new_string) + 1;
        if (new_size > buffer_size) {
            char *temp = GC_MALLOC(new_size);
            strcpy(temp, string);
            buffer_size = new_size;
            string      = temp;
        }

        strcat(string, new_string);
        if (node->next != NULL) {
            strcat(string, " ");
        }
        node = node->next;
    }

    printf("%s\n", string);

    return NewMalNIL();
}
MalType *readstring(node_t *node) {
    if (!IsString(node->data)) {
        printf("read-string arg is not a string\n");
        global_error = NewMalList(node);
        return NULL;
    }
    char *string = GetString(node->data);

    return read_str(string);
}
MalType *slurp(node_t *node) {
    // chatGPT wrote this one

    // unpack the filename
    if (!IsString(node->data)) {
        printf("slurp arg should be a string\n");
        global_error = NewMalList(node);
        return NULL;
    }
    char *filename = GetString(node->data);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL; // Return NULL if the file cannot be opened
    }

    // Move the file pointer to the end to determine the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET); // Move back to the beginning of the file

    // Allocate memory for the contents (+1 for the null terminator)
    char *buffer = (char *)GC_MALLOC(file_size + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL; // Return NULL if memory allocation fails
    }

    // Read the file contents into the buffer
    size_t bytes_read  = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0'; // Null-terminate the string

    fclose(file); // Close the file

    return NewMalString(buffer); // Return the contents of the file
}

// lists
MalType *list(node_t *node) {
    if (node->data == NULL) {
        // list is empty, allocate a new one
        node_t *new_list = GC_MALLOC(sizeof(node_t));
        new_list->next   = NULL;
        new_list->data   = NULL;
        node             = new_list;
    }
    MalType *arg = node->data;
    if (arg == NULL) {
        // empty list
        return NewMalList(GC_MALLOC(sizeof(MalList)));
    }

    return NewMalListCopy(node);
}
MalType *vec(node_t *node) {
    if (node->data == NULL) {
        // list is empty, allocate a new one
        node_t *new_list = GC_MALLOC(sizeof(node_t));
        new_list->next   = NULL;
        new_list->data   = NULL;
        node             = new_list;
    }

    MalType *arg = node->data;
    if (arg == NULL) {
        // empty list
        return NewMalVector(GC_MALLOC(sizeof(MalList)));
    }
    if (!IsListOrVector(arg)) {
        // TODO: raise an error
        printf("vec take a list or a vector as arg\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }

    return NewMalVector(GetList(arg));
}
MalType *list_question_mark(node_t *node) {
    if (node->data == NULL) {
        return NewMalFalse();
    }
    if (IsList(node->data)) {
        return NewMalTrue();
    }
    return NewMalFalse();
}
MalType *empty_question_mark(node_t *node) {
    if (node->data == NULL) {
        return NewMalFalse();
    }
    if (!(IsListOrVector(node->data))) {
        printf("arg is not a list\n");
        global_error = NewMalList(node);
        return NewMalFalse();
    }
    MalList *list = GetList(node->data);
    if (list->data == NULL) {
        return NewMalTrue();
    }
    return NewMalFalse();
}
MalType *count(node_t *node) {
    MalInt counter = 0;

    node_t *new_node = GC_MALLOC(sizeof(node_t));
    if (node->data == NULL) {
        // arg is empty
        counter = -1;
        return NewMalInt(counter);
    }
    if (!IsListOrVector(node->data)) {
        counter = 0;
        return NewMalInt(counter);
    }

    new_node = GetList(node->data);

    if (new_node->data == NULL) {
        // list is empty
        counter = 0;
        return NewMalInt(counter);
    }

    while (new_node != NULL) {
        counter += 1;
        new_node = new_node->next;
    }
    return NewMalInt(counter);
}
MalType *cons(node_t *node) {
    node_t  *new_list = GC_MALLOC(sizeof(node_t));
    MalType *ret      = NewMalList(new_list);
    if (node == NULL) {
        printf("cons need args");
        global_error = NewMalList(node);
        return ret;
    }
    if (node->next == NULL) {
        printf("cons without list");
        global_error = NewMalList(node);
        return ret;
    }
    if (node->next->data == NULL) {
        // allocate a new_list, this one is empty
        printf("node->next->data is NULL");
        global_error = NewMalList(node);
        return ret;
    }

    // arg 1
    MalType *arg1 = node->data;
    append(new_list, arg1, sizeof(MalType));

    // second arg
    MalType *old_list = node->next->data;

    // append the rest of the list
    node_t *old_list_node = GetList(old_list);
    while (old_list_node != NULL && old_list_node->data != NULL) {
        append(new_list, old_list_node->data, sizeof(MalType));
        old_list_node = old_list_node->next;
    }

    return ret;
}
MalType *concat(node_t *node) {
    node_t  *new_list = GC_MALLOC(sizeof(node_t));
    MalType *ret      = NewMalList(new_list);

    if (is_empty(node)) {
        printf("concat arg1 is NULL\n");
    }

    // node is a list of lists, a list of args to concat
    while (!is_empty(node)) {
        MalType *list      = node->data;
        MalList *list_node = GetList(list);
        if (!IsListOrVector(list)) {
            printf("concat arg in not list nor vector\n");
            global_error = NewMalList(node);
            // skipping...
            node = node->next;
            continue;
        }

        while (list_node != NULL && list_node->data != NULL) {
            append(new_list, list_node->data, sizeof(MalType));
            list_node = list_node->next;
        }
        node = node->next;
    }

    return ret;
}
MalType *nth(node_t *node) {
    MalType *nil = NewMalNIL();

    if (node == NULL) {
        printf("nth need args");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->data == NULL) {
        printf("nth args1 is empty list");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next == NULL) {
        printf("nth without indice");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next->data == NULL) {
        printf("nth node->next->data is NULL");
        global_error = NewMalList(node);
        return nil;
    }

    // arg 1 list
    MalType *arg1 = node->data;
    if (!IsListOrVector(arg1)) {
        printf("nth arg1 must be a list or a vector\n");
        global_error = NewMalList(node);
        return nil;
    }

    // second arg index
    MalType *arg2 = node->next->data;
    if (!IsInt(arg2)) {
        printf("nth arg2 must be a int\n");
        global_error = NewMalList(node);
        return nil;
    }

    MalList *list         = GetList(arg1);
    int      target_index = *GetInt(arg2);
    if (target_index < 0) {
        printf("index < 0\n");
        global_error = NewMalList(node);
        return nil;
    }
    for (int i = 0; i < target_index; i++) {

        list = list->next;
        if (is_empty(list)) {
            printf("nth index out of range\n");
            global_error = NewMalList(node);
            return nil;
        }
    }
    if (is_empty(list)) {
        return nil;
    }
    return list->data;
}
MalType *first(node_t *node) {
    // build a list : (arg, 0)
    node_t *list = GC_MALLOC(sizeof(node_t));

    append(list, node->data, sizeof(MalType));
    append(list, NewMalInt(0), sizeof(MalType));

    return nth(list);
}
MalType *rest(node_t *node) {
    MalType *arg = node->data;
    if (arg == NULL) {
        printf("rest takes an arg\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    if (!IsListOrVector(arg)) {
        printf("rest arg must be a list or a vecotr\n");
        global_error = NewMalList(node);
        // empty list
        return NewMalList(GC_MALLOC(sizeof(node_t)));
    }

    MalList *list = GetList(arg);
    // skip first element
    list = list->next;
    // allocate a new list
    MalList *new_list = GC_MALLOC(sizeof(node_t));
    // copy list into new_list
    while (!is_empty(list)) {
        append(new_list, list->data, sizeof(MalType));
        list = list->next;
    }

    return NewMalList(new_list);
}

MalType *equal(MalList *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();
    MalType *nil   = NewMalNIL();

    if (is_empty(node)) {
        printf("= with no arg \n");
        global_error = NewMalList(node);
        return nil;
    }
    if (is_empty(node->next)) {
        printf("= with only one arg\n");
        global_error = NewMalList(node);
        return nil;
    }

    MalType *arg1 = node->data;
    MalType *arg2 = node->next->data;

    int eval_hashmap = 0;

    if (arg1->type != arg2->type) {
        if (!(IsListOrVector(arg1) && IsListOrVector(arg2))) {
            return false;
        }
    }
    switch (arg1->type) {
    case MAL_HASHMAP:
        // fallback to list equality
        // accesing ListValue instead of Hashmap value work here
        // because bolth are typedef of node_t
        eval_hashmap = 1;
    case MAL_VECTOR:
        // fallback to list equality
    case MAL_LIST: {
        MalList *node1;
        MalList *node2;
        if (eval_hashmap == 0) {
            node1 = GetList(arg1);
            node2 = GetList(arg2);
        } else {
            node1 = GetHashmap(arg1);
            node2 = GetHashmap(arg2);
        }
        while (node1 != NULL && node2 != NULL) {
            // if two elements are empty:
            if (node1->data == NULL && node2->data == NULL) {
                return true;
            } else if (node1->data == NULL || node2->data == NULL) {
                // lenght are different
                return false;
            }

            node_t *zipped = GC_MALLOC(sizeof(node_t));
            zipped->data   = NULL;
            zipped->next   = NULL;
            append(zipped, node1->data, sizeof(MalType));
            append(zipped, node2->data, sizeof(MalType));

            if (IsFalse(equal(zipped))) {
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
        MalInt int1 = *GetInt(arg1);
        MalInt int2 = *GetInt(arg2);
        if (int1 == int2) {
            return true;
        }
        // false
        return false;
    }
    case MAL_STRING: {
        char *string1 = GetString(arg1);
        char *string2 = GetString(arg2);
        if (strcmp(string1, string2) == 0) {
            return true;
        }
        return false;
    }
    case MAL_KEYWORD: {
        char *symbol1 = GetKeyword(arg1);
        char *symbol2 = GetKeyword(arg2);
        if (strcmp(symbol1, symbol2) == 0) {
            return true;
        }
        return false;
    }
    case MAL_SYMBOL: {
        char *symbol1 = GetSymbol(arg1);
        char *symbol2 = GetSymbol(arg2);
        if (strcmp(symbol1, symbol2) == 0) {
            return true;
        }
        return false;
    }
    case MAL_FN_WRAPER: {
        // unpack and recursive call
        MalFn *fn1 = GetFnWrapper(arg1)->fn;
        MalFn *fn2 = GetFnWrapper(arg2)->fn;

        // wrap in a list
        node_t *list = GC_MALLOC(sizeof(node_t));
        append(list, fn1, sizeof(MalFn));
        append(list, fn2, sizeof(MalFn));

        return equal(list);
    }
    case MAL_FN: {
        // two function with same body and same params
        MalFn *fn1 = GetFn(arg1);
        MalFn *fn2 = GetFn(arg2);

        // build a new body_list for recursive equal call
        node_t *body_list = GC_MALLOC(sizeof(node_t));
        append(body_list, fn1->body, sizeof(MalType));
        append(body_list, fn2->body, sizeof(MalType));
        if (!equal(body_list)) {
            return false;
        }

        // param is a MalType->list of MalType->Symbol
        if (!IsListOrVector(fn1->param)) {
            printf("equal : fn1->param is not a list\n");
            global_error = NewMalList(node);
        }
        if (!IsListOrVector(fn2->param)) {
            printf("equal : fn2->param is not a list\n");
            global_error = NewMalList(node);
        }
        node_t *symbol_list_1 = GetList(fn1->param);
        node_t *symbol_list_2 = GetList(fn2->param);

        while (symbol_list_1 != NULL && symbol_list_2 != NULL) {
            // should check that every symbol is indeed a MAL_SYMBOL
            char *symbol1 = GetSymbol(symbol_list_1->data);
            char *symbol2 = GetSymbol(symbol_list_2->data);

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
        env_t *env1 = fn1->env->outer;
        env_t *env2 = fn2->env->outer;
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
        if (GetCoreFn(arg1) == GetCoreFn(arg2)) {
            return true;
        }
        return false;
    }
    // types are the same, so singletones (true false nil) can return true
    // directly
    default: {
        return true;
    }
    }

    printf("= fallback\n");
    global_error = NewMalList(node);
    return false;
}
MalType *apply(MalList *node) {
    /*
    ** takes at least two arguments.
    ** The first argument is a function
    ** and the last argument is a list (or vector).
    ** The function may be either
    ** a built-in core function,
    ** an user function constructed with the fn* special form,
    ** or a macro,
    ** not distinguished from the underlying user function).
    ** The arguments between the function and the last argument
    ** (if there are any)
    ** are concatenated with the final argument
    ** to create the arguments that are used to call the function.
    ** The apply function allows a function to be called
    ** with arguments that are contained in a list (or vector).
    */
    if (is_empty(node)) {
        printf("apply with no arg \n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    if (is_empty(node->next)) {
        printf("apply with only one arg\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }

    MalType *function   = node->data;
    MalList *apply_args = node->next;

    // concat args
    MalList *args = GC_MALLOC(sizeof(MalList));
    while (apply_args != NULL) {
        if (IsListOrVector(apply_args->data)) {
            // concat
            MalList *sublist = GetList(apply_args->data);
            while (!is_empty(sublist)) {
                append(args, sublist->data, sizeof(MalType));
                sublist = sublist->next;
            }

        } else {
            append(args, apply_args->data, sizeof(MalType));
        }
        apply_args = apply_args->next;
    }

    switch (function->type) {
    case MAL_CORE_FN: {
        return GetCoreFn(function)(args);
    }
    case MAL_FN_WRAPER: {
        MalList *result = GC_MALLOC(sizeof(MalList));
        append(result, function, sizeof(MalType));
        while (!is_empty(args)) {
            append(result, args->data, sizeof(MalType));
            args = args->next;
        }

        return EVAL(NewMalList(result), GetFnWrapper(function)->env);
    }
    default: {
        printf("aply must take a function as arg1\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    }
}
MalType *map(MalList *node) {
    /*
    ** takes a function
    ** and a list (or vector)
    ** and evaluates the function against
    ** every element of the list (or vector)
    ** one at a time
    ** and returns the results as a list
    */
    if (is_empty(node)) {
        printf("map with no arg \n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    if (is_empty(node->next)) {
        printf("map with only one arg\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }

    MalList *result = GC_MALLOC(sizeof(MalList));

    MalType *function = node->data;
    MalType *map_args = node->next->data;

    if (!IsCoreFn(function) && !IsFnWrapper(function)) {
        printf("map must take a function as arg 2\n");
        global_error = NewMalList(node);
        return NewMalList(result);
    }
    if (!IsListOrVector(map_args)) {
        printf("map must take a list or vector as arg 2\n");
        global_error = NewMalList(node);
        return NewMalList(result);
    }

    MalList *list_args = GetList(map_args);
    while (!is_empty(list_args)) {
        // append result apply(function, list_args->data)
        MalList *apply_call_args = GC_MALLOC(sizeof(MalList));
        append(apply_call_args, function, sizeof(MalType));
        append(apply_call_args, list_args->data, sizeof(MalType));

        MalType *apply_ret = apply(apply_call_args);

        append(result, apply_ret, sizeof(MalType));

        list_args = list_args->next;
    }

    return NewMalList(result);
}

// comparators
MalType *less(node_t *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();
    MalType *nil   = NewMalNIL();

    if (node == NULL) {
        printf("< with no arg \n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->data == NULL) {
        printf("< arg1 is NULL \n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next == NULL) {
        printf("< with only one arg\n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next->data == NULL) {
        printf("< arg2 is NULL \n");
        global_error = NewMalList(node);
        return nil;
    }

    // maybe i dont need to allocate new memory here ?
    MalType *arg1 = GC_MALLOC(sizeof(MalType));
    MalType *arg2 = GC_MALLOC(sizeof(MalType));
    arg1          = node->data;
    arg2          = node->next->data;

    if (!IsInt(arg1) || !IsInt(arg2)) {
        printf("< with non int parameters\n");
        global_error = NewMalList(node);
        return NULL;
    }
    if (*GetInt(arg1) < *GetInt(arg2)) {
        return true;
    }
    return false;
}
MalType *more(node_t *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();
    MalType *nil   = NewMalNIL();

    if (node == NULL) {
        printf("> with no arg \n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->data == NULL) {
        printf("> arg1 is NULL \n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next == NULL) {
        printf("> with only one arg\n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next->data == NULL) {
        printf("> arg2 is NULL \n");
        global_error = NewMalList(node);
        return nil;
    }

    // maybe i dont need to allocate new memory here ?
    MalType *arg1 = GC_MALLOC(sizeof(MalType));
    MalType *arg2 = GC_MALLOC(sizeof(MalType));
    arg1          = node->data;
    arg2          = node->next->data;

    if (!IsInt(arg1) || !IsInt(arg2)) {
        printf("> with non int parameters\n");
        global_error = NewMalList(node);
        return NULL;
    }
    if (*GetInt(arg1) > *GetInt(arg2)) {
        return true;
    }
    return false;
}
MalType *equal_less(node_t *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();
    MalType *nil   = NewMalNIL();

    if (node == NULL) {
        printf("<= with no arg \n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->data == NULL) {
        printf("<= arg1 is NULL \n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next == NULL) {
        printf("<= with only one arg\n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next->data == NULL) {
        printf("<= arg2 is NULL \n");
        global_error = NewMalList(node);
        return nil;
    }

    // maybe i dont need to allocate new memory here ?
    MalType *arg1 = GC_MALLOC(sizeof(MalType));
    MalType *arg2 = GC_MALLOC(sizeof(MalType));
    arg1          = node->data;
    arg2          = node->next->data;

    if (!IsInt(arg1) || !IsInt(arg2)) {
        printf("<= with non int parameters\n");
        global_error = NewMalList(node);
        return nil;
    }
    if (*GetInt(arg1) <= *GetInt(arg2)) {
        return true;
    }
    return false;
}
MalType *equal_more(node_t *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();
    MalType *nil   = NewMalNIL();

    if (node == NULL) {
        printf(">= with no arg \n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->data == NULL) {
        printf(">= arg1 is NULL \n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next == NULL) {
        printf(">= with only one arg\n");
        global_error = NewMalList(node);
        return nil;
    }
    if (node->next->data == NULL) {
        printf(">= arg2 is NULL \n");
        global_error = NewMalList(node);
        return nil;
    }

    // maybe i dont need to allocate new memory here ?
    MalType *arg1 = GC_MALLOC(sizeof(MalType));
    MalType *arg2 = GC_MALLOC(sizeof(MalType));
    arg1          = node->data;
    arg2          = node->next->data;

    if (!IsInt(arg1) || !IsInt(arg2)) {
        printf(">= with non int parameters\n");
        global_error = NewMalList(node);
        return nil;
    }
    if (*GetInt(arg1) >= *GetInt(arg2)) {
        return true;
    }
    return false;
}

// atoms
MalType *atom(node_t *node) {
    if (node->data == NULL) {
        printf("atom must take an arg \n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    return NewMalAtom(node->data);
}
MalType *atom_question_mark(node_t *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();

    if (node->data == NULL) {
        return false;
    }

    if (IsAtom(node->data)) {
        return true;
    } else {
        return false;
    }
}
MalType *deref(node_t *node) {
    MalType *atom = node->data;
    if (!IsAtom(atom)) {
        printf("deref arg is not an atom\n");
        global_error = NewMalList(node);
        return node->data;
    }
    return GetAtom(atom);
}
MalType *reset(node_t *node) {
    MalType *atom = node->data;
    if (!IsAtom(atom)) {
        printf("reset arg1 is not an atom\n");
        global_error = NewMalList(node);
        return node->data;
    }
    if (node->next == NULL) {
        printf("reset take two arg\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    memcpy(GetAtom(atom), node->next->data, sizeof(MalType));
    return GetAtom(atom);
}
MalType *swap(node_t *node) {
    MalType *nil = NewMalNIL();

    MalType *atom = node->data;
    if (!IsAtom(atom)) {
        printf("swap arg1 is not an atom\n");
        global_error = NewMalList(node);
        return node->data;
    }
    if (is_empty(node->next)) {
        printf("swap take two arg\n");
        global_error = NewMalList(node);
        return nil;
    }
    MalType *fn = node->next->data;

    // wrap the fn, atom and args into a list to call EVAL
    node_t *list = GC_MALLOC(sizeof(node_t));

    // list (fn, atom->value, arg1, arg2, ...)
    append(list, fn, sizeof(MalType));
    append(list, GetAtom(atom), sizeof(MalType));

    // add args if any
    node_t *arg = node->next->next;
    while (arg != NULL) {
        append(list, arg->data, sizeof(MalType));
        arg = arg->next;
    }

    MalType *ret;
    if (IsCoreFn(fn)) {
        // skip core fn
        ret = GetCoreFn(fn)(list->next);
    } else {
        ret = EVAL(NewMalList(list), GetFnWrapper(fn)->env);
    }

    // reset
    memcpy(atom->value.AtomValue, ret, sizeof(MalType));

    return GetAtom(atom);
}

// type prediacte
MalType *macro_question_mark(node_t *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();

    if (is_empty(node)) {
        return false;
    }
    MalType *arg = node->data;

    if (IsFnWrapper(arg)) {
        if (GetFnWrapper(arg)->is_macro) {
            return true;
        }
    }
    return false;
}
MalType *nil_tp(MalList *node) {
    if (IsNil(node->data)) {
        return NewMalTrue();
    }
    return NewMalFalse();
}
MalType *true_tp(MalList *node) {
    if (IsTrue(node->data)) {
        return NewMalTrue();
    }
    return NewMalFalse();
}
MalType *false_tp(MalList *node) {
    if (IsFalse(node->data)) {
        return NewMalTrue();
    }
    return NewMalFalse();
}
MalType *symbol_tp(MalList *node) {
    if (IsSymbol(node->data)) {
        return NewMalTrue();
    }
    return NewMalFalse();
}
MalType *vector_tp(MalList *node) {
    if (IsVector(node->data)) {
        return NewMalTrue();
    }
    return NewMalFalse();
}
MalType *hashmap_tp(MalList *node) {
    if (IsHashmap(node->data)) {
        return NewMalTrue();
    }
    return NewMalFalse();
}
MalType *keyword_tp(MalList *node) {
    if (IsKeyword(node->data)) {
        return NewMalTrue();
    }
    return NewMalFalse();
}
MalType *sequential_tp(MalList *node) {
    if (IsListOrVector(node->data)) {
        return NewMalTrue();
    }
    return NewMalFalse();
}

// create new type
MalType *symbol(MalList *node) {
    if (IsString(node->data)) {
        return NewMalSymbol(GetString(node->data));
    }
    // TODO: raise an error
    return NewMalNIL();
}
MalType *keyword(MalList *node) {
    if (IsKeyword(node->data)) {
        return node->data;
    }
    if (IsString(node->data)) {
        char *string = GetString(node->data);
        return NewMalKeyword(string);
    }
    global_error = NewMalList(node);
    return NewMalNIL();
}
MalType *vector(MalList *node) {
    MalType *arg = node->data;
    if (arg == NULL) {
        // empty vector
        return NewMalVector(GC_MALLOC(sizeof(MalList)));
    }

    // NOTE: maybe something like ListCopy here ?
    return NewMalVector(node);
}
MalType *hashmap(MalList *node) {
    /*
    ** takes a variable but even number of arguments
    ** and returns a new mal hash-map value
    ** with keys from the odd arguments
    ** and values from the even arguments respectively.
    */
    MalHashmap *hashmap = GC_MALLOC(sizeof(MalHashmap));
    if (is_empty(node)) {
        hashmap->next = NULL;
        return NewMalHashmap(hashmap);
    }

    while (!is_empty(node)) {
        if (is_empty(node->next)) {
            printf("hashmap takes a even number of args\n");
            global_error = NewMalList(node);
            return NewMalNIL();
        }

        if (!IsString(node->data) && !IsKeyword(node->data)) {
            printf("hashmap key can only be string or keyword, not %s\n",
                   pr_str(node->data, 0));
            global_error = NewMalList(node);
            return NewMalNIL();
        }
        // key
        append(hashmap, node->data, sizeof(MalType));

        // value
        append(hashmap, node->next->data, sizeof(MalType));

        node = node->next->next;
    }

    return NewMalHashmap(hashmap);
}

// hash-map
MalType *get_hashmap(MalList *node) {
    if (is_empty(node)) {
        printf("get without args\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    MalType *hashmap = node->data;
    if (!IsHashmap(hashmap)) {
        printf("get arg 1 must be hashmap\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }

    if (is_empty(node->next)) {
        printf("get without arg 2\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    MalType *key = node->next->data;
    if (!IsString(key) && !IsKeyword(key)) {
        printf("get arg 2 (key) must be a string or a keyword\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    char *key_string;
    if (IsString(key)) {
        key_string = GetString(key);
    } else {
        key_string = GetKeyword(key);
    }

    MalHashmap *hashmap_node = GetHashmap(hashmap);
    while (!is_empty(hashmap_node)) {
        if (is_empty(hashmap_node->next)) {
            printf("hashmap shouldnt be odd\n");
            global_error = NewMalList(node);
            return NewMalNIL();
        }

        MalType *current_key = hashmap_node->data;
        char    *current_key_string;
        if (IsString(current_key)) {
            current_key_string = GetString(current_key);
        } else {
            current_key_string = GetKeyword(current_key);
        }

        if (strcmp(key_string, current_key_string) == 0) {
            // found
            return hashmap_node->next->data;
        }
        hashmap_node = hashmap_node->next->next;
    }
    // no found
    return NewMalNIL();
}
MalType *contains(MalList *node) {
    if (is_empty(node)) {
        printf("contains without args\n");
        global_error = NewMalList(node);
        return NewMalFalse();
    }
    MalType *hashmap = node->data;
    if (!IsHashmap(hashmap)) {
        printf("contains arg must be hashmap\n");
        global_error = NewMalList(node);
        return NewMalFalse();
    }

    if (is_empty(node->next)) {
        printf("contains without arg 2\n");
        global_error = NewMalList(node);
        return NewMalFalse();
    }
    MalType *key = node->next->data;
    if (!IsString(key) && !IsKeyword(key)) {
        printf("contains arg 2 (key) must be a string or a keyword\n");
        global_error = NewMalList(node);
        return NewMalFalse();
    }
    char *key_string;
    if (IsString(key)) {
        key_string = GetString(key);
    } else {
        key_string = GetKeyword(key);
    }

    MalHashmap *hashmap_node = GetHashmap(hashmap);
    while (!is_empty(hashmap_node)) {
        if (is_empty(hashmap_node->next)) {
            return NewMalFalse();
        }

        MalType *current_key = hashmap_node->data;
        char    *current_key_string;
        if (IsString(current_key)) {
            current_key_string = GetString(current_key);
        } else {
            current_key_string = GetKeyword(current_key);
        }

        if (strcmp(key_string, current_key_string) == 0) {
            // found
            return NewMalTrue();
        }
        hashmap_node = hashmap_node->next->next;
    }
    // not found
    return NewMalFalse();
}
MalType *keys(MalList *node) {
    if (is_empty(node)) {
        printf("keys without args\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    MalType *hashmap = node->data;
    if (!IsHashmap(hashmap)) {
        printf("keys arg must be hashmap\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }

    MalList *keys = GC_MALLOC(sizeof(MalList));

    MalHashmap *hashmap_node = GetHashmap(hashmap);
    while (!is_empty(hashmap_node)) {
        if (is_empty(hashmap_node->next)) {
            printf("hashmap shouldnt be odd\n");
            global_error = NewMalList(node);
            return NewMalNIL();
        }

        MalType *key = hashmap_node->data;
        append(keys, key, sizeof(MalType));

        hashmap_node = hashmap_node->next->next;
    }
    return NewMalList(keys);
}
MalType *vals(MalList *node) {
    if (is_empty(node)) {
        printf("vals without args\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    MalType *hashmap = node->data;
    if (!IsHashmap(hashmap)) {
        printf("vals arg must be hashmap\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }

    MalList *vals = GC_MALLOC(sizeof(MalList));

    MalHashmap *hashmap_node = GetHashmap(hashmap);
    while (!is_empty(hashmap_node)) {
        if (is_empty(hashmap_node->next)) {
            printf("hashmap shouldnt be odd\n");
            global_error = NewMalList(node);
            return NewMalNIL();
        }

        MalType *val = hashmap_node->next->data;
        append(vals, val, sizeof(MalType));

        hashmap_node = hashmap_node->next->next;
    }
    return NewMalList(vals);
}
MalType *assoc(MalList *node) {
    /*
    ** takes a hash-map as the first argument
    ** and the remaining arguments are odd/even key/value pairs
    ** to "associate" (merge) into the hash-map.
    ** Note that the original hash-map is unchanged
    ** (remember, mal values are immutable),
    ** and a new hash-map containing
    ** the old hash-maps key/values
    ** plus the merged key/value arguments
    ** is returned.
    */
    if (is_empty(node)) {
        printf("assoc without args\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }

    MalType *arg1 = node->data;
    if (!IsHashmap(arg1)) {
        printf("assoc arg must be hashmap\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    MalHashmap *hashmap = GetHashmap(arg1);

    MalList *args = node->next;

    // add the elt from the first hashmap
    // to the new one
    MalHashmap *new_hashmap = GC_MALLOC(sizeof(MalHashmap));
    while (!is_empty(hashmap)) {
        append(new_hashmap, hashmap->data, sizeof(MalType));
        hashmap = hashmap->next;
    }

    // add the elts from the args to the new hashmap
    while (args != NULL) {
        MalType *elt = args->data;
        append(new_hashmap, elt, sizeof(MalType));

        args = args->next;
    }
    return NewMalHashmap(new_hashmap);
}
MalType *dissoc(MalList *node) {
    /*
    ** takes a hash-map
    ** and a list of keys to remove from the hash-map.
    ** Again, note that the original hash-map is unchanged
    ** and a new hash-map with the keys removed is returned.
    ** Key arguments that do not exist in the hash-map are ignored.
    */
    if (is_empty(node)) {
        printf("dissoc without args\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }

    MalType *arg1 = node->data;
    if (!IsHashmap(arg1)) {
        printf("dissoc arg must be hashmap\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    MalHashmap *hashmap = GetHashmap(arg1);

    if (is_empty(node->next)) {
        printf("dissoc without arg 2\n");
        global_error = NewMalList(node);
        return NewMalNIL();
    }
    MalList *keys = node->next;

    // add the elt from the first hashmap
    // to the new one
    // if the key is not in the removed
    MalHashmap *new_hashmap = GC_MALLOC(sizeof(MalHashmap));
    while (!is_empty(hashmap)) {
        if (is_empty(hashmap->next)) {
            printf("hashmap shouldnt be odd\n");
            global_error = NewMalList(node);
            return NewMalNIL();
        }

        // check if key is in keys to be removed
        // NOTE: confusing variable names
        MalType *key = hashmap->data;
        if (!IsString(key) && !IsKeyword(key)) {
            printf("contains arg 2 (key) must be a string or a keyword\n");
            global_error = NewMalList(node);
            return NewMalNIL();
        }
        char *key_string;
        if (IsString(key)) {
            key_string = GetString(key);
        } else {
            key_string = GetKeyword(key);
        }
        MalList *keys_copy            = keys;
        int      should_continue_flag = 0;
        while (keys_copy != NULL) {
            MalType *key_copy = keys_copy->data;
            if (!IsString(key_copy) && !IsKeyword(key_copy)) {
                printf("contains arg 2 (key_copy) must be a string or a "
                       "key_copyword\n");
                global_error = NewMalList(node);
                return NewMalNIL();
            }
            char *key_from_to_remove_list_string;
            if (IsString(key_copy)) {
                key_from_to_remove_list_string = GetString(key_copy);
            } else {
                key_from_to_remove_list_string = GetKeyword(key_copy);
            }
            if (strcmp(key_from_to_remove_list_string, key_string) == 0) {
                hashmap              = hashmap->next->next;
                should_continue_flag = 1;
                break;
            }

            keys_copy = keys_copy->next;
        }
        if (should_continue_flag) {
            continue;
        }

        append(new_hashmap, hashmap->data, sizeof(MalType));
        append(new_hashmap, hashmap->next->data, sizeof(MalType));
        hashmap = hashmap->next->next;
    }

    return NewMalHashmap(new_hashmap);
}

env_t *create_repl() {

    env_t *env = create_env(NULL, NULL, NULL);

    node_t *fn_ptr_list = GC_MALLOC(sizeof(node_t));

    node_t *symbol_list = GC_MALLOC(sizeof(node_t));

    // yes, hard coded.
    // why :'(
    append(fn_ptr_list, NewMalCoreFn(dissoc), sizeof(MalType));
    append(symbol_list, "dissoc", 6);

    append(fn_ptr_list, NewMalCoreFn(assoc), sizeof(MalType));
    append(symbol_list, "assoc", 5);

    append(fn_ptr_list, NewMalCoreFn(vals), sizeof(MalType));
    append(symbol_list, "vals", 4);

    append(fn_ptr_list, NewMalCoreFn(keys), sizeof(MalType));
    append(symbol_list, "keys", 4);

    append(fn_ptr_list, NewMalCoreFn(contains), sizeof(MalType));
    append(symbol_list, "contains?", 9);

    append(fn_ptr_list, NewMalCoreFn(get_hashmap), sizeof(MalType));
    append(symbol_list, "get", 3);

    append(fn_ptr_list, NewMalCoreFn(hashmap), sizeof(MalType));
    append(symbol_list, "hash-map", 8);

    append(fn_ptr_list, NewMalCoreFn(map), sizeof(MalType));
    append(symbol_list, "map", 3);

    append(fn_ptr_list, NewMalCoreFn(apply), sizeof(MalType));
    append(symbol_list, "apply", 5);

    append(fn_ptr_list, NewMalCoreFn(vector), sizeof(MalType));
    append(symbol_list, "vector", 6);

    append(fn_ptr_list, NewMalCoreFn(keyword), sizeof(MalType));
    append(symbol_list, "keyword", 7);

    append(fn_ptr_list, NewMalCoreFn(symbol), sizeof(MalType));
    append(symbol_list, "symbol", 6);

    append(fn_ptr_list, NewMalCoreFn(sequential_tp), sizeof(MalType));
    append(symbol_list, "sequential?", 11);

    append(fn_ptr_list, NewMalCoreFn(keyword_tp), sizeof(MalType));
    append(symbol_list, "keyword?", 8);

    append(fn_ptr_list, NewMalCoreFn(hashmap_tp), sizeof(MalType));
    append(symbol_list, "map?", 4);

    append(fn_ptr_list, NewMalCoreFn(vector_tp), sizeof(MalType));
    append(symbol_list, "vector?", 7);

    append(fn_ptr_list, NewMalCoreFn(symbol_tp), sizeof(MalType));
    append(symbol_list, "symbol?", 7);

    append(fn_ptr_list, NewMalCoreFn(false_tp), sizeof(MalType));
    append(symbol_list, "false?", 6);

    append(fn_ptr_list, NewMalCoreFn(true_tp), sizeof(MalType));
    append(symbol_list, "true?", 5);

    append(fn_ptr_list, NewMalCoreFn(nil_tp), sizeof(MalType));
    append(symbol_list, "nil?", 4);

    append(fn_ptr_list, NewMalCoreFn(macro_question_mark), sizeof(MalType));
    append(symbol_list, "macro?", 6);

    append(fn_ptr_list, NewMalCoreFn(rest), sizeof(MalType));
    append(symbol_list, "rest", 4);

    append(fn_ptr_list, NewMalCoreFn(first), sizeof(MalType));
    append(symbol_list, "first", 5);

    append(fn_ptr_list, NewMalCoreFn(nth), sizeof(MalType));
    append(symbol_list, "nth", 3);

    append(fn_ptr_list, NewMalCoreFn(vec), sizeof(MalType));
    append(symbol_list, "vec", 3);

    append(fn_ptr_list, NewMalCoreFn(concat), sizeof(MalType));
    append(symbol_list, "concat", 6);

    append(fn_ptr_list, NewMalCoreFn(cons), sizeof(MalType));
    append(symbol_list, "cons", 4);

    append(fn_ptr_list, NewMalCoreFn(swap), sizeof(MalType));
    append(symbol_list, "swap!", 5);

    append(fn_ptr_list, NewMalCoreFn(reset), sizeof(MalType));
    append(symbol_list, "reset!", 6);

    append(fn_ptr_list, NewMalCoreFn(deref), sizeof(MalType));
    append(symbol_list, "deref", 5);

    append(fn_ptr_list, NewMalCoreFn(atom_question_mark), sizeof(MalType));
    append(symbol_list, "atom?", 5);

    append(fn_ptr_list, NewMalCoreFn(atom), sizeof(MalType));
    append(symbol_list, "atom", 4);

    append(fn_ptr_list, NewMalCoreFn(slurp), sizeof(MalType));
    append(symbol_list, "slurp", 5);

    append(fn_ptr_list, NewMalCoreFn(readstring), sizeof(MalType));
    append(symbol_list, "read-string", 11);

    append(fn_ptr_list, NewMalCoreFn(println), sizeof(MalType));
    append(symbol_list, "println", 7);

    append(fn_ptr_list, NewMalCoreFn(str), sizeof(MalType));
    append(symbol_list, "str", 3);

    append(fn_ptr_list, NewMalCoreFn(prn), sizeof(MalType));
    append(symbol_list, "prn", 3);

    append(fn_ptr_list, NewMalCoreFn(prstr), sizeof(MalType));
    append(symbol_list, "pr-str", 6);

    append(fn_ptr_list, NewMalCoreFn(add), sizeof(MalType));
    append(symbol_list, "+", 1);

    append(fn_ptr_list, NewMalCoreFn(sub), sizeof(MalType));
    append(symbol_list, "-", 1);

    append(fn_ptr_list, NewMalCoreFn(mult), sizeof(MalType));
    append(symbol_list, "*", 1);

    append(fn_ptr_list, NewMalCoreFn(divide), sizeof(MalType));
    append(symbol_list, "/", 1);

    append(fn_ptr_list, NewMalCoreFn(prn), sizeof(MalType));
    append(symbol_list, "prn", 3);

    append(fn_ptr_list, NewMalCoreFn(list), sizeof(MalType));
    append(symbol_list, "list", 4);

    append(fn_ptr_list, NewMalCoreFn(list_question_mark), sizeof(MalType));
    append(symbol_list, "list?", 5);

    append(fn_ptr_list, NewMalCoreFn(empty_question_mark), sizeof(MalType));
    append(symbol_list, "empty?", 6);

    append(fn_ptr_list, NewMalCoreFn(count), sizeof(MalType));
    append(symbol_list, "count", 5);

    append(fn_ptr_list, NewMalCoreFn(equal), sizeof(MalType));
    append(symbol_list, "=", 1);

    append(fn_ptr_list, NewMalCoreFn(less), sizeof(MalType));
    append(symbol_list, "<", 1);

    append(fn_ptr_list, NewMalCoreFn(more), sizeof(MalType));
    append(symbol_list, ">", 1);

    append(fn_ptr_list, NewMalCoreFn(equal_less), sizeof(MalType));
    append(symbol_list, "<=", 2);

    append(fn_ptr_list, NewMalCoreFn(equal_more), sizeof(MalType));
    append(symbol_list, ">=", 2);

    while (symbol_list != NULL && fn_ptr_list != NULL) {
        set(env, NewMalSymbol(symbol_list->data), fn_ptr_list->data);

        symbol_list = symbol_list->next;
        fn_ptr_list = fn_ptr_list->next;
    }
    if ((symbol_list == NULL) ^ (fn_ptr_list == NULL)) {
        fprintf(stderr, "symbol_list and function_poiters_list are different "
                        "lenght, some have been discarded\n");
        global_error = NewMalSymbol("create_repl error\n");
    }

    return env;
}
