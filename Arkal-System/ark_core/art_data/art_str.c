/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

static art_u8_t *
art_string_num(art_u8_t *buf, art_u8_t *last, art_u64_t u64, 
        art_u8_t zero, art_u8_t hexadecimal, art_u32_t width);

inline void art_str_pset(art_pool_t *pool, art_str_t *p, art_u8_t *str) {
    p->len = art_strlen(str);
    p->str = art_palloc(pool, p->len, 0);
    art_memcpy(p->str, str, p->len);
}

/* 
 * please rember to free the space
 */
inline void art_str_mset(art_str_t *p, art_u8_t *str) {
    p->len = art_strlen(str);
    p->str = art_malloc(p->len);
    art_memcpy(p->str, str, p->len);
}

art_u8_t * 
art_fprintf(art_u8_t *buf, size_t max, const art_u8_t *fmt, ...) {
    art_u8_t      *p;
    va_list         args;

    va_start(args, fmt);
    p = art_string_core(buf, buf + max, fmt, args);
    va_end(args);

    return p;
}


art_u8_t *
art_string_core(art_u8_t *buf, art_u8_t *last, \
    const art_u8_t *fmt, va_list args) {
    art_u8_t        *p, pad;
    art_u32_t       n, width, frac_width, scale;
    art_u8_t        max_width, sign, hex;
    size_t          len;

    art_s64_t       s64;
    art_u64_t       u64, frac;  
    double          f;
    art_s32_t       d;
    art_str_t       *v;          

    while(*fmt && buf < last) {
        if (*fmt == '%') {
            /* get padding */
            pad = (art_u8_t)((*++fmt == '0') ? '0': ' ');
            width   = 0;
            sign    = 1;
            hex     = 0;
            frac_width = 0;
            max_width = 0;
            s64 = 0;
            u64 = 0;
            frac = 0;
            len = 0;

            /* get width */
            while ('0' <= *fmt && *fmt <= '9') {
                width = 10 * width + (*fmt - '0');
                fmt++;
            }

            /* get format */
            for(;;) {
                switch(*fmt) {
                case 'm' :
                    max_width = 1;
                    fmt++;
                    continue;

                case 'u' :
                    sign = 0;
                    fmt++;
                    continue;

                case 'x' :
                    hex = 1;
                    sign = 0;
                    fmt++;
                    continue;

                case 'X' :
                    hex = 2;
                    sign = 0;
                    fmt++;
                    continue;

                case '.' :
                    fmt++;

                    while ('0' <= *fmt && *fmt <= '9') {
                        frac_width = 10 * frac_width + (*fmt++ - '0');
                    }
                    if (frac_width >= 10) {
                        frac_width = 9;
                    }
                    
                    break;
                                
                default :
                    break;
                }

                break;
            }

            /* get type */
            switch (*fmt) {
                case 'i':
                    if (sign) {
                        s64 = (art_s32_t) va_arg(args, art_s32_t);
                    } else {
                        u64 = (art_u32_t) va_arg(args, art_u32_t);
                    }

                    if (max_width) {
                        width = ART_INT32_LEN;
                    }
                    break;

                case 'l':
                    if (sign) {
                        s64 = (art_s64_t) va_arg(args, art_s64_t);
                    } else {
                        u64 = (art_u64_t) va_arg(args, art_u64_t);
                    }

                    if (max_width) {
                        width = ART_INT64_LEN;
                    }
                    break;

                case 'p':
                    u64 = (art_uptr_t) va_arg(args, void *);
                    hex = 2;
                    sign = 0;
                    pad = '0';
                    width = 2 * sizeof(void *);
                    if (buf < last) {
                        *buf++ = '0';
                        *buf++ = 'x';
                    }
                    break;

                case 'c':
                    d = va_arg(args, int);
                    *buf++ = (art_u8_t) (d & 0xff);

                    fmt++;
                    continue;

                case 's':
                    p = va_arg(args, art_s8_t *);

                    while (*p && buf < last) {
                        *buf++ = *p++;
                    }

                    fmt++;
                    continue;

                case 'f':
                    f = va_arg(args, double);

                    if (f < 0) {
                        *buf++ = '-';
                        f = -f;
                    }

                    u64 = (art_s64_t) f;
                    frac = 0;

                    if (frac_width) {
                        scale = 1;
                        for (n = frac_width; n; n--) {
                            scale *= 10;
                        }

                        frac = (art_u64_t) ((f - (double) u64) * scale + 0.5);

                        if (frac == scale) {
                            u64++;
                            frac = 0;
                        }
                    }

                    buf = art_string_num(buf, last, u64, pad, 0, width);

                    if (frac_width) {
                        if (buf < last) {
                            *buf++ = '.';
                        }

                        buf = art_string_num(buf, last, frac,\
                            '0', 0, frac_width);
                    }

                    fmt++;
                    continue;

                case 'V':
                    v = va_arg(args, art_str_t *);

                    len = art_min(((size_t) (last - buf)), v->len);
                    buf = art_cpymem(buf, v->str, len);
                                
                    fmt++;
                    continue;

                default:
                    *buf++ = *fmt++;
                    continue;
            }

            if (sign) {
                if (s64 < 0) {
                    *buf++ = '-';
                    u64 = (art_u64_t) -s64;
                } else {
                    u64 = (art_u64_t) s64;
                }
            }

            buf = art_string_num(buf, last, u64, pad, hex, width);

            fmt++;

        } else {
            *buf++ = *fmt++;
        }
    }

    return buf;
}


static art_u8_t *
art_string_num(art_u8_t *buf, art_u8_t *last, art_u64_t u64, 
    art_u8_t zero, art_u8_t hexadecimal, art_u32_t width) {

    art_u8_t              *p, temp[ART_INT64_LEN + 1];
    size_t                len;
    art_u32_t             u32;
    static art_u8_t       hex[] = "0123456789abcdef";
    static art_u8_t       HEX[] = "0123456789ABCDEF";

    p = temp + ART_INT64_LEN;

    if (hexadecimal == 0) {
        if (u64 <= (art_u64_t) ART_MAX_UINT32_VALUE) {
            u32 = (art_u32_t) u64;
            do {
                *--p = (art_u8_t) (u32 % 10 + '0');
            } while (u32 /= 10);

        } else {
            do {
                *--p = (art_u8_t) (u64 % 10 + '0');
            } while (u64 /= 10);
        }
    } else if (hexadecimal == 1) {
        do {
            *--p = hex[(art_u32_t) (u64 & 0xf)];
        } while (u64 >>= 4);
    } else { 
        do {
            *--p = HEX[(art_u32_t) (u64 & 0xf)];
        } while (u64 >>= 4);
    }

    len = (temp + ART_INT64_LEN) - p;

    while (len++ < width && buf < last) {
        *buf++ = zero;
    }

    len = (temp + ART_INT64_LEN) - p;

    if (buf + len > last) {
        len = last - buf;
    }

    return (art_u8_t *)art_cpymem(buf, p, len);
}

void
art_str_rbtree_insert_value(art_rbtree_node_t *temp,
    art_rbtree_node_t *node, art_rbtree_node_t *sentinel) {
    art_str_rbtree_node_t      *n, *t;
    art_rbtree_node_t           **p;

    /*  
     * order by key->len->str's data
     */
    for ( ;; ) {

        n = (art_str_rbtree_node_t *) node;
        t = (art_str_rbtree_node_t *) temp;

        if (node->key != temp->key) {

            p = (node->key < temp->key) ? &temp->left : &temp->right;

        } else if (n->str.len != t->str.len) {

            p = (n->str.len < t->str.len) ? &temp->left : &temp->right;

        } else {
            p = (art_memcmp(n->str.str, t->str.str, n->str.len) < 0)
                ? &temp->left : &temp->right;
        }

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

art_str_rbtree_node_t *
art_str_rbtree_lookup(art_rbtree_t *rbtree, art_str_t *val, art_u32_t hash) {
    art_s32_t                   rc;
    art_str_rbtree_node_t       *n;
    art_rbtree_node_t           *node, *sentinel;

    node = rbtree->root;
    sentinel = rbtree->sentinel;

    while (node != sentinel) {

        n = (art_str_rbtree_node_t *) node;

        if (hash != node->key) {
            node = (hash < node->key) ? node->left : node->right;
            continue;
        }

        if (val->len != n->str.len) {
            node = (val->len < n->str.len) ? node->left : node->right;
            continue;
        }

        rc = art_memcmp(val->str, n->str.str, val->len);

        if (rc < 0) {
            node = node->left;
            continue;
        }

        if (rc > 0) {
            node = node->right;
            continue;
        }

        return n;
    }

    return NULL;
}