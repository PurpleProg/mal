#include "types.h"
#include <gc/gc.h>
#include <stdio.h>
#include <string.h>

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
    result->value.ListValue = list;
    return result;
}
MalType *NewMalListCopy(MalList *list) {
    MalType *result         = GC_MALLOC(sizeof(MalType));
    result->type            = MAL_LIST;
    result->value.ListValue = GC_MALLOC(sizeof(MalList));
    memcpy(result->value.ListValue, list, sizeof(MalList));
    return result;
}
MalType *NewMalVector(MalList *list) {
    MalType *result         = GC_MALLOC(sizeof(MalType));
    result->type            = MAL_VECTOR;
    result->value.ListValue = list;
    return result;
}
MalType *NewMalString(const char *string) {
    MalType *result           = GC_MALLOC(sizeof(MalType));
    result->type              = MAL_STRING;
    result->value.StringValue = GC_MALLOC(strlen(string));
    memcpy(result->value.StringValue, string, strlen(string));
    return result;
}
MalType *NewMalSymbol(const char *string) {
    MalType *result           = GC_MALLOC(sizeof(MalType));
    result->type              = MAL_SYMBOL;
    result->value.SymbolValue = GC_MALLOC(strlen(string));
    memcpy(result->value.SymbolValue, string, strlen(string));
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
MalType *NewMalCoreFunction(MalCoreFn func) {
    MalType *result           = GC_MALLOC(sizeof(MalType));
    result->type              = MAL_CORE_FN;
    result->value.CoreFnValue = func;
    return result;
}

int IsListOrVector(MalType *AST) {
    return AST->type == MAL_LIST || AST->type == MAL_VECTOR;
}
int IsSymbol(MalType *AST) {
    return AST->type == MAL_SYMBOL;
}
int IsInt(MalType *AST) {
    return AST->type == MAL_INT;
}
int IsList(MalType *AST) {
    return AST->type == MAL_LIST;
}
int IsVector(MalType *AST) {
    return AST->type == MAL_VECTOR;
}
int IsString(MalType *AST) {
    return AST->type == MAL_STRING;
}
int IsCoreFn(MalType *AST) {
    return AST->type == MAL_CORE_FN;
}
int IsFn(MalType *AST) {
    return AST->type == MAL_FN;
}
int IsFnWrapper(MalType *AST) {
    return AST->type == MAL_FN_WRAPER;
}
int IsNil(MalType *AST) {
    return AST->type == MAL_NIL;
}
int IsTrue(MalType *AST) {
    return AST->type == MAL_TRUE;
}
int IsFalse(MalType *AST) {
    return AST->type == MAL_FALSE;
}
int IsKeyword(MalType *AST) {
    return AST->type == MAL_KEYWORD;
}
int IsAtom(MalType *AST) {
    return AST->type == MAL_ATOM;
}
int IsHashmap(MalType *AST) {
    return AST->type == MAL_HASHMAP;
}

MalList *GetList(MalType *AST) {
    if (!IsListOrVector(AST)) {
        printf("GetList arg is not a list\n");
        return NULL;
    }
    return AST->value.ListValue;
}
MalSymbol *GetSymbol(MalType *AST) {
    if (!IsSymbol(AST)) {
        printf("GetSymbol arg is not a Symbol\n");
        return NULL;
    }
    return AST->value.SymbolValue;
}
MalInt *GetInt(MalType *AST) {
    if (!IsInt(AST)) {
        printf("GetInt arg is not Int\n");
    }
    return AST->value.IntValue;
}
MalList *GetVector(MalType *AST) {
    if (!IsList(AST)) {
        printf("GetList arg is not List\n");
    }
    return AST->value.ListValue;
}
MalString *GetString(MalType *AST) {
    if (!IsString(AST)) {
        printf("GetString arg is not String\n");
    }
    return AST->value.StringValue;
}
MalCoreFn GetCoreFn(MalType *AST) {
    if (!IsCoreFn(AST)) {
        printf("GetCoreFn arg is not CoreFn\n");
    }
    return AST->value.CoreFnValue;
}
MalFn *GetFn(MalType *AST) {
    if (!IsFn(AST)) {
        printf("GetFn arg is not Fn\n");
    }
    return AST->value.FnValue;
}
MalFnWraper *GetFnWrapper(MalType *AST) {
    if (!IsFnWrapper(AST)) {
        printf("GetFnWraper arg is not FnWraper\n");
    }
    return AST->value.FnWraperValue;
}
MalKeyword *GetKeyword(MalType *AST) {
    if (!IsKeyword(AST)) {
        printf("GetKeyword arg is not Keyword\n");
    }
    return AST->value.KeywordValue;
}
MalType *GetAtom(MalType *AST) {
    if (!IsAtom(AST)) {
        printf("GetType arg is not Type\n");
    }
    return AST->value.AtomValue;
}
MalHashmap *GetHashmap(MalType *AST) {
    if (!IsHashmap(AST)) {
        printf("GetHashmap arg is not Hashmap\n");
    }
    return AST->value.HashmapValue;
}
