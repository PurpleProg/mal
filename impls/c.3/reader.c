#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include "gc.h"
#include "linked_list.h"
#include "reader.h"
#include "types.h"


MalType * read_str(char * string) {

	reader_t * reader = GC_malloc(sizeof(reader_t));
	reader->position = 0;
	reader->tokens = tokenize(string);

	return read_form(reader);
}


MalType * read_form(reader_t * reader) {

	char * token = (char *)reader->tokens->data;

	// dereferencing token to get the first char
	switch (*token) {
		case '(':
			return read_list(reader, 0);
		case '[':
			return read_list(reader, 1);
		//case '"':
			//return read_list(reader, 1);
		default:
			return read_atom(reader);
	}
}

MalType * read_list(reader_t * reader, int vector) {
	MalType * list = GC_malloc(sizeof(MalType));
	if (vector == 1) {
		list->type = MAL_VECTOR;
	} else {
		list->type = MAL_LIST;
	}
	list->value.ListValue = GC_malloc(sizeof(MalList));

	list->value.ListValue->next = NULL;
	list->value.ListValue->data = NULL;

	// handle single parentesis input
	if (reader_peek(reader)->next == NULL) {
		printf("only one token\n");
		printf("how to do error handeling in C ?\n");
		return list;
	}

	char * token = (char *)reader_next(reader)->data;
	char * end_token;
	if (vector == 1) {
		end_token = "]";
	} else {
		end_token = ")";
	}
	while (strcmp(token, end_token) != 0) {

		MalType * new_form = GC_malloc(sizeof(MalType));
		new_form = read_form(reader);
		// append the ret of read_form to the current MalList
		append(list->value.ListValue, (void *)new_form, sizeof(MalType));
		// if (new_form->type == MAL_INT) {
		// 	printf("read : list appended %ld\n", *(new_form->value.IntValue));
		// }

		// if the tokens dont have a matching closing parentesis
		// if we reach end of file
		if (reader_peek(reader)->next == NULL) {
			printf("missing closing )\n");
			printf("current (last) token : '%s'\n", (char *)reader_peek(reader)->data);
			return list;
		}

		token = reader_next(reader)->data;
	}

	return list;
}


MalType * read_atom(reader_t * reader) {
	MalType * atom = GC_malloc(sizeof(MalType));

	char * token = (char *)reader->tokens->data;

	char * endptr = GC_malloc(strlen(token));
	long number = strtol(token, &endptr, 10);
	if (endptr != token) {
		atom->type = MAL_INT;
		atom->value.IntValue = GC_malloc(sizeof(MalInt));
		*atom->value.IntValue = number;
		return atom;
	}
	// token is not a number
	if (*token == '"') {
		//remove surrounding "
		token += sizeof(char);
		token[strlen(token) -1 ] = '\0';

		atom->type = MAL_STRING;
		atom->value.StringValue = GC_malloc(sizeof(MalString));
		memcpy(atom->value.StringValue, token, strlen(token));
		return atom;
	}
	// if token is not a number nor a string, it's a symbol
	atom->type = MAL_SYMBOL;
	atom->value.SymbolValue = GC_malloc(sizeof(MalSymbol));
	memcpy(atom->value.SymbolValue, token, strlen(token));

	return atom;
}


node_t * reader_next(reader_t * reader) {
	reader->position += 1;
	// pop can make tokens NULL
	pop(&(reader->tokens));
	return reader->tokens;
}

node_t * reader_peek(reader_t * reader) {
	return reader->tokens;
}


node_t * tokenize(char * string) {
    const char	*pattern = "[\\s,]*(~@|[\\[\\]\\{\\}\\(\\)'`~^@]|\"(?:\\\\.|[^\\\\\"])*\"?|;.*|[^\\s\\[\\]\\{\\}\\('\"`,;\\)]*)";

	// create the tokens linked list
	node_t * tokens = (node_t *)GC_malloc(sizeof(node_t));
	if (tokens == NULL) {
		printf("cant allocate tokens\n");
		return NULL;
	}
	tokens->data = NULL;
	tokens->next = NULL;

	// for pcre_compile
	pcre * re;
	const char * error;
	int erroffset = 0;

	// for pcre_exec
	int rc;
	int ovector[300];
	size_t start_offset = 0;

	// for pcre_get_substring
	const char * matched_string;

	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
	if (re == NULL) {
		printf("compil failed : %s\n", error);
		return NULL;
	}

	while((rc = pcre_exec(
			re,
			NULL,			/*EXTRA*/
			string,			/*subject*/
			strlen(string),	/*sizeof subject*/
			start_offset ,	/*start_offset*/
			0,				/*extra options*/
			ovector,
			300)
	) > 0) {

		if (rc == PCRE_ERROR_NOMATCH) {
			printf("No match\n");
		} else if (rc > 0) {
			// syntax of before last arg
			pcre_get_substring(string, ovector, rc, 1, &matched_string);

			if (*matched_string == ' ') {
				// skip token if it is whitespace
				start_offset = ovector[1];
				continue;
			}
			// DEBUG:
			// printf("token : '%s'\n", matched_string);
			append(tokens, (void * )matched_string, strlen(matched_string) + 1);

			pcre_free_substring(matched_string);
		} else {
			printf("Error : %d\n", rc);
			pcre_free(re);
			return NULL;
		}

		start_offset = ovector[1];

		if (start_offset >= strlen(string)) {
			break;
		}
	}

	pcre_free(re);
	return tokens;
}
