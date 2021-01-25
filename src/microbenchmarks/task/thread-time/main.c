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

#include <nanvix/runtime/runtime.h>
#include <nanvix/sys/perf.h>
#include <nanvix/sys/thread.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>

/**
 * @brief Number of tasks.
 */
#ifndef __NTASKS
#define __NTASKS 1
#endif

/**
 * @brief Number of benchmark iterations.
 */
#ifndef __NITERATIONS
#define __NITERATIONS 1
#endif

/**
 * @brief Iterations to skip on warmup.
 */
#ifndef __SKIP
#define __SKIP 1
#endif

/**
 * @brief Casts something to a uint32_t.
 */
#define UINT32(x) ((uint32_t)((x) & 0xffffffff))

/**
 * @brief Dump execution statistics.
 */
static void benchmark_dump_stats(uint64_t t0, uint64_t t1)
{
	/*[benchmarks][stream] ndispatcher npages fork join */
	uprintf(
		"[benchmarks][thread] %d %d %l %l",
		__NTASKS,
		__NTASKS * 2 * PAGE_SIZE,
		t0,
		t1
	);
}

/**
 * @brief Operation dummy.
 */
static void * dummy(void * args)
{
	UNUSED(args);
	return (NULL);
}

/**
 * @brief Benchmarks check the time to dispatch a task.
 */
static void benchmark_task_time(void)
{
	uint64_t t0, t1;
	kthread_t tid[__NTASKS];

	for (int i = 0; i < __NITERATIONS + __SKIP; ++i)
	{
		perf_start(0, PERF_CYCLES);

			for (int j = 0; j < __NTASKS; ++j)
				kthread_create(&tid[j], dummy, NULL);

		perf_stop(0);
		t0 = perf_read(0);

		perf_start(0, PERF_CYCLES);

			for (int j = 0; j < __NTASKS; ++j)
				kthread_join(tid[j], NULL);

		perf_stop(0);
		t1 = perf_read(0);

		if (i >= __SKIP)
			benchmark_dump_stats(t0, t1);
	}
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Launches a benchmark.
 */
int __main3(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	benchmark_task_time();

	return (0);
}

