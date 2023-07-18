#ifndef _MEM_H
#define _MEM_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef void *(*AllocInterface)(void *allocator, size_t size);

struct ArenaChunk {
    struct ArenaChunk *next;
    size_t size;
    uint8_t data[];
};
struct Arena {
    struct ArenaChunk *chunks, *current;
    size_t chunk_off;
    struct ArenaChunk first_chunk;
};

struct PoolChunk;
union PoolBlock {
    struct {
        union PoolBlock *next;
    } free;
    struct {
        struct PoolChunk *chunk;
    } alloced;
};
struct PoolChunk {
    struct PoolChunk *next, *last_free, *next_free;
    union PoolBlock *free_list;
    size_t initial_free_size;
    size_t size;
    uint8_t data[];
};
struct Pool {
    struct PoolChunk *chunks;
    // Blocks allocated from the start of the list
    // Newly free chunks added to end of the list
    struct PoolChunk *free_chunks, *free_chunks_end;
    bool growable;
    size_t elem_size;
    size_t biggest_chunk_size;
    struct PoolChunk first_chunk;
};

void *mem_alloc(size_t size);
void *mem_realloc(void *block, size_t newsize);
void *mem_free(void *block);
AllocInterface mem_alloc_interface(void);

// Will never return a NULL arena
struct Arena *arena_create(size_t initial_size);
void arena_destroy(struct Arena *arena);
void *arena_alloc(struct Arena *arena, size_t size);
void arena_reset(struct Arena *arena);
AllocInterface arena_alloc_interface(void);

// Will never return NULL pool
struct Pool *pool_create(size_t initial_size, size_t elem_size, bool growable);
void pool_destroy(struct Pool *pool);
void *pool_alloc(struct Pool *pool);
void *pool_free(struct Pool *pool, void *block);

#endif
