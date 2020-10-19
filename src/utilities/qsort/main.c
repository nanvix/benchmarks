/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
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
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>

#include <posix/stddef.h>
#include <posix/stdlib.h>

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

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Number of numbers.
 */
#ifndef __NUM_NUMBERS
#define __NUM_NUMBERS 31744
#endif

/* Import definitions. */
extern void nanvix_qsort(void *, size_t, size_t, int (*)(const void *, const void *));

/**
 * @group Timing Statistics
 */
/**@{*/
static uint64_t time_kernel;
/**@}*/

/**
 * @brief Numbers
 */
static int *numbers = NULL;

/**
 * #brief Comparison operator.
 */
static int cmp(const void *a, const void *b)
{
	return (*((int *)a) - (*(int *)b));
}

/**
 * @brief Initializes the benchmark.
 */
static void benchmark_setup(void)
{
	uassert((numbers = nanvix_malloc(__NUM_NUMBERS*sizeof(int))) != NULL);
}

/**
 * @brief Cleans up the benchmark.
 */
static void benchmark_cleanup(void)
{
	nanvix_free(numbers);
}

/**
 * @brief Benchmark kernel.
 */
static void benchmark_kernel(void)
{
	/* Initializer array of numbers. */
	usrand(13);
	for (int i = 0; i < __NUM_NUMBERS; i++)
		numbers[i] = urand();

	perf_start(0, PERF_CYCLES);

		nanvix_qsort(numbers, __NUM_NUMBERS, sizeof(int), cmp);

	perf_stop(0);
	time_kernel = perf_read(0);
}

/**
 * @brief Dump execution statistics.
 */
static void benchmark_dump_stats(void)
{
#ifndef NDEBUG
	uprintf("[benchmarks][qsort] numbers %d, time %l",
#else
	uprintf("[benchmarks][qsort] %d %l",
#endif
		__NUM_NUMBERS,
		time_kernel
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

	benchmark_setup();

	for (int i = 0; i < __NITERATIONS + __SKIP; i++)
	{
		benchmark_kernel();

		if (i >= __SKIP)
			benchmark_dump_stats();
	}

	benchmark_cleanup();

	return (0);
}
