/*
 * Copyright (C) Arthur
 */

#ifndef __ART_STR_H__
#define __ART_STR_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

#define ART_INT32_LEN            (sizeof("-2147483648") - 1)
#define ART_INT64_LEN            (sizeof("-9223372036854775808") - 1)

#define ART_MAX_UINT32_VALUE     (art_u32_t) 0xffffffff
#define ART_MAX_INT32_VALUE      (art_u32_t) 0x7fffffff

#define art_strlen(s)            strlen((const char *)s)

#define art_strchr(s1, c)        strchr((const char *)s1, (int)c)
#define art_strstr(s1, s2)       strstr((const char *)s1, (const char *)s2)

#define art_strcmp(s1, s2)       strcmp((const char *)s1, (const char *)s2)
#define art_strncmp(s1, s2, n)   strncmp((const char *)s1, (const char *)s2, n)

#define art_tolower(c)          \
(art_u8_t) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define art_toupper(c)          \
(art_u8_t) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

#define art_string(str)     { sizeof(str) - 1, (art_u8_t *)str }
#define art_null_string     { 0, NULL }

typedef struct {
    size_t      len;
    art_u8_t    *str;
} art_str_t;

/*
 *  %[padding][width][u][x|X][type]
 *  supported formats:
 *  %[0][width|m][u][x|X]i          art_s32_t / art_u32_t       
 *  %[0][width|m][u][x|X]l          art_s64_t / art_u64_t         
 *  %[0][width][.width]f            double, max valid number fits to %18.9f
 *  %p                              void*      / art_uptr_t (pointer)
 *  %s                              null-terminated string
 *  %c                              char       / art_s8_t
 * 
 *  Arthur type:
 *  %V                              art_str_t *
 */

inline void art_str_pset(art_pool_t *pool, art_str_t *p, art_u8_t *str);
inline void art_str_mset(art_str_t *p, art_u8_t *str);
art_u8_t * art_string_core(art_u8_t *buf, art_u8_t *last, \
    const art_u8_t *fmt, va_list args);

art_u8_t * art_fprintf(art_u8_t *buf, size_t max, const art_u8_t *fmt, ...);

/* red-black tree */

typedef struct {
    art_rbtree_node_t         node;
    art_str_t                 str;
} art_str_rbtree_node_t;

void
art_str_rbtree_insert_value(art_rbtree_node_t *temp,
    art_rbtree_node_t *node, art_rbtree_node_t *sentinel);

art_str_rbtree_node_t *
art_str_rbtree_lookup(art_rbtree_t *rbtree, art_str_t *val, art_u32_t hash);
#endif