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

#include <nanvix/runtime/fence.h>
#include <nanvix/runtime/barrier.h>
#include "../all-to-all.h"

#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)
	static barrier_t _barrier;
#else
	static struct fence_t _fence;
#endif

/*
 * Computes the Greatest Common Divisor of two numbers.
 */
PUBLIC void all_barrier_setup(int rank)
{
#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)
	int nodes[MPI_NODES_NR];

	UNUSED(rank);
	KASSERT(MPI_NODES_NR == PROCESSES_NR);

	/* Initializes list of nodes for stdbarrier. */
	for (int i = 0; i < MPI_NODES_NR; ++i)
		nodes[i] = MPI_BASE_NODE + i;

	_barrier = barrier_create(nodes, MPI_NODES_NR);
#else
	if (rank == 0)
		fence_init(&_fence, PROCESSES_NR);
	else
	{
		int busy = 10000;
		while (--busy);
	}
#endif
}

PUBLIC void all_barrier(void)
{
#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)
	barrier_wait(_barrier);
#else
	fence(&_fence);
#endif
}

PUBLIC void all_barrier_destroy(int rank)
{
	UNUSED(rank);

#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)
	barrier_destroy(_barrier);
#endif
}
