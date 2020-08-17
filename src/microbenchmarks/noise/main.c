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
#include <nanvix/sys/thread.h>
#include <nanvix/ulib.h>
#include <posix/stdint.h>
#include "kbench.h"

#ifndef __qemu_riscv32__

/**
 * @name Benchmark Parameters
 */
/**@{*/
#define NTHREADS_MIN                1  /**< Minimum Number of Worker Threads      */
#define NTHREADS_MAX  (THREAD_MAX - 1) /**< Maximum Number of Worker Threads      */
#define NTHREADS_STEP               1  /**< Increment on Number of Worker Threads */
#define FLOPS                 (10008)  /**< Number of Floating Point Operations   */
#define NIOOPS                  (100)  /**< Number of Floating Point Operations   */
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
static int NWORKERS;      /**< Number of Worker Threads */
static int NIDLE;         /**< Number of Idle Threads   */
static char *NOISE = "y"; /**< Noise On?                */
/**@}*/

/*============================================================================*
 * Profiling                                                                  *
 *============================================================================*/

/**
 * @brief Name of the benchmark.
 */
#define BENCHMARK_NAME "noise"

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
 * @param it    Benchmark iteration.
 * @oaram name  Benchmark name.
 * @param stats Execution statistics.
 */
static void benchmark_dump_stats(int it, const char *name, uint64_t *stats)
{
	uprintf(
#if (BENCHMARK_PERF_EVENTS >= 7)
		"[benchmarks][%s] %d %s %d %d %d %d %d %d %d %d %d",
#elif (BENCHMARK_PERF_EVENTS >= 5)
		"[benchmarks][%s] %d %s %d %d %d %d %d %d %d",
#else
		"[benchmarks][%s] %d %s %d %d %d",
#endif
		name,
		it,
		NOISE,
		NWORKERS,
		NIDLE,
#if (BENCHMARK_PERF_EVENTS >= 7)
		UINT32(stats[6]),
		UINT32(stats[5]),
#endif
#if (BENCHMARK_PERF_EVENTS >= 5)
		UINT32(stats[4]),
		UINT32(stats[3]),
		UINT32(stats[2]),
		UINT32(stats[1]),
#endif
		UINT32(stats[0])
	);
}

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
	uint64_t stats[BENCHMARK_PERF_EVENTS];

	for (int i = 0; i < NITERATIONS + SKIP; i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			perf_start(0, perf_events[j]);

				for (int k = 0; k < FLOPS; k += 9)
				{
					register float k1 = k*1.1;
					register float k2 = k*2.1;
					register float k3 = k*3.1;
					register float k4 = k*4.1;

					tmp += k1 + k2 + k3 + k4;
				}

			perf_stop(0);
			stats[j] = perf_read(0);
		}

		if (i >= SKIP)
			benchmark_dump_stats(i - SKIP, BENCHMARK_NAME, stats);
	}

	/* Avoid compiler optimizations. */
	t->scratch = tmp;

	return (NULL);
}

/**
 * @brief Issues some remote kernel calls.
 */
static void *task_idle(void *arg)
{
	UNUSED(arg);

	for (int i = 0; i < NITERATIONS + SKIP; i++)
	{
		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			for (int k = 0; k < NIOOPS; k++)
				kcall0(NR_SYSCALLS);
		}
	}

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
}

#endif

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Kernel Noise Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

#ifndef __qemu_riscv32__

	if (knode_get_num() != PROCESSOR_NODENUM_MASTER)
		return (0);

	uprintf(HLINE);

#ifndef NDEBUG

	benchmark_noise(NTHREADS_MAX/2, NTHREADS_MAX/2);

#else

	/* With noise. */
	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
		benchmark_noise(nthreads, NTHREADS_MAX - nthreads);

	/* No noise. */
	NOISE = "n";
	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
		benchmark_noise(nthreads, 0);

#endif

	uprintf(HLINE);

#endif

	return (0);
}
