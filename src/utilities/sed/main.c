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
#include <posix/stdlib.h>
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

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/* Import definitions. */
extern char *sed(const char *text, const char *pattern, const char *newpattern);

/**
 * @brief Length of text.
 */
#ifndef __TEXT_LENGTH
#define __TEXT_LENGTH 65536 - 1
#endif

/**
 * @brief Length of pattern.
 */
#ifndef __PATTERN_LENGTH
#define __PATTERN_LENGTH 8
#endif

/**
 * @brief Length of new pattern.
 */
#ifndef __NEWPATTERN_LENGTH
#define __NEWPATTERN_LENGTH 4
#endif

/**
 * @group Timing Statistics
 */
/**@{*/
static uint64_t time_kernel;
/**@}*/

/**
 * @brief Text.
 */
static char *text = NULL;

/**
 * @brief Pattern.
 */
static char pattern[__PATTERN_LENGTH + 1];

/**
 * @brief new pattern.
 */
static char newpattern[__NEWPATTERN_LENGTH + 1];

/**
 * @brief Initializes the benchmark.
 */
static void benchmark_setup(void)
{
	uassert((text = nanvix_malloc(__TEXT_LENGTH + 1)) != NULL);

	/* Initialize text. */
	for (int i = 0; i < __TEXT_LENGTH; i++)
		text[i] = urand()%95 + 32;
	text[__TEXT_LENGTH] = '\0';

	/* Initialize pattern. */
	for (int i = 0; i < __PATTERN_LENGTH; i++)
		pattern[i] = urand()%95 + 32;
	pattern[__PATTERN_LENGTH] = '\0';

	/* Initialize pattern. */
	for (int i = 0; i < __NEWPATTERN_LENGTH; i++)
		newpattern[i] = urand()%95 + 32;
	newpattern[__NEWPATTERN_LENGTH] = '\0';
}

/**
 * @brief Cleans up the benchmark.
 */
static void benchmark_cleanup(void)
{
	nanvix_free(text);
}

/**
 * @brief Benchmark kernel.
 */
static void benchmark_kernel(void)
{
	perf_start(0, PERF_CYCLES);

		char *newtext = sed(text, pattern, newpattern);

	perf_stop(0);
	time_kernel = perf_read(0);

	nanvix_free(newtext);
}

/**
 * @brief Dump execution statistics.
 */
static void benchmark_dump_stats(void)
{
#ifndef NDEBUG
	uprintf("[benchmarks][sed] text %d, pattern %d, time %l",
#else
	uprintf("[benchmarks][sed] %d %d %l",
#endif
		__TEXT_LENGTH,
		__PATTERN_LENGTH,
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
