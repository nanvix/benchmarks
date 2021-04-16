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

#if !__NANVIX_USES_LWMPI

#define __NEED_RESOURCE
#include <nanvix/hal/resource.h>

#endif

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

#define COMM_INFO_MAX 256

PRIVATE struct request 
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource; /**< Generic resource information. */

	/**
	 * Information.
	 */
	struct info
	{
		int source_rank;
		int target_rank;
		int source_node;
		int mailbox_port;
		int portal_port;
		size_t size;
	} info;

} requests[COMM_INFO_MAX];

PRIVATE struct resource_arrangement free_requests;
PRIVATE struct resource_arrangement buffered_requests;
PRIVATE spinlock_t ipc_lock;

/**
 * @brief Init request queue.
 */
PUBLIC void data_init(void)
{
	free_requests     = RESOURCE_ARRANGEMENT_INITIALIZER;
	buffered_requests = RESOURCE_ARRANGEMENT_INITIALIZER;

	for (int i = 0; i < COMM_INFO_MAX; ++i)
	{
		requests[i].resource = RESOURCE_INITIALIZER;
		resource_enqueue(&free_requests, &requests[i].resource);
	}

	spinlock_init(&ipc_lock);
}

/**
 * @brief Wanted source rank.
 */
PRIVATE int wanted_source_rank;

/**
 * @brief Verify if a request contains the wanted source rank.
 */
PRIVATE bool ipc_verify(struct resource * r)
{
	struct request * request = (struct request *) r;

	return (request->info.source_rank == wanted_source_rank);
}

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
	struct request req;

	int local  = knode_get_num();
	int remote = node_from_rank(outfd);
	int port   = port_from_rank(outfd);

	/* Open channels. */
	uassert((outbox    = kmailbox_open(remote, port)) >= 0);
	uassert((outportal = kportal_open(local, remote, port))  >= 0);

	/* Config. communication. */
	req.info.target_rank  = outfd;
	runtime_get_rank(&req.info.source_rank);
	req.info.source_node  = local;
	req.info.mailbox_port = kmailbox_get_port(outbox);
	req.info.portal_port  = kportal_get_port(outportal);
	req.info.size         = n;

#if DEBUG
	uprintf("[process %d] Send message (out:%d): source_rank:%d, target_rank:%d, source_node:%d, mailbox:%d, portal:%d | remote:%d, port:%d",
		req.info.source_rank,
		outfd,
		req.info.source_rank,
		req.info.target_rank,
		req.info.source_node,
		req.info.mailbox_port,
		req.info.portal_port,
		remote,
		port
	);
#endif

	/* Request a communication. */
	uassert(
		kmailbox_write(
			outbox,
			&req.info,
			sizeof(struct info)
		) == sizeof(struct info)
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

	int rank;
	int inbox;
	int inportal;
	struct request * req;

	runtime_get_rank(&rank);

	/* Gets input channels. */
	inbox    = stdinbox_get();
	inportal = stdinportal_get();

	req = NULL;

	do
	{
		spinlock_lock(&ipc_lock);

			/* Didn't the previous read the expected request? */
			if (req != NULL)
			{
				uassert(req->resource.next == NULL);
				resource_enqueue(&buffered_requests, &req->resource);
			}

			/* Does another process read this? */
			wanted_source_rank = infd;
			req = (struct request *) resource_remove_verify(&buffered_requests, ipc_verify);

			/* Not found? Alloc a new one. */
			if (req == NULL)
			{
				uassert((req = (struct request *) resource_dequeue(&free_requests)) != NULL);
				req->info.source_rank = -1;
				uassert(req->resource.next == NULL);
			}
			else
				uassert(req->resource.next == NULL);

		spinlock_unlock(&ipc_lock);

		if (req->info.source_rank == infd)
			break;

		/* Receive a Request. */
		uassert(
			kmailbox_read(
				inbox,
				&req->info,
				sizeof(struct info)
			) == sizeof(struct info)
		);

	} while (req->info.source_rank != infd);

#if DEBUG
	uprintf("[process %d] Read message (in:%d): source_rank:%d, target_rank:%d, mailbox:%d, portal:%d",
		rank,
		infd,
		req->info.source_rank,
		req->info.target_rank,
		req->info.mailbox_port,
		req->info.portal_port
	);
#endif

	/* Assert communication size. */
	uassert(req->info.size        == n);
	uassert(req->info.source_rank == infd);
	uassert(req->info.target_rank == rank);

	/* Read data. */
	uassert(
		kportal_allow(
			inportal,
			req->info.source_node,
			req->info.portal_port
		) == 0
	);
	uassert(kportal_read(inportal, data, n) == (ssize_t) n);

	spinlock_lock(&ipc_lock);
		req->info.source_rank = -1;
		uassert(req->resource.next == NULL);
		resource_enqueue(&free_requests, &req->resource);
	spinlock_unlock(&ipc_lock);

#endif

	_statistics[local_index].communication += perf_read(0);

	return (0);
}

