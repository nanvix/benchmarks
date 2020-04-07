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

#include "parameters.h"

#if (__TARGET_HAS_PORTAL && __DO_THROUGHPUT)

static struct benchmark_result result;
static char message[HPCS_MAX_MSG_SIZE];

static inline void do_write(int local, int remote, uint64_t message_size)
{
	int portal_out;

#ifndef NDEBUG
	umemset(message, 1, message_size);
#endif

	for (unsigned i = 1; i <= NITERATIONS; ++i)
	{
		if (local == 0)
			uprintf("[throughput] Iteration %d/%d", i, NITERATIONS);

		/* Open connector. */
		KASSERT((portal_out = kportal_open(local, remote, 0)) >= 0);

			/* Receptor needs to wait for your relative portal to be opened. */
			barrier_nodes();

			for (uint64_t sent = 0; sent < message_size; sent += 8*KB)
			{
				uint64_t size = (sent + (8*KB)) < message_size ? (8*KB) : (message_size - sent);

				/* Sends the message for current slave. */
				KASSERT(kportal_write(portal_out, message, size) == (int) size);
			}

			KASSERT(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_LATENCY, &result.latency) == 0);
			KASSERT(kportal_ioctl(portal_out, KPORTAL_IOCTL_GET_VOLUME, &result.volume) == 0);

#ifdef NDEBUG
		if (i >= SKIP)
#endif
		{
			// uprintf("benchmark;cluster;type;buffersize;latency;volume");
			if (local == 0)
				uprintf("portal;iocluster;write;%l;%l;%l", message_size, result.latency, result.volume);
			else
				uprintf("portal;ccluster;write;%l;%l;%l", message_size, result.latency, result.volume);
		}

		/* Closes connector. */
		KASSERT(kportal_close(portal_out) == 0);
	}
}

static inline void do_read(int local, int remote, uint64_t message_size)
{
	int portal_in;

	for (unsigned i = 1; i <= NITERATIONS; ++i)
	{
		if (local == 0)
			uprintf("[throughput] Iteration %d/%d", i, NITERATIONS);

		KASSERT((portal_in = kportal_create(local, 0)) >= 0);

#ifndef NDEBUG
		umemset(message, 0, message_size);
#endif
			/**
			 * Receptor needs to wait for your relative portal to be opened because
			 * the emitter can lose your allow signal if it does it before the opened.
			 */
			barrier_nodes();

			for (uint64_t received = 0; received < message_size; received += 8*KB)
			{
				uint64_t size = (received + (8*KB)) < message_size ? (8*KB) : (message_size - received);

				/* Receives the message. */
				KASSERT(kportal_allow(portal_in, remote, 0) == 0);
				KASSERT(kportal_read(portal_in, &message[received], size) == (int) size);
			}

#ifndef NDEBUG
			for (uint64_t k = 0; k < message_size; ++k)
				KASSERT(message[k] == 1);
#endif

			KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_LATENCY, &result.latency) == 0);
			KASSERT(kportal_ioctl(portal_in, KPORTAL_IOCTL_GET_VOLUME, &result.volume) == 0);

#ifdef NDEBUG
		if (i >= SKIP)
#endif
		{
			/* Header: "benchmark;cluster;type;buffersize;latency;volume" */
			if (local == 0)
				uprintf("portal;iocluster;read;%l;%l;%l", message_size, result.latency, result.volume);
			else
				uprintf("portal;ccluster;read;%l;%l;%l", message_size, result.latency, result.volume);
		}

		KASSERT(kportal_unlink(portal_in) == 0);
	}
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Broadcast Routine
 */
int do_throughput(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nnodes);
	UNUSED(message_size);

	if (index == 0)
		uprintf("[throughput] Init.");

	for (int i = 0; i < NUM_PORTAL_SIZE; ++i)
	{
		/* Runs the kernel. */
		if (index == 0)
		{
			uprintf("[throughput] IO -> CC");
			do_write(nodes[0], nodes[1], portal_sizes[i]);
			
			uprintf("[throughput] IO <- CC");
			do_read(nodes[0], nodes[1], portal_sizes[i]);
		}
		else
		{
			do_read(nodes[1], nodes[0], portal_sizes[i]);
			do_write(nodes[1], nodes[0], portal_sizes[i]);
		}
	}

	if (index == 0)
		uprintf("[throughput] Finished.");

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[throughput] Successfuly completed.");

	return (0);
}

#else

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Broadcast Routine
 */
int do_throughput(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nnodes);

	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(index);
	UNUSED(message_size);

	return (0);
}

#endif /* __TARGET_HAS_PORTAL && __DO_THROUGHPUT */
