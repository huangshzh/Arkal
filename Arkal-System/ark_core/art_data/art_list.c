/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

static inline art_state_t
art_list_init(art_list_t *list, art_pool_t *pool, art_u32_t n, size_t size);

static inline art_state_t
art_list_init(art_list_t *list, art_pool_t *pool, art_u32_t n, size_t size) {
    art_u32_t           i;
    art_list_part_t     *index;

    list->freeindex = art_palloc(pool, n * sizeof(art_list_part_t), 1);
    if (list->freeindex == NULL) {
        return ART_ERROR;
    }

    index = list->freeindex;
    for (i=0; i < (n - 1); i++) {
        index[i].next = &(index[i+1]);
    }

    list->last = NULL;
    list->head = NULL;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return ART_OK;
}

art_list_t *
art_list_create(art_pool_t *pool, art_u32_t n, size_t size) {
    art_list_t  *list;

    list = art_palloc(pool, sizeof(art_list_t), 1);
    if (list == NULL) {
        return NULL;
    }

    if (art_list_init(list, pool, n, size) != ART_OK) {
        return NULL;
    }

    return list;
}

/* 
 * I want to hide detail in my code, so 'art_list_push' return the 'void *'
 * point to the data address
 */
void *
art_list_push(art_list_t *list) {
    art_list_part_t  *index;
    art_u32_t   i;
    
    if (list->freeindex == NULL) {
        list->freeindex = \
        art_palloc(list->pool, list->nalloc * sizeof(art_list_part_t), 1);
        if (list->freeindex == NULL) {
            return NULL;
        }

        index = list->freeindex;
        for (i=0; i < (list->nalloc - 1); i++) {
            index[i].next = &(index[i+1]);
        }

        list->nalloc += list->nalloc;
    }

    index = list->freeindex;
    list->freeindex = index->next;
    index->next = NULL;
    if (index->elts == NULL) {
        index->elts = art_palloc(list->pool, list->size, 1);
        if (index->elts == NULL) {
            return NULL;
        }
    }

    if (list->last) {
        list->last->next = index;
        list->last = index;
    } else {
        list->last = index;
        list->head = index;
    }
    return index->elts;
}

/*
 * the same to 'art_list_push', input 'void *' that point to a data which
 * in argument 'list'
 */
art_state_t 
art_list_delete(art_list_t *list, void *d) {
    art_list_part_t  *index, *pre;
    pre = list->head;
    if (pre && pre->elts == d) {
        if (pre == list->last) {
            list->last = pre->next;
        }
        list->head = pre->next;
        pre->next = list->freeindex;
        list->freeindex = pre;
        return ART_OK;
    }
    
    for(index = pre->next; index; pre = index, index = index->next) {
        if (index->elts == d) {
            if (index == list->last) {
                list->last = pre;
            }
            pre->next = index->next;
            index->next = list->freeindex;
            list->freeindex = index;
            return ART_OK; 
        }
    }
    return ART_ERROR;
}









