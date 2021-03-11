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

#include "../config.h"

/*============================================================================*
 * Profiling                                                                  *
 *============================================================================*/

/**
 * @brief Name of the benchmark.
 */
#define BENCHMARK_NAME "lookup-core-usage"

/**
 * @brief Dump execution statistics.
 *
 * @param it     Benchmark iteration.
 * @param name   Benchmark name.
 * @param ustats User land heartbeat statistics.
 * @param kstats Kernel land heartbeat statistics.
 */
static void benchmark_dump_stats(
	int it,
	const char *name,
	uint64_t *stats
)
{
	uprintf(
		"[benchmarks][%s] %d %l %l %l %l %l %l",
		name,
		it,
		UINT64(stats[0]),
		UINT64(stats[1]),
		UINT64(stats[2]),
		UINT64(stats[3]),
		UINT64(stats[4]),
		UINT64(stats[5])
	);
}

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Lookup core usage kernel
 *
 * @param nthreads Number of working threads.
 */
void kernel_lookup_core_usage(void)
{
#if defined(__mppa256__) && __NANVIX_MICROKERNEL_THREAD_STATS

	kthread_t tid;
	const char *pname;
	uint64_t stats[BENCHMARK_PERF_EVENTS];

	tid = kthread_self();
	pname = nanvix_getpname();
	umemset((void *) stats, 0, (BENCHMARK_PERF_EVENTS * sizeof(uint64_t)));
	uassert(BENCHMARK_PERF_EVENTS >= 6);

	uassert(kthread_stats(tid, NULL, KTHREAD_STATS_EXEC_TIME) == 0);
	uassert(kthread_stats(tid, &stats[4], KTHREAD_STATS_EXEC_TIME) == 0);
	perf_start(0, PERF_CYCLES);
	perf_stop(0);
	stats[5] = perf_read(0);

	for (int i = 0; i < (NITERATIONS + SKIP); i++)
	{
#if __NANVIX_USE_TASKS
		uassert(kthread_stats(KTHREAD_DISPATCHER_TID, NULL, KTHREAD_STATS_EXEC_TIME) == 0);
#endif
		uassert(kthread_stats(KTHREAD_MASTER_TID, NULL, KTHREAD_STATS_EXEC_TIME) == 0);
		uassert(kthread_stats(tid, NULL, KTHREAD_STATS_EXEC_TIME) == 0);

		perf_start(0, PERF_CYCLES);

			/* Spawn threads. */
			nanvix_name_lookup(pname);

		perf_stop(0);
		stats[3] = perf_read(0);

		uassert(kthread_stats(tid, &stats[2], KTHREAD_STATS_EXEC_TIME) == 0);
		uassert(kthread_stats(KTHREAD_MASTER_TID, &stats[0], KTHREAD_STATS_EXEC_TIME) == 0);
#if __NANVIX_USE_TASKS
		uassert(kthread_stats(KTHREAD_DISPATCHER_TID, &stats[1], KTHREAD_STATS_EXEC_TIME) == 0);
#endif

		if (i >= SKIP)
		{
			benchmark_dump_stats(
				i - SKIP,
				BENCHMARK_NAME,
				stats
			);
		}
	}

#else

	uprintf("[benchmark][%s] Ignore core specific usage!", BENCHMARK_NAME1);

#endif
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Master Core Usage Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
int __main3(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	uprintf(HLINE);

	kernel_lookup_core_usage();

	uprintf(HLINE);

	return (0);
}
