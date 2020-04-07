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
static char messages[MAX_NUM_NODES * MAX_MESSAGE_SIZE];

static inline void do_work(const int * nodes, int nnodes, int index, int message_size)
{
	int local;
	int read_from;
	int write_to;
	int portal_in;
	int portal_out;

	local = nodes[index];

	for (unsigned int i = 1; i <= NITERATIONS; ++i)
	{
		if (index == 0)
			uprintf("Iteration %d/%d", i, NITERATIONS);

		/* Cleans the buffer. */
		umemset(messages, (-1), nnodes * message_size);

		/* Prepares the transfer data. */
		umemset(&messages[index * message_size], (char) local, message_size);

		KASSERT((portal_in = kportal_create(local, 0)) >= 0);

			for (int j = 1; j < nnodes; ++j)
			{
				/**
				 * Algorithm behavior:
				 * communication occurs in distinct pairs of reading and writing.
				 * Note: Module operation has generated an exception, it must be avoided.
				 */
				read_from = (index - j) < 0 ? (nnodes + (index - j)) : (index - j);
				write_to  = (index + j) < nnodes ? (index + j) : (index + j) - nnodes; /* Equivalent: (index + j) % nnodes */

				KASSERT((portal_out = kportal_open(local, nodes[write_to], 0)) >= 0);

				barrier_nodes();

				/* Master */
				if (index == 0)
				{
					/* Sync write. */
					KASSERT(
						kportal_write(
							portal_out,
							&messages[index * message_size],
							message_size
						) == message_size
					);

					/* Sync read. */
					KASSERT(kportal_allow(portal_in, nodes[read_from], 0) == 0);
					KASSERT(
						kportal_read(
							portal_in,
							&messages[read_from * message_size],
							message_size
						) == message_size
					);
				}

				/* Slave */
				else
				{

					/* Sync read. */
					KASSERT(kportal_allow(portal_in, nodes[read_from], 0) == 0);
					KASSERT(
						kportal_read(
							portal_in,
							&messages[read_from * message_size],
							message_size
						) == message_size
					);

					/* Sync write. */
					KASSERT(
						kportal_write(
							portal_out,
							&messages[index * message_size],
							message_size
						) == message_size
					);
				}

				KASSERT(kportal_close(portal_out) == 0);
			}

			/* Check messages. */
			for (int j = 0; j < nnodes; ++j)
			{
				char value = (char) nodes[j];
				char * curr_message = &messages[(j * message_size)];

				for (int k = 0; k < message_size; ++k)
					KASSERT(curr_message[k] == value);
			}

		KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &result.latency) == 0);
		KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &result.volume) == 0);

		/* Header: "benchmark;routine;iteration;nodenum;latency;volume" */
		uprintf("portal;allgather;%d;%d;%l;%l",
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
 * @brief Performance AllGather Routine
 */
int do_allgather(const int * nodes, int nnodes, int index, int message_size)
{
	if (index == 0)
		uprintf("[portal][allgather] Init.");

	/* Runs the kernel. */
	do_work(nodes, nnodes, index, message_size);

	if (index == 0)
		uprintf("[portal][allgather] Finished.");

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[portal][allgather] Successfuly completed.");

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

#endif /* __TARGET_HAS_PORTAL */
