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
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>
#include <nanvix/pm.h>

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Benchmarks name lookups.
 */
static void benchmark_lookup(void)
{
	int nodenum;
	const char *pname;
	uint64_t time_lookup;

	nodenum = knode_get_num();
	pname = nanvix_getpname();

	perf_start(0, PERF_CYCLES);
	uassert(name_lookup(pname) == nodenum);
	perf_stop(0);
	time_lookup = perf_read(0);

#ifndef NDEBUG
	uprintf("[benchmarks][lookup] %l",
#else
	uprintf("[benchmarks][lookup] %l",
#endif
		time_lookup
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

	benchmark_lookup();

	return (0);
}
