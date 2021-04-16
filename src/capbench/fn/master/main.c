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
		uprintf("[capbench][fn] timing statistics:");
		uprintf("[capbench][fn]   master:         %l", master);
		for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
		{
			uprintf("[capbench][fn]   slave %s%d:       %l",
				(i < 10) ? " " : "",
				i, slave[i]);
		}
		uprintf("[capbench][fn]   communication:  %l", communication());
		uprintf("[capbench][fn]   total time:     %l", total());
		uprintf("[capbench][fn] data exchange statistics:");
		uprintf("[capbench][fn]   data sent:            %d", (int)data_sent());
		uprintf("[capbench][fn]   number sends:         %d", nsend());
		uprintf("[capbench][fn]   data received:        %d", (int)data_received());
		uprintf("[capbench][fn]   number receives:      %d", nreceive());
		uprintf("---------------------------------------------");
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

