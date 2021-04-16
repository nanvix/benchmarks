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

#define COMM_INFO_MAX (2 * 256)

#define MATCH_INFO(data, _mode, _source, _target) \
	(data.mode == _mode && data.source_rank == _source && data.target_rank == _target)

#define INFO_MODE_REQUEST    1
#define INFO_MODE_PERMISSION 2

struct information 
{
	/*
	 * XXX: Don't Touch! This Must Come First!
	 */
	struct resource resource; /**< Generic resource information. */

	/**
	 * Information.
	 */
	struct data 
	{
		int mode;
		int source_rank;
		int target_rank;
		int source_node;
		int mailbox_port;
		int portal_port;
		size_t size;
	} data;
} infos[COMM_INFO_MAX];

PRIVATE struct resource_arrangement free_infos;
PRIVATE struct resource_arrangement buffered_infos;

PRIVATE spinlock_t ipc_lock;

/**
 * @brief Init request queue.
 */
PUBLIC void data_init(void)
{
	free_infos     = RESOURCE_ARRANGEMENT_INITIALIZER;
	buffered_infos = RESOURCE_ARRANGEMENT_INITIALIZER;

	for (int i = 0; i < COMM_INFO_MAX; ++i)
	{
		/* Populate free requests.    */
		infos[i].resource = RESOURCE_INITIALIZER;
		resource_enqueue(&free_infos, &infos[i].resource);
	}

	spinlock_init(&ipc_lock);
}

/**
 * @brief Wanted match.
 */
PRIVATE int verify_mode;
PRIVATE int verify_source;
PRIVATE int verify_target;

/**
 * @brief Verify if a request contains the wanted source rank.
 */
PRIVATE bool ipc_verify(struct resource * r)
{
	struct information * info = (struct information *) r;

	return (MATCH_INFO(info->data, verify_mode, verify_source, verify_target));
}

struct information * ipc_read_information(int source, int target, int mode)
{
	struct information * info;
	int inbox = stdinbox_get();

	info  = NULL;
	inbox = stdinbox_get();

	do
	{
		spinlock_lock(&ipc_lock);

			/* Didn't the previous read the expected information? */
			if (info != NULL)
				resource_enqueue(&buffered_infos, &info->resource);

			/* Does another process read this? */
			verify_source = source;
			verify_target = target;
			verify_mode   = mode;
			info = (struct information *) resource_remove_verify(&buffered_infos, ipc_verify);

			/* Not found? Alloc a new one. */
			if (info == NULL)
			{
				uassert((info = (struct information *) resource_dequeue(&free_infos)) != NULL);
				info->data.source_rank = -1;
			}

		spinlock_unlock(&ipc_lock);

		if (MATCH_INFO(info->data, mode, source, target))
			break;

		/* Receive a Request. */
		uassert(
			kmailbox_read(
				inbox,
				&info->data,
				sizeof(struct data)
			) == sizeof(struct data)
		);

	} while (!MATCH_INFO(info->data, mode, source, target));

	return (info);
}

void ipc_release_information(struct information * info)
{
	spinlock_lock(&ipc_lock);
		info->data.source_rank = -1;
		resource_enqueue(&free_infos, &info->resource);
	spinlock_unlock(&ipc_lock);
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

	int rank;
	int port;
	int local;
	int remote;
	int outbox;
	int outportal;
	struct information req;
	struct information * perm;

	runtime_get_rank(&rank);

	port   = port_from_rank(outfd);
	remote = node_from_rank(outfd);
	local  = knode_get_num();

#if DEBUG && DEBUG_IPC
		uprintf("\033[0%sm[process %d] P<- Waits a permission from process %d\033[0m",
			(rank == 0) ? ";31" : ((rank == 1) ? ";32" : ""),
			rank,
			outfd
		);
#endif

	/**
	 * Waits for a permission.
	 */
	perm = ipc_read_information(outfd, rank, INFO_MODE_PERMISSION);

		/* Assert permission information. */
		uassert(perm->data.source_rank == outfd);
		uassert(perm->data.target_rank == rank);
		uassert(perm->data.mode        == INFO_MODE_PERMISSION);

	ipc_release_information(perm);

	/* Open channels. */
	uassert((outbox    = kmailbox_open(remote, port)) >= 0);
	uassert((outportal = kportal_open(local, remote, port))  >= 0);

		/* Config. communication. */
		req.data.mode         = INFO_MODE_REQUEST;
		req.data.source_rank  = rank;
		req.data.target_rank  = outfd;
		req.data.source_node  = local;
		req.data.mailbox_port = kmailbox_get_port(outbox);
		req.data.portal_port  = kportal_get_port(outportal);
		req.data.size         = n;

#if DEBUG && DEBUG_IPC
		uprintf("\033[0%sm[process %d] ->R Send request to process %d: source_node:%d, mailbox:%d, portal:%d | remote:%d, port:%d\033[0m",
			(rank == 0) ? ";31" : ((rank == 1) ? ";32" : ""),
			req.data.source_rank,
			req.data.target_rank,
			req.data.source_node,
			req.data.mailbox_port,
			req.data.portal_port,
			remote,
			port
		);
#endif

		/* Request a communication. */
		uassert(
			kmailbox_write(
				outbox,
				&req.data,
				sizeof(struct data)
			) == sizeof(struct data)
		);

#if DEBUG && DEBUG_IPC
		uprintf("\033[0%sm[process %d] ->M Send message to process %d: source_node:%d, mailbox:%d, portal:%d | remote:%d, port:%d\033[0m",
			(rank == 0) ? ";31" : ((rank == 1) ? ";32" : ""),
			req.data.source_rank,
			req.data.target_rank,
			req.data.source_node,
			req.data.mailbox_port,
			req.data.portal_port,
			remote,
			port
		);
#endif

		/* Send data. */
		uassert(kportal_write(outportal, data, n) == (ssize_t) n);

	/* Close channels. */
	uassert(kmailbox_close(outbox)   == 0);
	uassert(kportal_close(outportal) == 0);

#if DEBUG && DEBUG_IPC
		uprintf("\033[0%sm[process %d] ->X Finish send from process %d\033[0m",
			(rank == 0) ? ";31" : ((rank == 1) ? ";32" : ""),
			rank,
			outfd
		);
#endif

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
	int port;
	int local;
	int remote;
	int inportal;
	int outbox;
	struct information perm;
	struct information * req;

	runtime_get_rank(&rank);

	/* Gets input channels. */
	local    = knode_get_num();
	remote   = node_from_rank(infd);
	port     = port_from_rank(infd);

	/* Open channels. */
	uassert((outbox = kmailbox_open(remote, port)) >= 0);

		/* Config. communication. */
		perm.data.mode         = INFO_MODE_PERMISSION;
		perm.data.source_rank  = rank;
		perm.data.target_rank  = infd;
		perm.data.source_node  = local;
		perm.data.mailbox_port = kmailbox_get_port(outbox);
		perm.data.portal_port  = -1;
		perm.data.size         = -1;

#if DEBUG && DEBUG_IPC
		uprintf("\033[0%sm[process %d] P-> Send a permission to process %d: source_node:%d, mailbox:%d, portal:%d | remote:%d, port:%d\033[0m",
			(rank == 0) ? ";31" : ((rank == 1) ? ";32" : ""),
			perm.data.source_rank,
			perm.data.target_rank,
			perm.data.source_node,
			perm.data.mailbox_port,
			perm.data.portal_port,
			remote,
			port
		);
#endif

		/* Request a communication. */
		uassert(
			kmailbox_write(
				outbox,
				&perm.data,
				sizeof(struct data)
			) == sizeof(struct data)
		);

	/* Close channels. */
	uassert(kmailbox_close(outbox) == 0);

#if DEBUG && DEBUG_IPC
	if (rank >= 0)
		uprintf("\33[0%sm[process %d] R<- Waits a request from process %d\33[0m",
			(rank == 0) ? ";31" : ((rank == 1) ? ";32" : ""),
			rank,
			infd
		);
#endif

	/**
	 * Waits for a request.
	 */
	req = ipc_read_information(infd, rank, INFO_MODE_REQUEST);

#if DEBUG && DEBUG_IPC
		uprintf("\033[0%sm[process %d] M<- Read message from process %d: mailbox:%d, portal:%d\033[0m",
			(rank == 0) ? ";31" : ((rank == 1) ? ";32" : ""),
			req->data.target_rank,
			req->data.source_rank,
			req->data.mailbox_port,
			req->data.portal_port
		);
#endif

		/* Assert communication size. */
		uassert(req->data.size        == n);
		uassert(req->data.source_rank == infd);
		uassert(req->data.target_rank == rank);
		uassert(req->data.mode        == INFO_MODE_REQUEST);

		inportal = stdinportal_get();

		/* Read data. */
		uassert(
			kportal_allow(
				inportal,
				req->data.source_node,
				req->data.portal_port
			) == 0
		);
		uassert(kportal_read(inportal, data, n) == (ssize_t) n);

	ipc_release_information(req);

#if DEBUG && DEBUG_IPC
		uprintf("\033[0%sm[process %d] X<- Finish read from process %d\033[0m",
			(rank == 0) ? ";31" : ((rank == 1) ? ";32" : ""),
			rank,
			infd
		);
#endif

#endif

	_statistics[local_index].communication += perf_read(0);

	return (0);
}

