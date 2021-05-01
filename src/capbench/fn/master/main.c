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

#include "../fn.h"

/**
 * @brief K-Means Clustering Benchmark
 */
int __main3(int argc, char **argv)
{
	int rank;

	/* Initialize runtime system. */
	if (runtime_init(argc, argv) != 0)
	{
		uprintf("[capbench] Unused node.");
		return (0);
	}

	/* Get rank. */
	runtime_get_rank(&rank);

	/* Master process? */
	if (rank == 0)
	{
		uprintf("Master process executing...");

		do_master();

		/* Print timing statistics. */
		uprintf("---------------------------------------------");
		uprintf("[capbench][fn][%s] timing statistics:", RUNTIME_RULE);
		uprintf("[capbench][fn][%s]   master:         %l", RUNTIME_RULE, master);
		for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
		{
			uprintf("[capbench][fn][%s]   slave %s%d:       %l",
				RUNTIME_RULE, 
				(i < 10) ? " " : "",
				i, slave[i]);
		}
		uprintf("[capbench][fn][%s]   communication:  %l", RUNTIME_RULE, communication());
		uprintf("[capbench][fn][%s]   total time:     %l", RUNTIME_RULE, total());
		uprintf("[capbench][fn][%s] data exchange statistics:", RUNTIME_RULE);
		uprintf("[capbench][fn][%s]   data sent:            %d", RUNTIME_RULE, (int)data_sent());
		uprintf("[capbench][fn][%s]   number sends:         %d", RUNTIME_RULE, nsend());
		uprintf("[capbench][fn][%s]   data received:        %d", RUNTIME_RULE, (int)data_received());
		uprintf("[capbench][fn][%s]   number receives:      %d", RUNTIME_RULE, nreceive());
		uprintf("---------------------------------------------");
	}
	else if ((rank % PROCS_PER_CLUSTER_MAX == 0) || (rank == 1))
	{
		uprintf("Submaster of cluster %d executing with rank %d...", cluster_get_num(), rank);

		do_submaster();

		uprintf("Submaster %d done...", rank);
	}
	else
	{
		uprintf("Slave process %d executing...", rank);

		do_slave();

		uprintf("Slave process %d done...", rank);
	}

	/* Shutdown runtime system. */
	runtime_finalize();

	return (0);
}

