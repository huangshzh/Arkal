/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

/* API */
volatile art_u32_t          art_current_msec;
volatile time_t             art_current_sec;
volatile art_time_t         *art_cached_time;
volatile art_str_t          *art_cached_time_gmt;
volatile art_str_t          *art_cached_time_log;

static art_u8_t  *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static art_u8_t  *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/* lock */
static art_atomic_t         art_time_lock;

static art_time_cache_t     art_time_cache_slot[ART_MAX_SLOT];
static size_t art_slot;


void
art_time_pool_creat(void) {        
    art_s32_t      i;
    art_s32_t      len1, len2;
    art_u8_t       *p;

    art_memzero(&art_time_cache_slot, sizeof(art_time_cache_slot));

    for(i = 0; i < ART_MAX_SLOT; i++) {
        /* GMT */
        art_time_cache_slot[i].gmt_time_str.len = \
		art_strlen("Mon, 28 Sep 1970 06:00:00 GMT") ;
                
        len1 = art_time_cache_slot[i].gmt_time_str.len;
        p = art_malloc(len1);
        art_memzero(p, len1);
        art_time_cache_slot[i].gmt_time_str.str = p;
        
        /* log */
        art_time_cache_slot[i].log_time_str.len = \
		art_strlen("1970/09/28 12:00:00");

        len2 = art_time_cache_slot[i].log_time_str.len;
        p = art_malloc(len2);
        art_memzero(p, len2);
        art_time_cache_slot[i].log_time_str.str = p;
    }

    art_slot = 0;
    art_time_updata();
}


void
art_time_updata(void) {
    struct timeval          tv;
    struct timezone         tz;
    struct tm               gmt, tm;
    time_t                  sec;
    art_u32_t               msec;
    art_s32_t               gmtoff;
    art_time_cache_t        *tp; 
    art_u8_t                *p0, *p1;

    if (!art_trylock(&art_time_lock)) {
        return;
    }

    gettimeofday(&tv, &tz);
    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;
    /* timer */
    art_current_sec  = sec;
    art_current_msec = msec;
    gmtoff = art_timezone(tz);


    tp = &art_time_cache_slot[art_slot];

    if (tp->time.sec == sec) {
        /* time's value change in 1s, just adjust msec value */
        tp->time.msec = msec;
        art_unlock(&art_time_lock);
        return;
    }


    if (art_slot >= ART_MAX_SLOT - 1) {
        art_slot = 0;
    } else {
        art_slot++;
    }

    tp = &art_time_cache_slot[art_slot];
        
    tp->time.sec = sec;
    tp->time.msec = msec;
    tp->time.gmtoff = gmtoff;


    /* get GMT */
    p0 = tp->gmt_time_str.str;

    art_gmtime(sec, &gmt);
    art_fprintf(p0, tp->gmt_time_str.len, "%s, %02i %s %4i %02i:%02i:%02i GMT",
        week[gmt.tm_wday], gmt.tm_mday,
        months[gmt.tm_mon - 1], gmt.tm_year,
        gmt.tm_hour, gmt.tm_min, gmt.tm_sec);

    /* get log time */
    p1 = tp->log_time_str.str;

    art_gmtime(sec + gmtoff * 60, &tm);
    art_fprintf(p1, tp->log_time_str.len,"%4i/%02i/%02i %02i:%02i:%02i",
        tm.tm_year, tm.tm_mon,
        tm.tm_mday, tm.tm_hour,
        tm.tm_min, tm.tm_sec);
        
    art_memory_barrier();

    art_cached_time = &(tp->time);
    art_cached_time_log = &(tp->log_time_str);
    art_cached_time_gmt = &(tp->gmt_time_str);

    art_unlock(&art_time_lock);
}

void
art_gmtime(time_t t, struct tm *tp)
{
    art_s32_t       yday;
    art_u32_t       sec, min, hour, mday, mon, year, wday, days, leap;

    /* the calculation is valid for positive time_t only */

    if (t < 0) {
        t = 0;
    }

    days = t / 86400;
    sec = t % 86400;

    /*
     * no more than 4 year digits supported,
     * truncate to December 31, 9999, 23:59:59
     */

    if (days > 2932896) {
        days = 2932896;
        sec = 86399;
    }

    /* January 1, 1970 was Thursday */

    wday = (4 + days) % 7;

    hour = sec / 3600;
    sec %= 3600;
    min = sec / 60;
    sec %= 60;

    /*
     * the algorithm based on Gauss' formula,
     * see src/core/ngx_parse_time.c
     */

    /* days since March 1, 1 BC */
    days = days - (31 + 28) + 719527;

    /*
     * The "days" should be adjusted to 1 only, however, some March 1st's go
     * to previous year, so we adjust them to 2.  This causes also shift of the
     * last February days to next year, but we catch the case when "yday"
     * becomes negative.
     */

    year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);

    yday = days - (365 * year + year / 4 - year / 100 + year / 400);

    if (yday < 0) {
        leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));
        yday = 365 + leap + yday;
        year--;
    }

    /*
     * The empirical formula that maps "yday" to month.
     * There are at least 10 variants, some of them are:
     * mon = (yday + 31) * 15 / 459
     * mon = (yday + 31) * 17 / 520
     * mon = (yday + 31) * 20 / 612
    */

    mon = (yday + 31) * 10 / 306;

    /* the Gauss' formula that evaluates days before the month */

    mday = yday - (367 * mon / 12 - 30) + 1;

    if (yday >= 306) {

    year++;
    mon -= 10;

    /*
     * there is no "yday" in Win32 SYSTEMTIME
     *
     * yday -= 306;
     */

    } else {

    mon += 2;

    /*
     * there is no "yday" in Win32 SYSTEMTIME
     *
     * yday += 31 + 28 + leap;
     */
    }

    tp->tm_sec = (art_s32_t) sec;
    tp->tm_min = (art_s32_t) min;
    tp->tm_hour = (art_s32_t) hour;
    tp->tm_mday = (art_s32_t) mday;
    tp->tm_mon = (art_s32_t) mon;
    tp->tm_year = (art_s32_t) year;
    tp->tm_wday = (art_s32_t) wday;
}
/* update on 18-11-28 end */