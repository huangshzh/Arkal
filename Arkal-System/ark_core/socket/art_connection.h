/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

typedef ssize_t (*art_recv_pt)(art_connection_t *c, u_char *buf, size_t size);
typedef ssize_t (*art_recv_chain_pt)(art_connection_t *c, art_chain_t *in,
    off_t limit);
typedef ssize_t (*art_send_pt)(art_connection_t *c, u_char *buf, size_t size);
typedef art_chain_t *(*art_send_chain_pt)(art_connection_t *c, art_chain_t *in,
    off_t limit);

struct art_connection_s {
    void               *data;
    art_event_t        *read;
    art_event_t        *write;

    art_socket_t        fd;

    art_recv_pt         recv;
    art_send_pt         send;
    art_recv_chain_pt   recv_chain;
    art_send_chain_pt   send_chain;

    art_listening_t    *listening;

    off_t               sent;

    art_log_t          *log;

    art_pool_t         *pool;

    int                 type;

    struct sockaddr    *sockaddr;
    socklen_t           socklen;
    art_str_t           addr_text;

    art_str_t           proxy_protocol_addr;
    in_port_t           proxy_protocol_port;

#if (art_SSL || art_COMPAT)
    art_ssl_connection_t  *ssl;
#endif

    struct sockaddr    *local_sockaddr;
    socklen_t           local_socklen;

    art_buf_t          *buffer;

    art_queue_t         queue;

    art_atomic_uint_t   number;

    art_uint_t          requests;

    unsigned            buffered:8;

    unsigned            log_error:3;     /* art_connection_log_error_e */

    unsigned            timedout:1;
    unsigned            error:1;
    unsigned            destroyed:1;

    unsigned            idle:1;
    unsigned            reusable:1;
    unsigned            close:1;
    unsigned            shared:1;

    unsigned            sendfile:1;
    unsigned            sndlowat:1;
    unsigned            tcp_nodelay:2;   /* art_connection_tcp_nodelay_e */
    unsigned            tcp_nopush:2;    /* art_connection_tcp_nopush_e */

    unsigned            need_last_buf:1;

#if (art_HAVE_AIO_SENDFILE || art_COMPAT)
    unsigned            busy_count:2;
#endif

#if (art_THREADS || art_COMPAT)
    art_thread_task_t  *sendfile_task;
#endif
};