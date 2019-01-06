/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

static art_state_t
art_file_mmap_remap(art_file_mmap_t *f) {
    size_t                  msize;

    if (art_file_mmap_close(f, 0) == ART_ERROR) {
        return ART_ERROR;
    }

    f->last += (f->end - f->start);
    if (f->file.mode == O_RDONLY) {
        msize = art_min(ART_MMAP_MAX_SIZE, (f->gross - f->last));
    } else {
        msize = ART_MMAP_MAX_SIZE;
    }
    f->start = \
    art_mmap(f->file.fd, msize, ART_MMAP_PROT_READ, ART_MMAP_PRIVATE, f->last);
    if (f->start == ART_MMAP_FAILED) {
        art_log_stderr(errno, "in art_file_mmap_init() " \
        "art_mmap(%i, %i, ..., %i) failed", \
        f->file.fd, msize, f->last);
    }
    f->cur = f->start;
    f->end = f->start + msize;
    return ART_OK;
}

art_file_mmap_t *
art_file_mmap_open(art_pool_t *pool, art_u8_t *name) {
    art_file_mmap_t         *f;
    art_file_t              *p;
    art_s32_t               mode;
    art_s32_t               create;
    mode_t                  access;
    struct stat             filestate;
    size_t                  msize;

    f = art_palloc(pool, sizeof(art_file_mmap_t), 1);
    if (f == NULL) {
        art_log_stderr(0, "in art_file_mmap_init() " \
        "art_palloc(%p, %i, 1) failed ", \
        pool, sizeof(art_file_mmap_t));
    }
    
    mode = O_RDONLY;
    create = 0;
    access = ART_FILE_DEFAULT_ACCESS;
    p = &(f->file);

    p->fd = art_file_open(p, name, mode, create, access);
    if (p->fd == ART_INVALID_FILE) {
        art_log_stderr(0, "in art_file_mmap_init() " \
        "art_file_open(%p, %s, %0ui, %0ui, %0ui) failed", \
        p, name, mode, create, access);
        return NULL;
    }

    fstat(p->fd, &filestate);
    f->last = 0;
    f->gross = filestate.st_size;

    msize = art_min(ART_MMAP_MAX_SIZE, f->gross);
    f->start = art_mmap(p->fd, msize, ART_MMAP_PROT_READ, ART_MMAP_PRIVATE, 0);
    if (f->start == ART_MMAP_FAILED) {
        art_log_stderr(errno, "in art_file_mmap_init() " \
        "art_mmap(%i, %i, ...) failed", \
        p->fd, msize);
    }

    f->cur = f->start;
    f->end = f->start + msize;
    return f;
}

art_file_mmap_t *
art_file_mmap_create(art_pool_t *pool, art_u8_t *name, off_t fsize) {
    art_file_mmap_t         *f;
    art_file_t              *p;
    art_s32_t               mode;
    art_s32_t               create;
    mode_t                  access;
    size_t                  msize;
    art_u8_t                c = '\0';

    f = art_palloc(pool, sizeof(art_file_mmap_t), 1);
    if (f == NULL) {
        art_log_stderr(0, "in art_file_mmap_create() " \
        "art_palloc(%p, %i, 1) failed ", \
        pool, sizeof(art_file_mmap_t));
        return NULL;
    }
    
    mode = O_RDWR;
    create = O_CREAT;
    access = ART_FILE_DEFAULT_ACCESS;
    p = &(f->file);

    p->fd = art_file_open(p, name, mode, create, access);
    if (p->fd == ART_INVALID_FILE) {
        art_log_stderr(errno, "in art_file_mmap_create() " \
        "art_file_open(%p, %s, %0ui, %0ui, %0ui) failed", \
        p, name, mode, create, access);
        return NULL;
    }
    /* 
     * creat a blank file 
     */
    lseek(p->fd, (fsize-1), SEEK_SET);
    art_write(p->fd, &c, 1);

    f->last = 0;
    f->gross = 0;

    msize = ART_MMAP_MAX_SIZE;
    f->start = art_mmap(p->fd, msize, ART_MMAP_PROT_WRITE, ART_MMAP_SHARED, 0);
    if (f->start == ART_MMAP_FAILED) {
        art_log_stderr(errno, "in art_file_mmap_create() " \
        "art_mmap(%i, %i, ...) failed", \
        p->fd, msize);
        return NULL;
    }
    f->cur = f->start;
    f->end = f->start + msize;
    return f;
}

art_state_t
art_file_mmap_close(art_file_mmap_t *f, art_u8_t f_init) {
    if (art_munmap(f->start, f->end) != 0) {
        art_log_stderr(errno, "in art_file_mmap_close() " \
        "art_munmap(%p, %i) failed", \
        f->start, (f->end - f->start));
        return ART_ERROR;
    }

    if (f_init) {
        art_file_close(&(f->file));
        f->last = 0;
        f->gross = 0;
        f->start = NULL;
        f->cur = NULL;
        f->end = NULL;
    }
    return ART_OK;
}

size_t
art_file_mmap_read(art_file_mmap_t *f, art_u8_t *buf, size_t len) {
    if(len > (f->end - f->cur)) {
        len = (size_t)(f->end - f->cur);
    }
    art_memcpy(buf, f->cur, len);
    f->cur += len;

    if (f->cur == f->end && (f->last + (f->end - f->start)) < f->gross) {
        art_file_mmap_remap(f);
        return len;
    }

    if (f->cur == f->end && (f->last + (f->end - f->start)) == f->gross) {
        f->last += (f->end - f->start);
        return len;
    }
    return len;
}

art_state_t
art_file_mmap_write(art_file_mmap_t *f, art_u8_t *buf, size_t len) {
    size_t  size;
    while(len > (f->end - f->cur)) {
        /* 
         * That would cause block, if the len more than file map
         */
        size = (size_t)(f->end - f->cur);    
        art_memcpy(f->cur, buf, size);
        f->cur += size;
        art_file_mmap_remap(f);
        len -= size;
    }

    art_memcpy(f->cur, buf, len);
    f->cur += len;
    if (f->cur == f->end) {
        art_file_mmap_remap(f);
    }
    return ART_OK;
}