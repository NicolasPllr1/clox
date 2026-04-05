#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "common.h"
#include "memory.h"
#include "value.h"


void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table *table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

static inline Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
  uint32_t hash = key->hash;
  uint32_t index = key->hash % capacity;
  for (;;) {
    Entry* entry = &entries[index];
    if (entry->key == key || entry->key == NULL) {
      return entry;
    }
    index = (index + 1) % capacity;
  }
}

static inline void adjust_capacity(Table* table, int new_capacity) {
  Entry* new_entries = ALLOCATE(Entry, new_capacity);

  for (int i = 0; i < new_capacity; i++) {
    new_entries[i].key = NULL;
    new_entries[i].value = NIL_VAL;
  }

  for (int i = 0;  i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key == NULL) continue;

    Entry* dest = findEntry(new_entries, new_capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);

  table->entries = new_entries;
  table->capacity = new_capacity;

}

bool tableSet(Table* table, ObjString* key, Value value ) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int new_capacity = GROW_CAPACITY(table->capacity);
    adjust_capacity(table, new_capacity);
  }
  Entry *entry = findEntry(table->entries,table->capacity, key);
  bool newEntry = entry->key == NULL;
  if (newEntry) {
    table->count++;
  }
  entry->key = key;
  entry->value = value;
  return newEntry;
}

void tableAddAll(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry* entry = &from->entries[i];
    if (entry->key != NULL) {
      tableSet(to, entry->key, entry->value);
    }
  }
}

bool tableGet(Table *table, ObjString *key, Value* value) {
  if (table->count == 0) return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  *value = entry->value;
  return true;
}

bool tableDelete(Table* table, ObjString* key) {
  if (table->count == 0) return false;

  Entry* entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL) return false;

  // Change the entry to a 'tombstone'
  entry->key = NULL;
  entry->value = BOOL_VAL(true);
  return true;
}
