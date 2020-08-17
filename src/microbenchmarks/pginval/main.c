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
#include <nanvix/ulib.h>
#include <nanvix/limits.h>
#include <posix/sys/stat.h>

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Dummy buffer used for tests.
 */
static char buffer[RMEM_BLOCK_SIZE];

/**
 * @brief Benchmarks invalidation of shared memory regions.
 */
static void benchmark_msync(void)
{
	int shmid;
	const char *shm_name = "cool-region";
	uint64_t time_msync;

	/* Leader. */
	if (kcluster_get_num() == PROCESSOR_CLUSTERNUM_LEADER)
	{
		uassert((shmid = __nanvix_shm_open(shm_name, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);

			uassert(__nanvix_shm_ftruncate(shmid, NANVIX_SHM_SIZE_MAX) == 0);

			umemset(buffer, 1, NANVIX_SHM_SIZE_MAX);
			uassert(__nanvix_shm_write(shmid, buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);
			umemset(buffer, 0, NANVIX_SHM_SIZE_MAX);
			uassert(__nanvix_shm_read(shmid, buffer, NANVIX_SHM_SIZE_MAX, 0) == NANVIX_SHM_SIZE_MAX);

			perf_start(0, PERF_CYCLES);
			uassert(__nanvix_shm_inval(shmid) == 0);
			perf_stop(0);
			time_msync = perf_read(0);

		uassert(__nanvix_shm_close(shmid) == 0);
		uassert(__nanvix_shm_unlink(shm_name) == 0);

#ifndef NDEBUG
		uprintf("[benchmarks][msync] %l",
#else
		uprintf("[benchmarks][msync] %l",
#endif
			time_msync
		);
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
	barrier_t barrier;
	int nodes[NANVIX_PROC_MAX];

	((void) argc);
	((void) argv);

	/* Build list of nodes. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i] = PROCESSOR_NODENUM_LEADER + i;

	barrier = barrier_create(nodes, NANVIX_PROC_MAX);
	uassert(BARRIER_IS_VALID(barrier));
	uassert(barrier_wait(barrier) == 0);

		benchmark_msync();

	uassert(barrier_wait(barrier) == 0);
	uassert(barrier_destroy(barrier) == 0);

	return (0);
}
