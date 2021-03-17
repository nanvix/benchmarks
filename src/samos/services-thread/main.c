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
#include <nanvix/runtime/barrier.h>
#include <nanvix/sys/thread.h>
#include <nanvix/sys/perf.h>
#include <nanvix/runtime/fence.h>
#include <nanvix/ulib.h>
#include <nanvix/limits.h>
#include <posix/sys/stat.h>

/**
 * @brief Number of Benchmark Iterations
 */
#ifndef __NITERATIONS
#define __NITERATIONS 50
#endif

/**
 * @brief Number of Warmup Iterations
 */
#ifndef __SKIP
#define __SKIP 10
#endif

/**
 * @brief Number of Working Processes
 */
#ifndef __NPROCS
#define __NPROCS 2
#endif

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Dummy buffer used for tests.
 */
static char buffer[NANVIX_SHM_SIZE_MAX];

/**
 * @brief Fence.
 */
static struct fence_t _fence;

static uint64_t time_shm;
static uint64_t time_heartbeat;
static uint64_t time_lookup;

/**
 * @brief Benchmarks heart beats.
 */
static void * benchmark_services_lookup(void * args)
{
	const char *pname;

	UNUSED(args);

	pname = nanvix_getpname();

	for (int i = 0; i < (__SKIP + __NITERATIONS); ++i)
	{
		fence(&_fence);

			perf_start(0, PERF_CYCLES);

				nanvix_name_lookup(pname);

			perf_stop(0);
			time_lookup = perf_read(0);

		fence(&_fence);
	}

	return (NULL);
}
/**
 * @brief Benchmarks heart beats.
 */
static void * benchmark_services_heartbeat(void * args)
{
	UNUSED(args);

	for (int i = 0; i < (__SKIP + __NITERATIONS); ++i)
	{
		fence(&_fence);

			perf_start(0, PERF_CYCLES);

				nanvix_name_heartbeat();

			perf_stop(0);
			time_heartbeat = perf_read(0);

		fence(&_fence);
	}

	return (NULL);
}

/**
 * @brief Benchmarks invalidation of shared memory regions.
 */
static void * benchmark_services_shm(void * args)
{
	int shmid;
	barrier_t barrier;
	int nodes[__NPROCS];
	const char *shm_name = "cool-region";

	UNUSED(args);

	/* Build list of nodes. */
	for (int i = 0; i < __NPROCS; i++)
		nodes[i] = PROCESSOR_NODENUM_LEADER + i;

	barrier = barrier_create(nodes, __NPROCS);
	uassert(BARRIER_IS_VALID(barrier));

	/* IPC initialization. */
	if (knode_get_num() == PROCESSOR_NODENUM_LEADER)
	{
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);
		uassert(__nanvix_shm_ftruncate(shmid, NANVIX_SHM_SIZE_MAX) == 0);
		umemset(buffer, 1, NANVIX_SHM_SIZE_MAX);
		uassert(__nanvix_shm_write(shmid, buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);
		umemset(buffer, 0, NANVIX_SHM_SIZE_MAX);
		uassert(__nanvix_shm_read(shmid, buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);

		uassert(barrier_wait(barrier) == 0);
	}
	else
	{
		uassert(barrier_wait(barrier) == 0);
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR, 0)) >= 0);
	}

	uassert(barrier_wait(barrier) == 0);

	if (knode_get_num() == PROCESSOR_NODENUM_LEADER)
	{
		for (int i = 0; i < __NITERATIONS + __SKIP; i++)
		{
			fence(&_fence);

			perf_start(0, PERF_CYCLES);

				uassert(__nanvix_shm_inval(shmid) == 0);

			perf_stop(0);
			time_shm = perf_read(0);

			fence(&_fence);

			if (i >= __SKIP)
			{
				uint64_t major = time_shm;
				major = major < time_heartbeat ? time_heartbeat : major;
				major = major < time_lookup    ? time_lookup    : major;
#ifndef NDEBUG
				uprintf("[benchmarks][services][thread] %l",
#else
				uprintf("[benchmarks][services][thread] %l",
#endif
					major
				);
			}
		}
	}

	uassert(barrier_wait(barrier) == 0);

	/* House keeping. */
	uassert(__nanvix_shm_close(shmid) == 0);
	if (knode_get_num() == PROCESSOR_NODENUM_LEADER)
		uassert(__nanvix_shm_unlink(shm_name) == 0);
	uassert(barrier_destroy(barrier) == 0);

	return (NULL);
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Launches a benchmark.
 */
int __main3(int argc, const char *argv[])
{
	kthread_t tids[2];

	((void) argc);
	((void) argv);

	if (knode_get_num() == PROCESSOR_NODENUM_LEADER)
	{
		fence_init(&_fence, 3);

		kthread_create(&tids[0], benchmark_services_heartbeat, NULL);
		kthread_create(&tids[1], benchmark_services_lookup, NULL);
	}

	benchmark_services_shm(NULL);

	if (knode_get_num() == PROCESSOR_NODENUM_LEADER)
	{
		for (int i = 0; i < 2; i++)
			kthread_join(tids[i], NULL);
	}

	return (0);
}
