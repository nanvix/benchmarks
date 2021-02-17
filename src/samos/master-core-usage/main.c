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
#define BENCHMARK_NAME "master-core-usage"

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
 * @param it               Benchmark iteration.
 * @oaram name             Benchmark name.
 * @param heartbeat_ustats User land heartbeat statistics.
 * @param heartbeat_kstats Kernel land heartbeat statistics.
 */
static void benchmark_dump_stats(
	int it,
	const char *name,
	uint64_t *heartbeat_ustats,
	uint64_t *heartbeat_kstats
)
{
	uprintf(
#if defined(__mppa256__)
		"[benchmarks][%s][u] %d %s %d %d %d %d %d %d %d",
#elif defined(__optimsoc__)
		"[benchmarks][%s][u] %d %s %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s][u] %d %s %d",
#endif
		name,
		it,
		"f",
#if defined(__mppa256__)
		UINT32(heartbeat_ustats[0]),
		UINT32(heartbeat_ustats[1]),
		UINT32(heartbeat_ustats[2]),
		UINT32(heartbeat_ustats[3]),
		UINT32(heartbeat_ustats[4]),
		UINT32(heartbeat_ustats[5]),
		UINT32(heartbeat_ustats[6])
#elif defined(__optimsoc__)
		UINT32(heartbeat_ustats[0]), /* instruction fetch        */
		UINT32(heartbeat_ustats[1]), /* load access              */
		UINT32(heartbeat_ustats[2]), /* store access             */
		UINT32(heartbeat_ustats[3]), /* instruction fetch stalls */
		UINT32(heartbeat_ustats[4]), /* dcache misses            */
		UINT32(heartbeat_ustats[5]), /* icache misses            */
		UINT32(heartbeat_ustats[6])  /* lsu stalls               */
#else
		UINT32(heartbeat_ustats[0])
#endif
	);

	uprintf(
#if defined(__mppa256__)
		"[benchmarks][%s][k] %d %s %d %d %d %d %d %d %d",
#elif defined(__optimsoc__)
		"[benchmarks][%s][k] %d %s %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s][k] %d %s %d",
#endif
		name,
		it,
		"f",
#if defined(__mppa256__)
		UINT32(heartbeat_kstats[0]),
		UINT32(heartbeat_kstats[1]),
		UINT32(heartbeat_kstats[2]),
		UINT32(heartbeat_kstats[3]),
		UINT32(heartbeat_kstats[4]),
		UINT32(heartbeat_kstats[5]),
		UINT32(heartbeat_kstats[6])
#elif defined(__optimsoc__)
		UINT32(heartbeat_kstats[0]), /* instruction fetch        */
		UINT32(heartbeat_kstats[1]), /* load access              */
		UINT32(heartbeat_kstats[2]), /* store access             */
		UINT32(heartbeat_kstats[3]), /* instruction fetch stalls */
		UINT32(heartbeat_kstats[4]), /* dcache misses            */
		UINT32(heartbeat_kstats[5]), /* icache misses            */
		UINT32(heartbeat_kstats[6])  /* lsu stalls               */
#else
		UINT32(heartbeat_kstats[0])
#endif
	);
}

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Fork-Join Kernel
 *
 * @param nthreads Number of working threads.
 */
void kernel_master_core_global_usage(void)
{
	kthread_t tid[NTHREADS_MAX];
	uint64_t heartbeat_ustats[BENCHMARK_PERF_EVENTS];
	uint64_t heartbeat_kstats[BENCHMARK_PERF_EVENTS];

	/* Save kernel parameters. */
	NTHREADS = nthreads;

	for (int i = 0; i < (NITERATIONS + SKIP); i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			perf_start(0, perf_events[j]);
			kstats(NULL, perf_events[j]);

				/* Spawn threads. */
				for (int k = 0; k < NHEARTBEATS; k++)
					nanvix_name_heartbeat();

			kstats(&heartbeat_kstats[j], perf_events[j]);
			perf_stop(0);
			heartbeat_ustats[j] = perf_read(0);
		}

		if (i >= SKIP)
		{
			benchmark_dump_stats(
				i - SKIP,
				BENCHMARK_NAME,
				heartbeat_ustats,
				heartbeat_kstats
			);
		}
	}
}

/**
 * @brief Fork-Join Kernel
 *
 * @param nthreads Number of working threads.
 */
void kernel_master_core_specific_usage(void)
{
	kthread_t tid[NTHREADS_MAX];
	uint64_t heartbeat_ustats[BENCHMARK_PERF_EVENTS];
	uint64_t heartbeat_kstats[BENCHMARK_PERF_EVENTS];

	/* Save kernel parameters. */
	NTHREADS = nthreads;

	for (int i = 0; i < (NITERATIONS + SKIP); i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			perf_start(0, perf_events[j]);
			kstats(NULL, perf_events[j]);

				/* Spawn threads. */
				for (int k = 0; k < NHEARTBEATS; k++)
					nanvix_name_heartbeat();

			kstats(&heartbeat_kstats[j], perf_events[j]);
			perf_stop(0);
			heartbeat_ustats[j] = perf_read(0);
		}

		if (i >= SKIP)
		{
			benchmark_dump_stats(
				i - SKIP,
				BENCHMARK_NAME,
				heartbeat_ustats,
				heartbeat_kstats
			);
		}
	}
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
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	uprintf(HLINE);

	kernel_master_core_global_usage();

	uprintf(HLINE);

	return (0);
}
