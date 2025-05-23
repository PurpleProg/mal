#include "types.h"
#include <gc/gc.h>
#include <string.h>

MalType *NewMalSymbol(const char *string) {
    MalType *result           = GC_MALLOC(sizeof(MalType));
    result->type              = MAL_SYMBOL;
    result->value.SymbolValue = GC_MALLOC(sizeof(MalSymbol));
    memcpy(result->value.SymbolValue, string, sizeof(MalSymbol));
    return result;
}
MalType *NewMalInt(MalInt a) {
    MalType *result        = GC_MALLOC(sizeof(MalType));
    result->type           = MAL_INT;
    result->value.IntValue = GC_MALLOC(sizeof(MalInt));
    memcpy(result->value.IntValue, &a, sizeof(MalInt));
    return result;
}
MalType *NewMalList(MalList *list) {
    MalType *result         = GC_MALLOC(sizeof(MalType));
    result->type            = MAL_LIST;
    result->value.ListValue = GC_MALLOC(sizeof(MalList));
    result->value.ListValue = list;
    return result;
}
MalType *NewMalVector(MalList *list) {
    MalType *result         = GC_MALLOC(sizeof(MalType));
    result->type            = MAL_VECTOR;
    result->value.ListValue = GC_MALLOC(sizeof(MalList));
    memcpy(result->value.ListValue, list, sizeof(MalList));
    return result;
}
MalType *NewMalString(const char *string) {
    MalType *result           = GC_MALLOC(sizeof(MalType));
    result->type              = MAL_STRING;
    result->value.StringValue = GC_MALLOC(strlen(string));
    memcpy(result->value.StringValue, string, strlen(string));
    return result;
}
MalType *NewMalTrue() {
    MalType *result         = GC_MALLOC(sizeof(MalType));
    result->type            = MAL_TRUE;
    result->value.TrueValue = NULL;
    return result;
}
MalType *NewMalFalse() {
    MalType *result          = GC_MALLOC(sizeof(MalType));
    result->type             = MAL_FALSE;
    result->value.FalseValue = NULL;
    return result;
}
MalType *NewMalNIL() {
    MalType *result        = GC_MALLOC(sizeof(MalType));
    result->type           = MAL_NIL;
    result->value.NilValue = NULL;
    return result;
}
MalType *NewMalAtom(MalType *MalObject) {
    MalType *result         = GC_MALLOC(sizeof(MalType));
    result->type            = MAL_ATOM;
    result->value.AtomValue = MalObject;
    return result;
}
