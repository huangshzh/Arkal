/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

art_fd_t
art_file_open(art_file_t *p, art_u8_t *name, \
    art_s32_t mode, art_s32_t create, mode_t access) {
    size_t          len;

    if (p == NULL || name == NULL) {
        art_log_stderr(0, "art_file_open(%p, %p, ...) failed", p, name);
        return ART_INVALID_FILE;
    }

    p->fd = art_open(name, mode, create, access);
    if (p->fd == ART_INVALID_FILE) {
        art_log_stderr(0, "art_open(%s, ...) failed (%i: %s)", \
            name, errno, strerror(errno));
        return ART_INVALID_FILE;
    }

    p->name.len = art_strlen(name);
    p->name.str = (art_u8_t *) art_malloc(p->name.len);
    if (p->name.str == NULL) {
        art_close(p->fd);
        art_log_stderr(0, "art_malloc(%ud) failed (%i: %s)", \
            p->name.len, errno, strerror(errno));
        return ART_INVALID_FILE;
    }
    art_memcpy(p->name.str, name, p->name.len);
    p->creattime = get_current_sec();
    p->mode = (mode|create);
    return p->fd;
}

art_state_t
art_file_close(art_file_t *p) {
    if (p == NULL) {
        return ART_ERROR;
    }

    if (art_close(p->fd) == -1) {
        art_log_stderr(errno, "art_file_reduced_close(fd-%i) failed ", p->fd);
        return ART_ERROR;
    }
    p->fd = ART_INVALID_FILE;
    
    if (p->name.str) {
        art_free(p->name.str);
        p->name.len = 0;
        p->name.str = NULL;
    }
    
    p->creattime = 0;
    p->mode = 0;
    return ART_OK;
}

