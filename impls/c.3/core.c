#include "core.h"
#include "EVAL.h"
#include "env.h"
#include "gc.h"
#include "hashmap.h"
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

    if (node == NULL) {
        fprintf(stderr, "at least two number pls\n");
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
        return 0;
    } else if (node->next == NULL) {
        printf("divide must take two arg (not 1)");
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

    return NewMalList(node);
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
        return ret;
    }
    if (node->next == NULL) {
        printf("cons without list");
        return ret;
    }
    if (node->next->data == NULL) {
        // allocate a new_list, this one is empty
        printf("node->next->data is NULL");
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

    if (node == NULL) {
        printf("concat need args\n");
        return ret;
    }
    if (node->data == NULL) {
        printf("concat arg1 is NULL\n");
        return ret;
    }

    // node is a list of lists, a list of args to concat
    while (node != NULL && node->data != NULL) {
        MalType *list      = node->data;
        MalList *list_node = GetList(list);
        if (!IsListOrVector(list)) {
            printf("concat arg in not list nor vector\n");
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
MalType *vec(node_t *node) {
    if (node->data == NULL) {
        // list is empty, allocate a new one
        node_t *new_list = GC_MALLOC(sizeof(node_t));
        new_list->next   = NULL;
        new_list->data   = NULL;
        node             = new_list;
    }

    MalType *arg = node->data;
    if (!IsListOrVector(arg)) {
        printf("vec take a list of vector as arg\n");
        return NewMalNIL();
    }

    return arg;
}
MalType *nth(node_t *node) {
    MalType *nil = NewMalNIL();

    if (node == NULL) {
        printf("nth need args");
        return nil;
    }
    if (node->data == NULL) {
        printf("nth args1 is empty list");
        return nil;
    }
    if (node->next == NULL) {
        printf("nth without indice");
        return nil;
    }
    if (node->next->data == NULL) {
        printf("nth node->next->data is NULL");
        return nil;
    }

    // arg 1 list
    MalType *arg1 = node->data;
    if (!IsListOrVector(arg1)) {
        printf("nth arg1 must be a list or a vector\n");
        return nil;
    }

    // second arg index
    MalType *arg2 = node->next->data;
    if (!IsInt(arg2)) {
        printf("nth arg2 must be a int\n");
        return nil;
    }

    MalList *list         = GetList(arg1);
    int      target_index = *GetInt(arg2);
    if (target_index < 0) {
        printf("index < 0\n");
        return nil;
    }
    for (int i = 0; i < target_index; i++) {

        list = list->next;
        if (is_empty(list)) {
            printf("nth index out of range\n");
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
        return NewMalNIL();
    }
    if (!IsListOrVector(arg)) {
        printf("rest arg must be a list or a vecotr\n");
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

MalType *equal(node_t *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();
    MalType *nil   = NewMalNIL();

    if (node == NULL) {
        printf("= with no arg \n");
        return nil;
    }
    if (node->data == NULL) {
        printf("= arg1 is NULL \n");
        return nil;
    }
    if (node->next == NULL) {
        printf("= with only one arg\n");
        return nil;
    }
    if (node->next->data == NULL) {
        printf("= arg2 is NULL \n");
        return nil;
    }

    MalType *arg1 = GC_MALLOC(sizeof(MalType));
    MalType *arg2 = GC_MALLOC(sizeof(MalType));
    arg1          = node->data;
    arg2          = node->next->data;

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
    case MAL_VECTOR:
        // fallback to list equality
    case MAL_LIST: {
        node_t *node1 = GetList(arg1);
        node_t *node2 = GetList(arg2);
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
        }
        if (!IsListOrVector(fn2->param)) {
            printf("equal : fn2->param is not a list\n");
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
    return false;
}

MalType *macro_question_mark(node_t *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();

    if (is_empty(node)) {
        return false;
    }
    MalType *arg = node->data;
    printf("macro? ast: %s\n", pr_str(arg, 0));

    if (IsFnWrapper(arg)) {
        if (GetFnWrapper(arg)->is_macro) {
            return true;
        }
    }
    return false;
}

// comparators
MalType *less(node_t *node) {
    MalType *false = NewMalFalse();
    MalType *true  = NewMalTrue();
    MalType *nil   = NewMalNIL();

    if (node == NULL) {
        printf("< with no arg \n");
        return nil;
    }
    if (node->data == NULL) {
        printf("< arg1 is NULL \n");
        return nil;
    }
    if (node->next == NULL) {
        printf("< with only one arg\n");
        return nil;
    }
    if (node->next->data == NULL) {
        printf("< arg2 is NULL \n");
        return nil;
    }

    // maybe i dont need to allocate new memory here ?
    MalType *arg1 = GC_MALLOC(sizeof(MalType));
    MalType *arg2 = GC_MALLOC(sizeof(MalType));
    arg1          = node->data;
    arg2          = node->next->data;

    if (!IsInt(arg1) || !IsInt(arg2)) {
        printf("< with non int parameters\n");
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
        return nil;
    }
    if (node->data == NULL) {
        printf("> arg1 is NULL \n");
        return nil;
    }
    if (node->next == NULL) {
        printf("> with only one arg\n");
        return nil;
    }
    if (node->next->data == NULL) {
        printf("> arg2 is NULL \n");
        return nil;
    }

    // maybe i dont need to allocate new memory here ?
    MalType *arg1 = GC_MALLOC(sizeof(MalType));
    MalType *arg2 = GC_MALLOC(sizeof(MalType));
    arg1          = node->data;
    arg2          = node->next->data;

    if (!IsInt(arg1) || !IsInt(arg2)) {
        printf("> with non int parameters\n");
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
        return nil;
    }
    if (node->data == NULL) {
        printf("<= arg1 is NULL \n");
        return nil;
    }
    if (node->next == NULL) {
        printf("<= with only one arg\n");
        return nil;
    }
    if (node->next->data == NULL) {
        printf("<= arg2 is NULL \n");
        return nil;
    }

    // maybe i dont need to allocate new memory here ?
    MalType *arg1 = GC_MALLOC(sizeof(MalType));
    MalType *arg2 = GC_MALLOC(sizeof(MalType));
    arg1          = node->data;
    arg2          = node->next->data;

    if (!IsInt(arg1) || !IsInt(arg2)) {
        printf("<= with non int parameters\n");
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
        return nil;
    }
    if (node->data == NULL) {
        printf(">= arg1 is NULL \n");
        return nil;
    }
    if (node->next == NULL) {
        printf(">= with only one arg\n");
        return nil;
    }
    if (node->next->data == NULL) {
        printf(">= arg2 is NULL \n");
        return nil;
    }

    // maybe i dont need to allocate new memory here ?
    MalType *arg1 = GC_MALLOC(sizeof(MalType));
    MalType *arg2 = GC_MALLOC(sizeof(MalType));
    arg1          = node->data;
    arg2          = node->next->data;

    if (!IsInt(arg1) || !IsInt(arg2)) {
        printf(">= with non int parameters\n");
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
        return node->data;
    }
    return GetAtom(atom);
}
MalType *reset(node_t *node) {
    MalType *atom = node->data;
    if (!IsAtom(atom)) {
        printf("reset arg1 is not an atom\n");
        return node->data;
    }
    if (node->next == NULL) {
        printf("reset take two arg\n");
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
        return node->data;
    }
    if (node->next == NULL) {
        printf("swap take two arg\n");
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
        // TODO: creating a new repl is not very optimal
        ret = EVAL(NewMalList(list), create_repl());
    } else {
        ret = EVAL(NewMalList(list), GetFnWrapper(fn)->env);
    }

    // reset
    memcpy(atom->value.AtomValue, ret, sizeof(MalType));

    return GetAtom(atom);
}

env_t *create_repl() {

    env_t *env = create_env(NULL, NULL, NULL);

    node_t *function_pointers_list = GC_MALLOC(sizeof(node_t));
    function_pointers_list->next   = NULL;
    function_pointers_list->data   = NULL;

    node_t *symbol_list = GC_MALLOC(sizeof(node_t));
    symbol_list->next   = NULL;
    symbol_list->data   = NULL;

    // yes, hard coded.
    append(function_pointers_list, NewMalCoreFunction(macro_question_mark),
           sizeof(MalType));
    append(symbol_list, "macro?", 6);

    append(function_pointers_list, NewMalCoreFunction(rest), sizeof(MalType));
    append(symbol_list, "rest", 4);

    append(function_pointers_list, NewMalCoreFunction(first), sizeof(MalType));
    append(symbol_list, "first", 5);

    append(function_pointers_list, NewMalCoreFunction(nth), sizeof(MalType));
    append(symbol_list, "nth", 3);

    append(function_pointers_list, NewMalCoreFunction(vec), sizeof(MalType));
    append(symbol_list, "vec", 3);

    append(function_pointers_list, NewMalCoreFunction(concat), sizeof(MalType));
    append(symbol_list, "concat", 6);

    append(function_pointers_list, NewMalCoreFunction(cons), sizeof(MalType));
    append(symbol_list, "cons", 4);

    append(function_pointers_list, NewMalCoreFunction(swap), sizeof(MalType));
    append(symbol_list, "swap!", 5);

    append(function_pointers_list, NewMalCoreFunction(reset), sizeof(MalType));
    append(symbol_list, "reset!", 6);

    append(function_pointers_list, NewMalCoreFunction(deref), sizeof(MalType));
    append(symbol_list, "deref", 5);

    append(function_pointers_list, NewMalCoreFunction(atom_question_mark),
           sizeof(MalType));
    append(symbol_list, "atom?", 5);

    append(function_pointers_list, NewMalCoreFunction(atom), sizeof(MalType));
    append(symbol_list, "atom", 4);

    append(function_pointers_list, NewMalCoreFunction(slurp), sizeof(MalType));
    append(symbol_list, "slurp", 5);

    append(function_pointers_list, NewMalCoreFunction(readstring),
           sizeof(MalType));
    append(symbol_list, "read-string", 11);

    append(function_pointers_list, NewMalCoreFunction(println),
           sizeof(MalType));
    append(symbol_list, "println", 7);

    append(function_pointers_list, NewMalCoreFunction(str), sizeof(MalType));
    append(symbol_list, "str", 3);

    append(function_pointers_list, NewMalCoreFunction(prn), sizeof(MalType));
    append(symbol_list, "prn", 3);

    append(function_pointers_list, NewMalCoreFunction(prstr), sizeof(MalType));
    append(symbol_list, "pr-str", 6);

    append(function_pointers_list, NewMalCoreFunction(add), sizeof(MalType));
    append(symbol_list, "+", 1);

    append(function_pointers_list, NewMalCoreFunction(sub), sizeof(MalType));
    append(symbol_list, "-", 1);

    append(function_pointers_list, NewMalCoreFunction(mult), sizeof(MalType));
    append(symbol_list, "*", 1);

    append(function_pointers_list, NewMalCoreFunction(divide), sizeof(MalType));
    append(symbol_list, "/", 1);

    append(function_pointers_list, NewMalCoreFunction(prn), sizeof(MalType));
    append(symbol_list, "prn", 3);

    append(function_pointers_list, NewMalCoreFunction(list), sizeof(MalType));
    append(symbol_list, "list", 4);

    append(function_pointers_list, NewMalCoreFunction(list_question_mark),
           sizeof(MalType));
    append(symbol_list, "list?", 5);

    append(function_pointers_list, NewMalCoreFunction(empty_question_mark),
           sizeof(MalType));
    append(symbol_list, "empty?", 6);

    append(function_pointers_list, NewMalCoreFunction(count), sizeof(MalType));
    append(symbol_list, "count", 5);

    append(function_pointers_list, NewMalCoreFunction(equal), sizeof(MalType));
    append(symbol_list, "=", 1);

    append(function_pointers_list, NewMalCoreFunction(less), sizeof(MalType));
    append(symbol_list, "<", 1);

    append(function_pointers_list, NewMalCoreFunction(more), sizeof(MalType));
    append(symbol_list, ">", 1);

    append(function_pointers_list, NewMalCoreFunction(equal_less),
           sizeof(MalType));
    append(symbol_list, "<=", 2);

    append(function_pointers_list, NewMalCoreFunction(equal_more),
           sizeof(MalType));
    append(symbol_list, ">=", 2);

    while (symbol_list != NULL && function_pointers_list != NULL) {
        set(env, symbol_list->data, function_pointers_list->data);

        symbol_list            = symbol_list->next;
        function_pointers_list = function_pointers_list->next;
    }
    if ((symbol_list == NULL) ^ (function_pointers_list == NULL)) {
        fprintf(stderr, "symbol_list and function_poiters_list are different "
                        "lenght, some have been discarded\n");
    }

    return env;
}
