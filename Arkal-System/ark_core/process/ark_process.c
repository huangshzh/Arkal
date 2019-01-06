/*
 * Copyright (C) Arthur
 */


#include <ark_config.h>
#include <ark_gheadf.h>


static ark_int32_t             ark_original_argc;
static ark_uint8_t             **ark_original_argv;
static ark_uint8_t             *ark_original_argv_last;

/* must be extern */
extern ark_uint8_t             **environ;

ark_state
ark_init_setproctitle(ark_int32_t argc, ark_uint8_t *const *argv) {
        size_t          size;
        ark_uint8_t     *p;
        ark_int32_t     i;


        ark_original_argc = argc;
        ark_original_argv = (ark_uint8_t **) argv;
        

        for (i = 0; environ[i]; i++) {
                size += strlen(environ[i]) + 1;
        }

        p = ark_malloc(size);
        if (p == NULL) {
                return ARK_ERROR;
        }

        ark_original_argv_last = ark_original_argv[0];

        for (i = 0; ark_original_argv[i]; i++) {
                if (ark_original_argv_last == ark_original_argv[i]) {
                        ark_original_argv_last = \
                        ark_original_argv[i] + strlen(ark_original_argv[i]) + 1;
                }
        }

        for (i = 0; environ[i]; i++) {
                if (ark_original_argv_last == environ[i]) {
                        size = strlen(environ[i]) + 1;
                        ark_original_argv_last = environ[i] + size;

                        ark_memcpy(p, (ark_uint8_t *) environ[i], size);
                        environ[i] = (ark_uint8_t *) p;
                        p += size;
                }
        }
        ark_original_argv_last--;
        
        return ARK_OK;
}


ark_state
ark_setproctitle(ark_uint8_t *title) {
        ark_uint8_t     *p;
        ark_int32_t     len;

        ark_original_argv[1] = NULL;

        p = ark_cpymem((ark_uint8_t *) ark_original_argv[0], \
                (ark_uint8_t *) "Arkal-System: ", \
                strlen("Arkal-System: "));

        p = ark_cpymem(p, (ark_uint8_t *)title, strlen(title));

        len = ark_original_argv_last - (ark_uint8_t *) p;

        if (len > 0) {
                ark_memset(p, ARK_SETPROCTITLE_PAD, len);
        } else {
                return ARK_ERROR;
        }

        return ARK_OK;
}
