#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memory.h"
#include "table.h"
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

/// Assumes the entry key is NULL.
/// Such entries are either tombstones - boolean (true) values - or empty cells
/// - nil values.
static inline bool is_tombstone(Entry *entry) { return IS_BOOL(entry->value); }

static inline Entry *findEntry(Entry *entries, int capacity, ObjString *key) {
  uint32_t index = key->hash % capacity;
  Entry *tombstone = NULL;

  for (;;) {
    Entry *entry = &entries[index];

    if (entry->key == NULL) {
      if (is_tombstone(entry) && tombstone == NULL) {
        tombstone = entry; // Record the _first_ tombstone we encounter
      } else {
        // Found an empty cell.
        // If a tombstone was found earlier, return it instead.
        return tombstone == NULL ? entry : tombstone;
      }
    }
    // NOTE: '==' string comparison
    if (entry->key == key) {
      // Hit! Target key is found
      return entry;
    }
    index = (index + 1) % capacity;
  }
}

/// Read only search. Looking in the table for the exact `chars` key.
ObjString *tableFindString(Table *table, const char *chars, int length,
                           int hash) {
  if (table->count == 0)
    return NULL;

  int capacity = table->capacity;
  Entry *entries = table->entries;

  uint32_t index = hash % capacity;

  for (;;) {
    Entry *entry = &entries[index];

    if (entry->key == NULL && IS_NIL(entry->value)) {
      // Stop if we find an empty non-tombstone entry.
      return NULL;
    } else if (entry->key->length == length && entry->key->hash == hash &&
               memcmp(entry->key, chars, length) == 0) {
      // We found it.
      return entry->key;
    }
    index = (index + 1) % capacity;
  }
}

static inline void adjust_capacity(Table *table, int new_capacity) {
  Entry *new_entries = ALLOCATE(Entry, new_capacity);

  for (int i = 0; i < new_capacity; i++) {
    new_entries[i].key = NULL;
    new_entries[i].value = NIL_VAL;
  }

  for (int i = 0; i < table->capacity; i++) {
    Entry *entry = &table->entries[i];
    if (entry->key == NULL)
      continue;
    table->count = 0;

    Entry *dest = findEntry(new_entries, new_capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

  FREE_ARRAY(Entry, table->entries, table->capacity);

  table->entries = new_entries;
  table->capacity = new_capacity;
}

bool tableSet(Table *table, ObjString *key, Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int new_capacity = GROW_CAPACITY(table->capacity);
    adjust_capacity(table, new_capacity);
  }
  Entry *entry = findEntry(table->entries, table->capacity, key);
  bool newEntry = entry->key == NULL;
  bool wasNotTombstone = !is_tombstone(entry);
  if (newEntry && wasNotTombstone) {
    table->count++;
  }
  entry->key = key;
  entry->value = value;
  return newEntry;
}

void tableAddAll(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; i++) {
    Entry *entry = &from->entries[i];
    if (entry->key != NULL) {
      tableSet(to, entry->key, entry->value);
    }
  }
}

bool tableGet(Table *table, ObjString *key, Value *value) {
  if (table->count == 0)
    return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  *value = entry->value;
  return true;
}

bool tableDelete(Table *table, ObjString *key) {
  if (table->count == 0)
    return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  if (entry->key == NULL)
    return false;

  // Change the entry to a 'tombstone'
  entry->key = NULL;
  entry->value = BOOL_VAL(true);
  return true;
}
