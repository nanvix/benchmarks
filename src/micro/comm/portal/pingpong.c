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
static char message_in[MAX_MESSAGE_SIZE];
static char message_out[MAX_MESSAGE_SIZE];

static inline void do_master(const int * nodes, int nslaves, int message_size)
{
	int local;
	int portal_in;
	int portal_out;

	local = nodes[0];

	umemset(message_out, local, message_size);

	for (unsigned int i = 1; i <= NITERATIONS; ++i)
	{
		uprintf("Iteration %d/%d", i, NITERATIONS);

		KASSERT((portal_in = kportal_create(local, 0)) >= 0);

			for (int j = 1; j <= nslaves; ++j)
			{
				/* One communication with a slave at a time. */
				barrier_nodes();

				umemset(message_in, local, message_size);

				KASSERT((portal_out = kportal_open(local, nodes[j], 0)) >= 0);

					KASSERT(kportal_allow(portal_in, nodes[j], 0) == 0);
					KASSERT(kportal_read(portal_in, message_in, message_size) == message_size);

					for (int k = 0; k < message_size; k++)
						KASSERT(message_in[k] == nodes[j]);

					KASSERT(kportal_write(portal_out, message_out, message_size) >= message_size);

				KASSERT(kportal_close(portal_out) == 0);
			}

		KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &result.latency) == 0);
		KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &result.volume) == 0);

		/* Header: "benchmark;routine;iteration;nodenum;latency;volume" */
		uprintf("portal;pingpong;%d;%d;%l;%l",
			i,
			local,
			result.latency,
			result.volume
		);

		KASSERT(kportal_unlink(portal_in) == 0);
	}
}

static inline void do_slave(const int * nodes, int nslaves, int index, int message_size)
{
	int local;
	int remote;
	int portal_in;
	int portal_out;

	local  = nodes[index];
	remote = nodes[0];

	umemset(message_out, local, message_size);

	for (unsigned int i = 1; i <= NITERATIONS; ++i)
	{
		KASSERT((portal_in = kportal_create(local, 0)) >= 0);
		KASSERT((portal_out = kportal_open(local, remote, 0)) >= 0);

			umemset(message_in, local, message_size);

			for (int j = 1; j <= nslaves; ++j)
			{
				barrier_nodes();

				/* It is my tyme? */
				if (index != j)
					continue;

				KASSERT(kportal_write(portal_out, message_out, message_size) >= message_size);

				KASSERT(kportal_allow(portal_in, remote, 0) == 0);
				KASSERT(kportal_read(portal_in, message_in, message_size) == message_size);

				for (int k = 0; k < message_size; k++)
					KASSERT(message_in[k] == remote);
			}

		KASSERT(kportal_close(portal_out) == 0);

		KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &result.latency) == 0);
		KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &result.volume) == 0);

		/* Header: "benchmark;routine;iteration;nodenum;latency;volume" */
		uprintf("portal;pingpong;%d;%d;%l;%l",
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
 * @brief Performance Ping-Pong Routine
 */
int do_pingpong(const int * nodes, int nnodes, int index, int message_size)
{
	if (index == 0)
		uprintf("[portal][pingpong] Init.");

	/* Runs the kernel. */
	if (index == 0)
		do_master(nodes, (nnodes - 1), message_size);
	else
		do_slave(nodes, (nnodes - 1), index, message_size);

	if (index == 0)
		uprintf("[portal][pingpong] Finished.");

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[portal][pingpong] Successfuly completed.");

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

#endif /* __TARGET_HAS_PORTAL */
