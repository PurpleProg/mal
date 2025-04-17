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


MalType * READ(char * line);
MalType * EVAL(MalType * AST, env_t * env);
char* PRINT(MalType * AST);
char* rep(char * line, env_t * env);


int main(void) {
	GC_INIT();
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	env_t * repl_env = create_repl();
	printf("%s", pr_env(repl_env));

	printf("user> ");
	while ((read = getline(&line, &len, stdin)) != -1) {
		printf("%s\n", rep(line, repl_env));
		printf("user> ");
	}
}


MalType * READ(char * line) {
	return read_str(line);
}

MalType * EVAL(MalType * AST, env_t * env) {
	if (AST == NULL) {
		return AST;
	}
	switch (AST->type) {
		case MAL_SYMBOL: {
			// NOTE: return the function from the repl env
			MalSymbol * symbol = AST->value.SymbolValue;
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
		case MAL_LIST: {
			// NOTE: call the function from the first element
			// and pass the rest of the list as args
			node_t * element = AST->value.ListValue;
			// if the list is empty
			if (element->data == NULL) {
				return AST;
			}

			//////////////////////////////
			// special forms
			if ( ((MalType *)(element->data))->type == MAL_SYMBOL) {
				MalSymbol * symbol = ((MalType *)(element->data))->value.SymbolValue;
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
					if ( ((MalType *)(element->next->data))->type == MAL_LIST) {
						bindings_list = ((MalType *)(element->next->data))->value.ListValue;
					} else {
						printf("let* takes a list as arg :/\n");
						return NULL;
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

					MalType * third_arg = EVAL(element->next->next->data, new_env);
					return third_arg;
				}
				// do
				if (strcmp(symbol, "do") == 0) {
					if (element->next == NULL) {
						printf("do shall take at least one arg");
						return NULL;
					}

					MalType * mal_list = GC_MALLOC(sizeof(MalType));
					mal_list->type = MAL_LIST;
					mal_list->value.ListValue = element->next;
					return EVAL(mal_list, env);
				}
				// if
				if (strcmp(symbol, "if") == 0) {
					if (element->next == NULL) {
						printf("if shall take at least one arg");
						return NULL;
					}
					MalType * condition = EVAL(element->next->data, env);
					if (condition->type == MAL_FALSE || condition->type == MAL_NIL || condition == NULL) {
						// condition is false
						if (element->next->next->next == NULL) {
							MalType * nil_ret = GC_MALLOC(sizeof(MalType));
							nil_ret->type = MAL_NIL;
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
					if (((MalType *)element->next->data)->type != MAL_LIST) {
						printf("fn parameters should be a list\n");
						return NULL;
					}
					if (element->next == NULL) {
						printf("fn dont have a body rn :/ \n");
						return NULL;
					}

					// create the function
					MalFn * function = GC_MALLOC(sizeof(MalFn));
					MalList * parameters = ((MalType *)element->next->data)->value.ListValue;
					MalType * body = element->next->next->data;

					// param is a MalList of MalType->MalSymbol.
					// create_env expect a MalList of (char * || MalSymbol *)
					// convert it
					MalList * formated_parameters = GC_MALLOC(sizeof(MalList));
					formated_parameters->data = NULL;
					formated_parameters->next = NULL;
					node_t * current_node = parameters;
					while (current_node != NULL && current_node->data != NULL) {
						if (((MalType *)current_node->data)->type != MAL_SYMBOL) {
							printf("args must be symbols\n");
							return NULL;
						}
						append(formated_parameters, ((MalType *)current_node->data)->value.SymbolValue, sizeof(MalTypeValue));
						current_node = current_node->next;
					}

					function->param = formated_parameters;
					function->body = body;
					function->env = env;


					// wrap function in MalType
					MalType * mal_type_function = GC_MALLOC(sizeof(MalType));
					mal_type_function->type = MAL_FN;
					mal_type_function->value.FnValue = function;

					return mal_type_function;
				}
			}
			///////////////////////////////////
			//functions
			//////////////////////////////////
			MalType * first_element = element->data;
			if (first_element == NULL) {
				printf("first element is NULL\n");
				return AST;
			}
			MalType * evaluated_first_element = EVAL(first_element, env);
			if (evaluated_first_element == NULL) {
				return AST;
			}
			if ( evaluated_first_element->type == MAL_FN) {
				MalFn * function = evaluated_first_element->value.FnValue;
				if (element->next == NULL) {
					return AST;
				}
				MalList * arg_list = GC_MALLOC(sizeof(MalList));
				arg_list->data = NULL;
				arg_list->next = NULL;
				// normalize arg into a MalList

				node_t * node = element->next;
				while (node != NULL) {
					MalType * arg = node->data;
					if (arg->type == MAL_LIST) {
						arg = EVAL(arg, env);
					}
					append(arg_list, arg, sizeof(MalType));
					node = node->next;
				}

				// wrap the MalList * arg list into a MalType
				MalType * mal_type_arg_list = GC_MALLOC(sizeof(MalType));
				mal_type_arg_list->type = MAL_LIST;
				mal_type_arg_list->value.ListValue = arg_list;

				// create the env for evaluation
				env_t * new_env = create_env(function->env, (MalList *)function->param, arg_list);
				// DEBUG:
				// printf("created a new function with this env : %s\n", pr_env(new_env));
				return EVAL(function->body, new_env);

			}
			//////////////////////////////////

			// calling eval on the first element of the list should return a MalCoreFn
			MalType * operation = EVAL(element->data, env);
			if (operation == NULL) {
				return NULL;
			}
			if (operation->type != MAL_CORE_FN) {
				return AST;
			}
			if (operation->value.CoreFnValue == NULL) {
				fprintf(stderr, "operation is null\n");
				return AST;
			}
			// skip symbol
			// this can make element NULL
			element = element->next;

			// print the rest of the list elements
			// node_t * copy = element;
			// while (copy != NULL) {
			// 	MalType * args = copy->data;
			// 	printf("args : %ld\n", *(args->value.IntValue));
			// 	copy = copy->next;
			// }

			// add the rest of the list to a list of evaluated things
			node_t * list = GC_MALLOC(sizeof(node_t));
			list->data = NULL;
			list->next = NULL;
			while (element != NULL) {
				MalType * evaluated_element = EVAL(element->data, env);
				if (evaluated_element != NULL) {
					append(list, (void *)evaluated_element, sizeof(MalType));
				}
				element = element->next;
			};

			MalType * result = (*(MalCoreFn)operation->value.CoreFnValue)(list);
			return result;

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
		case MAL_FN: {
			return AST;
		}
	}

	fprintf(stderr, "unmatched AST");
	return NULL;
}


MalType * EVAL_symbol(MalType * AST, env_t * env) {
	// NOTE: return the function from the repl env
	MalSymbol * symbol = AST->value.SymbolValue;
	return get(env, symbol);
}

char* PRINT(MalType * AST) {
	return pr_str(AST);
}


char* rep(char * line, env_t * env){
	return PRINT(EVAL(READ(line), env));
}
