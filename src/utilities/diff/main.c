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

/* Import definitions. */
extern int diff(const char *s, const char *t);

/**
 * @brief Length of text1.
 */
#ifndef __TEXT_LENGTH
#define __TEXT_LENGTH 32
#endif

/**
 * @group Timing Statistics
 */
/**@{*/
static uint64_t time_kernel;
/**@}*/

/**
 * @brief Text 1.
 */
static char *text1 = NULL;

/**
 * @brief Text 2.
 */
static char *text2 = NULL;

/**
 * @brief Initializes the benchmark.
 */
static void benchmark_setup(void)
{
	uassert((text1 = nanvix_malloc(__TEXT_LENGTH + 1)) != NULL);
	uassert((text2 = nanvix_malloc(__TEXT_LENGTH + 1)) != NULL);

	/* Initialize text1. */
	for (int i = 0; i < __TEXT_LENGTH; i++)
		text1[i] = urand()%95 + 32;
	text1[__TEXT_LENGTH] = '\0';

	/* Initialize text2. */
	for (int i = 0; i < __TEXT_LENGTH; i++)
		text2[i] = urand()%95 + 32;
	text2[__TEXT_LENGTH] = '\0';
}

/**
 * @brief Cleans up the benchmark.
 */
static void benchmark_cleanup(void)
{
	nanvix_free(text1);
	nanvix_free(text2);
}

/**
 * @brief Benchmark kernel.
 */
static void benchmark_kernel(void)
{
	perf_start(0, PERF_CYCLES);

		diff(text1, text2);

	perf_stop(0);
	time_kernel = perf_read(0);
}

/**
 * @brief Dump execution statistics.
 */
static void benchmark_dump_stats(void)
{
#ifndef NDEBUG
	uprintf("[benchmarks][diff] text %d, time %l",
#else
	uprintf("[benchmarks][diff] %d %l",
#endif
		__TEXT_LENGTH,
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
