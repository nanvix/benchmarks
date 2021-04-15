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

#if __NANVIX_USES_LWMPI

#include <mpi/datatype.h>

#else

#include <nanvix/sys/thread.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/ulib.h>
#include <nanvix/pm.h>

#endif

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
	return (_statistics[runtime_get_index()].total);
}

/**
 * @brief Updates total time.
 */
PUBLIC void update_total(uint64_t amnt)
{
	_statistics[runtime_get_index()].total += amnt;
}

/**
 * @brief Retrieves communication time.
 */
PUBLIC uint64_t communication()
{
	return (_statistics[runtime_get_index()].communication);
}

/**
 * @brief Updates communication time.
 */
PUBLIC void update_communication(uint64_t amnt)
{
	_statistics[runtime_get_index()].communication += amnt;
}

/**
 * @brief Retrieves amount of data sent.
 */
PUBLIC size_t data_sent()
{
	return (_statistics[runtime_get_index()].data_sent);
}

/**
 * @brief Retrieves amount of data received.
 */
PUBLIC size_t data_received()
{
	return (_statistics[runtime_get_index()].data_received);
}

/**
 * @brief Retrieves number of sends performed.
 */
PUBLIC unsigned nsend()
{
	return (_statistics[runtime_get_index()].nsend);
}

/**
 * @brief Retrieves number of receives performed.
 */
PUBLIC unsigned nreceive()
{
	return (_statistics[runtime_get_index()].nreceive);
}

/*============================================================================*
 * Communication                                                              *
 *============================================================================*/

#if !__NANVIX_USES_LWMPI

struct comm_info
{
	int target;
	int source;
	int mailbox_port;
	int portal_port;
	size_t size;
};

#endif

/**
 * @brief Sends data.
 */
PUBLIC uint64_t data_send(int outfd, void *data, size_t n)
{
	int local_index;

	local_index = runtime_get_index();

	_statistics[local_index].nsend++;
	_statistics[local_index].data_sent += n;

	perf_start(0, PERF_CYCLES);

#if __NANVIX_USES_LWMPI

	uassert(
		MPI_Send(
			data,
			n,
			MPI_BYTE,
			outfd,
			0,
			MPI_COMM_WORLD
		) == 0
	);

#else

	int outbox;
	int outportal;
	struct comm_info req;

	int local  = knode_get_num();
	int remote = node_from_rank(outfd);
	int port   = port_from_rank(outfd);

	/* Open channels. */
	uassert((outbox    = kmailbox_open(remote, port)) == 0);
	uassert((outportal = kportal_open(local, remote, port))  == 0);

	/* Config. communication. */
	req.target       = outfd;
	req.source       = local;
	req.mailbox_port = kmailbox_get_port(outbox);
	req.portal_port  = kportal_get_port(outportal);
	req.size         = n;

	/* Request a communication. */
	uassert(
		kmailbox_write(
			outbox,
			&req,
			sizeof(struct comm_info)
		) == sizeof(struct comm_info)
	);

	/* Send data. */
	uassert(kportal_write(outportal, data, n) == (ssize_t) n);

	/* Close channels. */
	uassert(kmailbox_close(outbox)   == 0);
	uassert(kportal_close(outportal) == 0);

#endif

	_statistics[local_index].communication += perf_read(0);

	return (0);
}

/**
 * @brief Receives data.
 */
PUBLIC uint64_t data_receive(int infd, void *data, size_t n)
{
	int local_index;

	local_index = runtime_get_index();

	_statistics[local_index].nreceive++;
	_statistics[local_index].data_received += n;

	perf_start(0, PERF_CYCLES);

#if __NANVIX_USES_LWMPI

	uassert(
		MPI_Recv(
			data,
			n,
			MPI_BYTE,
			infd,
			0,
			MPI_COMM_WORLD,
			MPI_STATUS_IGNORE
		) == 0
	);

#else

	int inbox;
	int inportal;
	struct comm_info req;

	/* Gets input channels. */
	inbox    = stdinbox_get();
	inportal = stdinportal_get();

	/* Receive a Request. */
	uassert(
		kmailbox_read(
			inbox,
			&req,
			sizeof(struct comm_info)
		) == sizeof(struct comm_info)
	);

	/* Assert communication size. */
	uassert(req.size   == n);
	uassert(req.target == infd);

	/* Read data. */
	uassert(
		kportal_allow(
			inportal,
			req.source,
			req.portal_port
		) == 0
	);
	uassert(kportal_read(inportal, data, n) == (ssize_t) n);

#endif

	_statistics[local_index].communication += perf_read(0);

	return (0);
}

