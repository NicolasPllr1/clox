#ifndef clox_debug_h
#define clox_debug_h
#include "chunk.h"

void disAssembleChunk(Chunk *chunk, const char *name);
int disAssembleInstruction(Chunk *chunk, int offset);

#endif
