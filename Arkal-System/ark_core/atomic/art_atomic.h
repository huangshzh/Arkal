/*
 * Copyright (C) Arthur
 */

#ifndef __ART_ATOMIC_H__
#define __ART_ATOMIC_H__

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

/*
 *      default SMP System
*/
#define ART_SMP_LOCK  "lock;"

/*
 *      some __asm__ option
*/
#define art_memory_barrier()        __asm__ volatile ("" ::: "memory")
#define art_cpu_pause()             __asm__ (".byte 0xf3, 0x90")
#define art_sched_yield()           sched_yield()

typedef art_s32_t                   art_atomic_s_t;
typedef art_u32_t                   art_atomic_u_t;
typedef volatile art_atomic_u_t     art_atomic_t;

/*
 *      ASM's format:
 *      __asm__　__volatile__
 *      ("Instruction List" : Output : Input : Clobber/Modify);
 *      
 *      Instruction List->      ART_SMP_LOCK
                                "    cmpxchgl  %3, %1;   "
                                "    sete      %0;       ":
 *      Output->                "=a" (res) :
 *      Input->                 "m" (*lock), "a" (old), "r" (set):
 *      Clobber/Modify->        "cc", "memory" 
 *      
 *      The "r" means the general register.
 *      The "=a" and "a" are the %eax register. 
 *      The "m" means the memory
 *      
 *      "cmpxchgl  r, [m]":             cmpxchgl  set, *lock
 *      
 *      if (eax == [m]) {               if (old == *lock) {
 *              zf = 1;                         zf = 1;
 *              [m] = r;                        *lock = set;
 *      } else {                        } else {
 *              zf = 0;                         zf = 0;
 *              eax = [m];                      old = *lock;
 *      }                               }
 *      
 *      "sete    %eax"                  res = zf;
 * 
 *       
*/

inline art_atomic_u_t
art_atomic_cmp_set(art_atomic_t *lock, art_atomic_u_t old, art_atomic_u_t set);

void
art_spinlock(art_atomic_t *lock, art_atomic_s_t value, art_u32_t spin);

#define art_trylock(lock)     (*(lock) == 0 && art_atomic_cmp_set(lock, 0, 1))
#define art_unlock(lock)      *(lock) = 0

#endif  /* update on 2018-12-18 */