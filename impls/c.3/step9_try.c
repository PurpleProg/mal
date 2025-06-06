#include "core.h"
#include "env.h"
#include "gc.h"
#include "linked_list.h"
#include "printer.h"
#include "reader.h"
#include "types.h"
#include <gc/gc.h>
#include <stdio.h>
#include <string.h>

MalType *READ(char *line);
MalType *EVAL(MalType *AST, env_t *env);
char    *PRINT(MalType *AST);
char    *rep(char *line, env_t *env);

env_t *repl_env;
// define the global error declared in type.h
// (so that it's global cause type.h is inclued everywere)
MalType *global_error = NULL;

MalType *eval(node_t *node) {
    MalType *ast = node->data;
    return EVAL(ast, repl_env);
}

int main(int argc, char *argv[]) {
    GC_INIT();
    char   *line = NULL;
    size_t  len  = 0;
    ssize_t read;

    repl_env     = create_repl();
    global_error = NULL;

    // add eval to the repl
    set(repl_env, NewMalString("eval"), NewMalCoreFn(eval));

    // add DEBUG-EVAL
    set(repl_env, NewMalSymbol("DEBUG-EVAL"), NewMalFalse());

    rep("(def! not (fn* (condition) (if condition false true)))", repl_env);
    rep("(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp "
        "f) \"\\nnil)\")) ) ))",
        repl_env);
    rep("(defmacro! cond "
        "  (fn* (& xs) "
        "    (" // func body
        "      if (> (count xs) 0) "
        "      (list "
        "        'if "                 // if
        "        (first xs) "          // condition
        "        (if (> (count xs) 1)" // true
        "          (nth xs 1)"
        "          (throw \"odd number of forms to cond\")"
        "        )"
        "        (cons 'cond (rest (rest xs)))" // false
        "      )"
        "    )" // func body
        "  )"
        ")",
        repl_env);

    // create *ARGV*
    node_t *args_list = GC_MALLOC(sizeof(node_t));
    for (int i = 2; i < argc; i++) {
        append(args_list, NewMalString(argv[i]), sizeof(MalType));
    }
    set(repl_env, NewMalString("*ARGV*"), NewMalList(args_list));

    // check if the interpretor is called with args
    if (argc > 1) {
        // call load-file on the first arg
        char *string = GC_MALLOC(strlen(argv[1]) + strlen("(load-file \"\")"));
        sprintf(string, "(load-file \"%s\")", argv[1]);
        rep(string, repl_env);
        return 0;
    }

    printf("user> ");
    while ((read = getline(&line, &len, stdin)) != -1) {
        printf("%s\n", rep(line, repl_env));
        printf("user> ");
    }
    return 1;
}

MalType *quasiquote(MalType *AST, env_t *env);
MalType *qq_iterative(MalList *args, env_t *env) {
    // AST is a list that DONT start with unquote

    if (args->data == NULL) {
        node_t *list_result = GC_MALLOC(sizeof(node_t));
        return NewMalList(list_result);
    }

    node_t *list_result = GC_MALLOC(sizeof(node_t));

    // iterate over reversed list
    node_t *reversed_list = GC_MALLOC(sizeof(node_t));
    reversed_list         = NULL;
    reverse_list(args, &reversed_list);
    node_t *node = reversed_list;

    while (node != NULL) {
        // NOTE: elt can be NULL, just an empty list
        MalType *elt = node->data;
        // if elt is a list starting with "split-unquote"
        if (IsListOrVector(elt)) {
            MalList *list = GetList(elt);
            if (!is_empty(list)) {
                MalType *elt_first_element = list->data;
                if (IsSymbol(elt_first_element)) {
                    if (strcmp(GetSymbol(elt_first_element),
                               "splice-unquote") == 0) {
                        // replace the current result with a list containing:
                        // the "concat" symbol,
                        // the second element of elt,

                        node_t *new_list_result = GC_MALLOC(sizeof(node_t));

                        append(new_list_result, NewMalSymbol("concat"),
                               sizeof(MalType));
                        append(new_list_result, list->next->data,
                               sizeof(MalType));
                        append(new_list_result, NewMalList(list_result),
                               sizeof(MalType));

                        // replace current result with new_result
                        list_result = new_list_result;

                        node = node->next;
                        continue;
                    } // if (first element == "splice-unquote")
                } // if (first_element->type == MAL_SYMBOL)
            } // if !is_empty list

        } // if (elt->type == MAL_LIST or vec)

        // Else replace the current result with a list containing:
        // the "cons" symbol,
        // the result of calling quasiquote with elt as argument,
        // then the previous result.

        // new list result
        node_t *new_list_result = GC_MALLOC(sizeof(node_t));

        append(new_list_result, NewMalSymbol("cons"), sizeof(MalType));
        append(new_list_result, quasiquote(elt, env), sizeof(MalType));
        append(new_list_result, NewMalList(list_result), sizeof(MalType));

        // replace current result with new_result
        list_result = new_list_result;

        node = node->next;
    } // iterate over elt in reverse order

    return NewMalList(list_result);
}
MalType *quasiquote(MalType *AST, env_t *env) {
    if (IsList(AST)) {
        MalList *list = GetList(AST);

        if (is_empty(list)) {
            return AST;
        }

        MalType *first_element = list->data;

        // if AST is a list starting with the unquote symbol
        if (IsSymbol(first_element)) {
            if (strcmp(GetSymbol(first_element), "unquote") == 0) {
                if (list->next == NULL) {
                    printf("unquoting nothing :/\n");
                    global_error = AST;
                    return NewMalNIL();
                }
                // return the second element
                return list->next->data;
            }
        }

        return qq_iterative(list, env);
    } else if (IsVector(AST)) {

        MalList *new_list = GC_MALLOC(sizeof(MalList));
        append(new_list, NewMalSymbol("vec"), sizeof(MalType));
        append(new_list, qq_iterative(GetVector(AST), env), sizeof(MalType));

        return NewMalList(new_list);
    } else if (IsSymbol(AST) || IsHashmap(AST)) {
        // If ast is a map or a symbol,
        // return a list containing:
        // the "quote" symbol,
        // then ast.

        node_t *list = GC_MALLOC(sizeof(node_t));

        append(list, NewMalSymbol("quote"), sizeof(MalType));
        append(list, AST, sizeof(MalType));

        return NewMalList(list);
    }
    // Else return ast unchanged.
    return AST;
}

MalType *READ(char *line) {
    return read_str(line);
}

MalType *EVAL(MalType *AST, env_t *env) {
    while (1) {
        // reset
        global_error = NULL;

        MalType *do_contain_debug_eval = get(env, NewMalSymbol("DEBUG-EVAL"));
        if (do_contain_debug_eval != NULL) {
            if (!IsFalse(do_contain_debug_eval) &&
                !IsNil(do_contain_debug_eval)) {
                printf("EVAL: %s\n", pr_str(AST, 1));
                // printf("ENV : %s\n", pr_env(env));
            }
        }
        if (AST == NULL) {
            return AST;
        }
        int eval_vector = 0;
        switch (AST->type) {
        case MAL_SYMBOL: {
            // TODO: maybe just return get(env, AST)
            // NOTE: get symbol from env
            // and eval (true, false, nil)
            char *symbol = GetSymbol(AST);

            MalType *ret = get(env, NewMalString(symbol));
            if (ret == NULL) {
                return global_error;
            }
            return ret;
        }
        case MAL_VECTOR: {
            eval_vector = 1;
            // fallthroug to LIST
        }
        case MAL_LIST: {
            node_t *element = GetList(AST);
            // if the list is empty
            if (element->data == NULL) {
                return AST;
            }

            MalType *first_element = element->data;
            if (first_element == NULL) {
                printf("first element is NULL\n");
                global_error = AST;
                return AST;
            }

            // special forms
            if (IsSymbol(first_element)) {
                char *symbol = GetSymbol(first_element);
                // def!
                if (strcmp(symbol, "def!") == 0) {
                    if (element->next == NULL) {
                        printf("def! shall take two args\n");
                        global_error = AST;
                        return AST;
                    }
                    if (element->next->next == NULL) {
                        printf("def! shall take two args\n");
                        global_error = AST;
                        return AST;
                    }
                    MalType *key   = element->next->data;
                    MalType *value = EVAL(element->next->next->data, env);

                    if (global_error != NULL) {
                        printf("def! value error\n");
                        return AST;
                    }

                    if (value != NULL) {
                        set(env, key, value);
                    }
                    return value;
                }
                // let*
                if (strcmp(symbol, "let*") == 0) {
                    if (element->next == NULL) {
                        printf("let* shall take two args");
                        global_error = AST;
                        return AST;
                    }
                    if (element->next->next == NULL) {
                        printf("let* shall take two args");
                        global_error = AST;
                        return AST;
                    }

                    // create the new_env
                    map_t *map     = GC_MALLOC(sizeof(map_t));
                    env_t *new_env = GC_MALLOC(sizeof(env_t));
                    new_env->outer = env;
                    new_env->data  = map;

                    // define the binding list AST[1]
                    MalList *bindings_list = NULL;
                    if (IsListOrVector(element->next->data)) {
                        bindings_list = GetList(element->next->data);
                    } else {
                        printf("let* takes a list or a vector as arg :/\n");
                        global_error = AST;
                        return AST;
                    }

                    // evaluate the binding list
                    node_t *binding = bindings_list;
                    while (binding != NULL) {
                        if (binding->next == NULL) {
                            printf("binding list is odd\n");
                            global_error = AST;
                            return AST;
                        }

                        MalType *key   = binding->data;
                        MalType *value = EVAL(binding->next->data, new_env);
                        if (global_error != NULL) {
                            printf("let* value error\n");
                            return value;
                        }
                        if (value != NULL) {
                            set(new_env, key, value);
                        }
                        binding = binding->next->next;
                    }

                    // TCO
                    env = new_env;
                    AST = element->next->next->data; // AST[2]
                    continue;
                }
                // do
                if (strcmp(symbol, "do") == 0) {
                    if (element->next == NULL) {
                        printf("do shall take at least one arg");
                        global_error = AST;
                        return AST;
                    }

                    // skip "do" symbol
                    element = element->next;

                    // evaluate each element
                    while (element->next != NULL) {
                        EVAL(element->data, env);
                        // if (global_error != NULL) {
                        //     printf("do eval element failed\n");
                        //     return AST;
                        // }
                        element = element->next;
                    };

                    AST = element->data;
                    continue;
                }
                // if
                if (strcmp(symbol, "if") == 0) {
                    if (element->next == NULL) {
                        printf("if shall take at least one arg");
                        global_error = AST;
                        return NewMalNIL();
                    }
                    MalType *condition = EVAL(element->next->data, env);

                    if (global_error != NULL) {
                        printf("if condition must return a boolean value\n");
                        return condition;
                    }

                    if (IsFalse(condition) || IsNil(condition)) {
                        // condition is false
                        if (element->next->next->next == NULL) {
                            return NewMalNIL();
                        }
                        AST = element->next->next->next->data;
                        continue;
                    }
                    // condition was true
                    if (element->next->next == NULL) {
                        printf("if without body\n");
                        global_error = AST;
                        return NewMalNIL();
                    }
                    AST = element->next->next->data;
                    continue;
                }
                // fn*
                if (strcmp(symbol, "fn*") == 0) {
                    if (is_empty(element->next)) {
                        printf("fn shall take at least one arg");
                        global_error = AST;
                        return NewMalNIL();
                    }
                    if (!IsListOrVector(element->next->data)) {
                        printf("fn parameters should be a list or a vector\n");
                        global_error = AST;
                        return NewMalNIL();
                    }
                    if (element->next == NULL) {
                        printf("fn dont have a body rn :/ \n");
                        global_error = AST;
                        return NewMalNIL();
                    }

                    MalType *parameters = element->next->data;
                    MalType *body       = element->next->next->data;
                    return NewMalFnWrapper(parameters, body, env);
                }
                // quote
                if (strcmp(symbol, "quote") == 0) {
                    if (element->next == NULL) {
                        return NewMalNIL();
                    }

                    // skip symbol
                    return element->next->data;
                }
                // quasiquote
                if (strcmp(symbol, "quasiquote") == 0) {
                    if (element->next == NULL) {
                        return NewMalNIL();
                    }
                    AST = quasiquote(element->next->data, env);
                    continue;
                }
                // defmacro!
                if (strcmp(symbol, "defmacro!") == 0) {
                    if (element->next == NULL) {
                        printf("defmacro! shall take two args");
                        global_error = AST;
                        return AST;
                    }
                    if (element->next->next == NULL) {
                        printf("defmacro! shall take two args");
                        global_error = AST;
                        return AST;
                    }
                    MalType *key   = element->next->data;
                    MalType *value = EVAL(element->next->next->data, env);

                    if (global_error != NULL) {
                        printf("defmacro! value error\n");
                        return value;
                    }

                    if (value == NULL) {
                        printf("defmacro! eval arg returned NULL\n");
                        global_error = AST;
                        return AST;
                    }
                    if (!IsFnWrapper(value)) {
                        printf("defmacro! arg must be a function\n");
                        global_error = AST;
                        return AST;
                    }

                    value->value.FnWraperValue->is_macro = 1;

                    set(env, key, value);

                    return value;
                }
                // try* / catch
                if (strcmp(symbol, "try*") == 0) {
                    if (is_empty(element->next)) {
                        return NewMalNIL();
                    }
                    MalType *form_a = element->next->data;

                    MalType *eval_a_ret = EVAL(form_a, env);

                    if (is_empty(element->next->next)) {
                        return eval_a_ret;
                    }

                    MalType *form_catch_b_c = element->next->next->data;

                    if (!IsList(form_catch_b_c)) {
                        printf("try* catch block is not in a list\n");
                        global_error = form_catch_b_c;
                        return global_error;
                    }

                    if (global_error != NULL) {
                        // TODO: exeption handleing
                        // (catch* B C)
                        MalList *list = GetList(form_catch_b_c);
                        if (is_empty(list)) {
                            printf("empty catch block\n");
                            global_error = AST;
                            return global_error;
                        }
                        if (is_empty(list->next)) {
                            printf("catch without B and C\n");
                            global_error = AST;
                            return global_error;
                        }
                        if (is_empty(list->next->next)) {
                            printf("catch without C\n");
                            global_error = AST;
                            return global_error;
                        }

                        MalType *form_b = list->next->data;
                        MalType *form_c = list->next->next->data;

                        if (!IsSymbol(form_b)) {
                            printf("catch B must be a symbol\n");
                            global_error = AST;
                            return global_error;
                        }

                        MalList *binds = GC_MALLOC(sizeof(MalList));
                        MalList *exprs = GC_MALLOC(sizeof(MalList));

                        append(binds, form_b, sizeof(MalType));
                        append(exprs, global_error, sizeof(MalType));

                        env_t *new_env = create_env(env, NewMalList(binds),
                                                    NewMalList(exprs));

                        env = new_env;
                        AST = form_c;
                        continue;
                    }
                    return eval_a_ret;
                }
            }

            // eval first element
            MalType *evaluated_first_element = EVAL(first_element, env);

            if (global_error != NULL) {
                printf("eval first elemtn failed\n");
                return evaluated_first_element;
            }

            switch (evaluated_first_element->type) {
            case MAL_FN_WRAPER: {
                int do_eval_macro =
                    GetFnWrapper(evaluated_first_element)->is_macro;

                MalFnWrapper *f = GetFnWrapper(evaluated_first_element);

                // skip evaluated first element;
                node_t *unevaluated_args = element->next;
                node_t *evaluated_args   = GC_MALLOC(sizeof(node_t));

                if (!do_eval_macro) {
                    // eval args in current env
                    while (unevaluated_args != NULL) {
                        MalType *arg           = unevaluated_args->data;
                        MalType *evaluated_arg = EVAL(arg, env);
                        if (global_error != NULL) {
                            printf("eval fn args failed\n");
                            return evaluated_arg;
                        }
                        append(evaluated_args, evaluated_arg, sizeof(MalType));
                        unevaluated_args = unevaluated_args->next;
                    }
                }

                // check for Clojure-style variadic function parameters
                node_t *binds;
                if (f->param == NULL) {
                    binds = NULL;
                } else {
                    binds = GetList(f->param);
                }
                node_t *exprs;
                if (do_eval_macro) {
                    exprs = unevaluated_args;
                } else {
                    exprs = evaluated_args;
                }
                // construc exprs
                // (make a list of the rest of the args when & in func binds)
                // iterate over func->binds
                // apply to a copy without the &
                while (binds != NULL && exprs != NULL) {
                    if (binds->data == NULL) {
                        break;
                    }
                    char *bind = GetSymbol(binds->data);
                    if (strcmp(bind, "&") == 0) {
                        // skip the & in the binds list
                        // construct a new binds list without the & for eval
                        // but dont mutate the fuctions binds

                        // NOTE: mutate the function binds

                        // make the current exprs a Maltlist that contain
                        // the rest of the exprs
                        exprs->data = NewMalListCopy(exprs);
                        exprs->next = NULL;
                        break;
                    }
                    // catch exprs empty
                    if (exprs->next == NULL && binds->next != NULL) {
                        char *bind = GetSymbol(binds->next->data);
                        if (strcmp(bind, "&") != 0) {
                            printf("not enough args passed to the function\n");
                            return AST;
                        }
                        // skip the & in the binds list
                        binds       = binds->next;
                        binds->data = binds->next->data;

                        // if there is no exprs, still put an empty MalList
                        exprs->next = GC_MALLOC(sizeof(node_t));
                        exprs->next->data =
                            NewMalListCopy(GC_MALLOC(sizeof(node_t)));
                        break;
                    }

                    binds = binds->next;
                    exprs = exprs->next;
                }

                // make a copy of binds without the & for evaluation
                node_t *binds_without_and_symbol = GC_MALLOC(sizeof(node_t));
                node_t *node                     = GetList(f->param);
                while (node != NULL) {
                    // skip &
                    if (node->data == NULL) {
                        // f with no binds
                        break;
                    }
                    char *bind = GetSymbol(node->data);
                    if (strcmp(bind, "&") == 0) {
                        node = node->next;
                        append(binds_without_and_symbol, node->data,
                               sizeof(MalType));
                        break;
                    }

                    append(binds_without_and_symbol, node->data,
                           sizeof(MalType));
                    node = node->next;
                }

                if (do_eval_macro) {
                    env_t *new_env =
                        create_env(f->env, NewMalList(binds_without_and_symbol),
                                   NewMalList(unevaluated_args));

                    if (new_env == NULL) {
                        printf("macro new_env is NULL\n");
                        global_error = AST;
                    }

                    // create a copy of ast
                    // to avoid mutating the macro while evaluating it's body
                    // NOTE: idk if it's necessary tho
                    // might remove this, idk
                    node_t  *list           = GC_MALLOC(sizeof(node_t));
                    MalType *func_body_copy = GC_MALLOC(sizeof(MalType));

                    // NOTE: can f->ast be a vector ? not shure
                    if (IsListOrVector(f->ast)) {
                        func_body_copy->type = MAL_LIST;
                        node_t *node         = GetList(f->ast);
                        while (!is_empty(node)) {
                            append(list, node->data, sizeof(MalType));
                            node = node->next;
                        }
                        // NOTE: setters ? pfffff
                        func_body_copy->value.ListValue = list;

                        AST = EVAL(func_body_copy, new_env);
                        if (global_error != NULL) {
                            printf("eval function failed\n");
                            return AST;
                        }
                        continue;
                    } else {
                        // dont create a copy
                        func_body_copy = f->ast;
                        AST            = EVAL(func_body_copy, new_env);
                        if (global_error != NULL) {
                            printf("eval function failed\n");
                            return AST;
                        }
                        continue;
                    }
                }

                // normal function (not macro)
                // TCO
                AST = f->ast;
                env = create_env(f->env, NewMalList(binds_without_and_symbol),
                                 NewMalList(evaluated_args));

                continue;
            }
            case MAL_CORE_FN: {
                MalType *core_fn = evaluated_first_element;

                // skip symbol
                element = element->next;

                // add the rest of the list to a list of evaluated things

                // define a list of evaluated element
                node_t *evaluated_list = GC_MALLOC(sizeof(node_t));

                // evaluate each element in the current list and add them
                while (element != NULL) {
                    MalType *evaluated_element = EVAL(element->data, env);
                    // if (global_error != NULL) {
                    //     return NewMalNIL();
                    // }

                    append(evaluated_list, (void *)evaluated_element,
                           sizeof(MalType));

                    element = element->next;
                };

                MalType *result = GetCoreFn(core_fn)(evaluated_list);
                return result;
            }
            default: {
                // list have nothing special, eval everything and return.

                // define a list of evaluated element
                node_t *evaluated_list = GC_MALLOC(sizeof(node_t));

                // evaluate each element in the current list and add them
                while (element != NULL) {
                    MalType *evaluated_element = EVAL(element->data, env);
                    if (global_error != NULL) {
                        printf("list elt eval failed\n");
                        return evaluated_element;
                    }
                    if (evaluated_element != NULL) {
                        append(evaluated_list, (void *)evaluated_element,
                               sizeof(MalType));
                    }
                    element = element->next;
                };

                if (eval_vector == 1) {
                    return NewMalVector(evaluated_list);
                }
                return NewMalList(evaluated_list);
            }
            }
        }
        case MAL_ATOM: {
            AST = AST->value.AtomValue;
            continue;
        }
        case MAL_HASHMAP: {
            // EVAL each value (and key, caus it's easyer like that)
            MalHashmap *hashmap = GetHashmap(AST);

            while (hashmap != NULL) {
                hashmap->data = EVAL(hashmap->data, env);
                if (global_error != NULL) {
                    printf("hashmap eval failed (peobably value)");
                    return hashmap->data;
                }
                hashmap = hashmap->next;
            };
            return AST;
        }
        default: {
            return AST;
        } // string, int, false, true, nil, fn, corefn, fnwraper, keyword
        }

        fprintf(stderr, "unmatched AST\n");
        global_error = AST;
        return NULL;
    }
}

char *PRINT(MalType *AST) {
    return pr_str(AST, 1);
}

char *rep(char *line, env_t *env) {
    MalType *read     = READ(line);
    MalType *eval_ret = EVAL(read, env);

    if (global_error != NULL) {
        // append Uncaught error: to the current error
        char *eval_error = pr_str(global_error, 0);

        char *err_str = GC_MALLOC(7 + strlen(eval_error));
        strcat(err_str, "error: ");
        strcat(err_str, eval_error);

        global_error = NewMalSymbol(err_str);
        printf("%s\n", pr_str(global_error, 0));
    }

    return PRINT(eval_ret);
}
