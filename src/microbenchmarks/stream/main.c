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
#define __SKIP 10
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
#define __OBJSIZE (8*1024)
#endif

/*============================================================================*
 * Profiling                                                                  *
 *============================================================================*/

/**
 * @brief Name of the benchmark.
 */
#define BENCHMARK_NAME "stream"

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

/**
 * Performance events.
 */
static int perf_events[BENCHMARK_PERF_EVENTS] = {
#if defined(__mppa256__)
	PERF_DTLB_STALLS,
	PERF_ITLB_STALLS,
	PERF_REG_STALLS,
	PERF_BRANCH_STALLS,
	PERF_DCACHE_STALLS,
	PERF_ICACHE_STALLS,
	PERF_CYCLES
#elif defined(__optimsoc__)
	MOR1KX_PERF_LSU_HITS,
	MOR1KX_PERF_BRANCH_STALLS,
	MOR1KX_PERF_ICACHE_HITS,
	MOR1KX_PERF_REG_STALLS,
	MOR1KX_PERF_ICACHE_MISSES,
	MOR1KX_PERF_IFETCH_STALLS,
	MOR1KX_PERF_LSU_STALLS,
#else
	0
#endif
};

/**
 * @brief Dump execution statistics.
 *
 * @param it      Benchmark iteration.
 * @oaram name    Benchmark name.
 * @param objsize Object size.
 * @param stats   Execution statistics.
 */
static void benchmark_dump_stats(int it, const char *name, size_t objsize, uint64_t *stats)
{
	uprintf(
#if defined(__mppa256__)
		"[benchmarks][%s] %d %d %d %l %l %l %l %l %l %l",
#elif defined(__optimsoc__)
		"[benchmarks][%s] %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s] %d %d %d %d",
#endif
		name,
		it,
		__NTHREADS,
		objsize,
#if defined(__mppa256__)
		stats[0],
		stats[1],
		stats[2],
		stats[3],
		stats[4],
		stats[5],
		stats[6]
#elif defined(__optimsoc__)
		UINT32(stats[0]),
		UINT32(stats[1]),
		UINT32(stats[2]),
		UINT32(stats[3]),
		UINT32(stats[4]),
		UINT32(stats[5]),
		UINT32(stats[6])
#else
		UINT32(stats[0])
#endif
	);
}

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Thread info.
 */
struct tdata
{
	int tnum;  /**< Thread Number */
	size_t start; /**< Start Byte    */
	size_t end;   /**< End Byte      */
} tdata[__NTHREADS] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Buffers.
 */
/**@{*/
static word_t obj1[__OBJSIZE/WORD_SIZE] ALIGN(CACHE_LINE_SIZE);
static word_t obj2[__OBJSIZE/WORD_SIZE] ALIGN(CACHE_LINE_SIZE);
/**@}*/

/**
 * @brief Fills words in memory.
 *
 * @param ptr Pointer to target memory area.
 * @param c   Character to use.
 * @param n   Number of bytes to be set.
 */
static inline void memfill(word_t *ptr, word_t c, size_t n)
{
	word_t *p;

	p = ptr;

	/* Set words. */
	for (size_t i = 0; i < n; i++)
		*p++ = c;
}

/**
 * @brief Copy words in memory.
 *
 * @param dest Target memory area.
 * @param src  Source memory area.
 * @param n    Number of bytes to be copied.
 */
static inline void memcopy(word_t *dest, const word_t *src, size_t n)
{
	word_t *d;       /* Write pointer. */
	const word_t* s; /* Read pointer.  */

	s = src;
	d = dest;

	/* Copy words. */
	for (size_t i = 0; i < n; i++)
		*d++ = *s++;
}

/**
 * @brief Move Bytes in Memory
 */
static void *task(void *arg)
{
	struct tdata *t = arg;
	int start = t->start;
	int end = t->end;
	uint64_t stats[BENCHMARK_PERF_EVENTS];

	/* Warm up. */
	memfill(&obj1[start], (word_t) - 1, end - start);
	memfill(&obj2[start], 0, end - start);

	for (int i = 0; i < __NITERATIONS + __SKIP; i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			perf_start(0, perf_events[j]);

				memcopy(&obj1[start], &obj2[start], end - start);

			perf_stop(0);
			stats[j] = perf_read(0);
		}

		if (i >= __SKIP)
			benchmark_dump_stats(i - __SKIP, BENCHMARK_NAME, __OBJSIZE, stats);
	}

	return (NULL);
}

/**
 * @brief Memory Move Benchmark Kernel
 */
static void kernel_memmove(void)
{
	size_t nbytes;
	kthread_t tid[__NTHREADS];

	nbytes = (__OBJSIZE/WORD_SIZE)/__NTHREADS;

	/* Spawn threads. */
	for (int i = 0; i < __NTHREADS; i++)
	{
		/* Initialize thread data structure. */
		tdata[i].start = nbytes*i;
		tdata[i].end = (i == (__NTHREADS - 1)) ? (__OBJSIZE/WORD_SIZE) : (i + 1)*nbytes;
		tdata[i].tnum = i;

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
