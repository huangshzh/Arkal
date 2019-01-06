/*
 * Copyright (C) Arthur
 */

#ifndef __ART_FILEMMAP_H__
#define __ART_FILEMMAP_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

/* 20M = 5120 * 4096 */
#define ART_MMAP_MAX_PAGE                   5120
#define ART_MMAP_MAX_SIZE                   (ART_MMAP_MAX_PAGE * ART_PAGESIZE)

#define art_mmap(fd, len, prot, flag, s)    mmap(NULL, len, prot, flag, fd, s)
#define art_munmap(start, end)              munmap(start, (end-start))

#define ART_MMAP_PROT_READ                  PROT_READ
#define ART_MMAP_PROT_EXEC                  PROT_EXEC
#define ART_MMAP_PROT_WRITE                 PROT_WRITE
#define ART_MMAP_PROT_NONE                  PROT_NONE

#define ART_MMAP_SHARED                     MAP_SHARED
#define ART_MMAP_PRIVATE                    MAP_PRIVATE
#define ART_MMAP_LOCKED                     MAP_LOCKED
#define ART_MMAP_ANONYMOUS                  MAP_ANONYMOUS
#define ART_MMAP_POPULATE                   MAP_POPULATE
#define ART_MMAP_NONBLOCK                   MAP_NONBLOCK

#define ART_MMAP_FAILED                     MAP_FAILED

typedef struct {
    art_file_t              file;

    off_t                   last;
    off_t                   gross;  

    art_u8_t                *start;
    art_u8_t                *cur;
    art_u8_t                *end;
}art_file_mmap_t;


art_file_mmap_t *
art_file_mmap_open(art_pool_t *pool, art_u8_t *name);
art_file_mmap_t *
art_file_mmap_create(art_pool_t *pool, art_u8_t *name, off_t fsize);
art_state_t
art_file_mmap_close(art_file_mmap_t *f, art_u8_t f_init);
size_t
art_file_mmap_read(art_file_mmap_t *f, art_u8_t *buf, size_t len);
art_state_t
art_file_mmap_write(art_file_mmap_t *f, art_u8_t *buf, size_t len);
#endif