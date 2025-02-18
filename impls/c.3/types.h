#ifndef TYPES_H
#define TYPES_H

#include "linked_list.h"

typedef struct MalType MalType;

typedef enum {
	MAL_SYMBOL,
    MAL_INT,
    MAL_LIST,
    MAL_CORE_FN,
} MalTypeEnum;

typedef signed long MalInt;
typedef char MalSymbol;
typedef node_t MalList;
typedef MalType * (*MalCoreFn)(node_t *);

typedef union {
    MalInt * IntValue;
    MalSymbol * SymbolValue;
    MalList * ListValue;
    MalCoreFn CoreFnValue;
} MalTypeValue;


struct MalType {
    MalTypeEnum type;
    MalTypeValue value;
};


#endif
