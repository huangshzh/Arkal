/*
 * Copyright (C) Arthur
 */

#ifndef __ART_LIST_H__
#define __ART_LIST_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

typedef struct art_list_part_s  art_list_part_t;

struct art_list_part_s {
    void                *elts;
    art_list_part_t     *next;
};

typedef struct {
    art_list_part_t     *freeindex;
    art_list_part_t     *last;
    art_list_part_t     *head;
    size_t              size;
    art_u32_t           nalloc;
    art_pool_t          *pool;
} art_list_t;

art_list_t *art_list_create(art_pool_t *pool, art_u32_t n, size_t size);
void *art_list_push(art_list_t *list);
art_state_t art_list_delete(art_list_t *list, void *d);
#endif