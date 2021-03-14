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
#include <nanvix/sys/perf.h>
#include <nanvix/sys/mutex.h>
#include <nanvix/sys/thread.h>
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

int inboxes[2];
int ports[2];
struct nanvix_mutex ms[3];

/**
 * @brief Benchmarks invalidation of shared memory regions.
 */
static void benchmark_pgfetch(void)
{
	int shmid;
	barrier_t barrier;
	int nodes[__NPROCS];
	uint64_t time_pgfetch;
	const char *shm_name = "cool-region";

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

	ktask_t *shm, *look, *beat;
	const char * pname = nanvix_getpname();

	for (int i = 0; i < __NITERATIONS + __SKIP; i++)
	{
		if (knode_get_num() == PROCESSOR_NODENUM_LEADER)
		{
			uprintf("----- === ----- start exec %d", i);

			perf_start(0, PERF_CYCLES);

				KASSERT((shm = __nanvix_shm_inval_task_alloc(shmid, inboxes[0], ports[0])) != NULL);
				KASSERT((look = nanvix_name_lookup_task_alloc(pname, inboxes[1], inboxes[1])) != NULL);
				KASSERT((beat = nanvix_name_heartbeat_task_alloc()) != NULL);

				KASSERT(ktask_wait(shm) == 0);
				KASSERT(ktask_wait(look) == 0);
				KASSERT(ktask_wait(beat) == 0);

			perf_stop(0);
			time_pgfetch = perf_read(0);

			uprintf("----- === ----- stop exec %d", i);

			if (i >= __SKIP)
			{
#ifndef NDEBUG
				uprintf("[benchmarks][pginval] %l",
#else
				uprintf("[benchmarks][pginval] %l",
#endif
					time_pgfetch
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
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

void * setup_mailbox(void * arg)
{
	int i = (int) (intptr_t) arg;

	__stdmailbox_setup();

	inboxes[i] = stdinbox_get();
	ports[i]   = stdinbox_get_port();

	nanvix_mutex_unlock(&ms[i]);

	nanvix_mutex_lock(&ms[2]);
	nanvix_mutex_unlock(&ms[2]);

	return (NULL);
}

/**
 * @brief Launches a benchmark.
 */
int __main3(int argc, const char *argv[])
{
	kthread_t tids[2];

	((void) argc);
	((void) argv);

	for (int i = 0; i < 3; i++)
		nanvix_mutex_init(&ms[i]);

	for (int i = 0; i < 3; i++)
		nanvix_mutex_lock(&ms[i]);

	for (int i = 0; i < 2; i++)
		kthread_create(&tids[i], setup_mailbox, (void *) (intptr_t) i);

	for (int i = 0; i < 2; i++)
		nanvix_mutex_lock(&ms[i]);

	benchmark_pgfetch();

	nanvix_mutex_unlock(&ms[2]);

	for (int i = 0; i < 2; i++)
		kthread_join(tids[i], NULL);

	return (0);
}
