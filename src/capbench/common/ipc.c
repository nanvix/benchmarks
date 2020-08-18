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

#include <nanvix/sys/perf.h>

#include <posix/stdint.h>
#include <posix/stddef.h>

#include <mputil/proc.h>
#include <mpi/datatype.h>
#include <mpi.h>

int rank;        /* Process Rank        */
MPI_Group group; /* Communication Group */

/* Timing statistics. */
uint64_t master = 0;              /* Time spent on master.        */
uint64_t spawn = 0;               /* Time spent spawning slaves   */
uint64_t slave[MPI_PROCESSES_NR]; /* Time spent on slaves.        */
uint64_t communication = 0;       /* Time spent on communication. */
uint64_t total = 0;               /* Total time.                  */

/* Data exchange statistics. */
size_t data_sent = 0;     /* Number of bytes received. */
unsigned nsend = 0;       /* Number of sends.          */
size_t data_received = 0; /* Number of bytes sent.     */
unsigned nreceive = 0;    /* Number of receives.       */

/*
 * Sends data.
 */
uint64_t data_send(int outfd, void *data, size_t n)
{
	nsend++;
	data_sent += n;

	perf_start(0, PERF_CYCLES);
	MPI_Send(
		data,
		n,
		MPI_BYTE,
		outfd,
		0,
		MPI_COMM_WORLD
	);
	communication += perf_read(0);

	return (0);
}

/*
 * Receives data.
 */
uint64_t data_receive(int infd, void *data, size_t n)
{
	nreceive++;
	data_received += n;

	perf_start(0, PERF_CYCLES);
	MPI_Recv(
		data,
		n,
		MPI_BYTE,
		infd,
		0,
		MPI_COMM_WORLD,
		MPI_STATUS_IGNORE
	);
	communication += perf_read(0);

	return (0);
}
