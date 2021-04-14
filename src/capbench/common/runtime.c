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

#include "../common.h"

#include <nanvix/sys/thread.h>

/**
 * Initializes runtime.
 */
PUBLIC void runtime_init(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
}

/**
 * Finalize runtime.
 */
PUBLIC void runtime_finalize(void)
{
	MPI_Finalize();
}

/**
 * Gets process rank.
 */
PUBLIC void runtime_get_rank(int * rank)
{
	MPI_Comm_rank(MPI_COMM_WORLD, rank);
}

/**
 * Gets process index.
 */
PUBLIC int runtime_get_index(void)
{
	return (curr_mpi_proc_index());
}

