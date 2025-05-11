#include <stdio.h>

char *READ(char *line);
char *EVAL(char *line);
char *PRINT(char *line);
char *rep(char *line);

int main(void) {
    char  *line = NULL;
    size_t len  = 0;

    printf("user> ");
    while (getline(&line, &len, stdin) != -1) {
        printf("%s", rep(line));
        printf("user> ");
    }
}

char *READ(char *line) {
    return line;
}

char *EVAL(char *line) {
    return line;
}

char *PRINT(char *line) {
    return line;
}

char *rep(char *line) {
    return PRINT(EVAL(READ(line)));
}
