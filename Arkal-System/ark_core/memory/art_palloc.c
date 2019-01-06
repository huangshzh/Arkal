/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

static inline void *\
art_memalign(size_t alignment, size_t size);

static inline void *\
art_palloc_small(art_pool_t *pool, size_t size, art_u32_t align);

static void *\
art_palloc_block(art_pool_t *pool, size_t size);

static void *\
art_palloc_large(art_pool_t *pool, size_t size);


static inline void *
art_memalign(size_t alignment, size_t size) {
	void	 	*p;
	size_t  	err;

	err = posix_memalign(&p, alignment, size);
	if (err) {
		p = NULL;
	}

	return p;
}

static inline void *
art_palloc_small(art_pool_t *pool, size_t size, art_u32_t align) {
	art_u8_t        *m;
	art_pool_t      *p;

	p = pool->current;
	do {
    	m = p->d.last;
    	if (align) {                                
		    m = art_align_ptr(m, ART_POOL_ALIGNMENT);    
    	}

        if ((size_t) (p->d.end - m) >= size) {     
			p->d.last = m + size;                   
			p->d.leisure = 0;
			return m;                               
        }

        p = p->d.next;                              
	} while (p);

	return art_palloc_block(pool, size);
}

static void *
art_palloc_block(art_pool_t *pool, size_t size) {
    art_u8_t        *m;
	size_t		    psize;
	art_pool_t      *p, *new;
	
    psize = (size_t)(pool->d.end - (art_u8_t *) pool);       
	m = (art_u8_t *)art_memalign(ART_POOL_ALIGNMENT, psize);
	if (m == NULL) {
        	return NULL;
	}

    new = (art_pool_t *) m;
	new->d.end = (art_u8_t *)m + psize;
	new->d.next = NULL;
	new->d.failed = 0;
	new->d.leisure = 0;

	m += sizeof(art_pool_data_t);
	m = art_align_ptr(m, ART_POOL_ALIGNMENT);
	new->d.last = m + size;

	for (p = pool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
		    pool->current = p->d.next;
        }
	}

	p->d.next = new;

	return m;
}

static void *
art_palloc_large(art_pool_t *pool, size_t size) {
	art_uptr_t        	    *p;
	art_pool_large_t        *large;
	size_t			        n;

	p = art_malloc(size);
	if (p == NULL) {
		return NULL;
	}
	
	n = 0;
	for(large = pool->large; large; large = large->next) {
        if(large->alloc == NULL) {
			large->alloc = p;
			return p;
        }    

        if(n++ > 3) {
			break;
        }
	}

	large = (art_pool_large_t *) \
        art_palloc_small(pool, sizeof(art_pool_large_t), 1);
	if (large == NULL) {
		art_free(p);
		return NULL;
	}

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;

	return p;
}

art_pool_t *
art_create_pool(size_t size) {
	art_pool_t *p;

	size = ((size < ART_MAX_ALLOC_FROM_POOL) ? size : ART_MAX_ALLOC_FROM_POOL);
	
	p = (art_pool_t *)art_memalign(ART_POOL_ALIGNMENT, size);
	if (p==NULL) {
		return NULL;
	}
	
	p->d.last = (art_u8_t *)p + sizeof(art_pool_t);
    p->d.end = (art_u8_t *)p + size;
	p->d.next = NULL;
	p->d.failed = 0;
	p->d.leisure = 0;
	size = size - sizeof(art_pool_t);
	p->max = size;
	if(p->max <= 0) {
        art_free(p);
        return NULL;
	}
	
    p->lock = 0;
	p->current = p;
	p->large = NULL;
	p->cleanup = NULL;

	return p;
}

void
art_destroy_pool(art_pool_t *pool) {
	art_pool_t		*p, *pre;
	art_pool_cleanup_t	*ph;
	art_pool_large_t 	*pl;
	
	for(ph = pool->cleanup; ph; ph = ph->next) {
        if(ph->handler) {
			ph->handler(ph->data);
        }
	}

	for(pl = pool->large; pl; pl = pl->next) {
        if(pl->alloc) {
			art_free(pl->alloc);   
		}
	}

    for(pre = pool, p = pool->d.next; /* void */; pre = p, p = p->d.next) {
        art_free(pre);

        if(p == NULL) {
			break;
        }
	}
}

void 
art_reset_pool(art_pool_t *pool) {
	art_pool_t 		*p;
	art_pool_large_t        *pl;

	for(pl = pool->large; pl; pl = pl->next) {
		if(pl->alloc) {
			art_free(pl->alloc);
        }
	}

	for(p = pool; p; p = p->d.next) {
        p->d.last = (art_u8_t *)p + sizeof(art_pool_t);
        p->d.failed = 0;
        p->d.leisure++;
	}

	pool->current = pool;
	pool->large = NULL;
}

void *
art_palloc(art_pool_t *pool, size_t size, art_u8_t init) {
	art_uptr_t      *p;

    art_spinlock(&(pool->lock), 1, 2048);

	if (size <= pool->max) {
        p = art_palloc_small(pool, size, 1);
    } else {
        p = art_palloc_large(pool, size);
	}

    art_unlock(&(pool->lock));
	
    if(init) {
        art_memzero(p, size);
	}
        
	return p;
}

void *
art_pnalloc(art_pool_t *pool, size_t size, art_u8_t init) {
	art_uptr_t      *p;

    art_spinlock(&(pool->lock), 1, 2048);

	if (size <= pool->max) {
        p = art_palloc_small(pool, size, 0);
	} else {
        p = art_palloc_large(pool, size);
	}

    art_unlock(&(pool->lock));

	if(init) {
        art_memzero(p, size);
	}

	return p;
}

/* get a large memory block that alignmently */
void *
art_pmemalign(art_pool_t *pool, size_t size, size_t alignment) {
    art_uptr_t              *p;
    art_pool_large_t        *large;
	size_t                  n;

	p = art_memalign(alignment, size);
	if (p == NULL) {
        	return NULL;
	}

    art_spinlock(&(pool->lock), 1, 2048);

	n = 0;
	for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL) {
			large->alloc = p;
			return p;
        }    

        if(n++ > 3) {
			break;
        }
	}

	large = (art_pool_large_t *) \
        art_palloc_small(pool, sizeof(art_pool_large_t), 1);
	if (large == NULL) {
        art_free(p);
        return NULL;
	}

    art_unlock(&(pool->lock));

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;

	return p;
}

art_state_t
art_pfree(art_pool_t *pool, art_uptr_t p) {
    art_pool_large_t        *l;
	
	/* Free the memory specified by 'p' */
	for (l = pool->large; l; l = l->next) {
        if ((void *)p == l->alloc) {
			art_free(l->alloc);
			l->alloc = NULL;
			return ART_OK;
        }
	}
	return ART_ERROR;
}

art_pool_cleanup_t *
art_pool_cleanup_add(art_pool_t *pool, size_t size) {
	art_pool_cleanup_t  	*c;

	c = (art_pool_cleanup_t *) \
        art_palloc_small(pool, sizeof(art_pool_cleanup_t), 1);
	if (c == NULL) {
		return NULL;
	}

	if (size) {
        c->data = art_palloc(pool, size, 1);
        if (c->data == NULL) {
			return NULL;
        }
	} else {
        c->data = NULL;
	}

	c->handler = NULL;

	c->next = pool->cleanup;
	pool->cleanup = c;
        
	return c;
}

void 
art_manage_small(art_pool_t *pool) {
    art_pool_t	*p, *pre;

    for(pre = pool, p = pool->d.next; p; /* void */) {
        if(p->d.leisure > LEISURETIMES) {
			pre->d.next = p->d.next;
			art_free(p);
			p=pre->d.next;
        }
        pre = p;
        p = p->d.next;
	}
}

/* update on 2018-12-18 */
