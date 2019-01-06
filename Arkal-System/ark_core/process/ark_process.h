/*
 * Copyright (C) Arthur
 */


#ifndef __ARK_PROCESS_H__
#define __ARK_PROCESS_H__


#include <ark_config.h>
#include <ark_gheadf.h>


#define ARK_SETPROCTITLE_PAD    '\0'

ark_state
ark_init_setproctitle(ark_int32_t argc, ark_uint8_t *const *argv);

ark_state
ark_setproctitle(ark_uint8_t *title);

#endif 