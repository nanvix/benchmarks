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

#include "../km.h"

/**
 * @brief K-Means Clustering Benchmark
 */
int __main3(int argc, char **argv)
{
	int rank;

	/* Initialize MPI runtime system. */
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/* Master process? */
	if (rank == 0)
	{
		uprintf("Master process executing...");

		do_master();

		/* Print timing statistics. */
		uprintf("---------------------------------------------");
		uprintf("[capbench][km] timing statistics:");
		uprintf("[capbench][km]   master:         %l", master);
		for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
		{
			uprintf("[capbench][km]   slave %s%d:       %l",
				(i < 10) ? " " : "",
				i, slave[i]);
		}
		uprintf("[capbench][km]   communication:  %l", communication());
		uprintf("[capbench][km]   total time:     %l", total());
		uprintf("[capbench][km] data exchange statistics:");
		uprintf("[capbench][km]   data sent:            %d", (int)data_sent());
		uprintf("[capbench][km]   number sends:         %d", nsend());
		uprintf("[capbench][km]   data received:        %d", (int)data_received());
		uprintf("[capbench][km]   number receives:      %d", nreceive());
		uprintf("---------------------------------------------");
	}
	else
	{
		uprintf("Slave process %d executing...", rank);

		do_slave();

		uprintf("Slave process %d done...", rank);
	}

	/* Shutdown MPI runtime system. */
	MPI_Finalize();

	return (0);
}
