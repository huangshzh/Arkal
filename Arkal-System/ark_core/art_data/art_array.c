/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

static inline art_state_t
art_array_init(art_array_t *array, art_pool_t *pool, art_u32_t n, size_t size) {
    array->elts = art_palloc(pool, n * size, 1);
    if (array->elts == NULL) {
        return ART_ERROR;
    }
    array->size = size;
    array->nalloc = n;
    array->used = 0;
    array->pool = pool;
    return ART_OK;
}

art_array_t *
art_array_creat(art_pool_t *pool, art_u32_t n, size_t size) {
    art_array_t     *array;
    array = art_pnalloc(pool, sizeof(art_array_t), 0);
    if (array == NULL) {
        return NULL;
    }

    if (art_array_init(array, pool, n, size) != ART_OK) {
        return NULL;
    }

    return array;
}

void *
art_array_push(art_array_t *array) {
    art_u8_t        *p;
    if (array->used >= array->nalloc) {
        /* the 'elts' is full, allocate a new space */
        p = art_pnalloc(array->pool, array->size * array->nalloc * 2, 1);
        if (p == NULL) {
            return NULL;
        }
        
        if (array->elts + array->size * array->nalloc != p) {
            /* normal state, we need move data */
            art_memcpy(p, array->elts, array->nalloc * array->size);
            array->elts = p;
            array->nalloc *= 2;
        } else {
            /* when the original space and new space are continuous*/
            array->nalloc += 2 * array->nalloc;
        }
    }

    p = array->elts + array->used * array->size;
    array->used += 1;
    
    return p;
}

void *
art_array_npush(art_array_t *array, size_t n) {
    art_u8_t        *p;
    size_t          m;
    if(array->used + n > array->nalloc) {
        for (m = 2; array->used + n > array->nalloc * m; m++);

        p = art_pnalloc(array->pool, array->size * array->nalloc * m, 1);
        if (p == NULL) {
            return NULL;
        }

        if (array->elts + array->size * array->nalloc != p) {
            art_memcpy(p, array->elts, array->nalloc * array->size);
            array->elts = p;
            array->nalloc *= m;
        } else {
            array->nalloc += m * array->nalloc;
        }
    }

    p = array->elts + array->used * array->size;
    array->used += n;
    
    return p;
}