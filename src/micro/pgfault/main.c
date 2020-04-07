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

#include <nanvix/runtime/rmem.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

#define NUM_PAGES RMEM_CACHE_SIZE

extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);

/**
 * @brief Dummy buffer 1.
 */
static char buffer[RMEM_BLOCK_SIZE];

void benchmark_pgfault(void)
{
	unsigned char *ptr;
	uint64_t time_alloc, time_remote, time_local, time_free;

	umemset(buffer, 0, RMEM_BLOCK_SIZE);

	/* Allocate many blocks.*/
#ifndef NDEBUG
	uprintf("[benchmarks][pgfault] allocating memory...");
#endif
		perf_start(0, PERF_CYCLES);
		uassert((ptr = nanvix_malloc(NUM_PAGES*RMEM_BLOCK_SIZE)) != NULL);
		perf_stop(0);
		time_alloc = perf_read(0);

#ifndef NDEBUG
	uprintf("[benchmarks][pgfault] benchmarking...");
#endif
	perf_start(0, PERF_CYCLES);
	for (int i = 0; i < NUM_PAGES; i++)
		umemcpy(&ptr[i*RMEM_BLOCK_SIZE], buffer, RMEM_BLOCK_SIZE);
	perf_stop(0);
	time_remote = perf_read(0);
	perf_start(0, PERF_CYCLES);
	for (int i = 0; i < NUM_PAGES; i++)
		umemcpy(&ptr[i*RMEM_BLOCK_SIZE], buffer, RMEM_BLOCK_SIZE);
	perf_stop(0);
	time_local = perf_read(0);

	/* Free all blocks. */
#ifndef NDEBUG
	uprintf("[benchmarks][pgfault] freeing memory...");
#endif
	perf_start(0, PERF_CYCLES);
		nanvix_free(ptr);
	perf_stop(0);
	time_free = perf_read(0);


#ifndef NDEBUG
	uprintf("[benchmarks][pgfault] alloc %l remote %l local %l free %l",
#else
	uprintf("[benchmarks][pgfault] %l %l %l %l",
#endif
		time_alloc,
		time_remote,
		time_local,
		time_free
	);
}

/**
 * @brief Syntetic Benchmark
 */
int __main3(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	benchmark_pgfault();

	return (0);
}
