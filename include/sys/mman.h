/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SYS_MMAN_H_
#define SYS_MMAN_H_

	#include <sys/types.h>
	#include <stddef.h>
	
	/**
	 * @brief Page protection options.
	 */
	/**@{*/
	#define PROT_NONE         0  /**< Page cannot be accessed. */
	#define PROT_EXEC   (1 << 0) /**< Page can be executed.    */
	#define PROT_WRITE  (1 << 1) /**< Page can be written.     */
	#define PROT_READ   (1 << 2) /**< Page can be read.        */
	/**@}*/

	/**
	 * @brief Flag options.
	 */
	/**@{*/
	#define MAP_FIXED   1 /**< Interpret address exactly. */
	#define MAP_PRIVATE 2 /**< Changes are private.       */
	#define MAP_SHARED  3 /**< Share changes.             */
	/**@}*/

	/**
	 * @brief Memory synchronization flags.
	 */
	/**@{*/
	#define MS_ASYNC      (1 << 0) /**< Perform asynchronous writes. */
	#define MS_SYNC       (1 << 1) /**< Perform synchronous writes.  */
	#define MS_INVALIDATE (1 << 2) /**< Invalidate cached data.      */
	/**@}*/

	/**
	 * @brief Mapping failed.
	 */
	#define MAP_FAILED NULL

	/* Forward definitions. */
	extern int shm_open(const char *, int, mode_t);
	extern int shm_unlink(const char *);
	extern void *mmap(void *, size_t, int, int, int, off_t);
	extern int munmap(void *, size_t);
	extern int msync(void *, size_t, int);

#endif /* SYS_MMAN_H_ */

