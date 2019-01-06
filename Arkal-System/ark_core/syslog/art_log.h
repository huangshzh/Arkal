/*
 * Copyright (C) Arthur
 */

#ifndef __ART_LOG_H__
#define __ART_LOG_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>


#define ART_LOG_STDERR      0       
#define ART_LOG_EMERG       1   /* Emergencies - system is unusable */
#define ART_LOG_ALERT       2   /* Action must be taken immediately */
#define ART_LOG_CRIT        3   /* Critical Conditions */
#define ART_LOG_ERROR       4   /* Error conditions */
#define ART_LOG_WARN        5   /* Warning conditions */
#define ART_LOG_NOTICE      6   /* Normal but significant condition */
#define ART_LOG_INFO        7   /* Informational */
#define ART_LOG_DEBUG       8   /* Debug-level messages */

#define ART_LOG_EMERG_PATH      "logs/emerg.log"
#define ART_LOG_ALERT_PATH      "logs/alert.log"
#define ART_LOG_CRIT_PATH       "logs/crit.log"
#define ART_LOG_ERROR_PATH      "logs/error.log"
#define ART_LOG_WARN_PATH       "logs/warn.log"
#define ART_LOG_NOTICE_PATH     "logs/notice.log"
#define ART_LOG_INFO_PATH       "logs/info.log"
#define ART_LOG_DEBUG_PATH      "debug/debug.log"

#define ART_MAX_ERROR_STR       2048
#define ART_SYS_NERR            135

#define art_errno               errno   

typedef struct art_log_s        art_log_t;
typedef void(*art_log_callback_pt)(void *data);

struct art_log_s{
    art_u8_t                level;
    art_file_t              file;
    art_u8_t                connection;
        
        
    art_log_callback_pt     handler;
    void                    *data;

    art_log_t               *next;
};

#define art_log_err(lv, log, args...) \
    if ((log)->level >= lv) art_log_error_core(lv, log, args)

art_state_t art_strerror_init(void);
art_log_t *art_log_init(art_u8_t *prefix, art_u8_t level);
art_state_t art_log_add(art_log_t *h, art_log_t *a);
void art_log_stderr(art_s32_t err, const art_u8_t *fmt, ...);

void
art_log_error_core(art_u8_t level, art_log_t *log, art_s32_t err,
    const art_s8_t *fmt, ...);

#endif