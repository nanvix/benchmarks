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
#include <nanvix/sys/mailbox.h>

#include "../comm.h"

#if __TARGET_HAS_MAILBOX

static struct benchmark_result result;
static char message[KMAILBOX_MESSAGE_SIZE];

static inline int build_footprint(const int * nodes, int nnodes, int local)
{
	int footprint = 0;

	for (int i = 0; i < nnodes; ++i)
		if (nodes[i] != local)
			footprint |= (1ULL << nodes[i]);

	return (footprint);
}

static inline void do_work(const int * nodes, int nnodes, int index)
{
	int local;
	int inbox;
	int outbox[PROCESSOR_NOC_NODES_NUM];
	int target;
	int expected, received;

	local = nodes[index];
	expected = build_footprint(nodes, nnodes, local);

	for (unsigned int i = 1; i <= NITERATIONS; ++i)
	{
		if (index == 0)
			uprintf("Iteration %d/%d", i, NITERATIONS);

		KASSERT((inbox = kmailbox_create(local, 0)) >= 0);

		/* Opens all output mailboxes. */
		target = (index + 1) < nnodes ? (index + 1) : ((index + 1) < nnodes);
		for (int j = 0; j < (nnodes - 1); ++j)
		{
			KASSERT((outbox[j] = kmailbox_open(nodes[target], 0)) >= 0);

			/* Ring behaviour. (index+1, index+2, ..., index-1) */
			target = (target + 1) < nnodes ? (target + 1) : ((target + 1) - nnodes);
		}

		umemset(message, local, KMAILBOX_MESSAGE_SIZE);

		barrier_nodes();

		/* Sends n-1 messages. */
		for (int j = 0; j < (nnodes - 1); ++j)
			KASSERT(kmailbox_write(outbox[j], message, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);

		/* Receives n-1 messages. */
		received = 0;
		for (int j = 0; j < (nnodes - 1); ++j)
		{
			umemset(message, (-1), KMAILBOX_MESSAGE_SIZE);

			KASSERT(kmailbox_read(inbox, message, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);

			received |= (1 << message[0]);
		}
		KASSERT(expected == received);

		/* Closes all output mailboxes. */
		for (int j = 0; j < (nnodes - 1); ++j)
			KASSERT(kmailbox_close(outbox[j]) == 0);
		
		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &result.latency) == 0);
		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_VOLUME, &result.volume) == 0);

		/* Header: "benchmark;routine;iteration;nodenum;latency;volume" */
		uprintf("mailbox;allgather;%d;%d;%l;%l",
			i,
			local,
			result.latency,
			result.volume
		);

		KASSERT(kmailbox_unlink(inbox) == 0);
	}
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance AllGather Routine
 */
int do_allgather(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(message_size);

	if (index == 0)
		uprintf("[mailbox][allgather] Init.");

	/* Runs the kernel. */
	do_work(nodes, nnodes, index);

	if (index == 0)
		uprintf("[mailbox][allgather] Finished.");

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[mailbox][allgather] Successfuly completed.");

	return (0);
}

#else

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance AllGather Routine
 */
int do_allgather(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(index);
	UNUSED(message_size);

	return (0);
}

#endif /* __TARGET_HAS_MAILBOX */
