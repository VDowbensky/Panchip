/**
 *******************************************************************************
 * @file     ring_buffer.h
 * @create   2024
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */

/** @file */

#ifndef ZEPHYR_INCLUDE_SYS_RING_BUFFER_H_
#define ZEPHYR_INCLUDE_SYS_RING_BUFFER_H_

// #include <kernel.h>
// #include <sys/util.h>
#include "PanSeries.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __ASSERT(test, fmt, ...) { }
#define __ASSERT_EVAL(expr1, expr2, test, fmt, ...) expr1
#define __ASSERT_NO_MSG(test) { if (!test) { while (1); } }

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/**
 * @def Z_MIN
 * @brief The smaller value between @p a and @p b.
 * @note Arguments are evaluated twice.
 */
/* Use Z_MIN for a GCC-only, single evaluation version */
#define Z_MIN(a, b) (((a) < (b)) ? (a) : (b))

/**
 * @brief Is @p x a power of two?
 * @param x value to check
 * @return true if @p x is a power of two, false otherwise
 */
static inline bool is_power_of_two(unsigned int x)
{
	return (x != 0U) && ((x & (x - 1U)) == 0U);
}

#define Z_EPERM 1               /**< Not owner */
#define Z_ENOENT 2              /**< No such file or directory */
#define Z_ESRCH 3               /**< No such context */
#define Z_EINTR 4               /**< Interrupted system call */
#define Z_EIO 5                 /**< I/O error */
#define Z_ENXIO 6               /**< No such device or address */
#define Z_E2BIG 7               /**< Arg list too long */
#define Z_ENOEXEC 8             /**< Exec format error */
#define Z_EBADF 9               /**< Bad file number */
#define Z_ECHILD 10             /**< No children */
#define Z_EAGAIN 11             /**< No more contexts */
#define Z_ENOMEM 12             /**< Not enough core */
#define Z_EACCES 13             /**< Permission denied */
#define Z_EFAULT 14             /**< Bad address */
#define Z_ENOTBLK 15            /**< Block device required */
#define Z_EBUSY 16              /**< Mount device busy */
#define Z_EEXIST 17             /**< File exists */
#define Z_EXDEV 18              /**< Cross-device link */
#define Z_ENODEV 19             /**< No such device */
#define Z_ENOTDIR 20            /**< Not a directory */
#define Z_EISDIR 21             /**< Is a directory */
#define Z_EINVAL 22             /**< Invalid argument */
#define Z_ENFILE 23             /**< File table overflow */
#define Z_EMFILE 24             /**< Too many open files */
#define Z_ENOTTY 25             /**< Not a typewriter */
#define Z_ETXTBSY 26            /**< Text file busy */
#define Z_EFBIG 27              /**< File too large */
#define Z_ENOSPC 28             /**< No space left on device */
#define Z_ESPIPE 29             /**< Illegal seek */
#define Z_EROFS 30              /**< Read-only file system */
#define Z_EMLINK 31             /**< Too many links */
#define Z_EPIPE 32              /**< Broken pipe */
#define Z_EDOM 33               /**< Argument too large */
#define Z_ERANGE 34             /**< Result too large */
#define Z_ENOMSG 35             /**< Unexpected message type */
#define Z_EDEADLK 45            /**< Resource deadlock avoided */
#define Z_ENOLCK 46             /**< No locks available */
#define Z_ENOSTR 60             /**< STREAMS device required */
#define Z_ENODATA 61            /**< Missing expected message data */
#define Z_ETIME 62              /**< STREAMS timeout occurred */
#define Z_ENOSR 63              /**< Insufficient memory */
#define Z_EPROTO 71             /**< Generic STREAMS error */
#define Z_EBADMSG 77            /**< Invalid STREAMS message */
#define Z_ENOSYS 88             /**< Function not implemented */
#define Z_ENOTEMPTY 90          /**< Directory not empty */
#define Z_ENAMETOOLONG 91       /**< File name too long */
#define Z_ELOOP 92              /**< Too many levels of symbolic links */
#define Z_EOPNOTSUPP 95         /**< Operation not supported on socket */
#define Z_EPFNOSUPPORT 96       /**< Protocol family not supported */
#define Z_ECONNRESET 104        /**< Connection reset by peer */
#define Z_ENOBUFS 105           /**< No buffer space available */
#define Z_EAFNOSUPPORT 106      /**< Addr family not supported */
#define Z_EPROTOTYPE 107        /**< Protocol wrong type for socket */
#define Z_ENOTSOCK 108          /**< Socket operation on non-socket */
#define Z_ENOPROTOOPT 109       /**< Protocol not available */
#define Z_ESHUTDOWN 110         /**< Can't send after socket shutdown */
#define Z_ECONNREFUSED 111      /**< Connection refused */
#define Z_EADDRINUSE 112        /**< Address already in use */
#define Z_ECONNABORTED 113      /**< Software caused connection abort */
#define Z_ENETUNREACH 114       /**< Network is unreachable */
#define Z_ENETDOWN 115          /**< Network is down */
#define Z_ETIMEDOUT 116         /**< Connection timed out */
#define Z_EHOSTDOWN 117         /**< Host is down */
#define Z_EHOSTUNREACH 118      /**< No route to host */
#define Z_EINPROGRESS 119       /**< Operation now in progress */
#define Z_EALREADY 120          /**< Operation already in progress */
#define Z_EDESTADDRREQ 121      /**< Destination address required */
#define Z_EMSGSIZE 122          /**< Message size */
#define Z_EPROTONOSUPPORT 123   /**< Protocol not supported */
#define Z_ESOCKTNOSUPPORT 124   /**< Socket type not supported */
#define Z_EADDRNOTAVAIL 125     /**< Can't assign requested address */
#define Z_ENETRESET 126         /**< Network dropped connection on reset */
#define Z_EISCONN 127           /**< Socket is already connected */
#define Z_ENOTCONN 128          /**< Socket is not connected */
#define Z_ETOOMANYREFS 129      /**< Too many references: can't splice */
#define Z_ENOTSUP 134           /**< Unsupported value */
#define Z_EILSEQ 138            /**< Illegal byte sequence */
#define Z_EOVERFLOW 139         /**< Value overflow */
#define Z_ECANCELED 140         /**< Operation canceled */

#define Z_EWOULDBLOCK EAGAIN    /**< Operation would block */

#define SIZE32_OF(x) (sizeof((x)) / sizeof(uint32_t))

/* The limit is used by algorithm for distinguishing between empty and full
 * state.
 */
#define RING_BUFFER_MAX_SIZE 0x80000000

#define RING_BUFFER_SIZE_ASSERT_MSG \
	"Size too big, if it is the ring buffer test check custom max size"
/**
 * @brief A structure to represent a ring buffer
 */
struct ring_buf {
	uint32_t head;          /**< Index in buf for the head element */
	uint32_t tail;          /**< Index in buf for the tail element */
	union ring_buf_misc {
		struct ring_buf_misc_item_mode {
			uint32_t dropped_put_count; /**< Running tally of the
			                             * number of failed put
			                             * attempts.
			                             */
		} item_mode;
		struct ring_buf_misc_byte_mode {
			uint32_t tmp_tail;
			uint32_t tmp_head;
		} byte_mode;
	} misc;
	uint32_t size;   /**< Size of buf in 32-bit chunks */

	union ring_buf_buffer {
		uint32_t *buf32; /**< Memory region for stored entries */
		uint8_t *buf8;
	} buf;
	uint32_t mask;   /**< Modulo mask if size is a power of 2 */

//	struct k_spinlock lock;
};

/**
 * @defgroup ring_buffer_apis Ring Buffer APIs
 * @ingroup datastructure_apis
 * @{
 */

/**
 * @brief Define and initialize a high performance ring buffer.
 *
 * This macro establishes a ring buffer whose size must be a power of 2;
 * that is, the ring buffer contains 2^pow 32-bit words, where @a pow is
 * the specified ring buffer size exponent. A high performance ring buffer
 * doesn't require the use of modulo arithmetic operations to maintain itself.
 *
 * The ring buffer can be accessed outside the module where it is defined
 * using:
 *
 * @code extern struct ring_buf <name>; @endcode
 *
 * @param name Name of the ring buffer.
 * @param pow Ring buffer size exponent.
 */
#define RING_BUF_ITEM_DECLARE_POW2(name, pow)			     \
	BUILD_ASSERT((1 << pow) < RING_BUFFER_MAX_SIZE,		     \
		     RING_BUFFER_SIZE_ASSERT_MSG);		     \
	static uint32_t __noinit _ring_buffer_data_##name[BIT(pow)]; \
	struct ring_buf name = {				     \
		.size = (BIT(pow)),				     \
		.mask = (BIT(pow)) - 1,				     \
		.buf = { .buf32 = _ring_buffer_data_##name }	     \
	}

/**
 * @brief Define and initialize a standard ring buffer.
 *
 * This macro establishes a ring buffer of an arbitrary size. A standard
 * ring buffer uses modulo arithmetic operations to maintain itself.
 *
 * The ring buffer can be accessed outside the module where it is defined
 * using:
 *
 * @code extern struct ring_buf <name>; @endcode
 *
 * @param name Name of the ring buffer.
 * @param size32 Size of ring buffer (in 32-bit words).
 */
#define RING_BUF_ITEM_DECLARE_SIZE(name, size32)		   \
	BUILD_ASSERT(size32 < RING_BUFFER_MAX_SIZE,		   \
		     RING_BUFFER_SIZE_ASSERT_MSG);		   \
	static uint32_t __noinit _ring_buffer_data_##name[size32]; \
	struct ring_buf name = {				   \
		.size = size32,					   \
		.buf = { .buf32 = _ring_buffer_data_##name }	   \
	}

/**
 * @brief Define and initialize a ring buffer for byte data.
 *
 * This macro establishes a ring buffer of an arbitrary size.
 *
 * The ring buffer can be accessed outside the module where it is defined
 * using:
 *
 * @code extern struct ring_buf <name>; @endcode
 *
 * @param name  Name of the ring buffer.
 * @param size8 Size of ring buffer (in bytes).
 */
#define RING_BUF_DECLARE(name, size8)				 \
	BUILD_ASSERT(size8 < RING_BUFFER_MAX_SIZE,		 \
		     RING_BUFFER_SIZE_ASSERT_MSG);		 \
	static uint8_t __noinit _ring_buffer_data_##name[size8]; \
	struct ring_buf name = {				 \
		.size = size8,					 \
		.buf = { .buf8 = _ring_buffer_data_##name }	 \
	}


/**
 * @brief Initialize a ring buffer.
 *
 * This routine initializes a ring buffer, prior to its first use. It is only
 * used for ring buffers not defined using RING_BUF_DECLARE,
 * RING_BUF_ITEM_DECLARE_POW2 or RING_BUF_ITEM_DECLARE_SIZE.
 *
 * Setting @a size to a power of 2 establishes a high performance ring buffer
 * that doesn't require the use of modulo arithmetic operations to maintain
 * itself.
 *
 * @param buf Address of ring buffer.
 * @param size Ring buffer size (in 32-bit words or bytes).
 * @param data Ring buffer data area (uint32_t data[size] or uint8_t data[size]
 *	       for bytes mode).
 */
static inline void ring_buf_init(struct ring_buf *buf,
				 uint32_t size,
				 void *data)
{
	__ASSERT(size < RING_BUFFER_MAX_SIZE, RING_BUFFER_SIZE_ASSERT_MSG);

	memset(buf, 0, sizeof(struct ring_buf));
	buf->size = size;
	buf->buf.buf32 = (uint32_t *)data;
	if (is_power_of_two(size)) {
		buf->mask = size - 1U;
	} else {
		buf->mask = 0U;
	}
}

/**
 * @brief Determine if a ring buffer is empty.
 *
 * @param buf Address of ring buffer.
 *
 * @return 1 if the ring buffer is empty, or 0 if not.
 */
int ring_buf_is_empty(struct ring_buf *buf);

/**
 * @brief Reset ring buffer state.
 *
 * @param buf Address of ring buffer.
 */
static inline void ring_buf_reset(struct ring_buf *buf)
{
	buf->head = 0;
	buf->tail = 0;
	memset(&buf->misc, 0, sizeof(buf->misc));
}

/**
 * @brief Determine free space in a ring buffer.
 *
 * @param buf Address of ring buffer.
 *
 * @return Ring buffer free space (in 32-bit words or bytes).
 */
uint32_t ring_buf_space_get(struct ring_buf *buf);

/**
 * @brief Return ring buffer capacity.
 *
 * @param buf Address of ring buffer.
 *
 * @return Ring buffer capacity (in 32-bit words or bytes).
 */
static inline uint32_t ring_buf_capacity_get(struct ring_buf *buf)
{
	return buf->size;
}

/**
 * @brief Determine used space in a ring buffer.
 *
 * @param buf Address of ring buffer.
 *
 * @return Ring buffer space used (in 32-bit words or bytes).
 */
uint32_t ring_buf_size_get(struct ring_buf *buf);

/**
 * @brief Write a data item to a ring buffer.
 *
 * This routine writes a data item to ring buffer @a buf. The data item
 * is an array of 32-bit words (from zero to 1020 bytes in length),
 * coupled with a 16-bit type identifier and an 8-bit integer value.
 *
 * @warning
 * Use cases involving multiple writers to the ring buffer must prevent
 * concurrent write operations, either by preventing all writers from
 * being preempted or by using a mutex to govern writes to the ring buffer.
 *
 * @param buf Address of ring buffer.
 * @param type Data item's type identifier (application specific).
 * @param value Data item's integer value (application specific).
 * @param data Address of data item.
 * @param size32 Data item size (number of 32-bit words).
 *
 * @retval 0 Data item was written.
 * @retval -EMSGSIZE Ring buffer has insufficient free space.
 */
int ring_buf_item_put(struct ring_buf *buf, uint16_t type, uint8_t value,
		      uint32_t *data, uint8_t size32);

/**
 * @brief Read a data item from a ring buffer.
 *
 * This routine reads a data item from ring buffer @a buf. The data item
 * is an array of 32-bit words (up to 1020 bytes in length),
 * coupled with a 16-bit type identifier and an 8-bit integer value.
 *
 * @warning
 * Use cases involving multiple reads of the ring buffer must prevent
 * concurrent read operations, either by preventing all readers from
 * being preempted or by using a mutex to govern reads to the ring buffer.
 *
 * @param buf Address of ring buffer.
 * @param type Area to store the data item's type identifier.
 * @param value Area to store the data item's integer value.
 * @param data Area to store the data item. Can be NULL to discard data.
 * @param size32 Size of the data item storage area (number of 32-bit chunks).
 *
 * @retval 0 Data item was fetched; @a size32 now contains the number of
 *         32-bit words read into data area @a data.
 * @retval -EAGAIN Ring buffer is empty.
 * @retval -EMSGSIZE Data area @a data is too small; @a size32 now contains
 *         the number of 32-bit words needed.
 */
int ring_buf_item_get(struct ring_buf *buf, uint16_t *type, uint8_t *value,
		      uint32_t *data, uint8_t *size32);

/**
 * @brief Allocate buffer for writing data to a ring buffer.
 *
 * With this routine, memory copying can be reduced since internal ring buffer
 * can be used directly by the user. Once data is written to allocated area
 * number of bytes written can be confirmed (see @ref ring_buf_put_finish).
 *
 * @warning
 * Use cases involving multiple writers to the ring buffer must prevent
 * concurrent write operations, either by preventing all writers from
 * being preempted or by using a mutex to govern writes to the ring buffer.
 *
 * @warning
 * Ring buffer instance should not mix byte access and item access
 * (calls prefixed with ring_buf_item_).
 *
 * @param[in]  buf  Address of ring buffer.
 * @param[out] data Pointer to the address. It is set to a location within
 *		    ring buffer.
 * @param[in]  size Requested allocation size (in bytes).
 *
 * @return Size of allocated buffer which can be smaller than requested if
 *	   there is not enough free space or buffer wraps.
 */
uint32_t ring_buf_put_claim(struct ring_buf *buf,
			    uint8_t **data,
			    uint32_t size);

/**
 * @brief Indicate number of bytes written to allocated buffers.
 *
 * @warning
 * Use cases involving multiple writers to the ring buffer must prevent
 * concurrent write operations, either by preventing all writers from
 * being preempted or by using a mutex to govern writes to the ring buffer.
 *
 * @warning
 * Ring buffer instance should not mix byte access and item access
 * (calls prefixed with ring_buf_item_).
 *
 * @param  buf  Address of ring buffer.
 * @param  size Number of valid bytes in the allocated buffers.
 *
 * @retval 0 Successful operation.
 * @retval -EINVAL Provided @a size exceeds free space in the ring buffer.
 */
int ring_buf_put_finish(struct ring_buf *buf, uint32_t size);

/**
 * @brief Write (copy) data to a ring buffer.
 *
 * This routine writes data to a ring buffer @a buf.
 *
 * @warning
 * Use cases involving multiple writers to the ring buffer must prevent
 * concurrent write operations, either by preventing all writers from
 * being preempted or by using a mutex to govern writes to the ring buffer.
 *
 * @warning
 * Ring buffer instance should not mix byte access and item access
 * (calls prefixed with ring_buf_item_).
 *
 * @param buf Address of ring buffer.
 * @param data Address of data.
 * @param size Data size (in bytes).
 *
 * @retval Number of bytes written.
 */
uint32_t ring_buf_put(struct ring_buf *buf, const uint8_t *data, uint32_t size);

/**
 * @brief Get address of a valid data in a ring buffer.
 *
 * With this routine, memory copying can be reduced since internal ring buffer
 * can be used directly by the user. Once data is processed it can be freed
 * using @ref ring_buf_get_finish.
 *
 * @warning
 * Use cases involving multiple reads of the ring buffer must prevent
 * concurrent read operations, either by preventing all readers from
 * being preempted or by using a mutex to govern reads to the ring buffer.
 *
 * @warning
 * Ring buffer instance should not mix byte access and item access
 * (calls prefixed with ring_buf_item_).
 *
 * @param[in]  buf  Address of ring buffer.
 * @param[out] data Pointer to the address. It is set to a location within
 *		    ring buffer.
 * @param[in]  size Requested size (in bytes).
 *
 * @return Number of valid bytes in the provided buffer which can be smaller
 *	   than requested if there is not enough free space or buffer wraps.
 */
uint32_t ring_buf_get_claim(struct ring_buf *buf,
			    uint8_t **data,
			    uint32_t size);

/**
 * @brief Indicate number of bytes read from claimed buffer.
 *
 * @warning
 * Use cases involving multiple reads of the ring buffer must prevent
 * concurrent read operations, either by preventing all readers from
 * being preempted or by using a mutex to govern reads to the ring buffer.
 *
 * @warning
 * Ring buffer instance should not mix byte access and  item mode
 * (calls prefixed with ring_buf_item_).
 *
 * @param  buf  Address of ring buffer.
 * @param  size Number of bytes that can be freed.
 *
 * @retval 0 Successful operation.
 * @retval -EINVAL Provided @a size exceeds valid bytes in the ring buffer.
 */
int ring_buf_get_finish(struct ring_buf *buf, uint32_t size);

/**
 * @brief Read data from a ring buffer.
 *
 * This routine reads data from a ring buffer @a buf.
 *
 * @warning
 * Use cases involving multiple reads of the ring buffer must prevent
 * concurrent read operations, either by preventing all readers from
 * being preempted or by using a mutex to govern reads to the ring buffer.
 *
 * @warning
 * Ring buffer instance should not mix byte access and  item mode
 * (calls prefixed with ring_buf_item_).
 *
 * @param buf  Address of ring buffer.
 * @param data Address of the output buffer. Can be NULL to discard data.
 * @param size Data size (in bytes).
 *
 * @retval Number of bytes written to the output buffer.
 */
uint32_t ring_buf_get(struct ring_buf *buf, uint8_t *data, uint32_t size);

/**
 * @brief Peek at data from a ring buffer.
 *
 * This routine reads data from a ring buffer @a buf without removal.
 *
 * @warning
 * Use cases involving multiple reads of the ring buffer must prevent
 * concurrent read operations, either by preventing all readers from
 * being preempted or by using a mutex to govern reads to the ring buffer.
 *
 * @warning
 * Ring buffer instance should not mix byte access and  item mode
 * (calls prefixed with ring_buf_item_).
 *
 * @warning
 * Multiple calls to peek will result in the same data being 'peeked'
 * multiple times. To remove data, use either @ref ring_buf_get or
 * @ref ring_buf_get_claim followed by @ref ring_buf_get_finish with a
 * non-zero `size`.
 *
 * @param buf  Address of ring buffer.
 * @param data Address of the output buffer. Cannot be NULL.
 * @param size Data size (in bytes).
 *
 * @retval Number of bytes written to the output buffer.
 */
uint32_t ring_buf_peek(struct ring_buf *buf, uint8_t *data, uint32_t size);

/**
 * @brief Get data pointer from a ring buffer.
 *
 * This routine get data pointer from a ring buffer @a buf.
 *
 * @warning
 * Use cases involving multiple reads of the ring buffer must prevent
 * concurrent read operations, either by preventing all readers from
 * being preempted or by using a mutex to govern reads to the ring buffer.
 *
 * @warning
 * Ring buffer instance should not mix byte access and  item mode
 * (calls prefixed with ring_buf_item_).
 *
 * @param buf  Address of ring buffer.
 * @param data Address of the output buffer. Can be NULL to discard data.
 * @param size Data size (in bytes).
 *
 * @retval Address of the output buffer.
 */
uint8_t *ring_buf_get_direct(struct ring_buf *buf, uint8_t *data, uint32_t size);

/**
 * @brief Set data pointer to a ring buffer.
 *
 * This routine set data pointer to a ring buffer @a buf.
 *
 * @warning
 * Use cases involving multiple writers to the ring buffer must prevent
 * concurrent write operations, either by preventing all writers from
 * being preempted or by using a mutex to govern writes to the ring buffer.
 *
 * @warning
 * Ring buffer instance should not mix byte access and item access
 * (calls prefixed with ring_buf_item_).
 *
 * @param buf Address of ring buffer.
 * @param data Address of data.
 * @param size Data size (in bytes).
 *
 * @retval buf Address of of bytes set.
 */
uint8_t *ring_buf_put_direct(struct ring_buf *buf, uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_SYS_RING_BUFFER_H_ */
