/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _PORTAL_SIMPLE_H_
#define _PORTAL_SIMPLE_H_

	#include "../comm.h"

	/**
	 * @brief Maximum of user threads.
	 */
	#ifdef __mppa256__
		#define _NTHREADS_MAX (K1BDP_CORES_NUM - 1)
	#else
		#define _NTHREADS_MAX (THREAD_MAX)
	#endif

	/**
	 * @name Benchmark Parameters
	 */
	/**@{*/
	#define NTHREADS_MIN              1 /**< Minimum Number of Working Threads      */
	#define NTHREADS_MAX  _NTHREADS_MAX /**< Maximum Number of Working Threads      */
	#define NTHREADS_STEP             1 /**< Increment on Number of Working Threads */
	/**@}*/

#endif
