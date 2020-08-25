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

#include <nanvix/kernel/kernel.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/sys/thread.h>
#include <nanvix/ulib.h>
#include <nanvix/ulib.h>

#include <posix/stdint.h>

/**
 * @brief Number of benchmark iterations.
 */
#ifndef __NITERATIONS
#define __NITERATIONS 1
#endif

/**
 * @brief Casts something to a uint32_t.
 */
#define UINT32(x) ((uint32_t)((x) & 0xffffffff))

/**
 * @brief Iterations to skip on warmup.
 */
#ifndef __SKIP
#define __SKIP 1
#endif

/**
 * @brief Number of Working Threads
 */
#ifndef __NTHREADS
#define __NTHREADS  (THREAD_MAX - 2)
#endif

/**
 * @brief Object Size
 */
#ifndef __OBJSIZE
#define __OBJSIZE 4096
#endif

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Thread info.
 */
struct tdata
{
	size_t start; /**< Start Byte */
} tdata[__NTHREADS] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Buffers.
 */
/**@{*/
static char obj1[__NTHREADS*__OBJSIZE] ALIGN(CACHE_LINE_SIZE);
static char obj2[__NTHREADS*__OBJSIZE] ALIGN(CACHE_LINE_SIZE);
/**@}*/

/**
 * @brief Dump execution statistics.
 */
static void benchmark_dump_stats(uint64_t time_kernel)
{
	uprintf(
		"[benchmarks][stream] %d %d %l",
		__NTHREADS,
		__NTHREADS*__OBJSIZE,
		time_kernel
	);
}

/**
 * @brief Move Bytes in Memory
 */
static void *task(void *arg)
{
	uint64_t time_kernel;
	struct tdata *t = arg;
	int start = t->start;

	for (int i = 0; i < __NITERATIONS + __SKIP; i++)
	{
		perf_start(0, PERF_CYCLES);

			umemcpy(&obj1[start], &obj2[start], __OBJSIZE);

		perf_stop(0);
		time_kernel = perf_read(0);

		if (i >= __SKIP)
			benchmark_dump_stats(time_kernel);
	}

	return (NULL);
}

/**
 * @brief Memory Move Benchmark Kernel
 */
static void kernel_memmove(void)
{
	kthread_t tid[__NTHREADS];

	/* Spawn threads. */
	for (int i = 0; i < __NTHREADS; i++)
	{
		/* Initialize thread data structure. */
		tdata[i].start = i*__OBJSIZE;

		kthread_create(&tid[i], task, &tdata[i]);
	}

	/* Wait for threads. */
	for (int i = 0; i < __NTHREADS; i++)
		kthread_join(tid[i], NULL);
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Memory Move Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
int __main3(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	kernel_memmove();

	return (0);
}
