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

#include <nanvix/sys/noc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/sys/thread.h>
#include <nanvix/kernel/kernel.h>
#include <nanvix/ulib.h>

#include <posix/stdint.h>

/**
 * @brief Number of benchmark iterations.
 */
#ifdef NDEBUG
	#define NITERATIONS 30
#else
	#define NITERATIONS 1
#endif

/**
 * @brief Casts something to a uint32_t.
 */
#define UINT32(x) ((uint32_t)((x) & 0xffffffff))

/**
 * @brief Iterations to skip on warmup.
 */
#ifdef NDEBUG
	#define SKIP 10
#else
	#define SKIP 0
#endif

/**
 * @brief Horizontal line.
 */
static const char *HLINE =
	"------------------------------------------------------------------------";

/*============================================================================*
 * Profiling                                                                  *
 *============================================================================*/

/**
 * @brief Name of the benchmark.
 */
#define BENCHMARK_NAME "kcall-local"

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
 * @param uland_stats User land execution statistics.
 * @param kland_stats Kernel execution statistics.
 */
static void benchmark_dump_stats(
	int it,
	const char *name,
	uint64_t *uland_stats,
	uint64_t *kland_stats
)
{
	uprintf(
#if defined(__mppa256__)
		"[benchmarks][%s][u] %d %d %d %d %d %d %d %d",
#elif defined(__optimsoc__)
		"[benchmarks][%s][u] %d %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s][u] %d %d",
#endif
		name,
		it,
#if defined(__mppa256__)
		UINT32(uland_stats[0]),
		UINT32(uland_stats[1]),
		UINT32(uland_stats[2]),
		UINT32(uland_stats[3]),
		UINT32(uland_stats[4]),
		UINT32(uland_stats[5]),
		UINT32(uland_stats[6])
#elif defined(__optimsoc__)
		UINT32(uland_stats[0]),
		UINT32(uland_stats[1]),
		UINT32(uland_stats[2]),
		UINT32(uland_stats[3]),
		UINT32(uland_stats[4]),
		UINT32(uland_stats[5]),
		UINT32(uland_stats[6])
#else
		UINT32(uland_stats[0])
#endif
	);

	uprintf(
#if defined(__mppa256__)
		"[benchmarks][%s][k] %d %d %d %d %d %d %d %d",
#elif defined(__optimsoc__)
		"[benchmarks][%s][k] %d %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s][k] %d %d",
#endif
		name,
		it,
#if defined(__mppa256__)
		UINT32(kland_stats[0]),
		UINT32(kland_stats[1]),
		UINT32(kland_stats[2]),
		UINT32(kland_stats[3]),
		UINT32(kland_stats[4]),
		UINT32(kland_stats[5]),
		UINT32(kland_stats[6])
#elif defined(__optimsoc__)
		UINT32(kland_stats[0]),
		UINT32(kland_stats[1]),
		UINT32(kland_stats[2]),
		UINT32(kland_stats[3]),
		UINT32(kland_stats[4]),
		UINT32(kland_stats[5]),
		UINT32(kland_stats[6])
#else
		UINT32(kland_stats[0])
#endif
	);
}

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Local Kernel Call Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
int __main2(int argc, const char *argv[])
{
	uint64_t uland_stats[BENCHMARK_PERF_EVENTS];
	uint64_t kland_stats[BENCHMARK_PERF_EVENTS];

	((void) argc);
	((void) argv);

	if (knode_get_num() != PROCESSOR_NODENUM_MASTER)
		return (0);

	uprintf(HLINE);

	for (int i = 0; i < NITERATIONS + SKIP; i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			kstats(NULL, perf_events[j]);
			perf_start(0, perf_events[j]);

				kcall0(NR_thread_get_id);

			kstats(&kland_stats[j], 0);
			perf_stop(0);
			uland_stats[j] = perf_read(0);
		}

		if (i >= SKIP)
		{
			benchmark_dump_stats(
				i - SKIP,
				BENCHMARK_NAME,
				uland_stats,
				kland_stats
			);
		}
	}

	uprintf(HLINE);

	return (0);
}
