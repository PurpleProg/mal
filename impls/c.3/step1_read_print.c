#include <stdio.h>
#include <string.h>
#include "reader.h"
#include "linked_list.h"


node_t * READ(char * line);
node_t * EVAL(node_t * tokens);
char* PRINT(node_t * tokens);
char* rep(char * line);


int main(void) {
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    printf("user> ");
    while ((read = getline(&line, &len, stdin)) != -1) {
        // replace newline char wih string terminator
        if (line[read - 1] == '\n') {
            line[read -1] = '\0';
        }
        printf("%s\n", rep(line));
        printf("user> ");
    }
}


node_t * READ(char * line) {
    return tokenize(line);
}


node_t * EVAL(node_t * tokens) {
    return tokens;
}


char* PRINT(node_t * tokens) {
    // PLACEHOLDER or something idk

    int total_lenght = 0;
    node_t * node = tokens;

    // calc lenght of line
    while (node != NULL) {
        total_lenght += strlen((char *)node->data);
        node = node->next;
    }

    char * line = malloc(total_lenght + 1);
    if (line == NULL) {
        fprintf(stderr, "cant allocate line buffer");
        return NULL;
    }
    line[0] = '\0';

    // catenate tokens in line
    node = tokens;
    while (node != NULL) {
        strcat(line, (char *)node->data);
        node = node->next;
    }

    return line;
}


char* rep(char * line) {
    return PRINT(EVAL(READ(line)));
}
