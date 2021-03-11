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

#ifndef __qemu_riscv32__

/**
 * @brief Max number of threas.
 */
#define NTHREADS_LOCAL_MAX (CORES_NUM - 1)

/**
 * @name Benchmark Kernel Parameters
 */
/**@{*/
static int NWORKERS;                 /**< Number of Worker Threads        */
static int NIDLES;                   /**< Number of Idle Threads          */
static int NIOS;                     /**< Number of IO Threads            */
static int SYSCALL_NR = NR_SYSCALLS; /**< Type of the syscall with 1 arg. */
/**@}*/

/**
 * @name Auxiliary variables
 */
/**@{*/
static struct fence_t _fence; /**< Global fence. */
/**@}*/

/*============================================================================*
 * Profiling                                                                  *
 *============================================================================*/

/**
 * @brief Name of the benchmark.
 */
#define BENCHMARK_NAME "noise"

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
 * @brief Time out verification
 */
static inline uint64_t time_out(uint64_t t0, uint64_t error)
{
	return ((clock_read() - t0 - error) >= EXECUTION_PERIOD);
}

/**
 * @brief Dump execution statistics.
 *
 * @param it    Benchmark iteration.
 * @oaram name  Benchmark name.
 * @param stats Execution statistics.
 */
static void benchmark_dump_stats(int it, const char *name, const char *rule, uint64_t *stats)
{
	uprintf(
#if (BENCHMARK_PERF_EVENTS >= 7)
		"[benchmarks][%s][%s] %d %d %d %d %d %l %l %l %l %l %l %l",
#elif (BENCHMARK_PERF_EVENTS >= 5)
		"[benchmarks][%s][%s] %d %d %d %d %d %l %l %l %l %l",
#else
		"[benchmarks][%s][%s] %d %d %d %d %d %l",
#endif
		name,
		rule,
		it,
		NWORKERS,
		NIDLES,
		NIOS,
		UINT64(stats[BENCHMARK_PERF_EVENTS]),
#if (BENCHMARK_PERF_EVENTS >= 7)
		UINT64(stats[6]),
		UINT64(stats[5]),
#endif
#if (BENCHMARK_PERF_EVENTS >= 5)
		UINT64(stats[4]),
		UINT64(stats[3]),
		UINT64(stats[2]),
		UINT64(stats[1]),
#endif
		UINT64(stats[0])
	);
}

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/*----------------------------------------------------------------------------*
 * Worker                                                                     *
 *----------------------------------------------------------------------------*/

/**
 * @brief Task info.
 */
static struct tdata
{
	float scratch;  /**< Scratch Variable  */
} tdata[NTHREADS_LOCAL_MAX] ALIGN(CACHE_LINE_SIZE);

/**
 * @brief Performs some FPU intensive computation.
 */
static void *task_worker(void *arg)
{
	int k;
	uint64_t t0, error;
	struct tdata *t = arg;
	register float tmp = t->scratch;
	uint64_t stats[BENCHMARK_PERF_EVENTS + 1];

	t0    = clock_read();
	error = clock_read() - t0;

	fence(&_fence);

	for (int i = 0; i < NITERATIONS + SKIP; i++)
	{
		stats[BENCHMARK_PERF_EVENTS] = 0;
		k = 0;
		t0 = clock_read();
		do
		{
			register float k1 = k*1.1;
			register float k2 = k*2.1;
			register float k3 = k*3.1;
			register float k4 = k*4.1;

			tmp += k1 + k2 + k3 + k4;
			k   += 9;

			stats[BENCHMARK_PERF_EVENTS]++;
		}
		while (!time_out(t0, error));

		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{
			perf_start(0, perf_events[j]);

				k = 0;
				t0 = clock_read();

				do
				{
					register float k1 = k*1.1;
					register float k2 = k*2.1;
					register float k3 = k*3.1;
					register float k4 = k*4.1;

					tmp += k1 + k2 + k3 + k4;
					k   += 9;
				}
				while (!time_out(t0, error));

			perf_stop(0);
			stats[j] = perf_read(0);
		}

		if (i >= SKIP)
			benchmark_dump_stats(i - SKIP, BENCHMARK_NAME, "worker", stats);
	}

	/* Avoid compiler optimizations. */
	t->scratch = tmp;

	return (NULL);
}

/*----------------------------------------------------------------------------*
 * Idle                                                                       *
 *----------------------------------------------------------------------------*/

/**
 * @brief Issues some remote kernel calls.
 */
static void *task_idle(void *arg)
{
	uint64_t t0, error;

	UNUSED(arg);

	t0    = clock_read();
	error = clock_read() - t0;

	fence(&_fence);

	for (int i = 0; i < NITERATIONS + SKIP; i++)
	{
		for (int j = 0; j < (BENCHMARK_PERF_EVENTS + 1); j++)
		{
			t0 = clock_read();
			do
				kcall0(SYSCALL_NR);
			while (!time_out(t0, error));
		}
	}

	return (NULL);
}

/*----------------------------------------------------------------------------*
 * Task                                                                       *
 *----------------------------------------------------------------------------*/

/**
 * @brief Issues some remote kernel calls.
 */
static void *task_io(void *arg)
{
	uint64_t t0, error;
	uint64_t stats[BENCHMARK_PERF_EVENTS + 1];

	UNUSED(arg);

	t0    = clock_read();
	error = clock_read() - t0;

	fence(&_fence);

	for (int i = 0; i < NITERATIONS + SKIP; i++)
	{
		stats[BENCHMARK_PERF_EVENTS] = 0;

		t0 = clock_read();
		do
		{
			nanvix_name_heartbeat();
			stats[BENCHMARK_PERF_EVENTS]++;
		}
		while (!time_out(t0, error));

		for (int j = 0; j < BENCHMARK_PERF_EVENTS; j++)
		{

			perf_start(0, perf_events[j]);

				t0 = clock_read();
				do
					nanvix_name_heartbeat();
				while (!time_out(t0, error));

			perf_stop(0);
			stats[j] = perf_read(0);
		}

		if (i >= SKIP)
			benchmark_dump_stats(i - SKIP, BENCHMARK_NAME, "io", stats);
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
static void benchmark_noise(int nworkers, int nidles, int nios)
{
	kthread_t tid_workers[NTHREADS_LOCAL_MAX];
	kthread_t tid_idles[NTHREADS_LOCAL_MAX];
	kthread_t tid_ios[NTHREADS_LOCAL_MAX];

	/* Save kernel parameters. */
	NWORKERS = nworkers;
	NIDLES   = nidles;
	NIOS     = nios;

	fence_init(&_fence, (nworkers + nidles + nios));

	/*
	 * Spawn idle threads first,
	 * so that we have a noisy system.
	 */
	for (int i = 0; i < nidles; i++)
		kthread_create(&tid_idles[i], task_idle, NULL);

	/* Spawn IO threads. */
	for (int i = 0; i < nios; i++)
		kthread_create(&tid_ios[i], task_io, NULL);

	/* Spawn worker threads. */
	for (int i = 0; i < nworkers; i++)
	{
		tdata[i].scratch = 0.0;
		kthread_create(&tid_workers[i], task_worker, &tdata[i]);
	}

	/* Wait for threads. */
	for (int i = 0; i < nworkers; i++)
		kthread_join(tid_workers[i], NULL);
	for (int i = 0; i < nios; i++)
		kthread_join(tid_ios[i], NULL);
	for (int i = 0; i < nidles; i++)
		kthread_join(tid_idles[i], NULL);
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
int __main3(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

#ifndef __qemu_riscv32__

	uprintf(HLINE);

#ifndef NDEBUG

	benchmark_noise(NTHREADS_LOCAL_MAX/3, NTHREADS_LOCAL_MAX/3, NTHREADS_LOCAL_MAX/3);

#else

	/**
	 * With noise:
	 * x = number of worker threads
	 * y = number of idle threads
	 * z = number of io threads
	 */
	for (int x = 0; x < NTHREADS_LOCAL_MAX; x += NTHREADS_STEP)
		for (int y = 0; y < NTHREADS_LOCAL_MAX; y += NTHREADS_STEP)
			for (int z = 0; z < NTHREADS_LOCAL_MAX; z += NTHREADS_STEP)
			{
				/**
				 * Mininum of one type thread and maximum of one
				 * type thread per user core.
				 */
				if (!WITHIN((x + y + z), 1, NTHREADS_LOCAL_MAX))
					continue;

				benchmark_noise(x, y, z);
			}

#endif

	uprintf(HLINE);

#endif

	return (0);
}
