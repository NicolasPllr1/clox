#ifndef clox_chunk_h
#define clox_chunk_h
#include "common.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_ADD,
  OP_SUBSTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_NOT,
  OP_GREATER,
  OP_LESSER,
  OP_EQUAL,
  OP_PRINT,
  OP_RETURN

} OpCode;

typedef struct {
  int count;
  int capacity;

  uint8_t *code;
  ValueArray constants;
  int *lines;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);

#endif
