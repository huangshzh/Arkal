/*
 * Copyright (C) Arthur
 */

#ifndef __ART_SYS_CONF_H__
#define __ART_SYS_CONF_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

typedef struct {
    art_file_mmap_t     *file;
    
    art_log_t           *log;
    art_pool_t          *pool;
    
    art_array_t         *block;         /* art_conf_block_t */  
    art_array_t         *token;         /* art_conf_token_t */

    art_array_t         *set;           /* art_conf_read_t */

} art_conf_t;

typedef struct {
    art_u8_t            effective;
    art_str_t           command;
    art_str_t           value;

} art_conf_token_t;

typedef struct {
    art_u8_t            effective;
    art_str_t           blockname;
    
    art_array_t         *token;
    art_u32_t           n;
} art_conf_block_t;

typedef art_u8_t *  \
        (*art_token_set)(art_conf_t *cf, art_conf_token_t *cmd, void *conf);
typedef art_u8_t *  \
        (*art_block_set)(art_conf_t *cf, art_conf_block_t *cmd, void *conf);

typedef union {
    art_token_set       tokenset;
    art_block_set       blockset;
} art_conf_operate_t;

typedef struct {
    art_str_t           name;
    art_u32_t           type;       /* token or block */
    art_conf_operate_t  operate;                 
    void                *callback;
    void                *data;
} art_conf_read_t;



#define ART_CONF_TEMPSIZE       4096
#define ART_CONF_BUFSIZE        4096

#define ART_CONF_TOKEN_NAME     1
#define ART_CONF_TOKEN_VALUE    2
#define ART_CONF_BLOCK_INSIDE   3
#define ART_CONF_BLOCK_DONE     4
#define ART_CONF_FILE_DONE      5
#define ART_CONF_BUF_OUTSIZE    6


art_conf_t *art_conf_init(art_pool_t *pool, art_u8_t *name, art_log_t *log);
#endif