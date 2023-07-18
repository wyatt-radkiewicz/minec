#include <assert.h>
#include <stdlib.h>

#include "mem.h"

inline void *mem_alloc(size_t size) {
    assert(size);
    return malloc(size);
}
void *mem_realloc(void *block, size_t newsize) {
    if (!block) {
        assert(newsize);
        return mem_alloc(newsize);
    } else {
        if (!newsize) {
            free(block);
            return NULL;
        } else {
            return realloc(block, newsize);
        }
    }
}
void *mem_free(void *block) {
    assert(block && "Possible double free?");
    free(block);
    return NULL;
}
void *_mem_alloc(void *dummy, size_t size) {
    return mem_alloc(size);
}
AllocInterface mem_alloc_interface(void) {
    return _mem_alloc;
}

struct Arena *arena_create(size_t initial_size) {
    struct Arena *arena = mem_alloc(sizeof(struct Arena) + initial_size);
    assert(arena && "Out of memory!");
    arena->chunks = &arena->first_chunk;
    arena->chunk_off = 0;
    arena->current = arena->chunks;
    arena->first_chunk.next = NULL;
    arena->first_chunk.size = initial_size;
    return arena;
}
void arena_destroy(struct Arena *arena) {
    assert(arena);
    struct ArenaChunk *chunk = arena->chunks;
    while (chunk) {
        struct ArenaChunk *next = chunk->next;
        if (chunk != &arena->first_chunk) mem_free(chunk);
        chunk = next;
    }
    mem_free(arena);
}
void *arena_alloc(struct Arena *arena, size_t size) {
    struct ArenaChunk *chunk, *last;
    void *blk;
    
    size = (size / sizeof(void *) + 1) * sizeof(void *);
    if (arena->current->size - arena->chunk_off < size) {
        arena->chunk_off = 0;

        last = arena->current;
        chunk = last->next;
        while (chunk) {
            if (chunk->size >= size) {
                arena->current = chunk;
                goto found_block;
            }
            last = chunk;
            chunk = chunk->next;
        }

        // Create a new chunk
        size_t chunk_size = last->size * 2;
        if (chunk_size < size)
            chunk_size = size * 2;
        chunk = mem_alloc(sizeof(struct ArenaChunk) + chunk_size);
        chunk->size = chunk_size;
        last->next = chunk;
        arena->current = chunk;
    }

found_block:
    blk = arena->current->data + arena->chunk_off;
    arena->chunk_off += size;
    return blk;
}
void arena_reset(struct Arena *arena) {
    arena->current = &arena->first_chunk;
    arena->chunk_off = 0;
}
AllocInterface arena_alloc_interface(void) {
    return (AllocInterface)arena_alloc;
}

struct Pool *pool_create(size_t initial_size, size_t elem_size, bool growable) {
    assert(initial_size && elem_size);
    const size_t blksz = sizeof(union PoolBlock);
    if (elem_size < blksz) {
        elem_size = blksz;
    } else {
        elem_size = ((elem_size + blksz) / blksz + 1) * blksz;
    }
    
    struct Pool *pool = mem_alloc(sizeof(struct Pool) + initial_size * elem_size);
    pool->chunks = &pool->first_chunk;
    pool->elem_size = elem_size;
    pool->biggest_chunk_size = initial_size;
    pool->growable = growable;
    pool->free_chunks = pool->chunks;
    pool->free_chunks_end = pool->free_chunks;

    pool->first_chunk.next = NULL;
    pool->first_chunk.last_free = NULL;
    pool->first_chunk.next_free = NULL;
    pool->first_chunk.free_list = NULL;
    pool->first_chunk.initial_free_size = 0;
    pool->first_chunk.size = initial_size;

    return pool;
}
void pool_destroy(struct Pool *pool) {
    struct PoolChunk *chunk = pool->chunks;
    while (chunk) {
        struct PoolChunk *next = chunk->next;
        if (chunk != &pool->first_chunk) mem_free(chunk);
        chunk = next;
    }
    mem_free(pool);
}
static void pool_unfree_chunk(struct Pool *pool, struct PoolChunk *chunk) {
    if (chunk->last_free) {
        chunk->last_free->next_free = chunk->next_free;
    } else {
        pool->free_chunks = chunk->next_free;
    }
    if (chunk->next_free) {
        chunk->next_free->last_free = chunk->last_free;
    } else {
        pool->free_chunks_end = chunk->last_free;
    }
}
void *pool_alloc(struct Pool *pool) {
    struct PoolChunk *chunk = pool->free_chunks;
    if (!chunk && pool->growable) {
        // Allocate a new chunk
        pool->biggest_chunk_size *= 2;
        chunk = mem_alloc(sizeof(struct PoolChunk) + pool->biggest_chunk_size * pool->elem_size);
        chunk->next = pool->chunks;
        pool->chunks->next = chunk->next;
        chunk->last_free = NULL; // Add to start of list (lots of free blocks)
        chunk->next_free = pool->free_chunks;
        if (pool->free_chunks) {
            pool->free_chunks->last_free = chunk;
        }
        if (!pool->free_chunks) {
            pool->free_chunks = chunk;
            pool->free_chunks_end = chunk;
        } else if (pool->free_chunks_end == pool->free_chunks) {
            pool->free_chunks_end = pool->free_chunks;
            pool->free_chunks = chunk;
        } else {
            pool->free_chunks = chunk;
        }
    } else if (!chunk && !pool->growable) {
        return NULL;
    }

    union PoolBlock *block;
    if (chunk->initial_free_size < chunk->size) {
        block = (union PoolBlock *)(chunk->data + pool->elem_size * chunk->initial_free_size++);
        if (chunk->initial_free_size == chunk->size) {
            pool_unfree_chunk(pool, chunk);
        }
        block->alloced.chunk = chunk;
        return block + 1;
    } else {
        block = chunk->free_list;
        chunk->free_list = block->free.next;
        if (!chunk->free_list) {
            pool_unfree_chunk(pool, chunk);
        }
        block->alloced.chunk = chunk;
        return block + 1;
    }
}
void *pool_free(struct Pool *pool, void *_block) {
    union PoolBlock *block = (union PoolBlock *)_block - 1;
    struct PoolChunk *chunk = block->alloced.chunk;

    bool was_free_chunk = chunk->initial_free_size < chunk->size || chunk->free_list;
    block->free.next = chunk->free_list;
    chunk->free_list = block;
    bool is_free_chunk = chunk->initial_free_size < chunk->size || chunk->free_list;
    if (!was_free_chunk && is_free_chunk) {
        // Add the chunk to the free chunks list
        if (pool->free_chunks_end) {
            chunk->last_free = pool->free_chunks_end;
            pool->free_chunks_end->next_free = chunk;
            pool->free_chunks_end = chunk;
        } else {
            pool->free_chunks = chunk;
            pool->free_chunks_end = chunk;
        }
        chunk->next_free = NULL;
    }

    return NULL;
}
