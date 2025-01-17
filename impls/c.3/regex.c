#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pcre.h>


int main() {
	const char	*pattern = "[A-Z][a-z]*";
	const char	*string = "Hello, World !";

	// for pcre_compile
	pcre * re;
	const char * error;
	int erroffset = 0;

	// for pcre_exec
	int rc;
	int ovector[30];
	int start_offset = 0;

	// for pcre_get_substring
	const char * matched_string;

	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
	if (re == NULL) {
		printf("compil failed : %s\n", error);
		return 1;
	}

	while(
		(rc = pcre_exec(
			re,
			NULL,				/*EXTRA*/
			string,			/*subject*/
			strlen(string),	/*sizeof subject*/
			start_offset ,					/*start_offset*/
			0,					/*extra options*/
			ovector,
			30)
	) > 0) {

		if (rc == PCRE_ERROR_NOMATCH) {
			printf("No match\n");
		} else if (rc > 0) {

			for (int i = 0; i < rc; i++) {
				pcre_get_substring(string, ovector, rc, i, &matched_string);
				printf("%s\n", matched_string);
				pcre_free_substring(matched_string);
			}
		} else {
			printf("Error : %d\n", rc);
			pcre_free(re);
			return 1;
		}

	start_offset = ovector[1];
	}



	pcre_free(re);
	return 0;
}
