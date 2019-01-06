/*
 * Copyright (C) Arthur
 * Copyright (C) Arthur-System 
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>

int
main(int argc, char *const *argv) {
    if (art_strerror_init() != ART_OK) {
        return 1;
    }
}