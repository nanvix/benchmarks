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

#include "../all-to-all.h"

/**
 * @brief K-Means Clustering Benchmark
 */
int __main3(int argc, char **argv)
{
	int rank;
	uint64_t t;

	/* Initialize runtime system. */
	if (runtime_init(argc, argv) != 0)
	{
		uprintf("[capbench] Unused node.");
		return (0);
	}

	/* Get rank. */
	runtime_get_rank(&rank);

	uprintf("PROCESSES_NR %d", PROCESSES_NR);

#if DEBUG
	uprintf("Process %d executing...", rank);
#endif

	all_barrier_setup(rank);

	if (rank == 0)
		uprintf("---------------------------------------------");

	for (size_t size = PROBLEM_SIZE_MIN; size <= PROBLEM_SIZE_MAX; size = PROBLEM_SIZE_STEP(size))
	{
		all_barrier();

		if (rank == 0)
			perf_start(0, PERF_CYCLES);

			do_work(rank, size);

		all_barrier();

		if (rank == 0)
		{
			t = perf_read(0);

			/* Print timing statistics. */
			uprintf("[capbench][all][%s][%s] %d %d %l",
				RUNTIME_RULE,
				RUNTIME_MODE,
				(int) PROBLEM_ITERATIONS,
				(int) size,
				t
			);
		}
	}

	if (rank == 0)
		uprintf("---------------------------------------------");

	all_barrier_destroy(rank);

#if DEBUG
	uprintf("Process %d done...", rank);
#endif

	/* Shutdown runtime system. */
	runtime_finalize();

	return (0);
}

