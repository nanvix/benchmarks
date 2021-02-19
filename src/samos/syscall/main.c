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
#define BENCHMARK_NAME0 "user-syscall"
#define BENCHMARK_NAME0 "dispatcher-syscall"

/**
 * @name Auxiliary variables
 */
/**@{*/
static struct fence_t _fence; /**< Global fence. */
/**@}*/

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
 * @param name             Benchmark name.
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
	if (heartbeat_ustats)
	{
		uprintf(
#if defined(__mppa256__) || defined(__optimsoc__)
			"[benchmarks][%s][u] %d %s %d %d %d %d %d %d %d",
#else
			"[benchmarks][%s][u] %d %s %d",
#endif
			name,
			it,
			"f",
#if defined(__mppa256__) || defined(__optimsoc__)
			UINT32(heartbeat_ustats[0]),
			UINT32(heartbeat_ustats[1]),
			UINT32(heartbeat_ustats[2]),
			UINT32(heartbeat_ustats[3]),
			UINT32(heartbeat_ustats[4]),
			UINT32(heartbeat_ustats[5]),
			UINT32(heartbeat_ustats[6])
#else
			UINT32(heartbeat_ustats[0])
#endif
		);
	}

	if (heartbeat_kstats)
	{
		uprintf(
#if defined(__mppa256__) || defined(__optimsoc__)
			"[benchmarks][%s][k] %d %s %d %d %d %d %d %d %d",
#else
			"[benchmarks][%s][k] %d %s %d",
#endif
			name,
			it,
			"f",
#if defined(__mppa256__) || defined(__optimsoc__)
			UINT32(heartbeat_kstats[0]),
			UINT32(heartbeat_kstats[1]),
			UINT32(heartbeat_kstats[2]),
			UINT32(heartbeat_kstats[3]),
			UINT32(heartbeat_kstats[4]),
			UINT32(heartbeat_kstats[5]),
			UINT32(heartbeat_kstats[6])
#else
			UINT32(heartbeat_kstats[0])
#endif
		);
	}
}

/*============================================================================*
 * Syscall scenarios                                                          *
 *============================================================================*/

/**
 * @brief Measures syscall time.
 */
static void do_syscall(const char * benchmark)
{
	fence(&_fence);

	for (int i = 0; i < (NITERATIONS + SKIP); i++)
	{
		/* Do syscalls. */
		for (int j = 0; j < 2*NHEARTBEATS; j++)
			(void) knode_get_num();
	}

	fence(&_fence);
}

/**
 * @brief Measures syscall time.
 */
static void do_syscall_measurements(const char * benchmark)
{
	uint64_t heartbeat_ustats[BENCHMARK_PERF_EVENTS];

	umenset(stats, 0, BENCHMARK_PERF_EVENTS * sizeof(uint64_t));

	fence(&_fence);

	for (int i = 0; i < (NITERATIONS + SKIP); i++)
	{
		perf_start(0, PERF_CYCLES);

			/* Do syscalls. */
			for (int j = 0; j < NHEARTBEATS; j++)
				(void) knode_get_num();

		perf_stop(0);
		stats[0] = perf_read(0);

		if (i >= SKIP)
		{
			benchmark_dump_stats(
				i - SKIP,
				benchmark,
				heartbeat_ustats,
				NULL
			);
		}
	}

	fence(&_fence);
}

/**
 * @brief Dummy task.
 *
 * @param arg Unused argument.
 */
static int dummy(ktask_args_t * args)
{
	UNUSED(args);

	do_syscall();

	return (TASK_RET_SUCCESS);
}

/**
 * @brief Measure syscall.
 *
 * @param arg Unused argument.
 */
static int measures(ktask_args_t * args)
{
	UNUSED(args);

	do_syscall_measurements(BENCHMARK_NAME1);

	return (TASK_RET_SUCCESS);
}
/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Measures syscall time.
 */
static void kernel_user_syscall(void)
{
	ktask_t task;

	uassert(ktask_create(&task, dummy, NULL, 0) == 0);
	uassert(ktask_dispatch(&task) == 0);

	do_syscall_measurements(BENCHMARK_NAME0);

	uassert(ktask_wait(&task) == 0);
}

/**
 * @brief Measures syscall time.
 */
static void kernel_user_syscall(void)
{
	ktask_t task;

	uassert(ktask_create(&task, measures, NULL, 0) == 0);
	uassert(ktask_dispatch(&task) == 0);

	do_syscall();

	uassert(ktask_wait(&task) == 0);
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

	kernel_user_syscall();

	kernel_dispatcher_syscall();

	uprintf(HLINE);

	return (0);
}
