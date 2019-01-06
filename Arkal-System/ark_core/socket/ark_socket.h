/*
 * Copyright (C) Arthur
 */


#ifndef __ARK_SOCKET_H__
#define __ARK_SOCKET_H__


#include <ark_config.h>
#include <ark_gheadf.h>


#define ark_init_sock                   socket
#define ark_bind_sock                   bind
#define ark_close_sock                  close
#define ark_listen_sock                 listen
#define ark_connect_socket              connect

/* value */
/* net */
#define ark_htons(value)                (ark_uint16_t) htons(value) 
#define ark_htonl(value)                (ark_uint32_t) htonl(value) 
/* host */
#define ark_ntohs(value)                (ark_uint16_t) ntohs(value)
#define ark_ntohl(value)                (ark_uint32_t) ntohl(value)

/* addr */
#define ARK_INET4_ADDRSTRLEN            (sizeof("255.255.255.255"))
#define ARK_INET6_ADDRSTRLEN            \
        (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:ffff.ffff")) 
#define ARK_COM_ADDRSTRLEN              \
        (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")) 

/* str->int */
#define ark_ipv4_pton(src, dst)         inet_pton(AF_INET, (char *)src, dst)
#define ark_ipv6_pton(src, dst)         inet_pton(AF_INET6, src, dst)
/* int->str */
#define ark_ipv4_ntop(src, dst)         inet_ntop(AF_INET, src, dst)
#define ark_ipv6_ntop(src, dst)         inet_ntop(AF_INET6, src, dst)

/*
 * nginx uses a default listen backlog value 511 on Linux.
 */
#define ARK_BACKLOG                     511

/*
 * socket function status
 */
#define ARK_SERVER                      0
#define ARK_CLIENT                      1

typedef struct sockaddr                 ark_sockaddr_t;
typedef struct sockaddr_in              ark_sockin4_t;
typedef struct sockaddr_in6             ark_sockin6_t;


typedef struct {
        unsigned                        inited:1;       /* inited socket */
        unsigned                        bound:1;        /* already bound */

        /* only can be in one state */
        unsigned                        listening:1;    /* star listening */
        unsigned                        connected:1;    /* connected */
} ark_socket_state_t;

typedef struct {
        ark_int32_t                     fd;
        ark_socket_state_t              state;

        ark_int32_t                     family;
        ark_uint8_t                     *addr_text;
        size_t                          addr_text_max_len;
        ark_uint8_t                     port;

        ark_sockaddr_t                  *sockaddr;
        socklen_t                       socklen;
} ark_init_socket_t;

ark_init_socket_t *
ark_init_socket(ark_pool_t *pool, ark_int32_t fa, \
        ark_uint8_t* addr, size_t addr_len, ark_uint8_t port);

/* base on ark_init_socket_t */
typedef struct {
        ark_int32_t                     fd;
        ark_socket_state_t              state;

        ark_int32_t                     family;
        ark_uint8_t                     *addr_text;
        size_t                          addr_text_max_len;
        ark_uint8_t                     port;

        ark_sockaddr_t                  *sockaddr;
        socklen_t                       socklen;
        
        /* 
         * Until there, that same to "ark_init_socket_t"
         * so we can change "ark_init_socket_t" to "ark_set_socket_t" quickly
         */

        /* 8bit */
        unsigned                        rcvbuf:1;
        unsigned                        rcvlowat:1;
        unsigned                        rcvtimeo:1;

        unsigned                        sndbuf:1;
        unsigned                        sndlowat:1;
        unsigned                        sndtimeo:1;

        unsigned                        reuseaddr:1;
        unsigned                        reuseport:1;

        ark_int32_t                     rcvbufsize;
        ark_int32_t                     rcvlow;
        struct timeval                  rcvout;

        ark_int32_t                     sndbufsize;
        ark_int32_t                     sndlow;
        struct timeval                  sndout;
} ark_socket_t;

typedef struct {
        /* 8bit */
        unsigned                        rcvbuf:1;
        unsigned                        rcvlowat:1;
        unsigned                        rcvtimeo:1;

        unsigned                        sndbuf:1;
        unsigned                        sndlowat:1;
        unsigned                        sndtimeo:1;

        unsigned                        reuseaddr:1;
        unsigned                        reuseport:1;

        ark_int32_t                     rcvbufsize;
        ark_int32_t                     rcvlow;
        struct timeval                  rcvout;

        ark_int32_t                     sndbufsize;
        ark_int32_t                     sndlow;
        struct timeval                  sndout;


} ark_socket_option_t;


static ark_socket_t *
ark_sock_set(ark_init_socket_t *os, ark_socket_option_t *opt);

ark_socket_t *
ark_sock_init(ark_pool_t *pool, ark_int32_t fa, ark_socket_option_t *opt, \
        ark_uint8_t* addr, size_t addr_len, ark_uint8_t port);

inline ark_state
ark_sock_bind(ark_socket_t *socket);

ark_state
ark_sock_status(ark_socket_t *sock, ark_uint8_t sta);

ark_sock_epoll
#endif