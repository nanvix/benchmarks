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

#include <nanvix/sys/thread.h>

#include <nanvix/sys/noc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/sys/thread.h>
#include <nanvix/kernel/kernel.h>
#include <nanvix/ulib.h>

#include <posix/stdint.h>

#include "monitoring.h"

/**
 * @brief Number of benchmark iterations.
 */
#define NITERATIONS 1

/**
 * @brief Casts something to a uint32_t.
 */
#define UINT32(x) ((uint32_t)((x) & 0xffffffff))

/**
 * @name Benchmark Parameters
 */
/**@{*/
#define NTHREADS_MIN        1 /**< Minimum Number of Worker Threads      */
#define NTHREADS_MAX     (12) /**< Maximum Number of Worker Threads      */
#define NTHREADS_STEP       1 /**< Increment on Number of Worker Threads */
#define FLOPS         (10008) /**< Number of Floating Point Operations   */
#define NIOOPS          (100) /**< Number of Floating Point Operations   */
/**@}*/

/**
 * @name Benchmark Kernel Parameters
 */
/**@{*/
static int NWORKERS;      /**< Number of Worker Threads */
static int NIDLE;         /**< Number of Idle Threads   */
/**@}*/

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Task info.
 */
static struct tdata
{
	float scratch;  /**< Scratch Variable  */
} tdata[NTHREADS_MAX] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Performs some FPU intensive computation.
 */
static void *task_worker(void *arg)
{
	struct tdata *t = arg;
	register float tmp = t->scratch;

	for (int k = 0; k < FLOPS; k += 9)
	{
		register float k1 = k*1.1;
		register float k2 = k*2.1;
		register float k3 = k*3.1;
		register float k4 = k*4.1;

		tmp += k1 + k2 + k3 + k4;
	}

	return (NULL);
}

/**
 * @brief Issues some remote kernel calls.
 */
static void *task_idle(void *arg)
{
	UNUSED(arg);

	for (int k = 0; k < NIOOPS; k++)
		kcall0(NR_SYSCALLS);

	return (NULL);
}

/*============================================================================*
 * Kernel Noise Benchmark                                                     *
 *============================================================================*/

/**
 * @brief Kernel Noise Benchmark Kernel
 *
 * @param nworkers Number of worker threads.
 * @param nidle    Number of idle threads.
 */
static void benchmark_noise(int nworkers, int nidle)
{
	kthread_t tid_workers[NTHREADS_MAX];
	kthread_t tid_idle[NTHREADS_MAX];

	/* Save kernel parameters. */
	NWORKERS = nworkers;
	NIDLE = nidle;

	while (true)
	{
		/*
		 * Spawn idle threads first,
		 * so that we have a noisy system.
		 */
		for (int i = 0; i < nidle; i++)
			kthread_create(&tid_idle[i], task_idle, NULL);

		/* Spawn worker threads. */
		for (int i = 0; i < nworkers; i++)
		{
			tdata[i].scratch = 0.0;
			kthread_create(&tid_workers[i], task_worker, &tdata[i]);
		}

		/* Wait for threads. */
		for (int i = 0; i < nworkers; i++)
			kthread_join(tid_workers[i], NULL);
		for (int i = 0; i < nidle; i++)
			kthread_join(tid_idle[i], NULL);

		delay(1, CLUSTER_FREQ);
	}
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Kernel Noise Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
void do_standalone(void)
{
	benchmark_noise(NTHREADS_MAX/2, NTHREADS_MAX/2);
}
