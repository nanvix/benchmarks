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

PRIVATE struct
{
	/* Timing statistics. */
	uint64_t communication; /* Time spent on communication. */
	uint64_t total;         /* Total time.                  */

	/* Data exchange statistics. */
	size_t data_sent;     /* Number of bytes received. */
	unsigned nsend;       /* Number of sends.          */
	size_t data_received; /* Number of bytes sent.     */
	unsigned nreceive;    /* Number of receives.       */
} _statistics[MPI_PROCS_PER_CLUSTER_MAX] = {
	[0 ... (MPI_PROCS_PER_CLUSTER_MAX - 1)] = {0, 0, 0, 0, 0, 0}
};

/*============================================================================*
 * Statistics                                                                 *
 *============================================================================*/

/**
 * @brief Retrieves total time.
 */
PUBLIC uint64_t total()
{
	return (_statistics[curr_mpi_proc_index()].total);
}

/**
 * @brief Updates total time.
 */
PUBLIC void update_total(uint64_t amnt)
{
	_statistics[curr_mpi_proc_index()].total += amnt;
}

/**
 * @brief Retrieves communication time.
 */
PUBLIC uint64_t communication()
{
	return (_statistics[curr_mpi_proc_index()].communication);
}

/**
 * @brief Updates communication time.
 */
PUBLIC void update_communication(uint64_t amnt)
{
	_statistics[curr_mpi_proc_index()].communication += amnt;
}

/**
 * @brief Retrieves amount of data sent.
 */
PUBLIC size_t data_sent()
{
	return (_statistics[curr_mpi_proc_index()].data_sent);
}

/**
 * @brief Retrieves amount of data received.
 */
PUBLIC size_t data_received()
{
	return (_statistics[curr_mpi_proc_index()].data_received);
}

/**
 * @brief Retrieves number of sends performed.
 */
PUBLIC unsigned nsend()
{
	return (_statistics[curr_mpi_proc_index()].nsend);
}

/**
 * @brief Retrieves number of receives performed.
 */
PUBLIC unsigned nreceive()
{
	return (_statistics[curr_mpi_proc_index()].nreceive);
}

/*============================================================================*
 * Communication                                                              *
 *============================================================================*/

/**
 * @brief Sends data.
 */
uint64_t data_send(int outfd, void *data, size_t n)
{
	int local_index;

	local_index = curr_mpi_proc_index();

	_statistics[local_index].nsend++;
	_statistics[local_index].data_sent += n;

	perf_start(0, PERF_CYCLES);
	uassert(MPI_Send(
			data,
			n,
			MPI_BYTE,
			outfd,
			0,
			MPI_COMM_WORLD
		) == 0);
	_statistics[local_index].communication += perf_read(0);

	return (0);
}

/**
 * @brief Receives data.
 */
uint64_t data_receive(int infd, void *data, size_t n)
{
	int local_index;

	local_index = curr_mpi_proc_index();

	_statistics[local_index].nreceive++;
	_statistics[local_index].data_received += n;

	perf_start(0, PERF_CYCLES);
	uassert(MPI_Recv(
			data,
			n,
			MPI_BYTE,
			infd,
			0,
			MPI_COMM_WORLD,
			MPI_STATUS_IGNORE
		) == 0);
	_statistics[local_index].communication += perf_read(0);

	return (0);
}
