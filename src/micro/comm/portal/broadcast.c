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
#include <nanvix/sys/portal.h>

#include "../comm.h"

#if __TARGET_HAS_PORTAL

static struct benchmark_result result;
static char message[MAX_MESSAGE_SIZE] ALIGN(sizeof(uint64_t));

static inline void do_master(const int * nodes, int nslaves, int message_size)
{
	int local;
	int portal_out[PROCESSOR_NOC_NODES_NUM];

	local = nodes[0];

	umemset(message, 1, message_size);

	for (unsigned int i = 1; i <= NITERATIONS; ++i)
	{
		uprintf("Iteration %d/%d", i, NITERATIONS);

		/* Open connector. */
		for (int j = 1; j <= nslaves; ++j)
			KASSERT((portal_out[j] = kportal_open(local, nodes[j], 0)) >= 0);
		
		/* Receptor needs to wait for your relative portal to be opened. */
		barrier_nodes();

		/* Sends the message for current slave. */
		for (int j = 1; j <= nslaves; ++j)
			KASSERT(kportal_write(portal_out[j], message, message_size) == message_size);

		for (int j = 1; j <= nslaves; ++j)	
			KASSERT(kportal_close(portal_out[j]) == 0);
	}
}

static inline void do_slave(const int * nodes, int index, int message_size)
{
	int local;
	int remote;
	int portal_in;

	local  = nodes[index];
	remote = nodes[0];

	for (unsigned int i = 1; i <= NITERATIONS; ++i)
	{
		KASSERT((portal_in = kportal_create(local, 0)) >= 0);

		umemset(message, 0, message_size);

		barrier_nodes();

		/* Receives the message. */
		KASSERT(kportal_allow(portal_in, remote, 0) == 0);
		KASSERT(kportal_read(portal_in, message, message_size) == message_size);

		for (int k = 0; k < message_size; ++k)
			KASSERT(message[k] == 1);

		KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &result.latency) == 0);
		KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &result.volume) == 0);

		/* Header: "benchmark;routine;iteration;nodenum;latency;volume" */
		uprintf("portal;broadcast;%d;%d;%l;%l",
			i,
			local,
			result.latency,
			result.volume
		);

		KASSERT(kportal_unlink(portal_in) == 0);
	}
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Broadcast Routine
 */
int do_broadcast(const int * nodes, int nnodes, int index, int message_size)
{
	if (index == 0)
		uprintf("[portal][broadcast] Init.");

	/* Runs the kernel. */
	if (index == 0)
		do_master(nodes, (nnodes - 1), message_size);
	else
		do_slave(nodes, index, message_size);

	if (index == 0)
		uprintf("[portal][broadcast] Finished.");

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[portal][broadcast] Successfuly completed.");

	return (0);
}

#else

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Broadcast Routine
 */
int do_broadcast(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(index);
	UNUSED(message_size);

	return (0);
}

#endif /* __TARGET_HAS_PORTAL */
