#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include "linked_list.h"


node_t * tokenize(char * string) {
    const char	*pattern = "[\\s,]*(~@|[\\[\\]\\{\\}\\(\\)'`~^@]|\"(?:\\\\.|[^\\\\\"])*\"?|;.*|[^\\s\\[\\]\\{\\}\\('\"`,;\\)]*)";

	node_t * tokens = (node_t *)malloc(sizeof(node_t));
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
	int start_offset = 0;

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
			pcre_get_substring(string, ovector, rc, 1, &matched_string);
			printf("DEBUG : token : '%s'\n", matched_string);
			append(tokens, matched_string, sizeof(matched_string));
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
