#ifndef TYPES_H
#define TYPES_H

#include "linked_list.h"

// MalType
typedef struct MalType MalType;

// make it global
extern MalType *global_error;

typedef node_t map_t;

int   map_set(map_t *map, MalType *key, MalType *value);
void *map_get(map_t *map, MalType *key);
int   map_contains(map_t *map, MalType *key);
void  map_remove(map_t *map, MalType *key);

typedef enum {
    MAL_SYMBOL,
    MAL_INT,
    MAL_LIST,
    MAL_VECTOR,
    MAL_STRING,
    MAL_CORE_FN,
    MAL_FN,
    MAL_FN_WRAPER,
    MAL_NIL,
    MAL_TRUE,
    MAL_FALSE,
    MAL_KEYWORD,
    MAL_ATOM,
    MAL_HASHMAP,
} MalTypeEnum;

typedef signed long        MalInt;
typedef char               MalSymbol;
typedef char               MalString;
typedef char               MalKeyword;
typedef node_t             MalList;
typedef node_t             MalVector;
typedef int                MalTrue;
typedef int                MalFalse;
typedef int                MalNil;
typedef map_t              MalHashmap;
typedef struct MalFn       MalFn;
typedef struct MalFnWraper MalFnWrapper;
typedef MalType *(*MalCoreFn)(MalList *);

// MAL_VECTOR use ListValue
// MAL_KEYWORD use SybolValue
// string and symbol could also share the same underlying type, but no.
typedef union MalTypeValue {
    MalInt       *IntValue;
    MalSymbol    *SymbolValue;
    MalList      *ListValue;
    MalCoreFn     CoreFnValue;
    MalFn        *FnValue;
    MalFnWrapper *FnWraperValue;
    MalTrue      *TrueValue;
    MalFalse     *FalseValue;
    MalNil       *NilValue;
    MalString    *StringValue;
    MalType      *AtomValue;
    MalHashmap   *HashmapValue;
    MalKeyword   *KeywordValue;
} MalTypeValue;

struct MalType {
    MalTypeEnum  type;
    MalTypeValue value;
};
////////////////////////

// env
typedef struct env {
    struct env *outer;
    map_t      *data;
} env_t;

// Function
// original fn* return type
struct MalFn {
    MalType *param;
    MalType *body;
    env_t   *env;
};
// new wrapper arount fn*
struct MalFnWraper {
    MalType *ast;
    MalType *param;
    env_t   *env;
    int      is_macro;
    // actual funciton
    struct MalFn *fn;
};

MalType *NewMalSymbol(const char *string);
MalType *NewMalInt(MalInt a);
MalType *NewMalList(MalList *list);
MalType *NewMalListCopy(MalList *list);
MalType *NewMalVector(MalList *list);
MalType *NewMalString(const char *string);
MalType *NewMalKeyword(const char *string);
MalType *NewMalTrue();
MalType *NewMalFalse();
MalType *NewMalNIL();
MalType *NewMalAtom(MalType *MalObject);
MalType *NewMalCoreFn(MalType *(*function)(MalList *));
MalType *NewMalFnWrapper(MalType *parameters, MalType *body, env_t *env);

int IsListOrVector(MalType *AST);
int IsSymbol(MalType *AST);
int IsInt(MalType *AST);
int IsList(MalType *AST);
int IsVector(MalType *AST);
int IsString(MalType *AST);
int IsCoreFn(MalType *AST);
int IsFn(MalType *AST);
int IsFnWrapper(MalType *AST);
int IsNil(MalType *AST);
int IsTrue(MalType *AST);
int IsFalse(MalType *AST);
int IsKeyword(MalType *AST);
int IsAtom(MalType *AST);
int IsHashmap(MalType *AST);

MalList      *GetList(MalType *AST);
MalSymbol    *GetSymbol(MalType *AST);
MalInt       *GetInt(MalType *AST);
MalList      *GetList(MalType *AST);
MalList      *GetVector(MalType *AST);
MalString    *GetString(MalType *AST);
MalCoreFn     GetCoreFn(MalType *AST);
MalFn        *GetFn(MalType *AST);
MalFnWrapper *GetFnWrapper(MalType *AST);
MalKeyword   *GetKeyword(MalType *AST);
MalType      *GetAtom(MalType *AST);
MalHashmap   *GetHashmap(MalType *AST);

#endif
