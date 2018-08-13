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

#ifndef _KERNEL_H_
#define _KERNEL_H_

	/**
	 * @brief Mega (10^6).
	 */
	#define MEGA (1000000)

	/**
	 * @brief MPPA-256 frequency (500 MHz).
	 */
	#define MPPA256_FREQ (400*MEGA)
	
	/**
	 * @brief Number of compute clusters.
	 */
	#define NR_CCLUSTER 16

	/**
	 * @brief Maximum buffer size (in bytes).
	 */
	#define BUFFER_SIZE_MAX (1024*1024)

	/**
	 * @brief Portal connector to master.
	 */
	#define PORTAL_MASTER "/mppa/portal/128:48"

	/**
	 * @brief Portal connector to slaves.
	 */
	#define PORTAL_SLAVES "/mppa/portal/[0..15]:49"

	/**
	 * @brief Sync connector to slaves.
	 */
	#define SYNC_MASTER "/mppa/sync/128:48"

	/**
	 * @brief Sync connector to slaves.
	 */
	#define SYNC_SLAVES "/mppa/sync/[0..15]:59"

#endif /* _KERNEL_H_ */

