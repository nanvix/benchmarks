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

#define __NEED_MM_RMEM_STUB

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/barrier.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>

/**
 * @brief Number of Benchmark Iterations
 */
#ifndef __NITERATIONS
#define __NITERATIONS 1
#endif

/**
 * @brief Number of Warmup Iterations
 */
#ifndef __SKIP
#define __SKIP 1
#endif

/**
 * @brief Write Size
 */
#ifndef __NUM_BLOCKS
#define __NUM_BLOCKS 8
#endif

/**
 * @brief Number of Working Processes
 */
#ifndef __NPROCS
#define __NPROCS 1
#endif

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Working Buffer
 */
static char buffer[RMEM_BLOCK_SIZE];

/**
 * @griup Timing Statistics
 */
/**@{*/
static uint64_t time_kernel;
/**@}*/

/**
 * @brief Data Blocks
 */
static rpage_t blks[__NUM_BLOCKS];

/**
 * @brief Benchmark setup.
 */
static void benchmark_setup(void)
{
	for (unsigned long i = 0; i < __NUM_BLOCKS; i++)
		uassert((blks[i] = nanvix_rmem_alloc()) != RMEM_NULL);
}

/**
 * @brief Benchmark cleanup.
 */
static void benchmark_cleanup(void)
{
	for (unsigned long i = 0; i < __NUM_BLOCKS; i++)
		uassert(nanvix_rmem_free(blks[i]) == 0);
}

/**
 * @brief Dump execution statistics.
 */
static void benchmark_dump_stats(void)
{
#ifndef NDEBUG
	uprintf("[benchmarks][memread] nprocs %d, read size %d, read, %l",
#else
	uprintf("[benchmarks][memread] %d %d %l",
#endif
		__NPROCS,
		__NUM_BLOCKS*RMEM_BLOCK_SIZE,
		time_kernel
	);
}

/**
 * @brief Benchmark Kernel
 */
static void benchmark_kernel(void)
{
	perf_start(0, PERF_CYCLES);

	/* Read and write. */
	for (unsigned long i = 0; i < __NUM_BLOCKS; i++)
	{
		uassert(nanvix_rmem_read(blks[i], buffer) == RMEM_BLOCK_SIZE);
		uassert(nanvix_rmem_write(blks[i], buffer) == RMEM_BLOCK_SIZE);
	}

	/* Free all blocks. */
	perf_stop(0);
	time_kernel = perf_read(0);
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Benchmarks reads from remote memory.
 */
int __main3(int argc, const char *argv[])
{
#if (__NPROCS > 1)
	barrier_t barrier;
	int nodes[__NPROCS];
#endif

	((void) argc);
	((void) argv);

#if (__NPROCS > 1)
	/* Build list of nodes. */
	for (int i = 0; i < __NPROCS; i++)
		nodes[i] = PROCESSOR_NODENUM_LEADER + i;

	barrier = barrier_create(nodes, __NPROCS);
	uassert(BARRIER_IS_VALID(barrier));
#endif

	benchmark_setup();

#if (__NPROCS > 1)
	uassert(barrier_wait(barrier) == 0);
#endif

	for (int i = 0; i < __NITERATIONS + __SKIP; i++)
	{
		benchmark_kernel();

		if (i >= __SKIP)
			benchmark_dump_stats();
	}

#if (__NPROCS > 1)
	uassert(barrier_wait(barrier) == 0);
#endif

	benchmark_cleanup();

#if (__NPROCS > 1)
	uassert(barrier_wait(barrier) == 0);
	uassert(barrier_destroy(barrier) == 0);
#endif

	return (0);
}