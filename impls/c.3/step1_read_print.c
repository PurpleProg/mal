#include <stdio.h>


char* READ(char string[]);
char* EVAL(char string[]);
char* PRINT(char string[]);
char* rep(char string[]);


int main(void)
{
    char input[255];

    do
    {
        printf("%s", rep(input));
        printf("user> ");
    } while (fgets(input, 255, stdin));
}


char* READ(char string[])
{
    return string;
}


char* EVAL(char string[])
{
    return string;
}


char* PRINT(char string[])
{
    return string;
}


char* rep(char string[])
{
    return PRINT(EVAL(READ(string)));
}
