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
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>
#include <posix/stdlib.h>

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Magic number.
 */
const unsigned MAGIC = 0xdeadbeef;

/**
 * @brief Benchmarks page fetches.
 */
static void benchmark_pgfetch(void)
{
	unsigned *ptr;
	uint64_t time_pgfetch;

	perf_start(0, PERF_CYCLES);

		uassert((ptr = nanvix_malloc(sizeof(unsigned))) != NULL);

		*ptr = MAGIC;

	perf_stop(0);
	time_pgfetch = perf_read(0);

	/* Checksum. */
	uassert(*ptr == MAGIC);

	nanvix_free(ptr);

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
