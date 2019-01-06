/*
 * Copyright (C) Arthur
 */

#ifndef __ART_RBTREE_H__
#define __ART_RBTREE_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

typedef art_u32_t       art_rbtree_key_t;
typedef art_s32_t       art_rbtree_key_int_t;

typedef struct art_rbtree_node_s        art_rbtree_node_t;   

struct art_rbtree_node_s {
    art_rbtree_key_t        key;
    art_rbtree_node_t       *left;
    art_rbtree_node_t       *right;
    art_rbtree_node_t       *parent;
    art_u8_t                color;
    art_u8_t                data;
};

typedef void (*art_rbtree_insert_pt) (art_rbtree_node_t *root,
    art_rbtree_node_t *node, art_rbtree_node_t *sentinel);

typedef struct {
    art_rbtree_node_t     *root;
    art_rbtree_node_t     *sentinel;
    art_rbtree_insert_pt   insert;
} art_rbtree_t;

#define art_rbt_red(node)               ((node)->color = 1)         /* red */
#define art_rbt_black(node)             ((node)->color = 0)         /* black */
#define art_rbt_is_red(node)            ((node)->color)
#define art_rbt_is_black(node)          (!art_rbt_is_red(node))

#define art_rbt_copy_color(n1, n2)      (n1->color = n2->color)
#define art_rbt_copy_left(n1, n2)       (n1->left = n2->left)
#define art_rbt_copy_right(n1, n2)      (n1->right = n2->right)
#define art_rbt_copy_parent(n1, n2)     (n1->parent = n2->parent)


/* a sentinel must be black */
#define art_rbtree_sentinel_init(node)  art_rbt_black(node)

#define art_rbtree_init(tree, s, i)                                           \
    art_rbtree_sentinel_init(s);                                              \
    (tree)->root = s;                                                         \
    (tree)->sentinel = s;                                                     \
    (tree)->insert = i

void 
art_rbtree_insert_value(art_rbtree_node_t *temp, art_rbtree_node_t *node,
    art_rbtree_node_t *sentinel);
void 
art_rbtree_insert_timer_value(art_rbtree_node_t *temp, art_rbtree_node_t *node,
    art_rbtree_node_t *sentinel);

void art_rbtree_insert(art_rbtree_t *tree, art_rbtree_node_t *node);
void art_rbtree_delete(art_rbtree_t *tree, art_rbtree_node_t *node);

inline art_rbtree_node_t *
art_rbtree_minimum(const art_rbtree_t *tree);
inline art_rbtree_node_t *
art_rbtree_maxumum(const art_rbtree_t *tree);
inline art_rbtree_node_t *
art_rbtree_suc(art_rbtree_t *tree, art_rbtree_node_t *node);
inline art_rbtree_node_t *
art_rbtree_pre(art_rbtree_t *tree, art_rbtree_node_t *node);


#endif