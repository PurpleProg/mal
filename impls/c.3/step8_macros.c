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

MalType *eval(node_t *node) {
    MalType *ast = node->data;
    return EVAL(ast, repl_env);
}

int main(int argc, char *argv[]) {
    GC_INIT();
    char   *line = NULL;
    size_t  len  = 0;
    ssize_t read;

    repl_env = create_repl();

    // add eval to the repl
    set(repl_env, NewMalString("eval"), NewMalCoreFunction(eval));

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

MalType *quasiquote(MalType *AST) {
    if (IsList(AST)) {
        MalList *list = GetList(AST);

        if (is_empty(list)) {
            printf("quasiquote empty list, returning AST\n");
            return AST;
        }

        MalType *first_element = list->data;

        // if AST is a list starting with the unquote symbol
        if (IsSymbol(first_element)) {
            if (strcmp(GetSymbol(first_element), "unquote") == 0) {
                if (list->next == NULL) {
                    printf("unquoting nothing :/\n");
                    return NewMalNIL();
                }
                printf("unquote\n");
                // return the second element
                return list->next->data;
            }
        }

        // AST is a list that DONT start with unquote

        // list result
        node_t *list_result = GC_MALLOC(sizeof(node_t));

        // iterate over reversed list
        node_t *reversed_list = GC_MALLOC(sizeof(node_t));
        reverse_list(list, &reversed_list);
        node_t *node = reversed_list;
        while (!is_empty(node)) {
            MalType *elt = node->data;
            if (elt == NULL) {
                printf("elt is NULL\n");
            }
            if (elt->value.ListValue == NULL) {
                printf("elt->value is NULL\n");
            }

            // if elt is a list starting with "split-unquote"
            if (IsListOrVector(elt)) {
                MalList *list = GetList(elt);
                if (is_empty(list)) {
                    return AST;
                }
                MalType *elt_first_element = list->data;
                if (IsSymbol(elt_first_element)) {
                    if (strcmp(GetSymbol(elt_first_element),
                               "splice-unquote") == 0) {
                        // replace the current result with a list containing:
                        // the "concat" symbol,
                        // the second element of elt,

                        // new list result
                        node_t *new_list_result = GC_MALLOC(sizeof(node_t));

                        append(new_list_result, NewMalSymbol("concat"),
                               sizeof(MalType));

                        // the second element of elt,
                        MalType *next_elt_element = list->next->data;
                        append(new_list_result, next_elt_element,
                               sizeof(MalType));

                        // then the previous result
                        append(new_list_result, NewMalList(list_result),
                               sizeof(MalType));

                        // replace current result with new_result
                        list_result = new_list_result;

                        node = node->next;
                        continue;
                    } // if (first element == "splice-unquote")
                } // if (first_element->type == MAL_SYMBOL)

            } // if (elt->type == MAL_LIST or vec)

            // Else replace the current result with a list containing:
            // the "cons" symbol,
            // the result of calling quasiquote with elt as argument,
            // then the previous result.

            // new list result
            node_t *new_list_result = GC_MALLOC(sizeof(node_t));

            // the "cons" symbol,
            append(new_list_result, NewMalSymbol("cons"), sizeof(MalType));

            // the result of calling quasiquote with elt as argument,
            MalType *ret = quasiquote(elt);
            // printf("rec quasiquote call : %s\n", pr_str(ret, 0));
            append(new_list_result, ret, sizeof(MalType));

            // then the previous result
            append(new_list_result, NewMalList(list_result), sizeof(MalType));

            // replace current result with new_result
            // memcpy(list_result, new_list_result, sizeof(node_t));
            list_result = new_list_result;

            node = node->next;
        } // iterate over elt in reverse order

        return NewMalList(list_result);

    } else if (IsVector(AST)) {
        MalList *new_list = GC_MALLOC(sizeof(MalList));
        append(new_list, NewMalSymbol("vec"), sizeof(MalType));
        append(new_list, NewMalListCopy(GetList(AST)), sizeof(MalType));

        return EVAL(quasiquote(NewMalList(new_list)), repl_env);
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
    // Such forms are not affected by evaluation,
    // so you may quote them as in the previous case
    // if implementation is easier.
    return AST;
}

MalType *READ(char *line) {
    return read_str(line);
}

MalType *EVAL(MalType *AST, env_t *env) {
    while (1) {

        MalType *do_contain_debug_eval = get(env, NewMalSymbol("DEBUG-EVAL"));
        if (do_contain_debug_eval != NULL) {
            if (!IsFalse(do_contain_debug_eval) &&
                !IsNil(do_contain_debug_eval)) {
                printf("EVAL: %s\n", pr_str(AST, 1));
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

            // true
            if (strcmp(symbol, "true") == 0) {
                return NewMalTrue();
            }
            // false
            if (strcmp(symbol, "false") == 0) {
                return NewMalFalse();
            }
            // nil
            if (strcmp(symbol, "nil") == 0) {
                return NewMalNIL();
            }

            MalType *ret = get(env, NewMalString(symbol));
            if (ret == NULL) {
                return NewMalNIL();
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
                return AST;
            }

            // special forms
            if (IsSymbol(first_element)) {
                char *symbol = GetSymbol(first_element);
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
                    MalType *key   = element->next->data;
                    MalType *value = EVAL(element->next->next->data, env);
                    // TODO: check if EVAL returned an error
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
                    map_t *map     = GC_MALLOC(sizeof(map_t));
                    env_t *new_env = GC_MALLOC(sizeof(env_t));
                    new_env->outer = env;
                    new_env->data  = map;

                    // define the binding list
                    MalList *bindings_list = NULL;
                    if (IsListOrVector(element->next->data)) {
                        bindings_list = GetList(element->next->data);
                    } else {
                        printf("let* takes a list or a vector as arg :/\n");
                        return AST;
                    }

                    // evaluate the binding list
                    node_t *binding = bindings_list;
                    while (binding != NULL) {
                        if (binding->next == NULL) {
                            printf("binding list is odd\n");
                            return AST;
                        }

                        MalType *key   = binding->data;
                        MalType *value = EVAL(binding->next->data, new_env);
                        if (value != NULL) {
                            set(new_env, key, value);
                        }
                        binding = binding->next->next;
                    }

                    // TCO
                    env = new_env;
                    AST = element->next->next->data;
                    continue;
                }
                // do
                if (strcmp(symbol, "do") == 0) {
                    if (element->next == NULL) {
                        printf("do shall take at least one arg");
                        return AST;
                    }

                    // skip "do" symbol
                    element = element->next;

                    // evaluate each element
                    while (element->next != NULL) {
                        EVAL(element->data, env);
                        element = element->next;
                    };

                    AST = element->data;
                    continue;
                }
                // if
                if (strcmp(symbol, "if") == 0) {
                    if (element->next == NULL) {
                        printf("if shall take at least one arg");
                        return NewMalNIL();
                    }
                    MalType *condition = EVAL(element->next->data, env);
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
                        return NewMalNIL();
                    }
                    AST = element->next->next->data;
                    continue;
                }
                // fn*
                if (strcmp(symbol, "fn*") == 0) {
                    if (element->next == NULL) {
                        printf("fn shall take at least one arg");
                        return NewMalNIL();
                    }
                    if (!IsListOrVector(element->next->data)) {
                        printf("fn parameters should be a list or a vector\n");
                        return NewMalNIL();
                    }
                    if (element->next == NULL) {
                        printf("fn dont have a body rn :/ \n");
                        return NewMalNIL();
                    }

                    // create the function
                    MalFn   *function   = GC_MALLOC(sizeof(MalFn));
                    MalType *parameters = element->next->data;
                    MalType *body       = element->next->next->data;

                    if (!IsListOrVector(parameters)) {
                        printf("args whould be a list or a vector\n");
                        return AST;
                    }

                    // function->param = formated_parameters;
                    function->param = parameters;
                    function->body  = body;
                    function->env   = env;

                    // wrap funtion for TCO
                    MalFnWraper *fnwraper = GC_MALLOC(sizeof(MalFnWraper));
                    fnwraper->ast         = element->next->next->data; // body
                    fnwraper->param       = element->next->data;
                    fnwraper->env         = env;
                    fnwraper->fn          = function;
                    fnwraper->is_macro    = 0;

                    // wrap function in MalType
                    MalType *mal_type_function = GC_MALLOC(sizeof(MalType));
                    mal_type_function->type    = MAL_FN_WRAPER;
                    mal_type_function->value.FnWraperValue = fnwraper;

                    return mal_type_function;
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
                    AST = quasiquote(element->next->data);
                    continue;
                }
                // defmacro!
                if (strcmp(symbol, "defmacro!") == 0) {
                    if (element->next == NULL) {
                        printf("defmacro! shall take two args");
                        return AST;
                    }
                    if (element->next->next == NULL) {
                        printf("defmacro! shall take two args");
                        return AST;
                    }
                    MalType *key   = element->next->data;
                    MalType *value = EVAL(element->next->next->data, env);
                    if (value == NULL) {
                        printf("defmacro! eval arg returned NULL\n");
                        return AST;
                    }
                    if (!IsFnWrapper(value)) {
                        printf("defmacro! arg must be a function\n");
                        return AST;
                    }

                    value->value.FnWraperValue->is_macro = 1;

                    set(env, key, value);

                    return value;
                }
            }

            // eval first element
            MalType *evaluated_first_element = EVAL(first_element, env);
            if (evaluated_first_element == NULL) {
                return AST;
            }

            switch (evaluated_first_element->type) {
            case MAL_FN_WRAPER: {
                int do_eval_macro =
                    GetFnWrapper(evaluated_first_element)->is_macro;

                MalFnWraper *f = GetFnWrapper(evaluated_first_element);

                // skip evaluated first element;
                node_t *unevaluated_args = element->next;
                node_t *evaluated_args   = GC_MALLOC(sizeof(node_t));

                if (!do_eval_macro) {
                    // eval args in current env
                    while (unevaluated_args != NULL) {
                        MalType *arg = unevaluated_args->data;
                        append(evaluated_args, EVAL(arg, env), sizeof(MalType));
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
                        printf("expr is NULL\n");
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
                        continue;
                    } else {
                        // dont create a copy
                        func_body_copy = f->ast;
                        AST            = EVAL(func_body_copy, new_env);
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
                printf("evaluated hmap data : %s\n", pr_str(hashmap->data, 0));
                hashmap = hashmap->next;
            };
            return AST;
        }
        default: {
            return AST;
        } // string, int, false, true, nil, fn, corefn, fnwraper, keyword
        }

        fprintf(stderr, "unmatched AST\n");
        return NULL;
    }
}

char *PRINT(MalType *AST) {
    return pr_str(AST, 1);
}

char *rep(char *line, env_t *env) {
    return PRINT(EVAL(READ(line), env));
}
