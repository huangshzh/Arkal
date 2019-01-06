/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

struct ngx_cycle_s {
    void                    ****conf_ctx;
    art_pool_t              *pool;

    art_log_t               *log;
    art_log_t               new_log;

    art_u32_t               log_use_stderr;  /* unsigned log_use_stderr:1; */

    art_connection_t        **files;
    art_connection_t        *free_connections;
    art_u32_t               free_connection_n;

    ngx_module_t            **modules;
    ngx_u32_t               modules_n;
    ngx_u32_t               modules_used;    /* unsigned modules_used:1; */

    ngx_queue_t             reusable_connections_queue;
    ngx_u32_t               reusable_connections_n;

    ngx_array_t             listening;
    ngx_array_t             paths;

    ngx_array_t             config_dump;
    ngx_rbtree_t            config_dump_rbtree;
    ngx_rbtree_node_t       config_dump_sentinel;

    ngx_list_t              open_files;
    ngx_list_t              shared_memory;

    ngx_u32_t               connection_n;
    ngx_u32_t               files_n;

    ngx_connection_t        *connections;
    ngx_event_t             *read_events;
    ngx_event_t             *write_events;

    ngx_cycle_t             *old_cycle;

    ngx_str_t               conf_file;
    ngx_str_t               conf_param;
    ngx_str_t               conf_prefix;
    ngx_str_t               prefix;
    ngx_str_t               lock_file;
    ngx_str_t               hostname;
};