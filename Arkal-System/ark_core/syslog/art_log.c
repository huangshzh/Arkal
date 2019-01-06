/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

static art_str_t         *art_sys_errlist;
static art_str_t         art_unknown_error = art_string("Unknown error");
static art_str_t         err_levels[] = {
    art_null_string,
    art_string("emerg"),
    art_string("alert"),
    art_string("crit"),
    art_string("error"),
    art_string("warn"),
    art_string("notice"),
    art_string("info"),
    art_string("debug")
};

static inline ssize_t art_log_write(art_fd_t fd, art_u8_t *buf, size_t n);
static art_u8_t *art_strerror(art_s32_t err,  art_u8_t *errstr, size_t size);
static art_u8_t *art_log_errno(art_u8_t *buf, art_u8_t *last, art_s32_t err);

static inline ssize_t
art_log_write(art_fd_t fd, art_u8_t *buf, size_t n) {
    return write(fd, buf, n);
}

art_state_t
art_strerror_init(void) {
    art_s8_t        *msg;
    art_u8_t        *p;
    size_t          len;
    art_s32_t       err;

    if (art_sys_errlist != NULL) {
        return ART_OK;
    }

    len = ART_SYS_NERR * sizeof(art_str_t);

    art_sys_errlist = art_malloc(len);
    if (art_sys_errlist == NULL) {
        goto failed;
    }

    for (err = 0; err < ART_SYS_NERR; err++) {
        msg = strerror(err);
        len = art_strlen(msg);

        p = art_malloc(len);
        if (p == NULL) {
            art_free(art_sys_errlist);
            goto failed;
        }

        art_memcpy(p, msg, len);
        art_sys_errlist[err].len = len;
        art_sys_errlist[err].str = p;
    }

    return ART_OK;

failed:
        err = errno;
        art_log_stderr(0, "art_alloc(%ud) failed (%d: %s)",\
                len, err, strerror(err));

        return ART_ERROR;
}

static art_u8_t *
art_strerror(art_s32_t err,  art_u8_t *errstr, size_t size) {
    art_str_t  *msg;

    msg = ((art_u32_t) err < ART_SYS_NERR) ? \
    &art_sys_errlist[err] : &art_unknown_error;
        
    size = art_min(size, msg->len);

    return art_cpymem(errstr, msg->str, size);
}

static art_u8_t *
art_log_errno(art_u8_t *buf, art_u8_t *last, art_s32_t err) {
    if (buf > last - 50) {
        /* leave a space for an error code */
        buf = last - 50;
        *buf++ = '.';
        *buf++ = '.';
        *buf++ = '.';
    }

    buf = art_fprintf(buf, last-buf, " (%i: ", err);

    buf = art_strerror(err, buf, last - buf);

    if (buf < last) {
        *buf++ = ')';
    }

    return buf;
}

void 
art_log_stderr(art_s32_t err, const art_u8_t *fmt, ...) {
    art_u8_t      *p, *last;
    va_list      args;
    art_u8_t      errstr[ART_MAX_ERROR_STR];

    last = errstr + ART_MAX_ERROR_STR;

    p = art_cpymem(errstr, "Arthur: ", sizeof("Arthur: "));

    va_start(args, fmt);
    p = art_string_core(p, last, fmt, args);
    va_end(args);

    if (err) {
        p = art_log_errno(p, last, err);
    }

    if (p > last - 1) {
        p = last - 1;
    }

    *p++ = '\n';
    
    (void)art_log_write(ART_STDERR, errstr, p - errstr);
}

art_log_t *
art_log_init(art_u8_t *prefix, art_u8_t level) {
    art_u8_t        *p, *name, *n;
    size_t          plen, nlen;
    art_log_t       *rlog;
    art_s32_t       err;
    size_t          len;

    if (level > ART_LOG_DEBUG || level < ART_LOG_STDERR) {
        art_log_stderr(0, "art_log_init() failed, level (%i) errno", level);
        return NULL;
    }

    if (art_strerror_init() != ART_OK) {
        art_log_stderr(0, "art_strerror_init() failed");
        return NULL;
    }

    rlog = art_malloc(sizeof(art_log_t));
    if (rlog == NULL) {
        len = sizeof(art_log_t);
        goto failed;
    }

    rlog->level = level;
    rlog->connection = 0;
    rlog->handler = NULL;
    rlog->data = NULL;
    rlog->next = NULL;

    switch (rlog->level) {
    case ART_LOG_STDERR:    n = "";  break;
    case ART_LOG_EMERG:     n = ART_LOG_EMERG_PATH;  break;
    case ART_LOG_ALERT:     n = ART_LOG_ALERT_PATH;  break;
    case ART_LOG_CRIT:      n = ART_LOG_CRIT_PATH;  break;
    case ART_LOG_ERROR:     n = ART_LOG_ERROR_PATH;  break;
    case ART_LOG_WARN:      n = ART_LOG_WARN_PATH;  break;
    case ART_LOG_NOTICE:    n = ART_LOG_NOTICE_PATH;  break;
    case ART_LOG_INFO:      n = ART_LOG_INFO_PATH;  break;
    case ART_LOG_DEBUG:     n = ART_LOG_DEBUG_PATH;  break;
    default :               n = "";  break;
    }

    name = n;
    nlen = art_strlen(name);
    if (nlen == 0) {
        rlog->file.fd = ART_STDERR;
        rlog->file.name.len = art_strlen("Standard error output");
        rlog->file.name.str = art_malloc(rlog->file.name.len);
        if (rlog->file.name.str == NULL) {
            len = rlog->file.name.len;
            goto failed;
        }
        art_memcpy(rlog->file.name.str, \
            "Standard error output", rlog->file.name.len);
        return rlog;
    }
    
    if (prefix == NULL) {
        prefix = ART_SYSTEM_PREFIX;
    }
    plen = art_strlen(prefix);

    if (plen) {
        p = art_malloc(plen + nlen + 2);
        art_memzero(p, (plen + nlen + 2));
        if (p == NULL) {
            goto failed;
        }

        name = art_cpymem(p, prefix, plen);
        if ((*(name - 1)) != '/') {
            plen++;
            *name++ = '/';
        }

        art_memcpy(name, n, nlen);
    }      

    art_file_open(&(rlog->file), p, \
        ART_FILE_WRONLY, \
        ART_FILE_OPEN_AND_APPEND, \
        ART_FILE_DEFAULT_ACCESS);

    if (rlog->file.fd == ART_INVALID_FILE) {
        rlog->file.fd = ART_STDERR;
        rlog->file.name.len = art_strlen("Standard error output");
        rlog->file.name.str = art_malloc(rlog->file.name.len);
        if (rlog->file.name.str == NULL) {
            len = rlog->file.name.len;
            goto failed;
        }
        art_memcpy(rlog->file.name.str, \
            "Standard error output", rlog->file.name.len);
    }

    if (p) {
        art_free(p);
    }

    return rlog;

failed:
        err = errno;
        art_log_stderr(0, "art_malloc(%ud) failed (%d: %s)",\
                len, err, strerror(err));
        if (p) {
            art_free(p);
        }

        if (rlog) {
            if (rlog->file.name.str) {  /* base on rlog */
                    art_free(rlog->file.name.str);
            }
            art_free(rlog);
        }
        
        return NULL;
}

art_state_t
art_log_add(art_log_t *h, art_log_t *a) {
    art_log_t      *p;
    if (h == NULL || a == NULL) {
        art_log_stderr(0, "art_log_add(%p %p) failed", h, a);
        return ART_ERROR;
    }

    if (h->level < a->level) {
        art_log_stderr(0, "Don't allow h->level(%i) < a->level(%i) ", \
        h->level, a->level);
    }

    for (p = h; p ; p = p->next) {
        if (p->level == a->level) {
            /* existed */
            return ART_OK;
        } 
        if (p->next) {
            if (p->next->level < a->level) {
                a->next = p->next;
                p->next = a;
                return ART_OK;
            }
        } else {
            p->next = a;
            return ART_OK;
        }
    }
}

void
art_log_error_core(art_u8_t level, art_log_t *log, art_s32_t err,
    const art_s8_t *fmt, ...) {
    va_list         args;
    art_u8_t        *p, *last, *msg;
    art_u32_t       wrote_stderr, debug_connection;
    art_u8_t        errstr[ART_MAX_ERROR_STR];

    last = errstr + ART_MAX_ERROR_STR;

    p = art_cpymem(errstr, get_time_log_str(), get_time_log_len());

    p = art_fprintf(p, last-p, " [%V] ", &err_levels[level]);

    va_start(args, fmt);
    p = art_string_core(p, last, fmt, args);
    va_end(args);

    if (err) {
        p = art_log_errno(p, last, err);
    }

    *p++ = '\n';

    for (/* void */; log; log = log->next) {
        if (log->level != level) {
            continue;
        }

        (void)art_log_write(log->file.fd, errstr, p - errstr);
        if (log->handler) {
            log->handler(log->data);
        }
    }


}