#include "core.h"
#include "env.h"
#include "gc.h"
#include "hashmap.h"
#include "linked_list.h"
#include "printer.h"
#include "reader.h"
#include "types.h"
#include <stdio.h>
#include <string.h>

MalType *READ(char *line);
MalType *EVAL(MalType *AST, env_t *env);
char    *PRINT(MalType *AST);
char    *rep(char *line, env_t *env);

int main(void) {
    GC_INIT();
    char   *line = NULL;
    size_t  len  = 0;
    ssize_t read;

    env_t *repl_env = create_repl();
    rep("(def! not (fn* (a) (if a false true)))", repl_env);

    printf("user> ");
    while ((read = getline(&line, &len, stdin)) != -1) {
        printf("%s\n", rep(line, repl_env));
        printf("user> ");
    }
}

MalType *READ(char *line) {
    return read_str(line);
}

MalType *EVAL(MalType *AST, env_t *env) {
    if (AST == NULL) {
        return AST;
    }
    switch (AST->type) {
    case MAL_SYMBOL: {
        // NOTE: return the function from the repl env
        MalSymbol *symbol = AST->value.SymbolValue;
        // true
        if (strcmp(symbol, "true") == 0) {
            MalType *true = GC_MALLOC(sizeof(MalType));
            true->type    = MAL_TRUE;
            return true;
        }
        // false
        if (strcmp(symbol, "false") == 0) {
            MalType *false = GC_MALLOC(sizeof(MalType));
            false->type    = MAL_FALSE;
            return false;
        }
        // nil
        if (strcmp(symbol, "nil") == 0) {
            MalType *nil_ret = GC_MALLOC(sizeof(MalType));
            nil_ret->type    = MAL_NIL;
            return nil_ret;
        }

        return get(env, symbol);
    }
    case MAL_LIST: {
        node_t *element = AST->value.ListValue;
        // if the list is empty
        if (element->data == NULL) {
            return AST;
        }

        //////////////////////////////
        // special forms
        if (((MalType *)(element->data))->type == MAL_SYMBOL) {
            MalSymbol *symbol = ((MalType *)(element->data))->value.SymbolValue;
            // def!
            if (strcmp(symbol, "def!") == 0) {
                if (element->next == NULL) {
                    printf("def! shall take two args");
                    return AST;
                }
                if (element->next->next == NULL) {
                    printf("def! shall take two args");
                    return AST;
                }
                MalSymbol *key =
                    ((MalType *)(element->next->data))->value.SymbolValue;
                MalType *value = EVAL(element->next->next->data, env);
                if (value != NULL) {
                    set(env, key, value);
                }
                return value;
            }
            // let*
            if (strcmp(symbol, "let*") == 0) {
                if (element->next == NULL) {
                    printf("let* shall take two args");
                    return AST;
                }
                if (element->next->next == NULL) {
                    printf("let* shall take two args");
                    return AST;
                }

                // create the new_env
                env_t *new_env = GC_MALLOC(sizeof(env_t));
                new_env->outer = env;
                map_t *map     = GC_MALLOC(sizeof(map_t));
                new_env->data  = map;

                // define the binding list
                MalList *bindings_list = NULL;
                if (((MalType *)(element->next->data))->type == MAL_LIST ||
                    ((MalType *)(element->next->data))->type == MAL_VECTOR) {
                    bindings_list =
                        ((MalType *)(element->next->data))->value.ListValue;
                } else {
                    printf("let* takes a list or a vector as arg :/\n");
                    return NULL;
                }

                // evaluate the binding list
                node_t *binding = bindings_list;
                while (binding != NULL) {
                    if (binding->next == NULL) {
                        printf("binding list is odd\n");
                        return AST;
                    }

                    MalSymbol *key =
                        ((MalType *)(binding->data))->value.SymbolValue;
                    MalType *value =
                        EVAL((MalType *)binding->next->data, new_env);
                    if (value != NULL) {
                        set(new_env, key, value);
                    }
                    binding = binding->next->next;
                }

                MalType *third_arg = EVAL(element->next->next->data, new_env);
                return third_arg;
            }
            // do
            if (strcmp(symbol, "do") == 0) {
                if (element->next == NULL) {
                    printf("do shall take at least one arg");
                    return NULL;
                }

                // skip "do" symbol
                element = element->next;

                // evaluate each element
                MalType *evaluated_element;
                while (element != NULL) {
                    evaluated_element = EVAL(element->data, env);
                    element           = element->next;
                };
                return evaluated_element;
            }
            // if
            if (strcmp(symbol, "if") == 0) {
                if (element->next == NULL) {
                    printf("if shall take at least one arg");
                    return NULL;
                }
                MalType *condition = EVAL(element->next->data, env);
                if (condition->type == MAL_FALSE ||
                    condition->type == MAL_NIL || condition == NULL) {
                    // condition is false
                    if (element->next->next->next == NULL) {
                        MalType *nil_ret = GC_MALLOC(sizeof(MalType));
                        nil_ret->type    = MAL_NIL;
                        return nil_ret;
                    }
                    return EVAL(element->next->next->next->data, env);
                } else {
                    // condition was true
                    if (element->next->next == NULL) {
                        printf("if without body\n");
                        return NULL;
                    }
                    return EVAL(element->next->next->data, env);
                }
            }
            // fn*
            if (strcmp(symbol, "fn*") == 0) {
                if (element->next == NULL) {
                    printf("fn shall take at least one arg");
                    return NULL;
                }
                if (((MalType *)element->next->data)->type != MAL_LIST &&
                    ((MalType *)element->next->data)->type != MAL_VECTOR) {
                    printf("fn parameters should be a list or a vector\n");
                    return NULL;
                }
                if (element->next == NULL) {
                    printf("fn dont have a body rn :/ \n");
                    return NULL;
                }

                // create the function
                MalFn   *function = GC_MALLOC(sizeof(MalFn));
                MalList *parameters =
                    ((MalType *)element->next->data)->value.ListValue;
                MalType *body = element->next->next->data;

                // param is a MalList of MalType->MalSymbol.
                // create_env expect a MalList of (char * || MalSymbol *)
                // convert it
                MalList *formated_parameters = GC_MALLOC(sizeof(MalList));
                formated_parameters->data    = NULL;
                formated_parameters->next    = NULL;
                node_t *current_node         = parameters;
                while (current_node != NULL && current_node->data != NULL) {
                    if (((MalType *)current_node->data)->type != MAL_SYMBOL) {
                        printf("args must be symbols\n");
                        return NULL;
                    }
                    append(formated_parameters,
                           ((MalType *)current_node->data)->value.SymbolValue,
                           sizeof(MalTypeValue));
                    current_node = current_node->next;
                }

                function->param = formated_parameters;
                function->body  = body;
                function->env   = env;

                // wrap function in MalType
                MalType *mal_type_function       = GC_MALLOC(sizeof(MalType));
                mal_type_function->type          = MAL_FN;
                mal_type_function->value.FnValue = function;

                return mal_type_function;
            }
        }
        ///////////////////////////////////

        // eval first element
        MalType *first_element = element->data;
        if (first_element == NULL) {
            printf("first element is NULL\n");
            return AST;
        }
        MalType *evaluated_first_element = EVAL(first_element, env);
        if (evaluated_first_element == NULL) {
            return AST;
        }

        //////////////////////////////////
        // functions call
        if (evaluated_first_element->type == MAL_FN) {
            MalFn *function = evaluated_first_element->value.FnValue;

            MalList *arg_list = GC_MALLOC(sizeof(MalList));
            arg_list->data    = NULL;
            arg_list->next    = NULL;

            // eval args in current env
            // and then pass them to the function env
            node_t *node = element->next;
            while (node != NULL) {
                MalType *arg = node->data;
                append(arg_list, EVAL(arg, env), sizeof(MalType));
                node = node->next;
            }

            MalType *expr_mal_type         = GC_MALLOC(sizeof(MalType));
            expr_mal_type->type            = MAL_LIST;
            expr_mal_type->value.ListValue = arg_list;

            // check for Clojure-style variadic function parameters
            node_t *binds = function->param;
            node_t *exprs = arg_list;

            while (binds != NULL && exprs != NULL) {
                if (binds->data == NULL) {
                    break;
                }
                if (strcmp((char *)binds->data, "&") == 0) {
                    // skip the & in the binds list
                    binds->data = binds->next->data;
                    binds->next = NULL;

                    // make the next exprs a list (wraped in a maltype) that
                    // contain the rest of the exprs
                    // define it
                    MalType *wrap         = GC_MALLOC(sizeof(MalType));
                    wrap->type            = MAL_LIST;
                    wrap->value.ListValue = GC_MALLOC(sizeof(node_t));
                    memcpy(wrap->value.ListValue, exprs, sizeof(node_t));
                    // assigne the list
                    exprs->data = wrap;
                    exprs->next = NULL;
                    printf("break\n");
                    break;
                }
                // catch exprs empty
                // broke regular functions
                if (exprs->next == NULL && binds->next != NULL) {
                    printf("expr is NULL\n");
                    if (strcmp((char *)binds->next->data, "&") != 0) {
                        printf("not enough args passed to the function\n");
                        return AST;
                    }
                    // skip the & in the binds list
                    binds       = binds->next;
                    binds->data = binds->next->data;

                    // if there is no exprs, still wrap a empty list (node_t *)
                    // in a MalType->type == MAL_LIST
                    MalType *wrap         = GC_MALLOC(sizeof(MalType));
                    wrap->type            = MAL_LIST;
                    wrap->value.ListValue = GC_MALLOC(sizeof(node_t));
                    // create a empty list
                    node_t *list = GC_MALLOC(sizeof(node_t));
                    list->data   = NULL;
                    list->next   = NULL;

                    // put empty list in the wrapper
                    memcpy(wrap->value.ListValue, list, sizeof(node_t));

                    // put the wrapper in the exprs list
                    exprs->next       = GC_MALLOC(sizeof(node_t));
                    exprs->next->data = wrap;
                    break;
                }

                binds = binds->next;
                exprs = exprs->next;
            }

            // create the env for evaluation
            env_t *new_env =
                create_env(function->env, (MalList *)function->param, arg_list);
            // DEBUG:
            printf("created a new function with this env : %s\n",
                   pr_env(new_env));
            return EVAL(function->body, new_env);
        }
        //////////////////////////////////

        //////////////////////////////////
        // core finctions
        // if the first list element is a core_fn, call it.
        if (evaluated_first_element->type == MAL_CORE_FN) {
            MalType *core_fn = evaluated_first_element;
            if (core_fn->value.CoreFnValue == NULL) {
                fprintf(stderr, "core_fn is null\n");
                return AST;
            }

            // skip symbol
            // this can make element NULL
            element = element->next;

            // add the rest of the list to a list of evaluated things

            // define a list of evaluated element
            node_t *evaluated_list = GC_MALLOC(sizeof(node_t));
            evaluated_list->data   = NULL;
            evaluated_list->next   = NULL;

            // evaluate each element in the current list and add them
            while (element != NULL) {
                MalType *evaluated_element = EVAL(element->data, env);
                if (evaluated_element != NULL) {
                    append(evaluated_list, (void *)evaluated_element,
                           sizeof(MalType));
                }
                element = element->next;
            };

            MalType *result =
                (*(MalCoreFn)core_fn->value.CoreFnValue)(evaluated_list);
            return result;
        }

        // list have nothing special, eval everything and return.
        // define a list of evaluated element
        node_t *evaluated_list = GC_MALLOC(sizeof(node_t));
        evaluated_list->data   = NULL;
        evaluated_list->next   = NULL;

        // evaluate each element in the current list and add them
        while (element != NULL) {
            MalType *evaluated_element = EVAL(element->data, env);
            if (evaluated_element != NULL) {
                append(evaluated_list, (void *)evaluated_element,
                       sizeof(MalType));
            }
            element = element->next;
        };

        // wrap evaluated list in a MalType
        MalType *wrapper         = GC_MALLOC(sizeof(MalType));
        wrapper->type            = MAL_LIST;
        wrapper->value.ListValue = evaluated_list;
        return wrapper;
    }
    case MAL_VECTOR: {
        // NOTE: MAL_VECTOR is a copy/paste of MAL_LIST
        node_t *element = AST->value.ListValue;
        // if the vector is empty
        if (element->data == NULL) {
            return AST;
        }

        //////////////////////////////
        // special forms
        if (((MalType *)(element->data))->type == MAL_SYMBOL) {
            MalSymbol *symbol = ((MalType *)(element->data))->value.SymbolValue;
            // def!
            if (strcmp(symbol, "def!") == 0) {
                if (element->next == NULL) {
                    printf("def! shall take two args");
                    return AST;
                }
                if (element->next->next == NULL) {
                    printf("def! shall take two args");
                    return AST;
                }
                MalSymbol *key =
                    ((MalType *)(element->next->data))->value.SymbolValue;
                MalType *value = EVAL(element->next->next->data, env);
                if (value != NULL) {
                    set(env, key, value);
                }
                return value;
            }
            // let*
            if (strcmp(symbol, "let*") == 0) {
                if (element->next == NULL) {
                    printf("let* shall take two args");
                    return AST;
                }
                if (element->next->next == NULL) {
                    printf("let* shall take two args");
                    return AST;
                }

                // create the new_env
                env_t *new_env = GC_MALLOC(sizeof(env_t));
                new_env->outer = env;
                map_t *map     = GC_MALLOC(sizeof(map_t));
                new_env->data  = map;

                // define the binding vector
                MalList *bindings_vector = NULL;
                if (((MalType *)(element->next->data))->type == MAL_VECTOR) {
                    bindings_vector =
                        ((MalType *)(element->next->data))->value.ListValue;
                } else {
                    printf("let* takes a vector as arg :/\n");
                    return NULL;
                }

                // evaluate the binding vector
                node_t *binding = bindings_vector;
                while (binding != NULL) {
                    if (binding->next == NULL) {
                        printf("binding vector is odd\n");
                        return AST;
                    }

                    MalSymbol *key =
                        ((MalType *)(binding->data))->value.SymbolValue;
                    MalType *value =
                        EVAL((MalType *)binding->next->data, new_env);
                    if (value != NULL) {
                        set(new_env, key, value);
                    }
                    binding = binding->next->next;
                }

                MalType *third_arg = EVAL(element->next->next->data, new_env);
                return third_arg;
            }
            // do
            if (strcmp(symbol, "do") == 0) {
                if (element->next == NULL) {
                    printf("do shall take at least one arg");
                    return NULL;
                }

                // skip "do" symbol
                element = element->next;

                // evaluate each element
                MalType *evaluated_element;
                while (element != NULL) {
                    evaluated_element = EVAL(element->data, env);
                    element           = element->next;
                };
                return evaluated_element;
            }
            // if
            if (strcmp(symbol, "if") == 0) {
                if (element->next == NULL) {
                    printf("if shall take at least one arg");
                    return NULL;
                }
                MalType *condition = EVAL(element->next->data, env);
                if (condition->type == MAL_FALSE ||
                    condition->type == MAL_NIL || condition == NULL) {
                    // condition is false
                    if (element->next->next->next == NULL) {
                        MalType *nil_ret = GC_MALLOC(sizeof(MalType));
                        nil_ret->type    = MAL_NIL;
                        return nil_ret;
                    }
                    return EVAL(element->next->next->next->data, env);
                } else {
                    // condition was true
                    if (element->next->next == NULL) {
                        printf("if without body\n");
                        return NULL;
                    }
                    return EVAL(element->next->next->data, env);
                }
            }
            // fn*
            if (strcmp(symbol, "fn*") == 0) {
                if (element->next == NULL) {
                    printf("fn shall take at least one arg");
                    return NULL;
                }
                if (((MalType *)element->next->data)->type != MAL_VECTOR) {
                    printf("fn parameters should be a vector\n");
                    return NULL;
                }
                if (element->next == NULL) {
                    printf("fn dont have a body rn :/ \n");
                    return NULL;
                }

                // create the function
                MalFn   *function = GC_MALLOC(sizeof(MalFn));
                MalList *parameters =
                    ((MalType *)element->next->data)->value.ListValue;
                MalType *body = element->next->next->data;

                // param is a MalVector of MalType->MalSymbol.
                // create_env expect a MalVector of (char * || MalSymbol *)
                // convert it
                MalList *formated_parameters = GC_MALLOC(sizeof(MalList));
                formated_parameters->data    = NULL;
                formated_parameters->next    = NULL;
                node_t *current_node         = parameters;
                while (current_node != NULL && current_node->data != NULL) {
                    if (((MalType *)current_node->data)->type != MAL_SYMBOL) {
                        printf("args must be symbols\n");
                        return NULL;
                    }
                    append(formated_parameters,
                           ((MalType *)current_node->data)->value.SymbolValue,
                           sizeof(MalTypeValue));
                    current_node = current_node->next;
                }

                function->param = formated_parameters;
                function->body  = body;
                function->env   = env;

                // wrap function in MalType
                MalType *mal_type_function       = GC_MALLOC(sizeof(MalType));
                mal_type_function->type          = MAL_FN;
                mal_type_function->value.FnValue = function;

                return mal_type_function;
            }
        }
        ///////////////////////////////////

        // eval first element
        MalType *first_element = element->data;
        if (first_element == NULL) {
            printf("first element is NULL\n");
            return AST;
        }
        MalType *evaluated_first_element = EVAL(first_element, env);
        if (evaluated_first_element == NULL) {
            return AST;
        }

        //////////////////////////////////
        // functions call
        if (evaluated_first_element->type == MAL_FN) {
            MalFn *function = evaluated_first_element->value.FnValue;

            MalList *arg_vector = GC_MALLOC(sizeof(MalList));
            arg_vector->data    = NULL;
            arg_vector->next    = NULL;
            // normalize arg into a MalVector

            node_t *node = element->next;
            while (node != NULL) {
                MalType *arg = node->data;
                if (arg->type == MAL_VECTOR) {
                    arg = EVAL(arg, env);
                }
                append(arg_vector, arg, sizeof(MalType));
                node = node->next;
            }

            // wrap the MalList * arg vector into a MalType
            MalType *mal_type_arg_vector         = GC_MALLOC(sizeof(MalType));
            mal_type_arg_vector->type            = MAL_VECTOR;
            mal_type_arg_vector->value.ListValue = arg_vector;

            // create the env for evaluation
            env_t *new_env = create_env(function->env,
                                        (MalList *)function->param, arg_vector);
            // DEBUG:
            // printf("created a new function with this env : %s\n",
            // pr_env(new_env));
            return EVAL(function->body, new_env);
        }
        //////////////////////////////////

        //////////////////////////////////
        // core finctions
        // if the first vector element is a core_fn, call it.
        if (evaluated_first_element->type == MAL_CORE_FN) {
            MalType *core_fn = evaluated_first_element;
            if (core_fn->value.CoreFnValue == NULL) {
                fprintf(stderr, "core_fn is null\n");
                return AST;
            }

            // skip symbol
            // this can make element NULL
            element = element->next;

            // add the rest of the vector to a vector of evaluated things

            // define a vector of evaluated element
            node_t *evaluated_vector = GC_MALLOC(sizeof(node_t));
            evaluated_vector->data   = NULL;
            evaluated_vector->next   = NULL;

            // evaluate each element in the current vector and add them
            while (element != NULL) {
                MalType *evaluated_element = EVAL(element->data, env);
                if (evaluated_element != NULL) {
                    append(evaluated_vector, (void *)evaluated_element,
                           sizeof(MalType));
                }
                element = element->next;
            };

            MalType *result =
                (*(MalCoreFn)core_fn->value.CoreFnValue)(evaluated_vector);
            return result;
        }

        // vector have nothing special, eval everything and return.
        // define a vector of evaluated element
        node_t *evaluated_vector = GC_MALLOC(sizeof(node_t));
        evaluated_vector->data   = NULL;
        evaluated_vector->next   = NULL;

        // evaluate each element in the current vector and add them
        while (element != NULL) {
            MalType *evaluated_element = EVAL(element->data, env);
            if (evaluated_element != NULL) {
                append(evaluated_vector, (void *)evaluated_element,
                       sizeof(MalType));
            }
            element = element->next;
        };

        // wrap evaluated vector in a MalType
        MalType *wrapper         = GC_MALLOC(sizeof(MalType));
        wrapper->type            = MAL_VECTOR;
        wrapper->value.ListValue = evaluated_vector;
        return wrapper;
    }
    case MAL_STRING: {
        return AST;
    }
    case MAL_INT: {
        return AST;
    }
    case MAL_TRUE: {
        return AST;
    }
    case MAL_FALSE: {
        return AST;
    }
    case MAL_NIL: {
        return AST;
    }
    case MAL_CORE_FN: {
        return AST;
    }
    case MAL_KEYWORD: {
        return AST;
    }
    case MAL_FN: {
        return AST;
    }
    }

    fprintf(stderr, "unmatched AST\n");
    return NULL;
}

MalType *EVAL_symbol(MalType *AST, env_t *env) {
    // NOTE: return the function from the repl env
    MalSymbol *symbol = AST->value.SymbolValue;
    return get(env, symbol);
}

char *PRINT(MalType *AST) {
    return pr_str(AST, 1);
}

char *rep(char *line, env_t *env) {
    return PRINT(EVAL(READ(line), env));
}
