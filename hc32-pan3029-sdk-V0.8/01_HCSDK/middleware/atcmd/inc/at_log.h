/*
 * NOTE: DO NOT include this file on the header file.
 */

#ifndef _AT_LOG_H_
#define _AT_LOG_H_
#include "hc32_ddl.h"

#ifndef LOG_TAG
#define DBG_TAG               "at"
#else
#define DBG_TAG               LOG_TAG
#endif /* LOG_TAG */

//#define AT_DEBUG

#ifdef AT_DEBUG
#define DBG_LVL               printf
#else
#define DBG_LVL               printf
#endif /* AT_DEBUG */

#define _PC_DEBUG_
#ifdef _PC_DEBUG_
#define pcDebug(format,...) printf("%s,%05d<"format">\n", __func__, __LINE__, ##__VA_ARGS__)  
#else
#define pcDebug(format,...)
#endif

#define AT_DEBUG
#ifdef AT_DEBUG
#define atDebug(format,...) printf(""format"\n", ##__VA_ARGS__)  
#else
#define atDebug(format,...)
#endif

#define LOG_E(format,...) printf("%s,%05d<"format">\n", __func__, __LINE__, ##__VA_ARGS__)
#define LOG_I(format,...) printf("%s,%05d<"format">\n", __func__, __LINE__, ##__VA_ARGS__)

#endif

