/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
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

#ifndef _SAMOS_H_
#define _SAMOS_H_

#include <nanvix/sys/perf.h>
#include <nanvix/sys/thread.h>
#include <nanvix/sys/task.h>
#include <nanvix/sys/noc.h>
#include <nanvix/ulib.h>
#include <nanvix/config.h>
#include <nanvix/limits.h>
#include <posix/stdint.h>
#include <posix/stddef.h>

/*============================================================================*
 * Parameters                                                                 *
 *============================================================================*/

	/**
	 * @name Benchmark Parameters
	 */
	/**@{*/
	#define NTHREADS_MIN                1  /**< Minimum Number of Working Threads      */
	#define NTHREADS_MAX  (THREAD_MAX - 1) /**< Maximum Number of Working Threads      */
	#define NTHREADS_STEP               1  /**< Increment on Number of Working Threads */
	/**@}*/

	/**
	 * @name Benchmark Parameters
	 */
	/**@{*/
	#define NTASKS_MIN                1  /**< Minimum Number of Working Tasks      */
	#define NTASKS_MAX  (THREAD_MAX - 1) /**< Maximum Number of Working Tasks      */
	#define NTASKS_STEP               1  /**< Increment on Number of Working Tasks */
	/**@}*/

	/**
	 * @brief Number of tasks.
	 */
	#ifndef NTASKS
		#ifdef NDEBUG
			#define NTASKS (NTHREAD_MAX - 1)
		#else
			#define NTASKS 1 
		#endif
	#endif

	/**
	 * @brief Number of dispatchers. Two or more will create user dispatcher
	 * using user threads.
	 *
	 * @details Used only if we want to evaluate the response time with more
	 * than one dispatcher.
	 */
	#ifndef NDISPATCHERS
		#define NDISPATCHERS 1
	#endif

/*============================================================================*
 * Statistics                                                                 *
 *============================================================================*/

	/**
	 * @brief Number of benchmark iterations.
	 */
	#ifndef NITERATIONS
		#ifdef NDEBUG
			#define NITERATIONS 50
		#else
			#define NITERATIONS 1
		#endif
	#endif

	/**
	 * @brief Iterations to skip on warmup.
	 */
	#ifndef SKIP
		#ifdef NDEBUG
			#define SKIP 10
		#else
			#define SKIP 0
		#endif
	#endif

	/**
	 * @brief Number of events to profile.
	 */
	#if defined(__mppa256__)
		#define BENCHMARK_PERF_EVENTS 7
	#elif defined(__optimsoc__)
		#define BENCHMARK_PERF_EVENTS 7
	#else
		#define BENCHMARK_PERF_EVENTS 1
	#endif

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

	/**
	 * @brief Casts something to a uint32_t.
	 */
	#define UINT32(x) ((uint32_t)((x) & 0xffffffff))

	/**
	 * @brief Horizontal line.
	 */
	static const char *HLINE =
		"------------------------------------------------------------------------";

	/**
	 * @brief Forces a platform-independent delay.
	 *
	 * @param cycles Delay in cycles.
	 *
	 * @author João Vicente Souto
	 */
	static inline void delay(int times, uint64_t cycles)
	{
		uint64_t t0, t1;

		for (int i = 0; i < times; ++i)
		{
			kclock(&t0);

			do
				kclock(&t1);
			while ((t1 - t0) < cycles);
		}
	}

#endif
