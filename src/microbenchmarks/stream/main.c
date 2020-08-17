/*
 * MIT License
 *
 * Copyright(c) 2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
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

#include <nanvix/sys/noc.h>
#include <nanvix/sys/thread.h>
#include <nanvix/ulib.h>
#include <posix/sys/types.h>
#include <posix/stdint.h>
#include "kbench.h"

/**
 * @name Benchmark Parameters
 */
/**@{*/
#define NTHREADS_MIN                2  /**< Minimum Number of Working Threads      */
#define NTHREADS_MAX  (THREAD_MAX - 1) /**< Maximum Number of Working Threads      */
#define NTHREADS_STEP               4  /**< Increment on Number of Working Threads */
#define OBJSIZE_MIN           (1*1024) /**< Minimum Object Size                    */
#define OBJSIZE_MAX           (8*1024) /**< Maximum Object Size                    */
#define OBJSIZE_STEP          (1*1024) /**< Object Size                            */
/**@}*/

/**
 * @brief Horizontal line.
 */
static const char *HLINE =
	"------------------------------------------------------------------------";

/**
 * @name Benchmark Kernel Parameters
 */
/**@{*/
static int NTHREADS;   /**< Number of Working Threads */
static size_t OBJSIZE; /**< Object Size               */
/**@}*/

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
		"[benchmarks][%s] %d %d %d %d %d %d %d %d %d %d",
#elif defined(__optimsoc__)
		"[benchmarks][%s] %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s] %d %d %d %d",
#endif
		name,
		it,
		NTHREADS,
		objsize,
#if defined(__mppa256__)
		UINT32(stats[0]),
		UINT32(stats[1]),
		UINT32(stats[2]),
		UINT32(stats[3]),
		UINT32(stats[4]),
		UINT32(stats[5]),
		UINT32(stats[6])
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
} tdata[NTHREADS_MAX] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Buffers.
 */
/**@{*/
static word_t obj1[OBJSIZE_MAX/WORD_SIZE] ALIGN(CACHE_LINE_SIZE);
static word_t obj2[OBJSIZE_MAX/WORD_SIZE] ALIGN(CACHE_LINE_SIZE);
/**@}*/

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

	for (int i = 0; i < NITERATIONS + SKIP; i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			perf_start(0, perf_events[j]);

				memcopy(&obj1[start], &obj2[start], end - start);

			perf_stop(0);
			stats[j] = perf_read(0);
		}

		if (i >= SKIP)
			benchmark_dump_stats(i - SKIP, BENCHMARK_NAME, OBJSIZE, stats);
	}

	return (NULL);
}

/**
 * @brief Memory Move Benchmark Kernel
 *
 * @param nthreads Number of working threads.
 * @param objsize  Object size.
 */
static void kernel_memmove(int nthreads, size_t objsize)
{
	size_t nbytes;
	kthread_t tid[NTHREADS_MAX];

	/* Save kernel parameters. */
	NTHREADS = nthreads;
	OBJSIZE = objsize;

	nbytes = (OBJSIZE/WORD_SIZE)/nthreads;

	/* Spawn threads. */
	for (int i = 0; i < nthreads; i++)
	{
		/* Initialize thread data structure. */
		tdata[i].start = nbytes*i;
		tdata[i].end = (i == (nthreads - 1)) ? (OBJSIZE/WORD_SIZE) : (i + 1)*nbytes;
		tdata[i].tnum = i;

		kthread_create(&tid[i], task, &tdata[i]);
	}

	/* Wait for threads. */
	for (int i = 0; i < nthreads; i++)
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
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	if (knode_get_num() != PROCESSOR_NODENUM_MASTER)
		return (0);

	uprintf(HLINE);

#ifndef NDEBUG

	kernel_memmove(1, OBJSIZE_MAX);

#else

	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
	{
		for (size_t objsize = OBJSIZE_MIN; objsize <= OBJSIZE_MAX; objsize += OBJSIZE_STEP)
			kernel_memmove(nthreads, objsize);
	}

#endif

	uprintf(HLINE);

	return (0);
}
