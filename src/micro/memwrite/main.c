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

#include <nanvix/runtime/runtime.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Number of blocks to allocate.
 */
#define NUM_PAGES RMEM_CACHE_SIZE

/**
 * @brief Dummy buffer 1.
 */
static char buffer1[RMEM_BLOCK_SIZE];

/**
 * @brief Memory Write Benchmark Kernel
 */
static void benchmark_memwrite(void)
{
	void *blks[NUM_PAGES];
	uint64_t time_alloc, time_kernel, time_free;

	/* Allocate memory .*/
#ifndef NDEBUG
	uprintf("[benchmarks][memwrite] allocating memory...");
#endif
	perf_start(0, PERF_CYCLES);
		for (int i = 0; i < NUM_PAGES; i++)
			uassert((blks[i] = nanvix_vmem_alloc(1)) != NULL);
	perf_stop(0);
	time_alloc = perf_read(0);

	umemset(buffer1, 1, RMEM_BLOCK_SIZE);

	/* Warmup. */
	for (int i = 0; i < NUM_PAGES; i++)
		uassert(nanvix_vmem_write(blks[i], buffer1, RMEM_BLOCK_SIZE) == RMEM_BLOCK_SIZE);

#ifndef NDEBUG
	uprintf("[benchmarks][memwrite] benchmarking...");
#endif
	perf_start(0, PERF_CYCLES);
		for (int i = 0; i < NUM_PAGES; i++)
			uassert(nanvix_vmem_write(blks[i], buffer1, RMEM_BLOCK_SIZE) == RMEM_BLOCK_SIZE);
	perf_stop(0);
	time_kernel = perf_read(0);

#ifndef NDEBUG
	uprintf("[benchmarks][memwrite] freeing memory...");
#endif
	perf_start(0, PERF_CYCLES);
		for (int i = NUM_PAGES - 1; i >= 0; i--)
			uassert(nanvix_vmem_free(blks[i]) == 0);
	perf_stop(0);
	time_free = perf_read(0);

#ifndef NDEBUG
	uprintf("[benchmarks][memwrite] alloc %l write %l free %l",
#else
	uprintf("[benchmarks][memwrite] %l %l %l",
#endif
		time_alloc,
		time_kernel,
		time_free
	);
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Syntetic Benchmark
 */
int __main3(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	benchmark_memwrite();

	return (0);
}

