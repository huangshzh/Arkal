/*
 * Copyright (C) Arthur
 * the idea from Nginx
 * Created by arthur on 18-11-13.
 */


/*
 * The time may be updated by signal handler or by several threads.
 * The time update operations are rare and require to hold the ngx_time_lock.
 * The time read operations are frequent, so they are lock-free and get time
 * values and strings from the current slot.  Thus thread may get the corrupted
 * values only if it is preempted while copying and then it is not scheduled
 * to run more than NGX_TIME_SLOTS seconds.
 * 
 * localtime() and localtime_r() are not Async-Signal-Safe functions, therefore,
 * they must not be called by a signal handler, so we use the cached
 * GMT offset value. Fortunately the value is changed only two times a year.
 *                                                          from Nginx
 */

#include <Global.h>
#include "Arthur_times.h"
void Arthur_

void
Arthur_time_init(void)
{
    memset(cached_time, 0, sizeof(cached_time));
    memset(cached_time_str, 0, sizeof(cached_time_str));

    times_slot=0;
    Ar_cached_time = &cached_time[times_slot];
    Arthur_time_update();
}

void
Arthur_time_update(){
    Ar_time_t       sec;
    Ar_uint64_t     msec;

    Ar_times_t      *tp;    /*-> cached_time */
    Ar_char8_t      *ts;    /*-> cached_time_str */
    
    Ar_tm_t         gmt;
    struct timeval   tv;

    /* False means that other  handler or threads are updating */
    if (!Ar_trylock(&Ar_time_lock)) {
        return;
    }

    Arthur_gettimeofday(&tv);
    sec=tv.tv_sec;
    msec=tv.tv_usec/1000;

    
    tp = &cached_time[times_slot];

    /* in 1s, just updata msec */
    if(tp->sec==sec){
        tp->msec=msec;
        Ar_unlock(&Ar_time_lock);
        return;
    }

    if(times_slot == ARTHUR_TIME_CACHED_SLOTS-1)
    {
        times_slot=0;
    }else{
        times_slot++;
    }

    tp = &cached_time[times_slot];

    tp->sec=sec;
    tp->msec=msec;


    Arthur_gmtime(sec, &gmt);

    ts=&cached_time_str[times_slot][0];

    ngx_sprintf(ts, "%s, %02d %s %4d %02d:%02d:%02d GMT",
                week[gmt.Ar_tm_wday], gmt.Ar_tm_mday,
                months[gmt.Ar_tm_mon - 1], gmt.Ar_tm_year,
                gmt.Ar_tm_hour, gmt.Ar_tm_min, gmt.Ar_tm_sec);

    Ar_unlock(&Ar_time_lock);
}

void
Arthur_gmtime(Ar_time_t t, Ar_tm_t *tp)
{
    Ar_int32_t   yday;
    Ar_uint32_t  sec, min, hour, mday, mon, year, wday, days, leap;

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
     *     mon = (yday + 31) * 15 / 459
     *     mon = (yday + 31) * 17 / 520
     *     mon = (yday + 31) * 20 / 612
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

    tp->Ar_tm_sec = (Ar_tm_sec_t) sec;
    tp->Ar__tm_min = (Ar_tm_min_t) min;
    tp->Ar_tm_hour = (Ar_tm_hour_t) hour;
    tp->Ar_tm_mday = (Ar_tm_mday_t) mday;
    tp->Ar_tm_mon = (Ar_tm_mon_t) mon;
    tp->Ar_tm_year = (Ar_tm_year_t) year;
    tp->Ar_tm_wday = (Ar_tm_wday_t) wday;
}