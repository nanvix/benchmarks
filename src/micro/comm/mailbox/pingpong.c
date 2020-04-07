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
static char message_in[KMAILBOX_MESSAGE_SIZE];
static char message_out[KMAILBOX_MESSAGE_SIZE];

static inline void do_master(const int * nodes, int nslaves)
{
	int local;
	int inbox;
	int outbox[PROCESSOR_NOC_NODES_NUM];

	local = nodes[0];

	umemset(message_out, local, KMAILBOX_MESSAGE_SIZE);

	for (unsigned int i = 1; i <= NITERATIONS; ++i)
	{
		uprintf("Iteration %d/%d", i, NITERATIONS);

		KASSERT((inbox = kmailbox_create(local, 0)) >= 0);

		/* Opens an outbox for each slave. */
		for (int j = 1; j <= nslaves; ++j)
			KASSERT((outbox[j] = kmailbox_open(nodes[j], 0)) >= 0);

		barrier_nodes();

		for (int j = 1; j <= nslaves; ++j)
		{
			umemset(message_in, local, KMAILBOX_MESSAGE_SIZE);

			KASSERT(kmailbox_read(inbox, message_in, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);

			int found = 0;
			for (int k = 1; k <= nslaves; ++k)
				found |= (message_in[0] == nodes[k]);
			KASSERT(found != 0);

			KASSERT(kmailbox_write(outbox[j], message_out, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);
		}

		/* Closes outboxes. */
		for (int j = 1; j <= nslaves; ++j)
			KASSERT(kmailbox_close(outbox[j]) == 0);

		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &result.latency) == 0);
		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_VOLUME, &result.volume) == 0);

		/* Header: "benchmark;routine;iteration;nodenum;latency;volume" */
		uprintf("mailbox;pingpong;%d;%d;%l;%l",
			i,
			local,
			result.latency,
			result.volume
		);

		KASSERT(kmailbox_unlink(inbox) == 0);
	}
}

static inline void do_slave(const int * nodes, int index)
{
	int local;
	int remote;
	int inbox;
	int outbox;

	local  = nodes[index];
	remote = nodes[0];

	umemset(message_out, local, KMAILBOX_MESSAGE_SIZE);

	for (unsigned int i = 1; i <= NITERATIONS; ++i)
	{
		KASSERT((inbox = kmailbox_create(local, 0)) >= 0);

		umemset(message_in, 0, KMAILBOX_MESSAGE_SIZE);

		barrier_nodes();

		KASSERT((outbox = kmailbox_open(remote, 0)) >= 0);

			KASSERT(kmailbox_write(outbox, message_out, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);
			KASSERT(kmailbox_read(inbox,   message_in,  KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);

			for (unsigned j = 0; j < KMAILBOX_MESSAGE_SIZE; ++j)
				KASSERT(message_in[j] == remote);
		
		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &result.latency) == 0);
		KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_VOLUME, &result.volume) == 0);

		/* Header: "benchmark;routine;iteration;nodenum;latency;volume" */
		uprintf("mailbox;pingpong;%d;%d;%l;%l",
			i,
			local,
			result.latency,
			result.volume
		);

		KASSERT(kmailbox_close(outbox) == 0);
		KASSERT(kmailbox_unlink(inbox) == 0);
	}
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Ping-Pong Routine
 */
int do_pingpong(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(message_size);

	if (index == 0)
		uprintf("[mailbox][pingpong] Init.");

	/* Runs the kernel. */
	if (index == 0)
		do_master(nodes, (nnodes - 1));
	else
		do_slave(nodes, index);

	if (index == 0)
		uprintf("[mailbox][pingpong] Finished.");

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[mailbox][pingpong] Successfuly completed.");

	return (0);
}

#else

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Ping-Pong Routine
 */
int do_pingpong(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(index);
	UNUSED(message_size);

	return (0);
}

#endif /* __TARGET_HAS_MAILBOX */
