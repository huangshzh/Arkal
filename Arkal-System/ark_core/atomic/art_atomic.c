/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

inline art_atomic_u_t
art_atomic_cmp_set(art_atomic_t *lock, art_atomic_u_t old, art_atomic_u_t set) {
    art_u8_t     res;

    __asm__ volatile (
        ART_SMP_LOCK
        "    cmpxchgl  %3, %1;   "
        "    sete      %0;       "
        : "=a" (res) : "m" (*lock), "a" (old), "r" (set) : "cc", "memory");

    return res;
}

void
art_spinlock(art_atomic_t *lock, art_atomic_s_t value, art_u32_t spin) {
    art_u32_t   i, n;
    art_s32_t   art_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
        
    for ( ;; ) {
        if (*lock == 0 && art_atomic_cmp_set(lock, 0, value)) {
            return;
        }

        if (art_ncpu > 1) {
            for (n = 1; n < spin; n <<= 1) {
                for (i = 0; i < n; i++) {
                    art_cpu_pause();
                }

                if (*lock == 0 && art_atomic_cmp_set(lock, 0, value)) {
                    return;
                }
            }
        }

        art_sched_yield();
    }
}