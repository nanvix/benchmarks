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

#include <nanvix/sys/noc.h>
#include <nanvix/sys/perf.h>

#include "comm.h"

#if __TARGET_HAS_PORTAL || __TARGET_HAS_MAILBOX

/**
 * @brief Horizontal line
 */
const char *HLINE =
	"------------------------------------------------------------------------";

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Fork-Join Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
int __main2(int argc, const char *argv[])
{
	int index;
	int nodenum;

	UNUSED(argc);
	UNUSED(argv);

	index   = -1;
	nodenum = knode_get_num();

	/* Finds the index of local node on nodenums vector. */
	for (int i = 0; i < NUM_NODES; ++i)
	{
		if (nodenum == nodenums[i])
		{
			index = i;
			break;
		}
	}

	if (nodenum == PROCESSOR_NODENUM_MASTER)
		uprintf(HLINE);

	/* Boot synchronize. */
	__stdsync_setup();

		if (nodenum == PROCESSOR_NODENUM_MASTER)
			uprintf("Fence.");

		/* Sync all clusters */
		stdsync_fence();

	__stdsync_cleanup();

	/* Common operations among routines. */
	/* Only involved nodes. */
	if (index >= 0)
	{
		/**
		 * Creates a barrier with all involved clusters.
		 * Note (valid only if multiplexation is not avaiable):
		 * the stdsync must be cleanup before call the barruer_setup
		 * because it uses the same local resources.
		 */
		barrier_nodes_setup(nodenums, NUM_NODES, (index == 0));

			/* Do the routine. */
			enabled_routine_fn(nodenums, NUM_NODES, index, MESSAGE_SIZE);

			/* Waits everyone finishes the routine. */
			barrier_nodes();

		/* Destroy barrier. */
		barrier_nodes_cleanup();
	}

	if (nodenum == PROCESSOR_NODENUM_MASTER)
		uprintf(HLINE);

	return (0);
}

#else

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Fork-Join Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
int __main2(int argc, const char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	return (0);
}
#endif