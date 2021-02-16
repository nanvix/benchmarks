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

/**
 * @name Benchmark Kernel Parameters
 */
/**@{*/
static int NTASKS; /**< Number of Working Tasks */
/**@}*/

/*============================================================================*
 * Profiling                                                                  *
 *============================================================================*/

/**
 * @brief Name of the benchmark.
 */
#define BENCHMARK_NAME "dispatch-wait"

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
 * @param it              Benchmark iteration.
 * @oaram name            Benchmark name.
 * @param dispatch_ustats User land dispatch statistics.
 * @param wait_ustats     User land wait statistics.
 * @param dispatch_kstats Kernel land dispatch statistics.
 * @param wait_kstats     Kernelland wait statistics.
 */
static void benchmark_dump_stats(
	int it,
	const char *name,
	uint64_t *dispatch_ustats, uint64_t *wait_ustats,
	uint64_t *dispatch_kstats, uint64_t *wait_kstats
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
		NTASKS,
#if defined(__mppa256__)
		UINT32(dispatch_ustats[0]),
		UINT32(dispatch_ustats[1]),
		UINT32(dispatch_ustats[2]),
		UINT32(dispatch_ustats[3]),
		UINT32(dispatch_ustats[4]),
		UINT32(dispatch_ustats[5]),
		UINT32(dispatch_ustats[6])
#elif defined(__optimsoc__)
		UINT32(dispatch_ustats[0]), /* instruction fetch        */
		UINT32(dispatch_ustats[1]), /* load access              */
		UINT32(dispatch_ustats[2]), /* store access             */
		UINT32(dispatch_ustats[3]), /* instruction fetch stalls */
		UINT32(dispatch_ustats[4]), /* dcache misses            */
		UINT32(dispatch_ustats[5]), /* icache misses            */
		UINT32(dispatch_ustats[6])  /* lsu stalls               */
#else
		UINT32(dispatch_ustats[0])
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
		NTASKS,
#if defined(__mppa256__)
		UINT32(dispatch_kstats[0]),
		UINT32(dispatch_kstats[1]),
		UINT32(dispatch_kstats[2]),
		UINT32(dispatch_kstats[3]),
		UINT32(dispatch_kstats[4]),
		UINT32(dispatch_kstats[5]),
		UINT32(dispatch_kstats[6])
#elif defined(__optimsoc__)
		UINT32(dispatch_kstats[0]), /* instruction fetch        */
		UINT32(dispatch_kstats[1]), /* load access              */
		UINT32(dispatch_kstats[2]), /* store access             */
		UINT32(dispatch_kstats[3]), /* instruction fetch stalls */
		UINT32(dispatch_kstats[4]), /* dcache misses            */
		UINT32(dispatch_kstats[5]), /* icache misses            */
		UINT32(dispatch_kstats[6])  /* lsu stalls               */
#else
		UINT32(dispatch_kstats[0])
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
		NTASKS,
#if defined(__mppa256__)
		UINT32(wait_ustats[0]),
		UINT32(wait_ustats[1]),
		UINT32(wait_ustats[2]),
		UINT32(wait_ustats[3]),
		UINT32(wait_ustats[4]),
		UINT32(wait_ustats[5]),
		UINT32(wait_ustats[6])
#elif defined(__optimsoc__)
		UINT32(wait_ustats[0]), /* instruction fetch        */
		UINT32(wait_ustats[1]), /* load access              */
		UINT32(wait_ustats[2]), /* store access             */
		UINT32(wait_ustats[3]), /* instruction fetch stalls */
		UINT32(wait_ustats[4]), /* dcache misses            */
		UINT32(wait_ustats[5]), /* icache misses            */
		UINT32(wait_ustats[6])  /* lsu stalls               */
#else
		UINT32(wait_ustats[0])
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
		NTASKS,
#if defined(__mppa256__)
		UINT32(wait_kstats[0]),
		UINT32(wait_kstats[1]),
		UINT32(wait_kstats[2]),
		UINT32(wait_kstats[3]),
		UINT32(wait_kstats[4]),
		UINT32(wait_kstats[5]),
		UINT32(wait_kstats[6])
#elif defined(__optimsoc__)
		UINT32(wait_kstats[0]), /* instruction fetch        */
		UINT32(wait_kstats[1]), /* load access              */
		UINT32(wait_kstats[2]), /* store access             */
		UINT32(wait_kstats[3]), /* instruction fetch stalls */
		UINT32(wait_kstats[4]), /* dcache misses            */
		UINT32(wait_kstats[5]), /* icache misses            */
		UINT32(wait_kstats[6])  /* lsu stalls               */
#else
		UINT32(wait_kstats[0])
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
static int task(ktask_args_t * args)
{
	UNUSED(args);
	return (TASK_RET_SUCCESS);
}

/**
 * @brief Dispatch-Wait Kernel
 *
 * @param ntasks Number of working tasks.
 */
static void kernel_dispatch_wait(int ntasks)
{
	ktask_t tasks[NTASKS_MAX];
	uint64_t dispatch_ustats[BENCHMARK_PERF_EVENTS];
	uint64_t wait_ustats[BENCHMARK_PERF_EVENTS];
	uint64_t dispatch_kstats[BENCHMARK_PERF_EVENTS];
	uint64_t wait_kstats[BENCHMARK_PERF_EVENTS];

	/* Save kernel parameters. */
	NTASKS = ntasks;

	for (int i = 0; i < (NITERATIONS + SKIP); i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			perf_start(0, perf_events[j]);
			kstats(NULL, perf_events[j]);

				/* Spawn tasks. */
				for (int k = 0; k < ntasks; k++)
				{
					uassert(ktask_create(&tasks[j], task, NULL, 0) == 0);
					uassert(ktask_dispatch(&tasks[j]) == 0);
				}

			kstats(&dispatch_kstats[j], perf_events[j]);
			perf_stop(0);
			dispatch_ustats[j] = perf_read(0);

			perf_start(0, perf_events[j]);
			kstats(NULL, perf_events[j]);

				/* Wait for tasks. */
				for (int k = 0; k < ntasks; k++)
					uassert(ktask_wait(&task[j]) == 0);

			kstats(&wait_kstats[j], perf_events[j]);
			perf_stop(0);
			wait_ustats[j] = perf_read(0);
		}

		if (i >= SKIP)
		{
			benchmark_dump_stats(
				i - SKIP,
				BENCHMARK_NAME,
				dispatch_ustats, wait_ustats,
				dispatch_kstats, wait_kstats
			);
		}
	}
}

/*============================================================================*
 * User Dispatcher Wrapper                                                    *
 *============================================================================*/

/**
 * @brief Main function of the dispatcher service.
 */
extern void task_loop(void);

/**
 * @brief User dispatcher.
 */
static void * _task_loop(void * args)
{
	/* Let the dispatcher locked on a spectific core. */
	kthread_set_affinity(1 << ((int) (intptr_t) args)); 

	task_loop();

	return (NULL);
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Dispatch-Wait Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
int __main2(int argc, const char *argv[])
{
	kthread_t tid[NDISPATCHERS];

	((void) argc);
	((void) argv);

	uprintf(HLINE);

	/* Build dispatchers (the kernel has one). */
	if (NDISPATCHERS > 1)
	{
		for (int i = 1; i < NDISPATCHERS; ++i)
			kthread_create(&tid[i], _task_loop, (void *) (intptr_t) (i + 1));

		delay(1, CLUSTER_FREQ);

		uprintf("[benchmark][dispatch-wait] Auxiliary dispatchers created.");
		uprintf(HLINE);
	}

#ifndef NDEBUG

	kernel_dispatch_wait(NTASKS_MAX);

#else

	for (int ntasks = NTASKS_MIN; ntasks <= NTASKS_MAX; ntasks += NTASKS_STEP)
		kernel_dispatch_wait(ntasks);

#endif

	uprintf(HLINE);

	/**
	 * Remove user's dispatchers.
	 *
	 * @TODO We need to create a task to the dispatcher of the USER call kthread_exit.
	 */
#if 0
	if (NDISPATCHERS > 1 && false)
	{
		for (int i = 1; i < NDISPATCHERS; ++i)
			kthread_join(tid[i], NULL);

		uprintf("[benchmark][dispatch-wait] Auxiliary dispatchers removed.");
		uprintf(HLINE);
	}
#endif

	return (0);
}
