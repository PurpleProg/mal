#include "core.h"
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

// #define DEBUG

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

    rep("(def! not (fn* (condition) (if condition false true)))", repl_env);
    rep("(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp "
        "f) "
        "\"\\nnil)\")) ) ))",
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
        "      (str \"no more\")"
        "    )" // func body
        "  )"
        ")",
        repl_env);
    // test diff macro vs function
    rep("(defmacro! test (fn* (& lst) (list (count lst) (pr-str lst))))",
        repl_env);
    rep("(def! ftest (fn* (& lst) (list (count lst) (pr-str lst))))", repl_env);
    // unless macro
    rep("(defmacro! unless (fn* (condition, iftrue, iffalse)"
        " (if (not condition) iftrue iffalse)"
        "))",
        repl_env);

    // add eval to the repl
    set(repl_env, "eval", NewMalCoreFunction(eval));

    // create *ARGV*
    node_t *args_list = GC_MALLOC(sizeof(node_t));
    for (int i = 2; i < argc; i++) {
        append(args_list, NewMalString(argv[i]), sizeof(MalType));
    }
    set(repl_env, "*ARGV*", NewMalList(args_list));

    // check if the interpretor is called with args
    if (argc > 1) {
        // call load-file on the first arg
        char *string = GC_MALLOC(strlen(argv[1]) + strlen("(load-file \"\")"));
        sprintf(string, "(load-file \"%s\")", argv[1]);
        printf("%s\n", rep(string, repl_env));
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
    if (AST->type == MAL_LIST || AST->type == MAL_VECTOR) {
        node_t *list = AST->value.ListValue;

        if (is_empty(list)) {
            printf("quasiquote empty list, returning AST\n");
            return AST;
        }

        MalType *first_element = list->data;

        // if AST is a list starting with the unquote symbol
        if (first_element->type == MAL_SYMBOL) {
            if (strcmp(first_element->value.SymbolValue, "unquote") == 0) {
                if (list->next == NULL) {
                    printf("unquoting nothing :/\n");
                    // TODO: return nil maybe
                    return NULL;
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
            if (elt->type == MAL_LIST || elt->type == MAL_VECTOR) {
                node_t *list = elt->value.ListValue;
                if (is_empty(list)) {
                    return AST;
                }
                MalType *elt_first_element = list->data;
                if (elt_first_element->type == MAL_SYMBOL) {
                    if (strcmp(elt_first_element->value.SymbolValue,
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

    } else if (AST->type == MAL_SYMBOL || AST->type == MAL_HASHMAP) {
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

MalType *EVAL_SYMBOL(MalType *AST, env_t *env) {
    // NOTE: get symbol from env
    // and eval (true, false, nil)
    char *symbol = AST->value.SymbolValue;

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

    MalType *ret = get(env, symbol);
    // returning NULL make TCO continue.
    if (ret == NULL) {
        return NewMalNIL();
    }
    return ret;
}
MalType *EVAL_LIST_FN_WRAPPER(MalType **ASTp, env_t **envp, node_t *element,
                              MalType *evaluated_first_element,
                              int      do_eval_macro) {
    MalType *AST = *ASTp;
    env_t   *env = *envp;

    MalFnWraper *f = evaluated_first_element->value.FnWraperValue;

    // skip evaluated first element;
    node_t *unevaluated_args = element->next;
    node_t *evaluated_args   = GC_MALLOC(sizeof(node_t));

    // handle macros
    if (!do_eval_macro) {
        // eval args in current env
        while (unevaluated_args != NULL) {
            MalType *arg = unevaluated_args->data;
            append(evaluated_args, EVAL(arg, env), sizeof(MalType));
            unevaluated_args = unevaluated_args->next;

            // printf("eval args : %s \n", pr_str(arg, 0));
        }
    }

    // check for Clojure-style variadic function parameters
    node_t *binds = f->param->value.ListValue;
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
        char *bind = ((MalType *)binds->data)->value.SymbolValue;
        if (strcmp(bind, "&") == 0) {

            printf("& found\n");

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
            char *bind = ((MalType *)binds->next->data)->value.SymbolValue;
            if (strcmp(bind, "&") != 0) {
                printf("not enough args passed to the function\n");
                return AST;
            }
            // skip the & in the binds list
            binds       = binds->next;
            binds->data = binds->next->data;

            // if there is no exprs, still put an empty MalList
            exprs->next       = GC_MALLOC(sizeof(node_t));
            exprs->next->data = NewMalListCopy(GC_MALLOC(sizeof(node_t)));
            break;
        }

        binds = binds->next;
        exprs = exprs->next;
    }

    // make a copy of binds without the & for evaluation
    node_t *binds_without_and_symbol = GC_MALLOC(sizeof(node_t));
    node_t *node                     = f->param->value.ListValue;
    while (node != NULL) {
        // skip &
        if (node->data == NULL) {
            // f with no binds
            break;
        }
        char *bind = ((MalType *)node->data)->value.SymbolValue;
        if (strcmp(bind, "&") == 0) {
            node = node->next;
            append(binds_without_and_symbol, node->data, sizeof(MalType));
            break;
        }

        append(binds_without_and_symbol, node->data, sizeof(MalType));
        node = node->next;
    }

    // skip TCO for macro,
    // alow EVAL twice afterward
    if (do_eval_macro) {
        env_t *new_env = create_env(
            f->env, NewMalList(binds_without_and_symbol), unevaluated_args);

        if (new_env == NULL) {
            printf("macro new_env is NULL\n");
        }
        // printf("f->param: %s\n", pr_str(f->param, 0));
        // printf("f->ast: %s\n", pr_str(f->ast, 0));
        // printf("binds no &: %s\n",
        //        pr_str(binds_without_and_symbol_wrapped_in_a_maltype, 0));
        // printf("eval macro env: %s\n", pr_env(new_env));

        // create a copy of ast
        // to avoid mutating the macro while evaluating it's body
        // NOTE: idk if it's necessary tho
        // might remove this, idk
        node_t  *list           = GC_MALLOC(sizeof(node_t));
        MalType *func_body_copy = GC_MALLOC(sizeof(MalType));

        // f->ast must be a MalList right ?
        // surely that wont cause a segfault
        if (f->ast->type == MAL_LIST) {
            func_body_copy->type = MAL_LIST;
            node_t *node         = f->ast->value.ListValue;
            while (!is_empty(node)) {
                append(list, node->data, sizeof(MalType));
                node = node->next;
            }
            func_body_copy->value.ListValue = list;

            printf("func body copy: %s\n", pr_str(func_body_copy, 0));

            return EVAL(func_body_copy, new_env);
        } else {
            // dont create a copy
            func_body_copy = f->ast;
            return EVAL(func_body_copy, new_env);
        }
    }
    // TCO
    *ASTp = f->ast;
    *envp = create_env(f->env, NewMalList(binds_without_and_symbol),
                       evaluated_args);
    // printf("eval fn env: %s\n", pr_env(*envp));

    return NULL;
}
MalType *EVAL_LIST_CORE_FN(MalType **ASTp, env_t **envp, node_t *element,
                           MalType *evaluated_first_element) {
    MalType *AST = *ASTp;
    env_t   *env = *envp;

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

    // evaluate each element in the current list and add them
    while (element != NULL) {
        MalType *evaluated_element = EVAL(element->data, env);
        // if (evaluated_element->type == MAL_LIST) {
        //	// fix VECTORS
        //	evaluated_element->type = ((MalType *)element->data)->type;
        // }

        if (evaluated_element == NULL) {
            // just skip it
            continue;
        }
        append(evaluated_list, (void *)evaluated_element, sizeof(MalType));

        element = element->next;
    };

    MalType *result = (*(MalCoreFn)core_fn->value.CoreFnValue)(evaluated_list);
    return result;
}
MalType *EVAL_LIST_DEFAULT(env_t **envp, node_t *element, int vector) {
    env_t *env = *envp;

    // list have nothing special, eval everything and return.
    // define a list of evaluated element
    node_t *evaluated_list = GC_MALLOC(sizeof(node_t));

    // evaluate each element in the current list and add them
    while (element != NULL) {
        MalType *evaluated_element = EVAL(element->data, env);
        if (evaluated_element != NULL) {
            append(evaluated_list, (void *)evaluated_element, sizeof(MalType));
        }
        element = element->next;
    };

    if (vector == 1) {
        return NewMalVector(evaluated_list);
    }
    return NewMalList(evaluated_list);
}
MalType *EVAL_LIST(MalType **ASTp, env_t **envp, int vector) {
    // trick that allow modifing the AST and env of EVAL from here
    MalType *AST = *ASTp;
    env_t   *env = *envp;

    node_t *element = AST->value.ListValue;
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
    if (first_element->type == MAL_SYMBOL) {
        char *symbol = first_element->value.SymbolValue;
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
            map_t *map     = GC_MALLOC(sizeof(map_t));
            env_t *new_env = GC_MALLOC(sizeof(env_t));
            new_env->outer = env;
            new_env->data  = map;

            // define the binding list
            MalList *bindings_list = NULL;
            if (((MalType *)(element->next->data))->type == MAL_LIST ||
                ((MalType *)(element->next->data))->type == MAL_VECTOR) {
                bindings_list =
                    ((MalType *)(element->next->data))->value.ListValue;
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

                MalSymbol *key =
                    ((MalType *)(binding->data))->value.SymbolValue;
                MalType *value = EVAL((MalType *)binding->next->data, new_env);
                if (value != NULL) {
                    set(new_env, key, value);
                }
                binding = binding->next->next;
            }

            // TCO
            *envp = new_env;
            *ASTp = element->next->next->data;
            return NULL;
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

            *ASTp = element->data;
            return NULL;
        }
        // if
        if (strcmp(symbol, "if") == 0) {
            if (element->next == NULL) {
                printf("if shall take at least one arg");
                return NewMalNIL();
            }
            MalType *condition = EVAL(element->next->data, env);
            if (condition->type == MAL_FALSE || condition->type == MAL_NIL) {
                // condition is false
                if (element->next->next->next == NULL) {
                    return NewMalNIL();
                }
                *ASTp = element->next->next->next->data;
                return NULL;
            }
            // condition was true
            if (element->next->next == NULL) {
                printf("if without body\n");
                return NewMalNIL();
            }
            *ASTp = element->next->next->data;
            return NULL;
        }
        // fn*
        if (strcmp(symbol, "fn*") == 0) {
            if (element->next == NULL) {
                printf("fn shall take at least one arg");
                return NewMalNIL();
            }
            if (((MalType *)element->next->data)->type != MAL_LIST &&
                ((MalType *)element->next->data)->type != MAL_VECTOR) {
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

            if (parameters->type != MAL_LIST &&
                (parameters->type != MAL_VECTOR)) {
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
            MalType *mal_type_function             = GC_MALLOC(sizeof(MalType));
            mal_type_function->type                = MAL_FN_WRAPER;
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
            *ASTp = quasiquote(element->next->data);
            return NULL;
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
            MalSymbol *key =
                ((MalType *)(element->next->data))->value.SymbolValue;
            MalType *value = EVAL(element->next->next->data, env);
            if (value == NULL) {
                printf("defmacro! eval arg returned NULL\n");
                return AST;
            }
            if (value->type != MAL_FN_WRAPER) {
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
        if (evaluated_first_element->value.FnWraperValue->is_macro) {
            // apply the function
            // to the (unevaluated) remaining elements of ast,
            // producing a new form

            // evaluate the new form in the env environment.
            // Of course, instead of recursively calling EVAL,
            // replace ast with the new form and restart the TCO loop.

            MalType *evaluated_macro = EVAL_LIST_FN_WRAPPER(
                ASTp, envp, element, evaluated_first_element, 1);

            *ASTp = EVAL(evaluated_macro, *envp);
            return NULL;
        }
        return EVAL_LIST_FN_WRAPPER(ASTp, envp, element,
                                    evaluated_first_element, 0);
    }
    case MAL_CORE_FN: {
        return EVAL_LIST_CORE_FN(ASTp, envp, element, evaluated_first_element);
    }
    default: {
        return EVAL_LIST_DEFAULT(envp, element, vector);
    }
    }
}
MalType *EVAL(MalType *AST, env_t *env) {
    while (1) {
#ifdef DEBUG
        printf("EVAL %s\n", pr_str(AST, 0));
#endif
        // printf("env: %s\n", pr_env(env));
        if (AST == NULL) {
            return AST;
        }
        switch (AST->type) {
        case MAL_SYMBOL: {
            return EVAL_SYMBOL(AST, env);
        }
        case MAL_LIST: {
            MalType *ret = GC_MALLOC(sizeof(MalType));
            ret          = EVAL_LIST(&AST, &env, 0);
            if (ret == NULL) {
                // TCO
                continue;
            } // else normal recurtion
            return ret;
        }
        case MAL_VECTOR: {
            MalType *ret = GC_MALLOC(sizeof(MalType));
            ret          = EVAL_LIST(&AST, &env, 1);
            if (ret == NULL) {
                // TCO
                continue;
            } // else normal recurtion
            return ret;
        }
        case MAL_ATOM: {
            AST = AST->value.AtomValue;
            continue;
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
