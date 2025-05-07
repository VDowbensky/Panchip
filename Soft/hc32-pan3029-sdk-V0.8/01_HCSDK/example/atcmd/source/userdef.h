#ifndef _USERDEF_H
#define _USERDEF_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "hc32_ddl.h"

/**
 * @brief Max value for unsined long integer.
 */
#define ULONG_MAX                      ( 0xFFFFFFFFUL )

/** pan error codes */
typedef enum
{
    PN_OK = 0,
    PN_ERROR = -1,
    PN_TIMEOUT = -2,
    PN_NOTREADY = -3,
}pn_result_t;

typedef signed char      s8;
typedef unsigned char    u8;
typedef signed short     s16;
typedef unsigned short   u16;
typedef signed long      s32;
typedef unsigned long    u32;

typedef signed char      S8;
typedef unsigned char    U8;
typedef signed short     S16;
typedef unsigned short   U16;
typedef signed long      S32;
typedef unsigned long    U32;


typedef signed char      rt_int8_t;
typedef unsigned char    rt_uint8_t;
typedef unsigned short   rt_uint16_t;
typedef signed short     rt_int16_t;
typedef signed long      rt_int32_t;
typedef unsigned long    rt_uint32_t;

typedef unsigned int     rt_size_t;      /**< Type for size number */

typedef int              rt_bool_t;      /**< boolean type */
typedef long             rt_base_t;      /**< Nbit CPU related date type */
typedef unsigned long    rt_ubase_t;     /**< Nbit unsigned CPU related data type */

typedef rt_base_t        rt_err_t;       /**< Type for error number */
typedef rt_uint32_t      rt_time_t;      /**< Type for time stamp */
typedef rt_uint32_t      rt_tick_t;      /**< Type for tick count */
typedef rt_base_t        rt_flag_t;      /**< Type for flags */
typedef rt_ubase_t       rt_dev_t;       /**< Type for device */
typedef rt_base_t        rt_off_t;       /**< Type for offset */


#define RT_ASSERT(x)
#define ALIGN(n)        __attribute__((aligned(n)))
#define RT_SECTION(x)   __attribute__((section(x)))
#define RT_WEAK         __attribute__((weak))
#define RT_UNUSED       __attribute__((unused))
#define RT_USED         __attribute__((used))

#define rt_inline       static __inline
#define rt_kprintf      printf
#define rt_memset       memset  
#define rt_strlen       strlen  
#define rt_strncpy      strncpy 
#define rt_strncmp      strncmp 
#define rt_memcpy       memcpy

#define RT_EOK              0   /* There is no error                        */
#define RT_ERROR            1   /* A generic error happens                  */
#define RT_ETIMEOUT         2   /* Timed out                                */
#define RT_EFULL            3   /* The resource is full                     */
#define RT_EEMPTY           4   /* The resource is empty                    */
#define RT_ENOMEM           5   /* No memory                                */
#define RT_ENOSYS           6   /* No system                                */
#define RT_EBUSY            7   /* Busy                                     */

/**
 * @def PN_NULL
 * Similar as the \c NULL in C library.
 */
#define PN_NULL             ((void *)0)

#define rt_inline           static __inline

struct rt_list_node
{
    struct rt_list_node     *next;          /* point to next node.                      */
    struct rt_list_node     *prev;          /* point to prev node.                      */
};

typedef struct rt_list_node rt_list_t;      /* Type for lists.                          */

#endif

