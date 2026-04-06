#ifndef clox_table_h
#define clox_table_h
#include "common.h"
#include "value.h"

// < 1 to ensure we always have empty buckets in the table
#define TABLE_MAX_LOAD 0.75

typedef struct {
  ObjString *key;
  Value value;

} Entry;

typedef struct {
  int count;
  int capacity;
  Entry *entries;
} Table;

void initTable(Table *table);
void FreeTable(Table *table);
bool tableSet(Table *table, ObjString *key, Value value);
bool tableGet(Table *table, ObjString *key, Value *value);
bool tableDelete(Table *table, ObjString *key);
void tableAddAll(Table *from, Table *to);

#endif
