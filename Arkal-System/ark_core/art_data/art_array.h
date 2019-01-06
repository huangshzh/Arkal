/*
 * Copyright (C) Arthur
 */

#ifndef __ART_ARRAY_H__
#define __ART_ARRAY_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

typedef struct {
    art_u8_t        *elts;
    size_t          size;
    art_u32_t       nalloc;
    art_u32_t       used;
    art_pool_t      *pool;
} art_array_t;

art_array_t *art_array_creat(art_pool_t *pool, art_u32_t n, size_t size);
void *art_array_push(art_array_t *array);
void *art_array_npush(art_array_t *array, size_t n);
#define art_array_gethead(array)    (array->elts)
#define art_array_nelts(array, n)   (array->elts + (n * array->size))

#endif

