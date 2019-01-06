/*
 * Copyright (C) Arthur
 */

#ifndef __ART_TIMES_H__
#define __ART_TIMES_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

/* DST, Day Saving Time */
#define art_timezone(ptz) \
(ptz.tz_dsttime ? ptz.tz_minuteswest + 60 : ptz.tz_minuteswest);

typedef struct {
    time_t          sec;
    art_u32_t       msec;
    art_s32_t       gmtoff; /* min */
}art_time_t;

/* cache */
typedef struct {
    art_time_t      time;
    art_str_t       gmt_time_str;
    art_str_t       log_time_str;
}art_time_cache_t;

#define ART_MAX_SLOT    64

/* API */
extern volatile art_u32_t           art_current_msec;
extern volatile time_t              art_current_sec;
extern volatile art_time_t          *art_cached_time;
extern volatile art_str_t           *art_cached_time_gmt;
extern volatile art_str_t           *art_cached_time_log;

#define get_current_msec()          (art_u32_t)art_current_msec
#define get_current_sec()           (art_u32_t)art_current_sec
#define get_timeofday()             (art_time_t *)art_cached_time

#define get_time_gmt_len()          (size_t)art_cached_time_gmt->len
#define get_time_gmt_str()          (art_s8_t *)art_cached_time_gmt->str
#define get_time_log_len()          (size_t)art_cached_time_log->len
#define get_time_log_str()          (art_s8_t *)art_cached_time_log->str

void art_time_pool_creat(void);
void art_time_updata(void);
void art_gmtime(time_t t, struct tm *tp);

#endif