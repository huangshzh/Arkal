/*
 * Copyright (C) Arthur
 */

#ifndef __ART_CONFIG_H__
#define __ART_CONFIG_H__

/* base data */
typedef int8_t          art_s8_t;            
typedef int16_t         art_s16_t;
typedef int32_t         art_s32_t;
typedef int64_t         art_s64_t;

typedef uint8_t         art_u8_t;   
typedef uint16_t        art_u16_t;
typedef uint32_t        art_u32_t;
typedef uint64_t        art_u64_t;

typedef intptr_t        art_sptr_t;
typedef uintptr_t       art_uptr_t;

/* state */
typedef int8_t          art_state_t;
#define ART_OK          0
#define ART_ERROR       -1
#define ART_AGAIN       -2
#define ART_BUSY        -3
#define ART_DONE        -4
#define ART_DECLINED    -5
#define ART_ABORT       -6

/* bool */
typedef int8_t          art_bool_t;
#define ART_TRUE        1
#define ART_FALSE       0

#define art_abs(value)              (((value) >= 0) ? (value) : - (value))
#define art_max(val1, val2)         ((val1 < val2) ? (val2) : (val1))
#define art_min(val1, val2)         ((val1 > val2) ? (val2) : (val1))

#define art_valid                   1
#define art_invalid                 0    
#define art_set_valid(flag)         flag = art_valid
#define art_set_invalid(flag)       flag = art_invalid
#define art_set_inversion(falg)     flag = ~flag

//#define ART_SYSTEM_PREFIX       "/home/arthur/arthur_system/"
#define ART_SYSTEM_PREFIX           "./"

#define ART_SPACE                   ' '
#define ART_TAB                     '\t'
#define ART_NUL                     '\0'
#define ART_CR                      '\r'
#define ART_LF                      '\n'


#endif