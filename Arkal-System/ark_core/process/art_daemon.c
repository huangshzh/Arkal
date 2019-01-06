/*
 * Copyright (C) Arthur
 */

#include <art_linux.h>
#include <art_config.h>
#include <art_core.h>


art_state_t
art_daemon(art_log_t *log)
{
    int  fd;

    switch (fork()) {
    case -1:
        art_log_err(ART_LOG_EMERG, log, art_errno, "fork() failed");
        return ART_ERROR;

    case 0:         /* chiled */
        break;

    default:        /* parent */
        exit(0);
    }

    if (setsid() == -1) {
        art_log_err(ART_LOG_EMERG, log, art_errno, "setsid() failed");
        return ART_ERROR;
    }

    umask(0);

    fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        art_log_err(ART_LOG_EMERG, log, art_errno, \
                "open(\"/dev/null\") failed");
        return ART_ERROR;
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
        art_log_err(ART_LOG_EMERG, log, art_errno, "dup2(STDIN) failed");
        return ART_ERROR;
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        art_log_err(ART_LOG_EMERG log, art_errno, "dup2(STDOUT) failed");
        return ART_ERROR;
    }

    if (fd > STDERR_FILENO) {
        if (close(fd) == -1) {
            art_log_err(ART_LOG_EMERG, log, art_errno, "close() failed");
            return ART_ERROR;
        }
    }

    return ART_OK;
}