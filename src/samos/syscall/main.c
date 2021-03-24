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
#define BENCHMARK_NAME1 "dispatcher-syscall"

/**
 * @brief Mumber of operations.
 */
#define NOPERATIONS 100

/**
 * @brief Max number of threads.
 */
#define NTHREADS_LOCAL_MAX (CORES_NUM - 1)

/**
 * @name Auxiliary variables
 */
/**@{*/
static struct fence_t _fence;        /**< Global fence. */
static int NUSERS;                   /**< Number of Worker Threads        */
static int NTAKERS;             /**< Number of Worker Threads        */
static int SYSCALL_NR = NR_SYSCALLS; /**< Type of the syscall with 1 arg. */
/**@}*/

/**
 * @brief Dump execution statistics.
 *
 * @param it     Benchmark iteration.
 * @param name   Benchmark name.
 * @param ustats User land statistics.
 * @param kstats Kernel land statistics.
 */
static void benchmark_dump_stats(
	int it,
	const char *name,
	uint64_t cycles
)
{
	uprintf(
		"[benchmarks][%s] %d %d %d %d %l",
		name,
		it,
		NOPERATIONS,
		NUSERS,
		NTAKERS,
		UINT64(cycles)
	);
}

/**
 * @brief Measures syscall time.
 */
static void do_syscall(const char * benchmark)
{
	uint64_t stats[2 * NITERATIONS];

	fence(&_fence);

	for (int i = 0; i < (2 * NITERATIONS + SKIP); i++)
	{
		perf_start(0, PERF_CYCLES);

			/* Do syscalls. */
			for (int j = 0; j < NOPERATIONS; j++)
				kcall0(SYSCALL_NR);

		perf_stop(0);

		if (i >= SKIP)
			stats[i - SKIP] = perf_read(0);
	}

	for (int i = 0; i < 2 * NITERATIONS; i++)
		benchmark_dump_stats(i, benchmark, stats[i]);
}

/**
 * @brief Measure syscall.
 *
 * @param arg Unused argument.
 */
static void * measures0(void * args)
{
	UNUSED(args);

	do_syscall(BENCHMARK_NAME0);

	return (NULL);
}

/**
 * @brief Measure syscall.
 *
 * @param arg Unused argument.
 */
static int measures1(ktask_args_t * args)
{
	UNUSED(args);

	do_syscall(BENCHMARK_NAME1);

	return (TASK_RET_SUCCESS);
}

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Measures syscall time.
 */
static void kernel_syscall(int nusers, int ndispatchers)
{
	ktask_t task;
	kthread_t tids[NTHREADS_LOCAL_MAX];

	NUSERS  = nusers;
	NTAKERS = ndispatchers;

	fence_init(&_fence, nusers + ndispatchers);

	for (int i = 0; i < nusers; i++)
		kthread_create(&tids[i], measures0, NULL);

	if (ndispatchers)
	{
		uassert(ktask_create(&task, measures1, NULL, 0) == 0);
		uassert(ktask_dispatch(&task) == 0);
		uassert(ktask_wait(&task) == 0);
	}

	for (int i = 0; i < nusers; i++)
		kthread_join(tids[i], NULL);
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

	for (int i = 0; i < 2; ++i)
		for (int j = 0; j <= NTHREADS_LOCAL_MAX; ++j)
			kernel_syscall(j, i);

	uprintf(HLINE);

	return (0);
}

