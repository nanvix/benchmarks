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

#define __NEED_MM_MANAGER

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/rmem.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Dummy buffer used for tests.
 */
static char buffer[RMEM_BLOCK_SIZE];

/**
 * @brief Benchmarks page fetches.
 */
static void benchmark_pgfetch(void)
{
	void *ptr;
	uint64_t time_pgfetch;

		uassert((ptr = nanvix_vmem_alloc(1)) != NULL);

	perf_start(0, PERF_CYCLES);

		uassert(nanvix_vmem_read(buffer, ptr, RMEM_BLOCK_SIZE) == RMEM_BLOCK_SIZE);

	perf_stop(0);
	time_pgfetch = perf_read(0);

	uassert(nanvix_vmem_free(ptr) == 0);

#ifndef NDEBUG
	uprintf("[benchmarks][pgfetch] %l",
#else
	uprintf("[benchmarks][pgfetch] %l",
#endif
		time_pgfetch
	);
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

	benchmark_pgfetch();

	return (0);
}
