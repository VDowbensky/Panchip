/*******************************************************************************
 *  @file:  userlog.c
 *  @brief: Log management,Macros(SYSTEM_LOG_LEVEL) must be defined.
 *
 *******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "userlog.h"

static uint8_t ifneedprint(ELOG_LEVEL level)
{
    if (level <= SYSTEM_LOG_LEVEL && level != LOG_LEVEL_OFF) {
        return 0;
    }

    return 1;
}

void userlog(ELOG_LEVEL level, const char *file, int line, char *fmt, ...)
{
    char loglevel[5];
    va_list args;

    if (ifneedprint(level)) {
        return;
    }

    switch (level) {
    case LOG_LEVEL_TEMP:
        strcpy(loglevel, "TMP");
        break;
    case LOG_LEVEL_DEBUG:
        strcpy(loglevel, "DEG");
        break;
    case LOG_LEVEL_INF:
        strcpy(loglevel, "INF");
        break;
    case LOG_LEVEL_WARNING:
        strcpy(loglevel, "WAR");
        break;
    case LOG_LEVEL_ERROR:
        strcpy(loglevel, "ERR");
        break;
    default:
        return;
    }

    printf("[%s][%s:%d] ", loglevel, file, line);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
