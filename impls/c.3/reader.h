#ifndef READER_H
#define READER_H 1

#include "linked_list.h"
#include "types.h"

typedef struct {
    node_t *tokens;
    int     position;
} reader_t;

MalType *read_str(char *string);

MalType *read_form(reader_t *reader);

MalType *read_hashmap(reader_t *reader);
MalType *read_list(reader_t *reader, int vector);
MalType *read_atom(reader_t *reader);

node_t *tokenize(char *string);

char *reader_next(reader_t *reader);
char *reader_peek(reader_t *reader);

#endif
