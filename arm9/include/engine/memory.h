#ifndef __MEMORY9__
#define __MEMORY9__

#define MAX_MALLOC 512

void *GetStackPointer();

void* alloc(size_t size, state_struct* state);
void* reAlloc(void* p, size_t size, state_struct* s);
void freeState(state_struct* state);
void initMalloc();

#endif
