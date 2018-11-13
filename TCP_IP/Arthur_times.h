/*
 * Copyright (C) Arthur
 * the idea from Nginx
 * Created by arthur on 18-11-13.
 */
#ifndef __ARTHUR_TIMES_H__
#define __ARTHUR_TIMES_H__
#include <Global.h>

  
typedef struct {
    Ar_time_t        sec;
    Ar_uint64_t      msec;
} Ar_times_t;

#define Arthur_gettimeofday(tvp)  (void) gettimeofday(tvp, NULL);

static Ar_char8_t  *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static Ar_char8_t  *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#define ARTHUR_TIME_CACHED_SLOTS  10
static Ar_int32_t       times_slot;
static Ar_atomic_t      Ar_time_lock;
static Ar_times_t       cached_time[ARTHUR_TIME_CACHED_SLOTS];                                //cached
static Ar_uchar8_t      
static Ar_uchar8_t      cached_time_str[ARTHUR_TIME_CACHED_SLOTS][sizeof("Mon, 28 Sep 1970 06:00:00 GMT")];

void Arthur_time_init(void);                                        //initialization
void Arthur_time_update(void);                                      //update  
void Arthur_time_sigsafe_update(void);                              //update by signal

void Arthur_gmtime(time_t t, ngx_tm_t *tp);                         //time_t->struct tm
#define Ar_tm_sec_t          int
#define Ar_tm_min_t          int
#define Ar_tm_hour_t         int
#define Ar_tm_mday_t         int
#define Ar_tm_mon_t          int
#define Ar_tm_year_t         int
#define Ar_tm_wday_t         int




/*
    volatile variable, read values from variable's address rather cached.
*/
extern volatile Ar_time_t   *Ar_cached_time;                         //times cached

#define ngx_time()           Ar_cached_time->sec
#define ngx_timeofday()      (ngx_time_t *) Ar_cached_time

/*
 * milliseconds elapsed since epoch and truncated to ngx_msec_t,
 * used in event timers
 */
extern volatile ngx_msec_t  ngx_current_msec;

#endif