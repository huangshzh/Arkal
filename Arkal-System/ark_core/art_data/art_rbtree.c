/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

static inline art_rbtree_node_t *
art_rbtree_min(art_rbtree_node_t *node, art_rbtree_node_t *sentinel);

static inline art_rbtree_node_t *
art_rbtree_max(art_rbtree_node_t *node, art_rbtree_node_t *sentinel);

static inline void
art_rbtree_left_rotate(art_rbtree_node_t **root,
    art_rbtree_node_t *sentinel, art_rbtree_node_t *node);

static inline void
art_rbtree_right_rotate(art_rbtree_node_t **root,
    art_rbtree_node_t *sentinel, art_rbtree_node_t *node);

static inline art_rbtree_node_t *
art_rbtree_min(art_rbtree_node_t *node, art_rbtree_node_t *sentinel) {
    while (node->left != sentinel) {
        node = node->left;
    }

    return node;
}

static inline art_rbtree_node_t *
art_rbtree_max(art_rbtree_node_t *node, art_rbtree_node_t *sentinel) {
    while (node->right != sentinel) {
        node = node->right;
    }

    return node;
}

static inline void
art_rbtree_left_rotate(art_rbtree_node_t **root, art_rbtree_node_t *sentinel, \
    art_rbtree_node_t *node) {
    art_rbtree_node_t       *temp;

    temp = node->right;
    node->right = temp->left;

    if (temp->left != sentinel) {
        temp->left->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->left) {
        node->parent->left = temp;

    } else {
        node->parent->right = temp;
    }

    temp->left = node;
    node->parent = temp;
} 

static inline void
art_rbtree_right_rotate(art_rbtree_node_t **root, art_rbtree_node_t *sentinel, \
    art_rbtree_node_t *node) {
    art_rbtree_node_t       *temp;

    temp = node->left;
    node->left = temp->right;

    if (temp->right != sentinel) {
        temp->right->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->right) {
        node->parent->right = temp;
    } else {
        node->parent->left = temp;
    }

    temp->right = node;
    node->parent = temp;
}

void
art_rbtree_insert_value(art_rbtree_node_t *temp, art_rbtree_node_t *node,
    art_rbtree_node_t *sentinel) {
    art_rbtree_node_t  **p;

    for ( ;; ) {

        p = (node->key < temp->key) ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    art_rbt_red(node);
}

void
art_rbtree_insert_timer_value(art_rbtree_node_t *temp, art_rbtree_node_t *node,
    art_rbtree_node_t *sentinel) {
    art_rbtree_node_t  **p;

    for ( ;; ) {

        /*
         * Timer values
         * 1) are spread in small range, usually several minutes,
         * 2) and overflow each 49 days, if milliseconds are stored in 32 bits.
         * The comparison takes into account that overflow.
         */

        /*  node->key < temp->key */

        p = ((art_rbtree_key_int_t) (node->key - temp->key) < 0)
            ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    art_rbt_red(node);
}

void 
art_rbtree_insert(art_rbtree_t *tree, art_rbtree_node_t *node) {
    art_rbtree_node_t  **root, *uncle, *sentinel;

    /* a binary tree insert */

    root = &tree->root;
    sentinel = tree->sentinel;

    if (*root == sentinel) {
        node->parent = NULL;
        node->left = sentinel;
        node->right = sentinel;
        art_rbt_black(node);
        *root = node;

        return;
    }

    tree->insert(*root, node, sentinel);

    /* rebalance tree */

    while (node != *root && art_rbt_is_red(node->parent)) {
        /* 
         * case A: uncle node is red
         *          a. black uncle node
         *          b. black parent node
         *          c. red grandparent
         *          d. grandparent node to be the operational node
         * case B: uncle node is black
         *          case 1: current operational node is right node
         *                  a. parent node to be the operational node
         *                  b. left rotate
         *          case 2: current operational node is left node
         *                  a. black parent node
         *                  b. red grandparent node
         *                  d. grandparent node as operational node to right
         *                     rotate
         */
        
        if (node->parent == node->parent->parent->left) {
            /* the node's parent is grandparent's left node */
            uncle = node->parent->parent->right;

            if (art_rbt_is_red(uncle)) {
                art_rbt_black(node->parent);
                art_rbt_black(uncle);
                art_rbt_red(node->parent->parent);
                node = node->parent->parent;
            
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    art_rbtree_left_rotate(root, sentinel, node);
                }

                art_rbt_black(node->parent);
                art_rbt_red(node->parent->parent);
                art_rbtree_right_rotate(root, sentinel, node->parent->parent);
            }

        } else {
            /* the node's parent is grandparent's right node */
            uncle = node->parent->parent->left;

            if (art_rbt_is_red(uncle)) {
                art_rbt_black(node->parent);
                art_rbt_black(uncle);
                art_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    art_rbtree_right_rotate(root, sentinel, node);
                }

                art_rbt_black(node->parent);
                art_rbt_red(node->parent->parent);
                art_rbtree_left_rotate(root, sentinel, node->parent->parent);                
            }
        }
    }

    art_rbt_black(*root);
}

void 
art_rbtree_delete(art_rbtree_t *tree, art_rbtree_node_t *node) {
    art_u8_t           red;
    art_rbtree_node_t  **root, *sentinel, *subst, *temp, *w;

    root = &tree->root;
    sentinel = tree->sentinel;

    /*
     *           (1)       (2)      (3)      (4)
     *            x         x        x        x
     *           / \   or  / \   or / \   or / \ 
     *         nil nil   nil  y    y  nil   a   b
     *                       / \           /  .....
     *                     nil nil        c  .......
     *                                   / \
     *                                 nil  y
     *  subst:    x         x        x        c     
     *  temp:    nil        y        y        y
     */

    if (node->left == sentinel) {
        /* case 1 that leaf node and case 2 only have left child */                   
        temp = node->right;
        subst = node;

    } else if (node->right == sentinel) {
        /* case 3 that only have right child */
        temp = node->left;                      
        subst = node;

    } else {
        /* case 4 that have two child */
        subst = art_rbtree_min(node->right, sentinel); 

        if (subst->left != sentinel) {
            temp = subst->left;         /* doesn't has this case, in fact */
        } else {
            temp = subst->right;
        }

    }   

    /*
     *         x         x        x       
     *        / \   or  / \   or / \   
     *      nil nil   nil  y    y  nil   
     *                    / \           
     *                  nil nil        
     */ 
    
    if (subst == *root) {               
        *root = temp;
        art_rbt_black(temp);

        /* DEBUG stuff */
        node->key = 0;
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        art_rbt_black(node);

        return;
    }

    /*
     *  manage the node that have two child
     */

    red = art_rbt_is_red(subst);        

    /* transplant */
    if (subst == subst->parent->left) {         
        subst->parent->left = temp;     
    } else {                            
        subst->parent->right =temp;     
    }

    if (subst == node) {                
        temp->parent = subst->parent;   

    } else {

        if (subst->parent == node) {     
            temp->parent = subst;

        } else {
            temp->parent = subst->parent;
        }  

        art_rbt_copy_color(subst, node);      
        art_rbt_copy_left(subst, node);       
        art_rbt_copy_right(subst, node);      
        art_rbt_copy_parent(subst, node);   

        if (node == *root) {
            *root = subst;

        } else {
            if (node == node->parent->left) {
                node->parent->left = subst;

            } else {
                node->parent->right = subst;
            }
        }

        if (subst->left != sentinel) {
            subst->left->parent = subst;
        }

        if (subst->right != sentinel) {
            subst->right->parent = subst;
        }
    }

    /* DEBUG stuff */
    node->key = 0;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    art_rbt_black(node);

    if (red) {
        return;
    }

    /* rebalance tree */
    /*
     * case 1: temp node's brother node is red
     *          a. black brother node
     *          b. red parent node
     *          c. parent node as operational node to left rotate
     *          d. reset brother node
     * case 2: temp node's brother node is black and 
     *         brother node's two child is black
     *          a. red brother node
     *          b. set parent node as new 'temp'
     * case 3: temp node's brother node is black and 
     *         brother node's right node is black and left node is red
     *          a. black brother node's left node
     *          b. red brother node
     *          c. brother node as operational node to right rotate
     *          d. reset brother node
     * case 4: temp node's brother node is black and
     *         brother node's right node is red 
     *          a. copy parent node's color to brother node
     *          b. black parent node
     *          c. black brother node's right node
     *          d. parent node as operational node to left rotate
     *          e. set 'temp ' as root
     */
    while (temp != *root && art_rbt_is_black(temp)) {

        if (temp == temp->parent->left) {
            w = temp->parent->right;            /* set brother node */

            /* case 1 */
            if (art_rbt_is_red(w)) {
                art_rbt_black(w);
                art_rbt_red(temp->parent);
                art_rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }
            /* case 2 */
            if (art_rbt_is_black(w->left) && art_rbt_is_black(w->right)) {
                art_rbt_red(w);
                temp = temp->parent;

            } else {
                /* case 3 */
                if (art_rbt_is_black(w->right)) {
                    art_rbt_black(w->left);
                    art_rbt_red(w);
                    art_rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }
                /* case 4 */
                art_rbt_copy_color(w, temp->parent);
                art_rbt_black(temp->parent);
                art_rbt_black(w->right);
                art_rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }

        } else {
            w = temp->parent->left;

            if (art_rbt_is_red(w)) {
                art_rbt_black(w);
                art_rbt_red(temp->parent);
                art_rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            if (art_rbt_is_black(w->left) && art_rbt_is_black(w->right)) {
                art_rbt_red(w);
                temp = temp->parent;

            } else {
                if (art_rbt_is_black(w->left)) {
                    art_rbt_black(w->right);
                    art_rbt_red(w);
                    art_rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                art_rbt_copy_color(w, temp->parent);
                art_rbt_black(temp->parent);
                art_rbt_black(w->left);
                art_rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }

    art_rbt_black(temp);
}

inline art_rbtree_node_t *
art_rbtree_minimum(const art_rbtree_t *tree) {
    art_rbtree_node_t *node, *sentinel;

    node = tree->root;
    sentinel = tree->sentinel;

    return art_rbtree_min(node, sentinel);
}

inline art_rbtree_node_t *
art_rbtree_maximum(const art_rbtree_t *tree) {
    art_rbtree_node_t *node, *sentinel;
    
    node = tree->root;
    sentinel = tree->sentinel;

    return art_rbtree_max(node, sentinel);
}

inline art_rbtree_node_t *
art_rbtree_suc(art_rbtree_t *tree, art_rbtree_node_t *node) {
    art_rbtree_node_t  *root, *sentinel, *parent;

    sentinel = tree->sentinel;

    if (node->right != sentinel) {
        return art_rbtree_min(node->right, sentinel);
    }

    root = tree->root;

    for ( ;; ) {
        parent = node->parent;

        if (node == root) {
            return NULL;
        }

        if (node == parent->left) {
            return parent;
        }

        node = parent;
    }
}

inline art_rbtree_node_t *
art_rbtree_pre(art_rbtree_t *tree, art_rbtree_node_t *node) {
    art_rbtree_node_t  *root, *sentinel, *parent;

    sentinel = tree->sentinel;

    if (node->left != sentinel) {
        return art_rbtree_min(node->left, sentinel);
    }

    root = tree->root;

    for ( ;; ) {
        parent = node->parent;

        if (node == root) {
            return NULL;
        }

        if (node == parent->right) {
            return parent;
        }

        node = parent;
    }
}

/* update 2018-12-27 */