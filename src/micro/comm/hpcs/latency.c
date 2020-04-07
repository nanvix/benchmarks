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

#include "parameters.h"

#if (__TARGET_HAS_MAILBOX && __DO_LATENCY)

static struct benchmark_result result;
static char message[KMAILBOX_MSG_SIZE];

static inline void do_write(int local, int remote)
{
	int outbox;
	uint64_t mailbox_size;

	mailbox_size = KMAILBOX_MSG_SIZE;

#ifndef NDEBUG
	umemset(message, 1, KMAILBOX_MSG_SIZE);
#endif

	for (unsigned i = 1; i <= NITERATIONS; ++i)
	{
		if (local == 0)
			uprintf("[latency] Iteration %d/%d", i, NITERATIONS);

		/* Open connector. */
		KASSERT((outbox = kmailbox_open(remote)) >= 0);

			/* Receptor needs to wait for your relative mailbox to be opened. */
			barrier_nodes();

			/* Sends the message for current slave. */
			KASSERT(kmailbox_write(outbox, message, KMAILBOX_MSG_SIZE) == KMAILBOX_MSG_SIZE);

			KASSERT(kmailbox_ioctl(outbox, MAILBOX_IOCTL_GET_LATENCY, &result.latency) == 0);
			KASSERT(kmailbox_ioctl(outbox, MAILBOX_IOCTL_GET_VOLUME, &result.volume) == 0);

#ifdef NDEBUG
		if (i >= SKIP)
#endif
		{
			/* Header: ("benchmark;cluster;type;buffersize;latency;volume" */
			if (local == 0)
				uprintf("mailbox;iocluster;write;%l;%l;%l", mailbox_size, result.latency, result.volume);
			else
				uprintf("mailbox;ccluster;write;%l;%l;%l", mailbox_size, result.latency, result.volume);
		}

		/* Closes connector. */
		KASSERT(kmailbox_close(outbox) == 0);
	}
}

static inline void do_read(int local)
{
	int inbox;
	uint64_t mailbox_size;
	
	mailbox_size = KMAILBOX_MSG_SIZE;

	for (unsigned i = 1; i <= NITERATIONS; ++i)
	{
		if (local == 0)
			uprintf("[latency] Iteration %d/%d", i, NITERATIONS);

		KASSERT((inbox = kmailbox_create(local)) >= 0);

#ifndef NDEBUG
		umemset(message, 0, KMAILBOX_MSG_SIZE);
#endif

			/**
			 * Receptor needs to wait for your relative mailbox to be opened because
			 * the emitter can lose your allow signal if it does it before the opened.
			 */
			barrier_nodes();

			/* Receives the message. */
			KASSERT(kmailbox_read(inbox, message, KMAILBOX_MSG_SIZE) == KMAILBOX_MSG_SIZE);

#ifndef NDEBUG
			for (int k = 0; k < KMAILBOX_MSG_SIZE; ++k)
				KASSERT(message[k] == 1);
#endif

			KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &result.latency) == 0);
			KASSERT(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_VOLUME, &result.volume) == 0);

#ifdef NDEBUG
		if (i >= SKIP)
#endif
		{
			/* Header: ("benchmark;cluster;type;buffersize;latency;volume" */
			if (local == 0)
				uprintf("mailbox;iocluster;read;%l;%l;%l", mailbox_size, result.latency, result.volume);
			else
				uprintf("mailbox;ccluster;read;%l;%l;%l", mailbox_size, result.latency, result.volume);
		}

		KASSERT(kmailbox_unlink(inbox) == 0);
	}
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Broadcast Routine
 */
int do_latency(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nnodes);
	UNUSED(message_size);

	if (index == 0)
		uprintf("[latency] Init MAILBOX SIZE %d.", KMAILBOX_MSG_SIZE);

	/* Runs the kernel. */
	if (index == 0)
	{
		uprintf("[latency] IO -> CC");
		do_write(nodes[0], nodes[1]);
		
		uprintf("[latency] IO <- CC");
		do_read(nodes[0]);
	}
	else
	{
		do_read(nodes[1]);
		do_write(nodes[1], nodes[0]);
	}

	if (index == 0)
		uprintf("[latency] Finished.");

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[latency] Successfuly completed.");

	return (0);
}

#else

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Broadcast Routine
 */
int do_latency(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nnodes);

	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(index);
	UNUSED(message_size);

	return (0);
}

#endif /* __TARGET_HAS_MAILBOX && __DO_LATENCY */
