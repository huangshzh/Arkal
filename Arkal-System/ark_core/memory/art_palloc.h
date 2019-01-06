/*
 * Copyright (C) Arthur
 */


#ifndef __ART_PALLOC_H__
#define __ART_PALLOC_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

#define art_malloc(size)            malloc(size)
#define art_free(p)                 free(p)  
#define art_memzero(buf, n)         (void) memset(buf, 0, n)
#define art_memset(buf, c, n)       (void) memset(buf, c, n) 
#define art_memcpy(dst, src, n)     (void) memcpy(dst, src, n)    
#define art_cpymem(dst, src, n)     (((art_u8_t *) memcpy(dst, src, n)) + (n))
#define art_memmove(dst, src, n)    (void) memmove(dst, src, n)
#define art_movemem(dst, src, n)    (((u_char *) memmove(dst, src, n)) + (n))
#define art_memcmp(s1, s2, n)  \
memcmp((const char *) s1, (const char *) s2, n)

/* alignment */
#define art_align(d, a)         (((d) + (a - 1)) & ~(a - 1))
#define art_align_ptr(p, a)     (art_u8_t *)\
(((art_uptr_t)p + ((art_uptr_t)a - 1)) & ~((art_uptr_t)a - 1))

/*
 * ART_MAX_ALLOC_FROM_POOL should be (art_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define ART_PAGESIZE            4096                      
#define ART_MAX_ALLOC_FROM_POOL (ART_PAGESIZE - 1)                  
#define ART_POOL_ALIGNMENT      16  

/* 
 * when a memory doesn't work over a period of time , free it
*/
#define LEISURETIMES            15

typedef struct art_pool_recycle_s       art_pool_recycle_t;
typedef struct art_pool_file_s          art_pool_file_t;
typedef struct art_pool_cleanup_s       art_pool_cleanup_t;
typedef struct art_pool_large_s         art_pool_large_t;
typedef struct art_pool_data_s          art_pool_data_t;
typedef struct art_pool_s               art_pool_t;

typedef void(*art_pool_cleanup_pt)(void *data);

struct art_pool_cleanup_s {
	art_pool_cleanup_pt     handler;
	void		            *data;
	art_pool_cleanup_t   	*next;
};

struct art_pool_large_s {
	art_pool_large_t     	*next;
	art_uptr_t		        *alloc;
};

struct art_pool_data_s {                                               
	art_u8_t               	*last;                                 
	art_u8_t               	*end;                                 
	art_pool_t              *next;                                 
	art_u32_t           	failed;  
	art_u32_t           	leisure;     
};

struct art_pool_s {
	art_pool_data_t      	d;
    art_atomic_t            lock;                                    
	size_t              	max;                                  
	art_pool_t           	*current;                                                            
	art_pool_large_t     	*large;                                
	art_pool_cleanup_t	    *cleanup;                            
};


art_pool_t *art_create_pool(size_t size);
void art_destroy_pool(art_pool_t *pool);
void art_reset_pool(art_pool_t *pool);

void *art_palloc(art_pool_t *pool, size_t size, art_u8_t init);
void *art_pnalloc(art_pool_t *pool, size_t size, art_u8_t init);
void *art_pmemalign(art_pool_t *pool, size_t size, size_t alignment);
art_state_t  art_pfree(art_pool_t *pool, art_uptr_t p);

art_pool_cleanup_t *art_pool_cleanup_add(art_pool_t *p, size_t size);

void art_manage_small(art_pool_t *pool);

#endif
