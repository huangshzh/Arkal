/*
 * Copyright (C) Arthur
 */


#include <ark_config.h>
#include <ark_gheadf.h>


static ark_socket_t *
ark_sock_set(ark_init_socket_t *os, ark_socket_option_t *opt) {
        ark_socket_t    *socket;
        ark_int32_t     fd;
        socklen_t       olen;
        ark_int32_t     reuse;

        socket = (ark_socket_t *)os;
        fd = socket->fd;

        /* size = "sizeof(int)" */
        olen = sizeof(ark_int32_t);

        if (opt->reuseaddr) {
                reuse = 1;
                if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, \
                        (void *)&(reuse), &olen) != -1) {
                        socket->reuseaddr = 1;
                }
        }

        if (opt->reuseport) {
                reuse = 1;
                if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, \
                        (void *)&(reuse), &olen) != -1) {
                        socket->reuseport = 1;
                }
        }

        if (opt->rcvbuf) {
                if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, \
                (void *)&(opt->rcvbufsize), &olen) != -1) {
                        socket->rcvbufsize = opt->rcvbufsize;
                        socket->rcvbuf = 1;
                } else {
                        socket->rcvbufsize = -1;
                }
        } else {
                if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, \
                (void *)&(socket->rcvbufsize), &olen) == -1) {
                        socket->rcvbufsize = -1;
                }
        }

        if (opt->sndbuf) {
                if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, \
                (void *)&(opt->sndbufsize), &olen) != -1) {
                        socket->sndbufsize = opt->sndbufsize;
                        socket->sndbuf = 1;
                } else {
                        socket->sndbufsize = -1;
                }
        } else {
                if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, \
                (void *)&(socket->sndbufsize), &olen) == -1) {
                        socket->sndbufsize = -1;
                }
        }

        if (opt->rcvlowat) {
                if (setsockopt(fd, SOL_SOCKET, SO_RCVLOWAT, \
                (void *)&(opt->rcvlow), &olen) != -1) {
                        socket->rcvlow = opt->rcvlow;
                        socket->rcvlowat = 1;
                } else {
                        socket->rcvlow = -1;
                }
        } else {
                if (getsockopt(fd, SOL_SOCKET, SO_RCVLOWAT, \
                (void *)&(socket->rcvlow), &olen) == -1) {
                        socket->rcvlow = -1;
                }
        }

        if (opt->sndlowat) {
                if (setsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, \
                (void *)&(opt->sndlow), &olen) != -1) {
                        socket->sndlow = opt->sndlow;
                        socket->sndlowat = 1;
                } else {
                        socket->sndlow = -1;
                }
        } else {
                if (getsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, \
                (void *)&(socket->sndlow), &olen) == -1) {
                        socket->sndlow = -1;
                }
        }

        /* size = "sizeof(struct timeval)" */
        olen = sizeof(struct timeval);
        if (opt->rcvtimeo) {
                if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, \
                (void *)&(opt->rcvout), &olen) != -1) {
                        ark_memcpy(&(socket->rcvout), &(opt->rcvout), olen);
                        socket->rcvtimeo = 1;
                }
        }

        if (opt->sndtimeo) {
                if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, \
                (void *)&(opt->sndout), &olen) != -1) {
                        ark_memcpy(&(socket->sndout), &(opt->sndout), olen)
                        socket->sndtimeo = 1;
                }
        }

        return socket;
}


ark_socket_t *
ark_sock_init(ark_pool_t *pool, ark_int32_t fa, ark_socket_option_t *opt,\
        ark_uint8_t* addr, size_t addr_len, ark_uint8_t port) {
        ark_init_socket_t       *socket;
        ark_sockin4_t           *IPv4;
        ark_sockin6_t           *IPv6;

        switch (fa) {
        case AF_INET:
                /* leave spare space for "ark_socket_t" */
                /* we can take the operation that  
                 * (ark_socket_t *) = (ark_init_socket_t *) 
                 */
                socket = ark_palloc(pool, sizeof(ark_socket_t), 1);

                socket->family = fa;
                socket->addr_text = ark_pnalloc(pool, ARK_INET4_ADDRSTRLEN, 1);
                if (socket->addr_text == NULL) {
                        return NULL;
                }
                socket->addr_text_max_len = ARK_INET4_ADDRSTRLEN;
                if (addr_len > socket->addr_text_max_len) {
                        addr_len = socket->addr_text_max_len;
                }
                ark_memcpy(socket->addr_text, addr, addr_len);
                socket->port = port;
                
                IPv4 = ark_pnalloc(pool, sizeof(ark_sockin4_t), 1);
                if (IPv4 == NULL) {
                        return NULL;
                }
                IPv4->sin_faminly = socket->family;
                IPv4->sin_port = ark_htons(socket->port);
                ark_ipv4_pton(socket->addr_text, &(IPv4->sin_addr));
                socket->socklen = sizeof(ark_sockin4_t);
                socket->sockaddr = (ark_sockaddr_t *)IPv4;
                
                socket->fd = ark_init_sock(fa, SOCK_STREAM, 0);
                if (socket->fd == -1) {
                        return NULL;
                }
                socket->state.inited = 1;

                break;

        case AF_INET6:
                /* leave spare space for "ark_socket_t" */
                /* we can take the operation that  
                 * (ark_socket_t *) = (ark_init_socket_t *) 
                 */
                socket = ark_palloc(pool, sizeof(ark_socket_t), 1);
                
                socket->family = fa;
                socket->addr_text = ark_pnalloc(pool, ARK_COM_ADDRSTRLEN, 1);
                if (socket->addr_text == NULL) {
                        return NULL;
                }
                socket->addr_text_max_len = ARK_COM_ADDRSTRLEN;
                if (addr_len > socket->addr_text_max_len) {
                        addr_len = socket->addr_text_max_len;
                }
                ark_memcpy(socket->addr_text, addr, addr_len);
                socket->port = port;

                IPv6 = ark_pnalloc(pool, sizeof(ark_sockin6_t), 1);
                if (IPv6 == NULL) {
                        return NULL;
                }
                IPv6->sin6_faminly = socket->family;
                IPv6->sin6_port = ark_htons(socket->port);
                ark_ipv6_pton(socket->addr_text, &(IPv6->sin6_addr));
                socket->socklen = sizeof(ark_sockin6_t);
                socket->sockaddr = (ark_sockaddr_t *)IPv6;
                
                socket->fd = ark_init_sock(fa, SOCK_STREAM, 0);
                if (socket->fd == -1) {
                        return NULL;
                }
                socket->state.inited = 1;

                break;
        
        default:
                return NULL;
        }

        return ark_set_socket(socket, opt);
}

inline ark_state 
ark_sock_bind(ark_socket_t *sock) {
        if(socket == NULL) {
                return ARK_ERROR;
        }

        if (ark_bind_sock(sock->fd, sock->sockaddr, sock->socklen) == 0) {
                sock->state.bound = 1;
                return ARK_OK;
        } else {
                sock->state.bound = 0;
                return ARK_ERROR;
        }
}


ark_state
ark_sock_status(ark_socket_t *sock, ark_uint8_t sta) {
        ark_int32_t             s;
        ark_int32_t             fd;
        fd = sock->fd;
        switch (sta) {
        case ARK_SERVER:             
                /* listening */
                if (sock->sock->state.bound == 0) {
                        return ARK_ERROR;
                }

                if (ark_listen_sock(fd, ARK_BACKLOG) == 0) {
                        sock->state.listening = 1;
                        return ARK_OK;
                } else {
                        sock->state.listening = 0;
                        return ARK_ERROR;
                }
                break;
        case ARK_CLIENT:
                /* connect */
                if (ark_connect_sock(fd, sock->sockaddr, sock->socklen) == 0) {
                        if (sock->state.bound == 0) {
                                getsockname(fd, sock->sockaddr, sock->socklen);
                        }
                        sock->state.connecte = 1;
                        return ARK_OK;
                } else {
                        sock->state.connecte = 0;
                        return ARK_ERROR;
                }
                break;
        default:
                return ARK_ERROR;
        }
}