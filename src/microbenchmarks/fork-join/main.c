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
#include <nanvix/sys/perf.h>
#include <nanvix/sys/thread.h>
#include <nanvix/ulib.h>
#include <posix/stdint.h>
#include "kbench.h"

/**
 * @name Benchmark Parameters
 */
/**@{*/
#define NTHREADS_MIN                1  /**< Minimum Number of Working Threads      */
#define NTHREADS_MAX  (THREAD_MAX - 1) /**< Maximum Number of Working Threads      */
#define NTHREADS_STEP               1  /**< Increment on Number of Working Threads */
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
static int NTHREADS; /**< Number of Working Threads */
/**@}*/

/*============================================================================*
 * Profiling                                                                  *
 *============================================================================*/

/**
 * @brief Name of the benchmark.
 */
#define BENCHMARK_NAME "fork-join"

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
 * @param it          Benchmark iteration.
 * @oaram name        Benchmark name.
 * @param fork_ustats User land fork statistics.
 * @param join_ustats User land join statistics.
 * @param fork_kstats Kernel land fork statistics.
 * @param join_kstats Kernel land join statistics.
 */
static void benchmark_dump_stats(
	int it,
	const char *name,
	uint64_t *fork_ustats, uint64_t *join_ustats,
	uint64_t *fork_kstats, uint64_t *join_kstats
)
{
	uprintf(
#if defined(__mppa256__)
		"[benchmarks][%s][u] %d %s %d %d %d %d %d %d %d %d",
#elif defined(__optimsoc__)
		"[benchmarks][%s][u] %d %s %d %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s][u] %d %s %d %d",
#endif
		name,
		it,
		"f",
		NTHREADS,
#if defined(__mppa256__)
		UINT32(fork_ustats[0]),
		UINT32(fork_ustats[1]),
		UINT32(fork_ustats[2]),
		UINT32(fork_ustats[3]),
		UINT32(fork_ustats[4]),
		UINT32(fork_ustats[5]),
		UINT32(fork_ustats[6])
#elif defined(__optimsoc__)
		UINT32(fork_ustats[0]), /* instruction fetch        */
		UINT32(fork_ustats[1]), /* load access              */
		UINT32(fork_ustats[2]), /* store access             */
		UINT32(fork_ustats[3]), /* instruction fetch stalls */
		UINT32(fork_ustats[4]), /* dcache misses            */
		UINT32(fork_ustats[5]), /* icache misses            */
		UINT32(fork_ustats[6])  /* lsu stalls               */
#else
		UINT32(fork_ustats[0])
#endif
	);

	uprintf(
#if defined(__mppa256__)
		"[benchmarks][%s][k] %d %s %d %d %d %d %d %d %d %d",
#elif defined(__optimsoc__)
		"[benchmarks][%s][k] %d %s %d %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s][k] %d %s %d %d",
#endif
		name,
		it,
		"f",
		NTHREADS,
#if defined(__mppa256__)
		UINT32(fork_kstats[0]),
		UINT32(fork_kstats[1]),
		UINT32(fork_kstats[2]),
		UINT32(fork_kstats[3]),
		UINT32(fork_kstats[4]),
		UINT32(fork_kstats[5]),
		UINT32(fork_kstats[6])
#elif defined(__optimsoc__)
		UINT32(fork_kstats[0]), /* instruction fetch        */
		UINT32(fork_kstats[1]), /* load access              */
		UINT32(fork_kstats[2]), /* store access             */
		UINT32(fork_kstats[3]), /* instruction fetch stalls */
		UINT32(fork_kstats[4]), /* dcache misses            */
		UINT32(fork_kstats[5]), /* icache misses            */
		UINT32(fork_kstats[6])  /* lsu stalls               */
#else
		UINT32(fork_kstats[0])
#endif
	);

	uprintf(
#if defined(__mppa256__)
		"[benchmarks][%s][u] %d %s %d %d %d %d %d %d %d %d",
#elif defined(__optimsoc__)
		"[benchmarks][%s][u] %d %s %d %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s][u] %d %s %d %d",
#endif
		name,
		it,
		"j",
		NTHREADS,
#if defined(__mppa256__)
		UINT32(join_ustats[0]),
		UINT32(join_ustats[1]),
		UINT32(join_ustats[2]),
		UINT32(join_ustats[3]),
		UINT32(join_ustats[4]),
		UINT32(join_ustats[5]),
		UINT32(join_ustats[6])
#elif defined(__optimsoc__)
		UINT32(join_ustats[0]), /* instruction fetch        */
		UINT32(join_ustats[1]), /* load access              */
		UINT32(join_ustats[2]), /* store access             */
		UINT32(join_ustats[3]), /* instruction fetch stalls */
		UINT32(join_ustats[4]), /* dcache misses            */
		UINT32(join_ustats[5]), /* icache misses            */
		UINT32(join_ustats[6])  /* lsu stalls               */
#else
		UINT32(join_ustats[0])
#endif
	);

	uprintf(
#if defined(__mppa256__)
		"[benchmarks][%s][k] %d %s %d %d %d %d %d %d %d %d",
#elif defined(__optimsoc__)
		"[benchmarks][%s][k] %d %s %d %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s][k] %d %s %d %d",
#endif
		name,
		it,
		"j",
		NTHREADS,
#if defined(__mppa256__)
		UINT32(join_kstats[0]),
		UINT32(join_kstats[1]),
		UINT32(join_kstats[2]),
		UINT32(join_kstats[3]),
		UINT32(join_kstats[4]),
		UINT32(join_kstats[5]),
		UINT32(join_kstats[6])
#elif defined(__optimsoc__)
		UINT32(join_kstats[0]), /* instruction fetch        */
		UINT32(join_kstats[1]), /* load access              */
		UINT32(join_kstats[2]), /* store access             */
		UINT32(join_kstats[3]), /* instruction fetch stalls */
		UINT32(join_kstats[4]), /* dcache misses            */
		UINT32(join_kstats[5]), /* icache misses            */
		UINT32(join_kstats[6])  /* lsu stalls               */
#else
		UINT32(join_kstats[0])
#endif
	);
}

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Dummy task.
 *
 * @param arg Unused argument.
 */
static void *task(void *arg)
{
	((void) arg);

	return (NULL);
}

/**
 * @brief Fork-Join Kernel
 *
 * @param nthreads Number of working threads.
 */
void kernel_fork_join(int nthreads)
{
	kthread_t tid[NTHREADS_MAX];
	uint64_t fork_ustats[BENCHMARK_PERF_EVENTS];
	uint64_t join_ustats[BENCHMARK_PERF_EVENTS];
	uint64_t fork_kstats[BENCHMARK_PERF_EVENTS];
	uint64_t join_kstats[BENCHMARK_PERF_EVENTS];

	/* Save kernel parameters. */
	NTHREADS = nthreads;

	for (int i = 0; i < (NITERATIONS + SKIP); i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			perf_start(0, perf_events[j]);
			kstats(NULL, perf_events[j]);

				/* Spawn threads. */
				for (int k = 0; k < nthreads; k++)
					kthread_create(&tid[k], task, NULL);

			kstats(&fork_kstats[j], perf_events[j]);
			perf_stop(0);
			fork_ustats[j] = perf_read(0);

			perf_start(0, perf_events[j]);
			kstats(NULL, perf_events[j]);

				/* Wait for threads. */
				for (int k = 0; k < nthreads; k++)
					kthread_join(tid[k], NULL);

			kstats(&join_kstats[j], perf_events[j]);
			perf_stop(0);
			join_ustats[j] = perf_read(0);
		}

		if (i >= SKIP)
		{
			benchmark_dump_stats(
				i - SKIP,
				BENCHMARK_NAME,
				fork_ustats, join_ustats,
				fork_kstats, join_kstats
			);
		}
	}
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Fork-Join Benchmark
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

	kernel_fork_join(NTHREADS_MAX);

#else

	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
		kernel_fork_join(nthreads);

#endif

	uprintf(HLINE);

	return (0);
}
