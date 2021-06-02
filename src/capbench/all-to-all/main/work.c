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

/* Job mapping. */
#define NEXT(_n) ((_n + 1) % PROCESSES_NR)

/* Data. */
PRIVATE char tasks[PROCESSES_NR][PROBLEM_SIZE_MAX];

PUBLIC void do_submaster(void){}

PUBLIC void do_slave(void){}

PUBLIC void do_work(int rank, size_t size)
{
	for (int it = 0; it < PROBLEM_ITERATIONS; ++it)
	{
		for (int i = 0; i < PROCESSES_NR; ++i)
		{
			/* Skip comm. with itself. */
			if (rank == i)
				continue;

			/* Odd send + receive. */
			if (rank < i)
			{
#if DEBUG
				uprintf("%d + rank %d -> %d...", i, rank, i);
#endif /* DEBUG */

				tasks[rank][0] = rank;
				data_send(i, &tasks[rank], size);

#if DEBUG
				uprintf("%d + rank %d <- %d...", i, rank, i);
#endif /* DEBUG */

				data_receive(i, &tasks[rank], size);
				KASSERT(tasks[rank][0] == i);
			}

			/* Even receive + send. */
			else
			{

#if DEBUG
				uprintf("%d - rank %d <- %d...", i, rank, i);
#endif /* DEBUG */

				data_receive(i, &tasks[rank], size);
				KASSERT(tasks[rank][0] == i);

#if DEBUG
				uprintf("%d - rank %d -> %d", i, rank, i);
#endif /* DEBUG */

				tasks[rank][0] = rank;
				data_send(i, &tasks[rank], size);
			}
		}
	}
}

