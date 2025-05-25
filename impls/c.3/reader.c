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
    case '\'': {
        MalType *ret         = GC_MALLOC(sizeof(MalType));
        ret->type            = MAL_LIST;
        ret->value.ListValue = GC_MALLOC(sizeof(node_t));

        // wrap the string "quote" in a MalType
        MalType *quote           = GC_MALLOC(sizeof(MalType));
        quote->type              = MAL_SYMBOL;
        quote->value.SymbolValue = GC_MALLOC(5);
        memcpy(quote->value.SymbolValue, "quote", 5);

        // get the next token and wrap it in a new reader
        char     *next_token = reader_next(reader);
        reader_t *new_reader = GC_MALLOC(sizeof(reader_t));
        new_reader->position = 0;
        new_reader->tokens   = GC_MALLOC(sizeof(node_t));

        new_reader->tokens->data = next_token;
        new_reader->tokens->next = reader->tokens->next;

        append(ret->value.ListValue, quote, sizeof(MalType));
        append(ret->value.ListValue, read_form(new_reader), sizeof(MalType));

        return ret;
    }
    case '@': {
        // get the atom name and wrap it in a new reader
        char     *atom_name  = reader_next(reader);
        reader_t *new_reader = GC_MALLOC(sizeof(reader_t));
        new_reader->position = 0;
        new_reader->tokens   = GC_MALLOC(sizeof(node_t));
        append(new_reader->tokens, atom_name, strlen(atom_name));

        MalType *ret         = GC_MALLOC(sizeof(MalType));
        ret->type            = MAL_LIST;
        ret->value.ListValue = GC_MALLOC(sizeof(node_t));

        // wrap the string "deref" in a MalType
        MalType *deref           = GC_MALLOC(sizeof(MalType));
        deref->type              = MAL_SYMBOL;
        deref->value.SymbolValue = GC_MALLOC(5);
        memcpy(deref->value.SymbolValue, "deref", 5);

        append(ret->value.ListValue, deref, sizeof(MalType));
        append(ret->value.ListValue, read_form(new_reader), sizeof(MalType));

        return ret;
    }
    case '`': {
        MalType *ret         = GC_MALLOC(sizeof(MalType));
        ret->type            = MAL_LIST;
        ret->value.ListValue = GC_MALLOC(sizeof(node_t));

        // wrap the string "quasiquote" in a MalType
        MalType *quasiquote           = GC_MALLOC(sizeof(MalType));
        quasiquote->type              = MAL_SYMBOL;
        quasiquote->value.SymbolValue = GC_MALLOC(10);
        memcpy(quasiquote->value.SymbolValue, "quasiquote", 10);

        // get the next token and wrap it in a new reader
        char     *next_token = reader_next(reader);
        reader_t *new_reader = GC_MALLOC(sizeof(reader_t));
        new_reader->position = 0;
        new_reader->tokens   = GC_MALLOC(sizeof(node_t));

        new_reader->tokens->data = next_token;
        new_reader->tokens->next = reader->tokens->next;

        append(ret->value.ListValue, quasiquote, sizeof(MalType));
        append(ret->value.ListValue, read_form(new_reader), sizeof(MalType));

        return ret;
    }
    case '~': {
        // might be ~ or ~@
        if (strcmp(token, "~@") == 0) {
            // splice-splice-unquote
            MalType *ret         = GC_MALLOC(sizeof(MalType));
            ret->type            = MAL_LIST;
            ret->value.ListValue = GC_MALLOC(sizeof(node_t));

            // wrap the string "splice-unquote" in a MalType
            MalType *splice_unquote           = GC_MALLOC(sizeof(MalType));
            splice_unquote->type              = MAL_SYMBOL;
            splice_unquote->value.SymbolValue = GC_MALLOC(14);
            memcpy(splice_unquote->value.SymbolValue, "splice-unquote", 14);

            // get the next token and wrap it in a new reader
            char     *next_token = reader_next(reader);
            reader_t *new_reader = GC_MALLOC(sizeof(reader_t));
            new_reader->position = 0;
            new_reader->tokens   = GC_MALLOC(sizeof(node_t));

            new_reader->tokens->data = next_token;
            new_reader->tokens->next = reader->tokens->next;

            append(ret->value.ListValue, splice_unquote, sizeof(MalType));
            append(ret->value.ListValue, read_form(new_reader),
                   sizeof(MalType));

            return ret;

        } // if token == ~@
        // else unquote
        MalType *ret         = GC_MALLOC(sizeof(MalType));
        ret->type            = MAL_LIST;
        ret->value.ListValue = GC_MALLOC(sizeof(node_t));

        // wrap the string "unquote" in a MalType
        MalType *unquote           = GC_MALLOC(sizeof(MalType));
        unquote->type              = MAL_SYMBOL;
        unquote->value.SymbolValue = GC_MALLOC(7);
        memcpy(unquote->value.SymbolValue, "unquote", 7);

        // get the next token and wrap it in a new reader
        char     *next_token = reader_next(reader);
        reader_t *new_reader = GC_MALLOC(sizeof(reader_t));
        new_reader->position = 0;
        new_reader->tokens   = GC_MALLOC(sizeof(node_t));

        new_reader->tokens->data = next_token;
        new_reader->tokens->next = reader->tokens->next;

        append(ret->value.ListValue, unquote, sizeof(MalType));
        append(ret->value.ListValue, read_form(new_reader), sizeof(MalType));

        return ret;
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
        printf("hashmap key : %s\n", pr_str(key, 0));

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

        printf("hashmap value : %s\n", pr_str(value, 0));
        map_set(hashmap->value.HashmapValue, key->value.StringValue, value);

        token = reader_next(reader);
    }

    printf("hashmap : %s\n", pr_str(hashmap, 0));
    return hashmap;
}
MalType *read_list(reader_t *reader, int vector) {
    MalType *list = GC_malloc(sizeof(MalType));
    if (vector == 1) {
        list->type = MAL_VECTOR;
    } else {
        list->type = MAL_LIST;
    }
    list->value.ListValue       = GC_malloc(sizeof(MalList));
    list->value.ListValue->next = NULL;
    list->value.ListValue->data = NULL;

    // handle single parentesis input
    if (reader->tokens->next == NULL) {
        printf("only one token\n");
        return list;
    }

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
            printf("last token : '%s'\n", reader_peek(reader));
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

    char *endptr = GC_malloc(strlen(token));
    long  number = strtol(token, &endptr, 10);

    if (endptr != token) {
        atom->type            = MAL_INT;
        atom->value.IntValue  = GC_malloc(sizeof(MalInt));
        *atom->value.IntValue = number;
        return atom;
    }

    // token is not a number
    if (*token == '"') {
        // remove surrounding "
        token += sizeof(char);
        token[strlen(token) - 1] = '\0';

        char *new_string = GC_MALLOC(strlen(token));

        // transform  \" "
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
                    // just skip it ?
                    i_token++;
                }

            } else {
                new_string[i_new_string] = token[i_token];
            }
            i_new_string += 1;
        }

        atom->type              = MAL_STRING;
        atom->value.StringValue = GC_malloc(strlen(token));
        memcpy(atom->value.StringValue, new_string, strlen(new_string));
        return atom;
    }
    if (*token == ':') {
        // remove first :
        token += sizeof(char);
        atom->type              = MAL_KEYWORD;
        atom->value.SymbolValue = GC_MALLOC(strlen(token));
        memcpy(atom->value.SymbolValue, token, strlen(token));
        printf("keyword: %s\n", pr_str(atom, 0));
        return atom;
    }

    // special forms
    // true
    if (strcmp(token, "true") == 0) {
        MalType *true = GC_MALLOC(sizeof(MalType));
        true->type    = MAL_TRUE;
        return true;
    }
    // false
    if (strcmp(token, "false") == 0) {
        MalType *false = GC_MALLOC(sizeof(MalType));
        false->type    = MAL_FALSE;
        return false;
    }
    // nil
    if (strcmp(token, "nil") == 0) {
        MalType *nil_ret = GC_MALLOC(sizeof(MalType));
        nil_ret->type    = MAL_NIL;
        return nil_ret;
    }

    // if token is not a number nor a string nor a keyword, it's a symbol
    atom->type              = MAL_SYMBOL;
    atom->value.SymbolValue = GC_malloc(strlen(token));
    memcpy(atom->value.SymbolValue, token, strlen(token));

    return atom;
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
