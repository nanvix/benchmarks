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

#include <nanvix/sys/perf.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/portal.h>
#include <nanvix/sys/mailbox.h>

#include "parameters.h"

#if (__TARGET_HAS_PORTAL && __TARGET_HAS_MAILBOX && __DO_COMM_SATURATION)

// static struct benchmark_result result;
static char message[HPCS_MAX_MSG_SIZE];

#define ACK_OP          0
#define ERROR_OP        1
#define REMOTE_READ_OP  2
#define REMOTE_WRITE_OP 3

/**
 * @brief Control message
 */
struct control_message
{
	int op;           /**< Operation type  */
	int targetnum;    /**< Target nodenum. */
	uint64_t size;    /**< Message size.   */
};

#define CONTROL_MSG_SIZE (sizeof(struct control_message))

static struct control_message request;
static struct control_message response;

static uint64_t error;

static inline void do_server(int local, int nclients)
{
	int inbox;
	int outbox;
	int portalid;

	response.op        = ACK_OP;
	response.targetnum = local;

	for (unsigned i = 1; i <= NITERATIONS; ++i)
	{
		uprintf("[saturation] Iteration %d/%d", i, NITERATIONS);

		/* Create incoming mailbox. */
		KASSERT((inbox = kmailbox_create(local)) >= 0);

		barrier_nodes();

		for (int j = 0; j < nclients; ++j)
		{
			/* Receive a request. */
			KASSERT(kmailbox_read(inbox, &request, CONTROL_MSG_SIZE) == CONTROL_MSG_SIZE);

			/* Prepare communication. */
			if (request.op == REMOTE_READ_OP)
			{
				/* Open connector. */
				KASSERT((portalid = kportal_open(local, request.targetnum, 0)) >= 0);
#ifndef NDEBUG
				umemset(message, 1, request.size);
#endif
			}
			else
			{
				/* Open connector. */
				KASSERT((portalid = kportal_create(local)) >= 0);
#ifndef NDEBUG
				umemset(message, 0, request.size);
#endif
			}

			response.size = request.size;

			/* Send request ack. */
			KASSERT((outbox = kmailbox_open(request.targetnum)) >= 0);
				KASSERT(kmailbox_write(outbox, &response, CONTROL_MSG_SIZE) == CONTROL_MSG_SIZE);
			KASSERT(kmailbox_close(outbox) == 0);

			/* Do the communication. */
			if (request.op == REMOTE_READ_OP)
			{
				for (uint64_t sent = 0; sent < request.size; sent += 8*KB)
				{
					uint64_t size = (sent + (8*KB)) < request.size ? (8*KB) : (request.size - sent);

					/* Sends the message for current slave. */
					KASSERT(kportal_write(portalid, message, size) == (int) size);
				}

				/* Closes connector. */
				KASSERT(kportal_close(portalid) == 0);
			}
			else
			{
				for (uint64_t received = 0; received < request.size; received += 8*KB)
				{
					uint64_t size = (received + (8*KB)) < request.size ? (8*KB) : (request.size - received);

					/* Receives the message. */
					KASSERT(kportal_allow(portalid, request.targetnum, 0) == 0);
					KASSERT(kportal_read(portalid, &message[received], size) == (int) size);
				}

#ifndef NDEBUG
				for (uint64_t k = 0; k < request.size; ++k)
					KASSERT(message[k] == 2);
#endif
				
				/* Closes connector. */
				KASSERT(kportal_unlink(portalid) == 0);
			}
		}

		KASSERT(kmailbox_unlink(inbox) == 0);

#ifdef NDEBUG
		if (i >= SKIP)
#endif
		{
			struct saturation_result result;

			result_read(nclients, &result);

			/* Header: "benchmark;buffersize;opening;requesting;communication;closing" */
			uprintf("saturation;%l;%l;%l;%l;%l",
				result.message_size,
				result.opening,
				result.requesting,
				result.communication,
				result.closing
			);
		}
	}
}

static inline void do_remote_read(int local, int remote, uint64_t message_size)
{
	int inbox;
	int outbox;
	int portalid;
	uint64_t t0;
	uint64_t t1;
	struct saturation_result result;

	request.op          = REMOTE_READ_OP;
	request.targetnum   = local;
	request.size        = message_size;
	result.message_size = message_size;

	for (uint64_t i = 1; i <= NITERATIONS; ++i)
	{
		response.op   = -1;
		response.size = -1;

#ifndef NDEBUG
		umemset(message, 0, message_size);
#endif

		kclock(&t0);
			/* Opening channels. */
			KASSERT((inbox  = kmailbox_create(local)) >= 0);
			KASSERT((outbox = kmailbox_open(remote)) >= 0);
			KASSERT((portalid = kportal_create(local)) >= 0);
		kclock(&t1);
		result.opening = ((t1 - t0) - error);

		barrier_nodes();

			/* Send a request. */
			KASSERT(kmailbox_write(outbox, &request, CONTROL_MSG_SIZE) == CONTROL_MSG_SIZE);
			KASSERT(kmailbox_ioctl(outbox, MAILBOX_IOCTL_GET_LATENCY, &result.requesting) == 0);

			/* Wait the confirmation. */
			KASSERT(kmailbox_read(inbox, &response, CONTROL_MSG_SIZE) == CONTROL_MSG_SIZE);
			KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &t1) == 0);
		result.requesting += t1;

		KASSERT(response.op == ACK_OP);
		KASSERT(response.size == message_size);

			for (uint64_t received = 0; received < message_size; received += 8*KB)
			{
				uint64_t size = (received + (8*KB)) < message_size ? (8*KB) : (message_size - received);

				/* Receives the message. */
				KASSERT(kportal_allow(portalid, remote, 0) == 0);
				KASSERT(kportal_read(portalid, &message[received], size) == (int) size);
			}

#ifndef NDEBUG
			for (uint64_t k = 0; k < request.size; ++k)
				KASSERT(message[k] == 1);
#endif
		KASSERT(kportal_ioctl(portalid, KPORTAL_IOCTL_GET_LATENCY, &result.communication) == 0);

		kclock(&t0);
			/* Closing channels. */
			KASSERT(kportal_unlink(portalid) == 0);
			KASSERT(kmailbox_close(outbox) == 0);
			KASSERT(kmailbox_unlink(inbox) == 0);
		kclock(&t1);
		result.closing = ((t1 - t0) - error);

#ifdef NDEBUG
		if (i >= SKIP)
#endif
		{	
			result_write(&result);
		}
	}
}

static inline void do_remote_write(int local, int remote, uint64_t message_size)
{
	int inbox;
	int outbox;
	int portalid;
	uint64_t t0;
	uint64_t t1;
	struct saturation_result result;

	request.op          = REMOTE_WRITE_OP;
	request.targetnum   = local;
	request.size        = message_size;
	result.message_size = message_size;

#ifndef NDEBUG
	umemset(message, 2, message_size);
#endif

	for (uint64_t i = 1; i <= NITERATIONS; ++i)
	{
		response.op   = -1;
		response.size = -1;

		kclock(&t0);
			/* Opening channels. */
			KASSERT((inbox  = kmailbox_create(local)) >= 0);
			KASSERT((outbox = kmailbox_open(remote)) >= 0);
			KASSERT((portalid = kportal_open(local, remote, 0)) >= 0);
		kclock(&t1);
		result.opening = ((t1 - t0) - error);

		barrier_nodes();

			/* Send a request. */
			KASSERT(kmailbox_write(outbox, &request, CONTROL_MSG_SIZE) == CONTROL_MSG_SIZE);
			KASSERT(kmailbox_ioctl(outbox, MAILBOX_IOCTL_GET_LATENCY, &result.requesting) == 0);

			/* Wait the confirmation. */
			KASSERT(kmailbox_read(inbox, &response, CONTROL_MSG_SIZE) == CONTROL_MSG_SIZE);
			KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &t1) == 0);
		result.requesting += t1;

		KASSERT(response.op == ACK_OP);
		KASSERT(response.size == message_size);

			for (uint64_t sent = 0; sent < message_size; sent += 8*KB)
			{
				uint64_t size = (sent + (8*KB)) < message_size ? (8*KB) : (message_size - sent);

				/* Sends the message for current slave. */
				KASSERT(kportal_write(portalid, message, size) == (int) size);
			}
		KASSERT(kportal_ioctl(portalid, KPORTAL_IOCTL_GET_LATENCY, &result.communication) == 0);

		kclock(&t0);
			/* Closing channels. */
			KASSERT(kportal_close(portalid) == 0);
			KASSERT(kmailbox_close(outbox) == 0);
			KASSERT(kmailbox_unlink(inbox) == 0);
		kclock(&t1);
		result.closing = ((t1 - t0) - error);

		#ifdef NDEBUG
		if (i >= SKIP)
#endif
		{	
			result_write(&result);
		}
	}
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Gather Routine
 */
int do_comm_saturation(const int * nodes, int nnodes, int index, int message_size)
{
	uint64_t t0;
	uint64_t t1;

	UNUSED(nnodes);
	UNUSED(message_size);

	if (index == 0)
		uprintf("[saturation] Init.");

	/* Get the error on read the clock. */
	kclock(&t0);
	kclock(&t1);

	error = ((t1 - t0) - error);

	for (int i = 0; i < NUM_PORTAL_SIZE; ++i)
	{
		/* Runs the kernel. */
		if (index == 0)
			do_server(nodes[0], (nnodes - 1));
		else
		{
			if (index % 2 == 0)
				do_remote_read(nodes[index], nodes[0], portal_sizes[i]);
			else
				do_remote_write(nodes[index], nodes[0], portal_sizes[i]);
		}
	}

	if (index == 0)
		uprintf("[saturation] Finished.");

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[saturation] Successfuly completed.");

	return (0);
}

#else

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Gather Routine
 */
int do_comm_saturation(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(index);
	UNUSED(message_size);

	return (0);
}

#endif /* __TARGET_HAS_PORTAL && __TARGET_HAS_MAILBOX && defined(__DO_COMM_SATURATION) */
