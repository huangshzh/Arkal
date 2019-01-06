/*
 * Copyright (C) Arthur
 */

#ifndef __ART_FILE_H__
#define __ART_FILE_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

/*
 *  The operations of "art_file.h" base on "fcntl.h", I will pakaging some
 *  functions on my code
 */
#define ART_STDIN       STDIN_FILENO    /* 0 */
#define ART_STDOUT      STDOUT_FILENO   /* 1 */
#define ART_STDERR      STDERR_FILENO   /* 2 */

#define ART_FILE_RDONLY             O_RDONLY
#define ART_FILE_WRONLY             O_WRONLY
#define ART_FILE_RDWR               O_RDWR

#define ART_FILE_APPEND             O_APPEND
#define ART_FILE_CREATE             O_CREAT
#define ART_FILE_EXCL               O_EXCL
#define ART_FILE_NONBLOCK           O_NONBLOCK
#define ART_FILE_TRUNC              O_TRUNC
#define ART_FILE_OPEN_AND_APPEND    (O_CREAT|O_APPEND)

#define ART_FILE_DEFAULT_ACCESS     0644
#define ART_FILE_OWNER_ACCESS       0600

#define ART_INVALID_FILE            -1
#define ART_FILE_ERROR              -1

#define art_open(name, mode, create, access) \
        open((const char *) name, mode|create, access)

#define art_close(fd)               close(fd)
#define art_read(fd, buf, len)      read(fd, buf, len)
#define art_write(fd, buf, len)     write(fd, buf, len)

typedef struct state                art_file_info_t;


typedef art_s32_t       art_fd_t;

typedef struct {
    art_fd_t    fd;
    art_str_t   name;
    time_t      creattime;
    art_s32_t   mode;
} art_file_t;  

art_fd_t
art_file_open(art_file_t *p, art_u8_t *name, \
        art_s32_t mode, art_s32_t create, mode_t access);

art_state_t art_file_close(art_file_t *p);

#define art_file_read(f, buf, len)      read((f)->fd, buff, len)
#define art_file_write(f, buf, len)     write((f)->fd, buff, len)
#endif