#include <stdio.h>
#include <string.h>
#include "gc.h"
#include "types.h"
#include "core.h"
#include "reader.h"
#include "printer.h"
#include "linked_list.h"
#include "hashmap.h"
#include "env.h"
#include "types.h"


MalType * READ(char * line);
MalType * EVAL(MalType * AST, env_t * env);
char* PRINT(MalType * AST);
char* rep(char * line, env_t * env);


env_t * repl_env;

MalType * eval(node_t * node) {
    MalType * ast = node->data;
	return EVAL(ast, repl_env);
}


int main(int argc, char *argv[]) {
	GC_INIT();
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	repl_env = create_repl();

	rep("(def! not (fn* (a) (if a false true)))", repl_env);
	rep("(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp f) \"\\nnil)\")) ) ))", repl_env);
	rep("(def! test (fn* (a) (str a)))", repl_env);

	// add eval to the repl
	set(repl_env, "eval", wrap_function(eval));

	// create *ARGV*
	MalType * args_list = GC_MALLOC(sizeof(MalType));
	args_list->type = MAL_LIST;
	args_list->value.ListValue = GC_MALLOC(sizeof(node_t));
	for (int i = 2; i<argc; i++) {
		char * arg = argv[i];
		MalType * wraped_arg = GC_MALLOC(sizeof(MalType));
		wraped_arg->type = MAL_STRING;
		wraped_arg->value.StringValue = GC_MALLOC(strlen(arg));
		strcat(wraped_arg->value.StringValue, argv[i]);
		append(args_list->value.ListValue, wraped_arg, sizeof(MalType));
	}
	set(repl_env, "*ARGV*", args_list);

	// DEBUG:
	// printf("repl created : %s\n", pr_env(repl_env));

	// check if the interpretor is called with args
	if (argc > 1) {

		// call load-file on the first arg
		char * string = GC_MALLOC(strlen(argv[1]) + strlen("(load-file \"\")"));
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


MalType * READ(char * line) {
	return read_str(line);
}

MalType * EVAL_SYMBOL(MalType * AST, env_t * env) {
	// NOTE: return the function from the repl env
	// or special forms as strings
	char * symbol = AST->value.SymbolValue;
	// def!
	if (strcmp(symbol, "def!") == 0) {
		return AST;
	}
	// let*
	if (strcmp(symbol, "let*") == 0) {
		return AST;
	}
	// fn*
	if (strcmp(symbol, "fn*") == 0) {
		return AST;
	}
	// if
	if (strcmp(symbol, "if") == 0) {
		return AST;
	}
	// do
	if (strcmp(symbol, "do") == 0) {
		return AST;
	}
	// true
	if (strcmp(symbol, "true") == 0) {
		MalType * true = GC_MALLOC(sizeof(MalType));
		true->type = MAL_TRUE;
		return true;
	}
	// false
	if (strcmp(symbol, "false") == 0) {
		MalType * false = GC_MALLOC(sizeof(MalType));
		false->type = MAL_FALSE;
		return false;
	}
	// nil
	if (strcmp(symbol, "nil") == 0) {
		MalType * nil_ret = GC_MALLOC(sizeof(MalType));
		nil_ret->type = MAL_NIL;
		return nil_ret;
	}

	return get(env, symbol);
}

MalType * EVAL_LIST_FN_WRAPPER(MalType ** ASTp, env_t ** envp, node_t * element, MalType * evaluated_first_element) {
	MalType * AST = *ASTp;
	env_t * env = *envp;

	MalFnWraper * f = evaluated_first_element->value.FnWraperValue;

	node_t * args = GC_MALLOC(sizeof(node_t));
	args->data = NULL;
	args->next = NULL;

	// skip evaluated first element;
	node_t * node = element->next;

	// eval args in current env
	while (node != NULL) {
		MalType * arg = node->data;

		append(args, EVAL(arg, env), sizeof(MalType));
		node = node->next;
	}

	// check for Clojure-style variadic function parameters
	// segfault here swap
	// f is NULL
	node_t * binds = f->param->value.ListValue;
	node_t * exprs = args;
	while (binds != NULL && exprs != NULL) {
		if (binds->data == NULL) {
			break;
		}
		char * bind = ((MalType *)binds->data)->value.SymbolValue;
		if ( strcmp(bind, "&") == 0 ) {
			// skip the & in the binds list
			binds->data = binds->next->data;
			binds->next = NULL;

			// make the next exprs a list (wraped in a maltype) that contain the rest of the exprs
			//define it
			MalType * wrap = GC_MALLOC(sizeof(MalType));
			wrap->type = MAL_LIST;
			wrap->value.ListValue = GC_MALLOC(sizeof(node_t));
			memcpy(wrap->value.ListValue, exprs, sizeof(node_t));

			// assigne the list
			exprs->data = wrap;
			exprs->next = NULL;
			break;
		}
		// catch exprs empty
		if (exprs->next == NULL && binds->next != NULL) {
			printf("expr is NULL\n");
			char * bind = ((MalType *)binds->next->data)->value.SymbolValue;
			if ( strcmp(bind, "&") != 0 ) {
				printf("not enough args passed to the function\n");
				return AST;
			}
			// skip the & in the binds list
			binds = binds->next;
			binds->data = binds->next->data;


			// if there is no exprs, still wrap a empty list (node_t *) in a MalType->type == MAL_LIST
			MalType * wrap = GC_MALLOC(sizeof(MalType));
			wrap->type = MAL_LIST;
			wrap->value.ListValue = GC_MALLOC(sizeof(node_t));
			// create a empty list
			node_t * list = GC_MALLOC(sizeof(node_t));
			list->data = NULL;
			list->next = NULL;

			// put empty list in the wrapper
			memcpy(wrap->value.ListValue, list, sizeof(node_t));

			// put the wrapper in the exprs list
			exprs->next = GC_MALLOC(sizeof(node_t));
			exprs->next->data = wrap;
			break;
		}

		binds = binds->next;
		exprs = exprs->next;
	}

	// TCO
	*ASTp = f->ast;
	*envp = create_env(f->env, f->param, args);
	return NULL;
}
MalType * EVAL_LIST_CORE_FN(MalType ** ASTp, env_t ** envp, node_t * element, MalType * evaluated_first_element) {
	MalType * AST = *ASTp;
	env_t * env = *envp;

	MalType * core_fn = evaluated_first_element;
	if (core_fn->value.CoreFnValue == NULL) {
		fprintf(stderr, "core_fn is null\n");
		return AST;
	}

	// skip symbol
	// this can make element NULL
	element = element->next;

	// add the rest of the list to a list of evaluated things

	// define a list of evaluated element
	node_t * evaluated_list = GC_MALLOC(sizeof(node_t));
	evaluated_list->data = NULL;
	evaluated_list->next = NULL;

	// evaluate each element in the current list and add them
	while (element != NULL) {
		MalType * evaluated_element = EVAL(element->data, env);
		//if (evaluated_element->type == MAL_LIST) {
		//	// fix VECTORS
		//	evaluated_element->type = ((MalType *)element->data)->type;
		//}

		if (evaluated_element == NULL) {
			// just skip it
			continue;
		}
		append(evaluated_list, (void *)evaluated_element, sizeof(MalType));

		element = element->next;
	};

	MalType * result = (*(MalCoreFn)core_fn->value.CoreFnValue)(evaluated_list);
	return result;

}

MalType * EVAL_LIST(MalType ** ASTp, env_t ** envp, int vector) {
	// trick that allow modifing the AST and env of EVAL from here
	MalType * AST = *ASTp;
	env_t * env = *envp;

	node_t * element = AST->value.ListValue;
	// if the list is empty
	if (element->data == NULL) {
		printf("list is empty\n");
		return AST;
	}

	//////////////////////////////////
	// eval first element
	MalType * first_element = element->data;
	if (first_element == NULL) {
		printf("first element is NULL\n");
		return AST;
	}
	MalType * evaluated_first_element = EVAL(first_element, env);
	if (evaluated_first_element == NULL) {
		printf("firs element eval returne null\n");
		return AST;
	}
	//////////////////////////////////

	switch (evaluated_first_element->type) {
		case MAL_FN_WRAPER: {
			// EVAL handle NULL for TCO and non-NULL for recurtion
			return EVAL_LIST_FN_WRAPPER(ASTp, envp, element, evaluated_first_element);
		}
		case MAL_CORE_FN: {
			return EVAL_LIST_CORE_FN(ASTp, envp, element, evaluated_first_element);
		}
		case MAL_SYMBOL: {
			// intentional fall through
			// special forms
			char * symbol = ((MalType *)(element->data))->value.SymbolValue;
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
				MalSymbol * key = ((MalType *)(element->next->data))->value.SymbolValue;
				MalType * value = EVAL(element->next->next->data, env);
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
				env_t * new_env = GC_MALLOC(sizeof(env_t));
				new_env->outer = env;
				map_t * map = GC_MALLOC(sizeof(map_t));
				new_env->data = map;

				// define the binding list
				MalList * bindings_list = NULL;
				if ( ((MalType *)(element->next->data))->type == MAL_LIST ||  ((MalType *)(element->next->data))->type == MAL_VECTOR) {
					bindings_list = ((MalType *)(element->next->data))->value.ListValue;
				} else {
					printf("let* takes a list or a vector as arg :/\n");
					return AST;
				}

				// evaluate the binding list
				node_t * binding = bindings_list;
				while (binding != NULL){
					if (binding->next == NULL) {
						printf("binding list is odd\n");
						return AST;
					}

					MalSymbol * key = ((MalType *)(binding->data))->value.SymbolValue;
					MalType * value = EVAL((MalType *)binding->next->data, new_env);
					if (value != NULL) {
						set(new_env, key, value);
					}
					binding = binding->next->next;
				}

				// TCO
				*envp = new_env;
				*ASTp = element->next->next->data;
				return NULL;

				// without TCO :
				// MalType * third_arg = EVAL(element->next->next->data, new_env);
				// return third_arg;
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
				// MalType * evaluated_element;
				while (element->next != NULL) {
					// evaluated_element = EVAL(element->data, env);
					EVAL(element->data, env);
					element = element->next;
				};


				*ASTp = element->data;
				return NULL;

				// return evaluated_element;
			}
			// if
			if (strcmp(symbol, "if") == 0) {
				if (element->next == NULL) {
					printf("if shall take at least one arg");
					MalType * nil_ret = GC_MALLOC(sizeof(MalType));
					nil_ret->type = MAL_NIL;
					return nil_ret;
				}
				MalType * condition = EVAL(element->next->data, env);
				if (condition->type == MAL_FALSE || condition->type == MAL_NIL || condition == NULL) {
					// condition is false
					if (element->next->next->next == NULL) {
						MalType * nil_ret = GC_MALLOC(sizeof(MalType));
						nil_ret->type = MAL_NIL;
						return nil_ret;
					}
					*ASTp = element->next->next->next->data;
					return NULL;
				} else {
					// condition was true
					if (element->next->next == NULL) {
						printf("if without body\n");
						MalType * nil_ret = GC_MALLOC(sizeof(MalType));
						nil_ret->type = MAL_NIL;
						return nil_ret;
					}
					*ASTp = element->next->next->data;
					return NULL;
				}
			}
			// fn*
			if (strcmp(symbol, "fn*") == 0) {
				if (element->next == NULL) {
					printf("fn shall take at least one arg");
					MalType * nil_ret = GC_MALLOC(sizeof(MalType));
					nil_ret->type = MAL_NIL;
					return nil_ret;
				}
				if (((MalType *)element->next->data)->type != MAL_LIST && ((MalType *)element->next->data)->type != MAL_VECTOR) {
					printf("fn parameters should be a list or a vector\n");
					MalType * nil_ret = GC_MALLOC(sizeof(MalType));
					nil_ret->type = MAL_NIL;
					return nil_ret;
				}
				if (element->next == NULL) {
					printf("fn dont have a body rn :/ \n");
					MalType * nil_ret = GC_MALLOC(sizeof(MalType));
					nil_ret->type = MAL_NIL;
					return nil_ret;
				}

				// create the function
				MalFn * function = GC_MALLOC(sizeof(MalFn));
				MalType * parameters = element->next->data;
				MalType * body = element->next->next->data;

				if (parameters->type != MAL_LIST && (parameters->type != MAL_VECTOR)) {
					printf("args whould be a list or a vector\n");
					return AST;
				}

				// function->param = formated_parameters;
				function->param = parameters;
				function->body = body;
				function->env = env;

				// wrap funtion for TCO
				MalFnWraper * fnwraper = GC_MALLOC(sizeof(MalFnWraper));
				fnwraper->ast = element->next->next->data; // body
				fnwraper->param = element->next->data;
				fnwraper->env = env;
				fnwraper->fn = function;

				// wrap function in MalType
				MalType * mal_type_function = GC_MALLOC(sizeof(MalType));
				mal_type_function->type = MAL_FN_WRAPER;
				mal_type_function->value.FnWraperValue = fnwraper;

				return mal_type_function;
			}

			// fallthrough to default
			// remove the compiler warning
			__attribute__((fallthrough));
		}
		default: {
			// list have nothing special, eval everything and return.
			// define a list of evaluated element
			node_t * evaluated_list = GC_MALLOC(sizeof(node_t));
			evaluated_list->data = NULL;
			evaluated_list->next = NULL;

			// evaluate each element in the current list and add them
			while (element != NULL) {
				MalType * evaluated_element = EVAL(element->data, env);
				if (evaluated_element != NULL) {
					append(evaluated_list, (void *)evaluated_element, sizeof(MalType));
				}
				element = element->next;
			};

			// wrap evaluated list in a MalType
			MalType * wrapper = GC_MALLOC(sizeof(MalType));
			if (vector == 1) {
				wrapper->type = MAL_VECTOR;
			} else {
				wrapper->type = MAL_LIST;
			}
			wrapper->value.ListValue = evaluated_list;
			return wrapper;

		}
	}
}

MalType * EVAL(MalType * AST, env_t * env) {
	while (1 == 1) {
		if (AST == NULL) {
			return AST;
		}
		switch (AST->type) {
			case MAL_SYMBOL: {return EVAL_SYMBOL(AST, env);}
			case MAL_LIST: {
				MalType * ret = GC_MALLOC(sizeof(MalType));
				ret = EVAL_LIST(&AST, &env, 0);
				if (ret == NULL) {
					// TCO
					continue;
				} // else normal recurtion
				return ret;
			}
			case MAL_VECTOR: {
				MalType * ret = GC_MALLOC(sizeof(MalType));
				ret = EVAL_LIST(&AST, &env, 1);
				if (ret == NULL) {
					// TCO
					continue;
				} // else normal recurtion
				return ret;
			}
			case MAL_ATOM: {AST = AST->value.AtomValue; continue;}
			default : {return AST;} // string, int, false, true, nil, fn, corefn, fnwraper, keyword
		}

		fprintf(stderr, "unmatched AST\n");
		return NULL;

	}
}

char* PRINT(MalType * AST) {
	return pr_str(AST, 1);
}


char* rep(char * line, env_t * env){
	return PRINT(EVAL(READ(line), env));
}
