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
 * @brief Number of dispatchers.
 */
#ifndef __NDISPATCHERS
#define __NDISPATCHERS (1 + ((__NTASKS - 1) < (CORES_NUM - 3) ? (__NTASKS - 1) : (CORES_NUM - 3)))
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
 * @brief Forces a platform-independent delay.
 *
 * @param cycles Delay in cycles.
 *
 * @author João Vicente Souto
 */
static void delay(int times, uint64_t cycles)
{
	uint64_t t0, t1;

	for (int i = 0; i < times; ++i)
	{
		kclock(&t0);

		do
			kclock(&t1);
		while ((t1 - t0) < cycles);
	}
}

/**
 * @brief Dump execution statistics.
 */
static void benchmark_dump_stats(uint64_t t0, uint64_t t1)
{
	/*[benchmarks][stream] ndispatcher npages dispatch wait */
	uprintf(
		"[benchmarks][task] %d %d %l %l",
		__NTASKS,
		__NTASKS * 2 * PAGE_SIZE,
		t0,
		t1
	);
}

/**
 * @brief Operation dummy.
 */
static int dummy(struct task_args * args)
{
	UNUSED(args);
	return (TASK_RET_SUCCESS);
}

extern void task_loop(void);

/**
 * @brief User dispatcher.
 */
static void * _task_loop(void * args)
{
	kthread_set_affinity(1 << ((int) (intptr_t) args)); 

	task_loop();

	return (NULL);
}

/**
 * @brief Benchmarks check the time to dispatch a task.
 */
static void benchmark_task_time(void)
{
	uint64_t t0, t1;
	struct task task[__NTASKS];
	kthread_t tid[__NDISPATCHERS];

	/* Build dispatchers (the kernel has one). */
	for (int i = 1; i < __NDISPATCHERS; ++i)
		kthread_create(&tid[i], _task_loop, (void *) (intptr_t) (i + 1));

	delay(1, CLUSTER_FREQ);

	for (int i = 0; i < __NITERATIONS + __SKIP; ++i)
	{
		perf_start(0, PERF_CYCLES);

			for (int j = 0; j < __NTASKS; ++j)
			{
				uassert(ktask_create(&task[j], dummy, NULL, 0) == 0);
				uassert(ktask_dispatch(&task[j]) == 0);
			}

		perf_stop(0);
		t0 = perf_read(0);

		perf_start(0, PERF_CYCLES);

			for (int j = 0; j < __NTASKS; ++j)
				uassert(ktask_wait(&task[j]) == 0);

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

