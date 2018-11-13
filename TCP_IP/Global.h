/*
 * Copyright (C) Arthur
 * the idea from Nginx
 * Created by arthur on 18-11-13.
 */
#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <atomic_ops.h>

/* formatting data type */
typedef int8_t          Ar_char8_t;
typedef int16_t         Ar_int16_t;
typedef int32_t         Ar_int32_t;
typedef int64_t         Ar_int64_t;

typedef uint8_t         Ar_uchar8_t;
typedef uint16_t        Ar_uint16_t;
typedef uint32_t        Ar_uint32_t;
typedef uint64_t        Ar_uint64_t;

typedef intptr_t        Ar_intptr_t;
typedef uintptr_t       Ar_uintptr_t;

/* time.h */
typedef time_t          Ar_time_t;
typedef struct tm       Ar_tm_t;
#define Ar_tm_sec       tm_sec
#define Ar_tm_min       tm_min
#define Ar_tm_hour      tm_hour
#define Ar_tm_mday      tm_mday
#define Ar_tm_mon       tm_mon
#define Ar_tm_year      tm_year
#define Ar_tm_wday      tm_wday
#define Ar_tm_isdst     tm_isdst

/* atomic_ops.h */
typedef AO_t                        Ar_atomic_uint_t;
typedef volatile Ar_atomic_uint_t   Ar_atomic_t;

Ar_atomic_uint_t
Arthur_atomic_cmp_set(Ar_atomic_t *lock, Ar_atomic_t old,
    Ar_atomic_t set);

#define Arthur_atomic_cmp_set(lock, old, set)                                    \
    __sync_bool_compare_and_swap(lock, old, set)

#define Ar_trylock(lock)  (*(lock) == 0 && Arthur_atomic_cmp_set(lock, 0, 1))  /* P */
#define Ar_unlock(lock)    *(lock) = 0                                      /* V */

#endif