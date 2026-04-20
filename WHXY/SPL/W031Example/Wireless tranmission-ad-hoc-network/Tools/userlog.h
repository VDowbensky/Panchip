/*******************************************************************************
 *  @file:  userlog.h
 *  @brief: Log management
 *******************************************************************************/
#ifndef USERLOG_H
#define USERLOG_H

#include "pro_config.h"

#define BASLOG(level, ...) \
    userlog(level, __FILE__, __LINE__, __VA_ARGS__)

//log level
typedef enum{
	LOG_LEVEL_ERROR			= 0, 
	LOG_LEVEL_WARNING		= 1, 
  LOG_LEVEL_INF			  = 2, 
	LOG_LEVEL_DEBUG			= 3, 
	LOG_LEVEL_TEMP			= 4,
  LOG_LEVEL_OFF			  = 5 
}ELOG_LEVEL;

#define SYSTEM_LOG_LEVEL    PAN_CFG_LOG_LEVEL

void userlog(ELOG_LEVEL level, const char *file, int line, char * fmt, ...);

#endif
