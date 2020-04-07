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

#include <nanvix/sys/thread.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/portal.h>
#include <nanvix/sys/mailbox.h>

#include "parameters.h"

#if __TARGET_HAS_PORTAL && __TARGET_HAS_MAILBOX

/**
 * @brief Number of running threads.
 */
static int _nthreads = 0;

/**
 * @brief Communicates the remote port through the mailbox.
 */
struct box_message
{
	int remote_port;
};

/**
 * @brief Auxiliar macro to get the size of a box message.
 */
#define BOX_MSG_SIZE (sizeof(struct box_message))

/**
 * @brief Stores the results.
 */
static struct benchmark_result results[NTHREADS_MAX];

static inline void * do_master(void * _tid)
{
	int tid;
	int local;
	int remote;
	int inportal[(NTHREADS_MAX / 3) + 1];
	int outportal[(NTHREADS_MAX / 3) + 1];
	int inbox[(NTHREADS_MAX / 3) + 1];
	int outbox[(NTHREADS_MAX / 3) + 1];
	int ports[(NTHREADS_MAX / 3) + 1];
	int nports;
	char message[MAX_MESSAGE_SIZE];
	struct box_message box_msg;

	tid = (int) ((intptr_t) _tid);
	local  = nodenums[0];
	remote = nodenums[1];

	barrier_cores_setup(tid, (_nthreads < 3 ? _nthreads : 3));

	for (unsigned int iter = 1; iter <= NITERATIONS; ++iter)
	{
		if (tid == 0)
			uprintf("Iteration %d/%d", iter, NITERATIONS);

		nports = 0;
		for (int port = tid; port < _nthreads; port += 3)
		{
			ports[nports] = port;

			/* OBS.: Use the tid to specify the port. */
			KASSERT((inportal[nports] = kportal_create(local, port)) >= 0);
			KASSERT((outportal[nports] = kportal_open(local, remote, port)) >= 0);

			KASSERT((inbox[nports] = kmailbox_create(local, port)) >= 0);
			KASSERT((outbox[nports] = kmailbox_open(remote, port)) >= 0);

			nports++;
		}

		barrier_cores();

		for (int i = 0; i < nports; ++i)
		{
			KASSERT(kmailbox_read(inbox[i], (void *) &box_msg, BOX_MSG_SIZE) == BOX_MSG_SIZE);

			KASSERT(kportal_allow(inportal[i], remote, box_msg.remote_port) == 0);
			KASSERT(kportal_read(inportal[i], message, MESSAGE_SIZE) == MESSAGE_SIZE);

			for (int k = 0; k < MESSAGE_SIZE; k++)
				KASSERT(message[k] == ports[i]);

			/* Sends a message to MASTER know which port to allow. */
			box_msg.remote_port = (outportal[i] % KPORTAL_PORT_NR);
			KASSERT(kmailbox_write(outbox[i], (void *) &box_msg, BOX_MSG_SIZE) == BOX_MSG_SIZE);

			KASSERT(kportal_write(outportal[i], message, MESSAGE_SIZE) >= MESSAGE_SIZE);
		}

		for (int i = 0; i < nports; ++i)
		{
			KASSERT(kmailbox_close(outbox[i]) == 0);
			KASSERT(kmailbox_unlink(inbox[i]) == 0);

			KASSERT(kportal_ioctl(inportal[i], KPORTAL_IOCTL_GET_LATENCY, &results[ports[i]].latency) == 0);
			KASSERT(kportal_ioctl(inportal[i], KPORTAL_IOCTL_GET_VOLUME, &results[ports[i]].volume) == 0);

			KASSERT(kportal_close(outportal[i]) == 0);
			KASSERT(kportal_unlink(inportal[i]) == 0);
		}

		barrier_cores();
	}

	barrier_cores_cleanup(tid);

	return (NULL);
}

static inline void * do_slave(void * _tid)
{
	int tid;
	int local;
	int remote;
	int inportal;
	int outportal;
	int inbox;
	int outbox;
	char message[MAX_MESSAGE_SIZE];
	struct box_message box_msg;

	tid = (int) ((intptr_t) _tid);
	local  = nodenums[1];
	remote = nodenums[0];

	barrier_cores_setup(tid, _nthreads);

	for (unsigned int iter = 1; iter <= NITERATIONS; ++iter)
	{
		KASSERT((inportal = kportal_create(local, tid)) >= 0);
		KASSERT((outportal = kportal_open(local, remote, tid)) >= 0);

		KASSERT((inbox = kmailbox_create(local, tid)) >= 0);
		KASSERT((outbox = kmailbox_open(remote, tid)) >= 0);

		barrier_cores();

		/* Sends a message to MASTER know which port to allow. */
		box_msg.remote_port = outportal % KPORTAL_PORT_NR;
		KASSERT(kmailbox_write(outbox, (void *) &box_msg, BOX_MSG_SIZE) == BOX_MSG_SIZE);

		umemset(message, tid, MESSAGE_SIZE);
		KASSERT(kportal_write(outportal, message, MESSAGE_SIZE) == MESSAGE_SIZE);

		/* Reads a message to know which port to allow on master. */
		KASSERT(kmailbox_read(inbox, (void *) &box_msg, BOX_MSG_SIZE) == BOX_MSG_SIZE);

		KASSERT(kportal_allow(inportal, remote, box_msg.remote_port) == 0);
		KASSERT(kportal_read(inportal, message, MESSAGE_SIZE) == MESSAGE_SIZE);

		for (int k = 0; k < MESSAGE_SIZE; k++)
			KASSERT(message[k] == tid);

		KASSERT(kmailbox_close(outbox) == 0);
		KASSERT(kmailbox_unlink(inbox) == 0);

		KASSERT(kportal_ioctl(inportal, KPORTAL_IOCTL_GET_LATENCY, &results[tid].latency) == 0);
		KASSERT(kportal_ioctl(inportal, KPORTAL_IOCTL_GET_VOLUME, &results[tid].volume) == 0);

		KASSERT(kportal_close(outportal) == 0);
		KASSERT(kportal_unlink(inportal) == 0);

		barrier_cores();
	}

	barrier_cores_cleanup(tid);

	return (NULL);
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Ping-Pong Routine
 */
int do_ikc_complex(const int * nodes, int nnodes, int index, int message_size)
{
	kthread_t tid[NTHREADS_MAX];

	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(message_size);

	if (index == 0)
		uprintf("[stress][ikc][complex] Init.");

	/* Runs the kernel. */
	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
	{
		_nthreads = nthreads;

		if (index == 0)
			uprintf("[stress][ikc][complex] Running with %d threads.", _nthreads);

		barrier_nodes();

		if (index == 0)
		{
			/* Creates slaves threads (tid = 1..nthreads). */
			for (int i = 1; i < (_nthreads < 3 ? _nthreads : 3); ++i)
				kthread_create(&tid[i], &do_master, ((void *) ((intptr_t) i)));

			/* "Create" thread master (tid = 0). */
			do_master(0);

			/* Joins slaves threads (tid = 1..nthreads). */
			for (int i = 1; i < (_nthreads < 3 ? _nthreads : 3); ++i)
				kthread_join(tid[i], NULL);
		}
		else
		{
			/* Creates slaves threads (tid = 1..nthreads). */
			for (int i = 1; i < nthreads; ++i)
				kthread_create(&tid[i], &do_slave, ((void *) ((intptr_t) i)));

			/* "Create" thread master (tid = 0). */
			do_slave(0);

			/* Joins slaves threads (tid = 1..nthreads). */
			for (int i = 1; i < nthreads; ++i)
				kthread_join(tid[i], NULL);
		}

		barrier_nodes();

		if (index == 0)
			uprintf("[stress][ikc][complex] Finished with %d threads.", _nthreads);
	}

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[stress][ikc][complex] Successfuly completed.");

	return (0);
}

#else

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Ping-Pong Routine
 */
int do_ikc_complex(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(index);
	UNUSED(message_size);

	return (0);
}

#endif /* __TARGET_HAS_PORTAL */
