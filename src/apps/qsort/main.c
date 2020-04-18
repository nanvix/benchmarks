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

#include <nanvix/runtime/rmem.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>
#include <posix/stddef.h>

/**
 * @brief Number of numbers.
 */
#define NUM_NUMBERS 65536

/* Import definitions. */
extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);
extern void nanvix_qsort(void *, size_t, size_t, int (*)(const void *, const void *));

/**
 * @brief Syntetic Benchmark
 */
int __main2(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	unsigned *numbers;
	uint64_t time_alloc, time_sort, time_free;

	__runtime_setup(0);

		/* Unblock spawner. */
		uassert(stdsync_fence() == 0);
		uprintf("[nanvix][benchmark] server alive");

		__runtime_setup(3);

		/* Allocate many blocks.*/
#ifndef NDEBUG
		uprintf("[apps][qsort] allocating numbers...");
#endif
		perf_start(0, PERF_CYCLES);
			uassert((numbers = nanvix_malloc(NUM_NUMBERS*sizeof(unsigned))) != NULL);
			for (unsigned i = 0; i < NUM_NUMBERS; i++)
				numbers[i] = NUM_NUMBERS-1;
		perf_stop(0);
		time_alloc = perf_read(0);

		/* Sort. */
#ifndef NDEBUG
		uprintf("[apps][qsort] sorting numbers...");
#endif
		perf_start(0, PERF_CYCLES);
			umemset(numbers, 1, NUM_NUMBERS*sizeof(unsigned));
		perf_stop(0);
		time_sort = perf_read(0);

		/* Free all blocks. */
#ifndef NDEBUG
		uprintf("[apps][qsort] freeing numbers...");
#endif
		perf_start(0, PERF_CYCLES);
			nanvix_free(numbers);
		perf_stop(0);
		time_free = perf_read(0);


#ifndef NDEBUG
		uprintf("[apps][qsort] alloc %l sort %l free %l",
#else
		uprintf("[apps][qsort] %l %l %l",
#endif
			time_alloc,
			time_sort,
			time_free
		);

		nanvix_shutdown();

	__runtime_cleanup();

	return (0);
}
