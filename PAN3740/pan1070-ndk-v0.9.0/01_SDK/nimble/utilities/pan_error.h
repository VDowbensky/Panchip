/*
 * pan_error.h
 *
 */

#ifndef PAN_ERROR_H_
#define PAN_ERROR_H_

typedef int ret_code_t;

/** @defgroup PAN_ERR Error Codes Base number definitions
 * @{ */
#define PAN_ERR_BASE_NUM     		(0x700)
/* @} */

#define PAN_SUCCESS                 (PAN_ERR_BASE_NUM + 0)  //!< successful
#define PAN_ERR_NO_MEM              (PAN_ERR_BASE_NUM + 1)  //!< No Memory for operation
#define PAN_ERR_NO_FOUND            (PAN_ERR_BASE_NUM + 2)  //!< No found
#define PAN_ERR_NO_SUPPORTED        (PAN_ERR_BASE_NUM + 3)  //!< No supported
#define PAN_ERR_INVALID_PARAM       (PAN_ERR_BASE_NUM + 4)  //!< Invalid parameter
#define PAN_ERR_INVALID_STATE       (PAN_ERR_BASE_NUM + 5)  //!< Invalid state
#define PAN_ERR_INVALID_LENGTH      (PAN_ERR_BASE_NUM + 6)  //!< Invalid length
#define PAN_ERR_INVALID_FLAGS       (PAN_ERR_BASE_NUM + 7)  //!< Invalid flags
#define PAN_ERR_INVALID_DATA        (PAN_ERR_BASE_NUM + 8)  //!< Invalid data
#define PAN_ERR_NULL                (PAN_ERR_BASE_NUM + 9)  //!< Null Pointer
#define PAN_ERR_TIMEOUT             (PAN_ERR_BASE_NUM + 10) //!< Operation timed out
#define PAN_ERR_INVALID_MEM_ADDR    (PAN_ERR_BASE_NUM + 11) //!< Bad Memory Address
#define PAN_ERR_BUSY                (PAN_ERR_BASE_NUM + 12) //!< Busy
#define PAN_ERR_RESOURCES           (PAN_ERR_BASE_NUM + 12) //!< No resources


#endif /* PAN_ERROR_H_ */
