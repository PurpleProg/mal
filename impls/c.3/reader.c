#include "reader.h"
#include "gc.h"
#include "linked_list.h"
#include "printer.h"
#include "types.h"
#include <gc/gc.h>
#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MalType *read_str(char *string) {

    reader_t *reader = GC_malloc(sizeof(reader_t));
    reader->position = 0;
    reader->tokens   = tokenize(string);

    return read_form(reader);
}

MalType *read_form(reader_t *reader) {

    char *token = reader_peek(reader);

    // dereferencing token to get the first char
    switch (*token) {
    case '(': return read_list(reader, 0);
    case '[': return read_list(reader, 1);
    case '{': return read_hashmap(reader);
    /* ############ READER MACRO ############## */
    case '@': {
        reader_next(reader); // skip '
        MalList *list = GC_MALLOC(sizeof(MalList));
        append(list, NewMalSymbol("deref"), sizeof(MalType));
        append(list, read_form(reader), sizeof(MalType));

        return NewMalList(list);
    }
    case '\'': {
        reader_next(reader); // skip '
        MalList *list = GC_MALLOC(sizeof(MalList));
        append(list, NewMalSymbol("quote"), sizeof(MalType));
        append(list, read_form(reader), sizeof(MalType));

        return NewMalList(list);
    }
    case '`': {
        reader_next(reader); // skip `
        MalList *list = GC_MALLOC(sizeof(MalList));
        append(list, NewMalSymbol("quasiquote"), sizeof(MalType));
        append(list, read_form(reader), sizeof(MalType));

        return NewMalList(list);
    }
    case '~': {
        // might be ~or ~@
        if (strcmp(token, "~@") == 0) {
            reader_next(reader); // skip ~@
            MalList *list = GC_MALLOC(sizeof(MalList));
            append(list, NewMalSymbol("splice-unquote"), sizeof(MalType));
            append(list, read_form(reader), sizeof(MalType));

            return NewMalList(list);
        }
        reader_next(reader); // skip ~
        MalList *list = GC_MALLOC(sizeof(MalList));
        append(list, NewMalSymbol("unquote"), sizeof(MalType));
        append(list, read_form(reader), sizeof(MalType));

        return NewMalList(list);
    }

    default: return read_atom(reader);
    }
}

MalType *read_hashmap(reader_t *reader) {
    MalType *hashmap            = GC_malloc(sizeof(MalType));
    hashmap->type               = MAL_HASHMAP;
    hashmap->value.HashmapValue = GC_malloc(sizeof(MalHashmap));

    // handle single parentesis input
    if (reader->tokens->next == NULL) {
        printf("read hashmap only one token\n");
        return hashmap;
    }

    char *token = reader_next(reader);

    char *end_token = "}";
    while (strcmp(token, end_token) != 0) {

        // if the tokens dont have a matching end token
        // or reach end of file
        if (reader->tokens->next == NULL) {
            printf("last token : '%s'\n", reader_peek(reader));
            printf("unbalanced\n");
            return hashmap;
        }

        if (*token == ';') {
            // skip comments
            token = reader_next(reader);
            continue;
        }

        MalType *key = GC_malloc(sizeof(MalType));
        key          = read_form(reader);

        // append the ret of read_form to the current hashmap
        // that is the key
        if (!IsString(key) && !IsKeyword(key)) {
            printf("key can only be string or keyword\n");
            return hashmap;
        }

        token = reader_next(reader);

        /********* after key, repeat for the value ***********/

        // if the tokens dont have a matching end token
        // or reach end of file
        if (reader->tokens->next == NULL) {
            printf("last token : '%s'\n", reader_peek(reader));
            printf("unbalanced\n");
            return hashmap;
        }

        if (*token == ';') {
            // skip comments
            token = reader_next(reader);
            continue;
        }
        MalType *value = GC_malloc(sizeof(MalType));
        value          = read_form(reader);

        map_set(hashmap->value.HashmapValue, key, value);

        token = reader_next(reader);
    }

    return hashmap;
}
MalType *read_list(reader_t *reader, int vector) {
    MalType *list;
    if (vector == 1) {
        list = NewMalVector(GC_MALLOC(sizeof(MalList)));
    } else {
        list = NewMalList(GC_MALLOC(sizeof(MalList)));
    }

    // handle single parentesis input
    if (reader->tokens->next == NULL) {
        printf("only one token\n");
        return list;
    }

    // skip ( or [
    char *token = reader_next(reader);

    char *end_token;
    if (vector == 1) {
        end_token = "]";
    } else {
        end_token = ")";
    }
    while (strcmp(token, end_token) != 0) {

        // if the tokens dont have a matching end token
        // or reach end of file
        if (reader->tokens->next == NULL) {
            // TODO: raise error
            printf("unbalanced\n");
            return list;
        }

        if (*token == ';') {
            // skip comments
            token = reader_next(reader);
            continue;
        }

        MalType *new_form = GC_malloc(sizeof(MalType));
        new_form          = read_form(reader);

        // append the ret of read_form to the current MalList
        append(list->value.ListValue, (void *)new_form, sizeof(MalType));

        token = reader_next(reader);
    }

    return list;
}
MalType *read_atom(reader_t *reader) {
    MalType *atom = GC_malloc(sizeof(MalType));

    char *token = reader_peek(reader);

    // check if token is a number
    char *endptr = GC_malloc(strlen(token));
    long  number = strtol(token, &endptr, 10);

    if (endptr != token) {
        return NewMalInt(number);
    }
    // token is not a number

    if (*token == '"') {
        // remove surrounding "
        token += sizeof(char);
        token[strlen(token) - 1] = '\0';

        char *new_string = GC_MALLOC(strlen(token));

        // apply \ patterns
        int i_new_string = 0;
        for (unsigned long i_token = 0; i_token < strlen(token); i_token++) {
            if (token[i_token] == '\\') {
                if (token[i_token + 1] == '\\') {
                    // add the second / and skip the first /
                    new_string[i_new_string] = token[i_token + 1];
                    i_token++;
                } else if (token[i_token + 1] == 'n') {
                    new_string[i_new_string] = '\n';
                    i_token++;
                } else if (token[i_token + 1] == '\"') {
                    new_string[i_new_string] = '\"';
                    i_token++;
                } else {
                    // unreconnized /patern
                    // TODO: raise error
                    // just skip it ?
                    i_token++;
                }

            } else {
                new_string[i_new_string] = token[i_token];
            }
            i_new_string += 1;
        }

        return NewMalString(new_string);
    }
    if (*token == ':') {
        // remove first :
        token += sizeof(char);
        return NewMalKeyword(token);
    }

    // special forms
    if (strcmp(token, "true") == 0) {
        return NewMalTrue();
    }
    if (strcmp(token, "false") == 0) {
        return NewMalFalse();
    }
    if (strcmp(token, "nil") == 0) {
        return NewMalNIL();
    }

    // if token is not a number nor a string nor a keyword
    // it must be a symbol
    return NewMalSymbol(token);
}

char *reader_next(reader_t *reader) {
    reader->position += 1;
    // pop can make tokens NULL
    pop(&(reader->tokens));
    return reader->tokens->data;
}
char *reader_peek(reader_t *reader) {
    return reader->tokens->data;
}

node_t *tokenize(char *string) {
    const char *pattern = "[\\s,]*(~@|[\\[\\]\\{\\}\\(\\)'`~^@]|\"(?:\\\\.|[^"
                          "\\\\\"])*\"?|;.*|[^\\s\\[\\]\\{\\}\\('\"`,;\\)]*)";

    // create the tokens linked list
    node_t *tokens = (node_t *)GC_malloc(sizeof(node_t));
    if (tokens == NULL) {
        printf("cant allocate tokens\n");
        return NULL;
    }
    tokens->data = NULL;
    tokens->next = NULL;

    // for pcre_compile
    pcre       *re;
    const char *error;
    int         erroffset = 0;

    // for pcre_exec
    int    rc;
    int    ovector[300];
    size_t start_offset = 0;

    // for pcre_get_substring
    const char *matched_string;

    re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {
        printf("compil failed : %s\n", error);
        return NULL;
    }

    while ((rc = pcre_exec(re, NULL,       /*EXTRA*/
                           string,         /*subject*/
                           strlen(string), /*sizeof subject*/
                           start_offset,   /*start_offset*/
                           0,              /*extra options*/
                           ovector, 300)) > 0) {

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
            append(tokens, (void *)matched_string, strlen(matched_string) + 1);

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
