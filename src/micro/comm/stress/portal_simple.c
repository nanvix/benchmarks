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
#include <nanvix/sys/portal.h>

#include "parameters.h"

#if __TARGET_HAS_PORTAL

/**
 * @brief Number of running threads.
 */
static int _nthreads = 0;

/**
 * @brief Stores the results.
 */
static struct benchmark_result results[NTHREADS_MAX];

static inline void * do_master(void * _tid)
{
	int tid;
	int local;
	int remote;
	int inportal[NTHREADS_MAX];
	int outportal[NTHREADS_MAX];
	char message[MAX_MESSAGE_SIZE];

	tid = (int) ((intptr_t) _tid);
	local  = nodenums[0];
	remote = nodenums[1];

	for (unsigned int iter = 1; iter <= NITERATIONS; ++iter)
	{
		if (tid == 0)
			uprintf("Iteration %d/%d", iter, NITERATIONS);

		for (int port = 0; port < _nthreads; ++port)
		{
			KASSERT((inportal[port] = kportal_create(local, port)) >= 0);
			KASSERT((outportal[port] = kportal_open(local, remote, port)) >= 0);
		}

		for (int i = 0; i < _nthreads; ++i)
		{
			KASSERT(kportal_allow(inportal[i], remote, i) == 0);
			KASSERT(kportal_read(inportal[i], message, MESSAGE_SIZE) == MESSAGE_SIZE);

			for (int k = 0; k < MESSAGE_SIZE; k++)
				KASSERT(message[k] == i);

			KASSERT(kportal_write(outportal[i], message, MESSAGE_SIZE) >= MESSAGE_SIZE);
		}

		for (int i = 0; i < _nthreads; ++i)
		{
			KASSERT(kportal_ioctl(inportal[i], KPORTAL_IOCTL_GET_LATENCY, &results[i].latency) == 0);
			KASSERT(kportal_ioctl(inportal[i], KPORTAL_IOCTL_GET_VOLUME, &results[i].volume) == 0);

			KASSERT(kportal_close(outportal[i]) == 0);
			KASSERT(kportal_unlink(inportal[i]) == 0);
		}
	}

	return (NULL);
}

static inline void * do_slave(void * _tid)
{
	int tid;
	int local;
	int remote;
	int inportal;
	int outportal;
	char message[MAX_MESSAGE_SIZE];

	tid       = (int) ((intptr_t) _tid);
	local     = nodenums[1];
	remote    = nodenums[0];
	inportal  = -1;
	outportal = -1;

	barrier_cores_setup(tid, _nthreads);

	for (unsigned int iter = 1; iter <= NITERATIONS; ++iter)
	{
		for (int i = 0; i < _nthreads; ++i)
		{
			if (i == tid)
			{
				KASSERT((inportal = kportal_create(local, tid)) >= 0);
				KASSERT((outportal = kportal_open(local, remote, tid)) >= 0);
			}

			barrier_cores();
		}

		umemset(message, tid, MESSAGE_SIZE);
		KASSERT(kportal_write(outportal, message, MESSAGE_SIZE) == MESSAGE_SIZE);

		KASSERT(kportal_allow(inportal, remote, tid) == 0);

		KASSERT(kportal_read(inportal, message, MESSAGE_SIZE) == MESSAGE_SIZE);

		for (int k = 0; k < MESSAGE_SIZE; k++)
			KASSERT(message[k] == tid);

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
int do_portal_simple(const int * nodes, int nnodes, int index, int message_size)
{
	kthread_t tid[NTHREADS_MAX];

	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(message_size);

	if (index == 0)
		uprintf("[stress][portal][simple] Init.");

	/* Runs the kernel. */
	for (int nthreads = NTHREADS_MIN; nthreads <= NTHREADS_MAX; nthreads += NTHREADS_STEP)
	{
		_nthreads = nthreads;

		if (index == 0)
			uprintf("[stress][portal][simple] Running with %d threads.", _nthreads);

		barrier_nodes();

		if (index == 0)
			do_master(0);
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
			uprintf("[stress][portal][simple] Finished with %d threads.", _nthreads);
	}

	/* Synchronizes. */
	barrier_nodes();

	if (index == 0)
		uprintf("[stress][portal][simple] Successfuly completed.");

	return (0);
}

#else

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Performance Ping-Pong Routine
 */
int do_portal_simple(const int * nodes, int nnodes, int index, int message_size)
{
	UNUSED(nodes);
	UNUSED(nnodes);
	UNUSED(index);
	UNUSED(message_size);

	return (0);
}

#endif /* __TARGET_HAS_PORTAL */
