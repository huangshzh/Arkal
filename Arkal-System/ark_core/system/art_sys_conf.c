/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

static art_u8_t
art_conf_readcommand(art_u8_t **src, art_u8_t *dst, size_t boundary);

static art_state_t
art_conf_parse(art_conf_t *conf, art_u8_t *confbuf, size_t buf_size);


static art_u8_t
art_conf_readcommand(art_u8_t **src, art_u8_t *dst, size_t boundary) {
    art_u8_t    intext, block, list, lf;
    art_u8_t    *begin, *start, *read, *valid;
    size_t      usage;

    art_set_invalid(intext);
    art_set_invalid(block);
    art_set_invalid(list);
    art_set_invalid(lf);
    begin = *src;
    start = *src;
    read = *src;
    valid = NULL;
    usage = 0;

    for ( ;; ) {
        /* 
         * ignore space, tab, LF 
         * the space, TAB and LF before the data will ignore in 'art_conf_parse'
         * in there we ignore the space and TAB, LF which after the data
         */
        for (valid = read; (intext == art_invalid && \
            (*read == ART_SPACE || *read == ART_TAB || *read == ART_LF)); \
            /* void */) {
            read++;
            if (lf == art_valid) {
                /* if we use '\', we can retract */
                start = read;
            }
            
        };

        if (lf == art_valid) {
            art_set_invalid(lf);
        }

        /* support '\' */
        if (*read == '\\' && *(read + 1) != '\\') {
            if (valid - start > (ART_CONF_BUFSIZE - usage)) {
                return ART_CONF_BUF_OUTSIZE;
            }
            art_set_valid(lf);
            dst = art_cpymem(dst, start, (valid - start));
            usage += (valid - start);
            start = ++read;
            continue;
        }

        /* we don't check the pairing-valid */
        if (*read == '\"' || *read == '\'') {
            *src = ++read;
            continue;
        }

        /* a command done, return token value*/
        if (intext == art_invalid && *read == ';') {
            if (valid - start > (ART_CONF_BUFSIZE - usage)) {
                return ART_CONF_BUF_OUTSIZE;
            }
            dst = art_cpymem(dst, start, (valid - start));
            *src = ++read;
            return ART_CONF_TOKEN_VALUE;
        }

        /* block inside, return block name */
        if (intext == art_invalid && *read == '{') {
            if (valid - start > (ART_CONF_BUFSIZE - usage)) {
                return ART_CONF_BUF_OUTSIZE;
            }
            dst = art_cpymem(dst, start, (valid - start));
            *src = ++read;
            return ART_CONF_BLOCK_INSIDE;            
        }

        /* block outside, return 'ART_CONF_BLOCK_DONE' */
        if (intext == art_invalid && *read == '}') {
            *src = ++read;
            return ART_CONF_BLOCK_DONE;
        }

        /* token name */
        if (*read == '=') {
            if (valid - start > (ART_CONF_BUFSIZE - usage)) {
                return ART_CONF_BUF_OUTSIZE;
            }
            dst = art_cpymem(dst, start, (valid - start));
            *src = ++read;
            return ART_CONF_TOKEN_NAME;  
        }

        read++;
        if ((read - begin) >= boundary) {
            break;
        }
    }
    return ART_CONF_FILE_DONE;
}

static art_state_t
art_conf_parse(art_conf_t *conf, art_u8_t *confbuf, size_t buf_size) {
    art_u8_t            *start, *cur, *end;
    art_u8_t            *buf_end;
    art_u8_t            type;
    art_u8_t            inblock;
    art_u8_t            buf[ART_CONF_BUFSIZE];
    art_conf_block_t    *block;
    art_conf_token_t    *token;

    conf->block = art_array_creat(conf->pool, 5, sizeof(art_conf_block_t));
    if (conf->block == NULL) {
        return ART_ERROR;
    }
    conf->token = art_array_creat(conf->pool, 5, sizeof(art_conf_token_t));
    if (conf->token == NULL) {
        return ART_ERROR;
    }

    start = confbuf;
    cur = confbuf;
    end = confbuf + buf_size;
    art_set_invalid(inblock);

    while (cur <= end) {
        /* annotation */
        if (*cur == '#') {
            for (/* void */; cur <= end && *cur != ART_LF; cur++);
            continue;
        }
        /*
         * ignore the space, TAB and LF which before the data
         */
        if (*cur == ART_SPACE || *cur == ART_TAB || *cur == ART_LF) {
            cur++;
            continue;
        }

        /* read from buf */
        art_memzero(buf, ART_CONF_BUFSIZE);
        type = art_conf_readcommand(&cur, buf, (end - cur));
        /*
         *  type :
         *  ART_CONF_TOKEN_NAME : buf save the token name
         *  ART_CONF_TOKEN_VALUE : buf save the token value
         *  ART_CONF_BLOCK_INSIDE : buf save the block name and remind that the 
         *                          tokens, we will read from buf later, are 
         *                          belong to a block 
         *  ART_CONF_BLOCK_DONE : buf save nothing and remind that block done
         *  ART_CONF_FILE_DONE : done
         *  ART_CONF_BUF_OUTSIZE : the buf can't save the token or name
         */
        switch (type) {
        case ART_CONF_TOKEN_NAME:
            if (inblock == art_invalid){
                token = art_array_push(conf->token);
                art_str_pset(conf->pool, &token->command, buf);
            } else {
                token = art_array_push(block->token);
                art_str_pset(conf->pool, &token->command, buf);
                block->n++;
            }
            break;

        case ART_CONF_TOKEN_VALUE:
            art_str_pset(conf->pool, &token->value, buf);
            break;
        
        case ART_CONF_BLOCK_INSIDE:
            block = art_array_push(conf->block);
            art_str_pset(conf->pool, &block->blockname, buf);
            block->token = \
            art_array_creat(conf->pool, 5, sizeof(art_conf_token_t));
            block->n = 0;
            art_set_valid(inblock);
            break;
        
        case ART_CONF_BLOCK_DONE:
            art_set_invalid(inblock);
            break;

        case ART_CONF_FILE_DONE:
            return ART_OK;
            break;

        case ART_CONF_BUF_OUTSIZE:
            return ART_ERROR;
            break;
        }
    }
    return ART_OK;
}

/* get data from configuration file */
art_conf_t *
art_conf_init(art_pool_t *pool, art_u8_t *name, art_log_t *log) {
    art_conf_t      *conf;
    art_pool_t      *temp;
    art_u8_t        *confbuf;
    size_t          buf_size;
    size_t          i, readed;

    conf = art_palloc(pool, sizeof(art_conf_t), 1);
    if (conf == NULL) {
        return NULL;
    }
    conf->log = log;
    conf->pool = pool;

    temp = art_create_pool(ART_CONF_TEMPSIZE);
    conf->file = art_file_mmap_open(temp, name);
    if (conf->file == NULL) {
        art_destroy_pool(temp);
        return NULL;
    }

    buf_size = (size_t)conf->file->gross;
    if (buf_size <= 0) {
        art_destroy_pool(temp);
        return NULL;
    }

    confbuf = art_palloc(temp, buf_size, 1);
    if (confbuf == NULL) {
        art_destroy_pool(temp);
        return NULL;
    }
    
    for (readed = 0; readed < buf_size; /* void */) {
        i = art_file_mmap_read(conf->file, \
                                (art_u8_t *)(confbuf + readed), \
                                buf_size);

        readed += i;
        if (i <= 0) {
            break;
        }
    }

    art_conf_parse(conf, confbuf, buf_size);

    if (art_file_mmap_close(conf->file, 0) == ART_OK) {
        conf->file = NULL;
        art_destroy_pool(temp);
    } else {
        /* 有问题 */;
    }

    conf->set = art_array_creat(conf->pool, 5, sizeof(art_conf_read_t));
    if (conf->set == NULL) {
        return NULL;
    }

    return conf;
}