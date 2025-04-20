#ifndef TYPES_H
#define TYPES_H

#include "linked_list.h"


// MalType
typedef struct MalType MalType;

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
} MalTypeEnum;

typedef signed long MalInt;
typedef char MalSymbol;
typedef char MalString;
typedef node_t MalList;
typedef MalType * (*MalCoreFn)(node_t *);
typedef struct MalFn MalFn;
typedef struct MalFnWraper MalFnWraper;
typedef int MalTrue;
typedef int MalFalse;
typedef int MalNil;

// MAL_VECTOR use ListValue
// MAL_KEYWORD use SybolValue
// string and symbol could also share the same underlying type, but no.
typedef union {
    MalInt * IntValue;
    MalSymbol * SymbolValue;
    MalList * ListValue;
    MalCoreFn CoreFnValue;
    MalFn* FnValue;
    MalFnWraper * FnWraperValue;
    MalTrue * TrueValue;
    MalFalse * FalseValue;
    MalNil * NilValue;
    MalString * StringValue;
} MalTypeValue;


struct MalType {
    MalTypeEnum type;
    MalTypeValue value;
};
////////////////////////


// map
typedef node_t map_t;

// env
typedef struct env {
    struct env * outer;
    map_t * data;
} env_t;

// Function
// original fn* return type
struct MalFn {
    MalType * param;
    MalType * body;
    env_t * env;
};
// new wrapper arount fn*
struct MalFnWraper {
    MalType * ast;
    MalType * param;
    env_t * env;
    // actual funciton
    struct MalFn * fn;
};

#endif
