#ifndef TYPES_H
#define TYPES_H

#include "linked_list.h"

typedef enum {
	MAL_SYMBOL,
    MAL_INT,
    MAL_LIST,
} MalTypeEnum;

typedef long MalInt;
typedef char MalSymbol;
typedef node_t MalList;

typedef union {
    MalInt * IntValue;
    MalSymbol * SymbolValue;
    MalList * ListValue;
} MalTypeValue;


typedef struct {
    MalTypeEnum type;
    MalTypeValue value;
} MalType;


#endif
