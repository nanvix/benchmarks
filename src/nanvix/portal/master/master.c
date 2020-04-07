/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mppaipc.h>

#include <nanvix/syscalls.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "../kernel.h"

/**
 * @brief Benchmark parameters.
 */
/**@{*/
static int nclusters = 0;         /**< Number of remotes processes.    */
static int niterations = 0;       /**< Number of benchmark parameters. */
static int bufsize = 0;           /**< Buffer size.                    */
static const char *kernel = NULL; /**< Benchmark kernel.               */
/**@}*/

/**
 * @brief ID of slave processes.
 */
static int pids[NANVIX_PROC_MAX];

/**
 * @brief Buffer.
 */
static char buffer[NANVIX_PROC_MAX*BUFFER_SIZE_MAX];

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 */
static void spawn_remotes(void)
{
	int syncid;
	int nodenum;
	char master_node[4];
	char first_remote[4];
	char last_remote[4];
	char niterations_str[4];
	char bufsize_str[20];
	int nodes[nclusters + 1];
	const char *argv[] = {
		"/portal-slave",
		master_node,
		first_remote,
		last_remote,
		niterations_str,
		bufsize_str,
		kernel,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create synchronization point. */
	assert((syncid = sys_sync_create(nodes, nclusters + 1, SYNC_ALL_TO_ONE)) >= 0);

	/* Spawn remotes. */
	sprintf(master_node, "%d", nodenum);
	sprintf(first_remote, "%d", 0);
	sprintf(last_remote, "%d", nclusters);
	sprintf(niterations_str, "%d", niterations);
	sprintf(bufsize_str, "%d", bufsize);
	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);

	/* Sync. */
	assert(sys_sync_wait(syncid) == 0);

	/* House keeping. */
	assert(sys_sync_unlink(syncid) == 0);
}

/**
 * @brief Wait for remote processes.
 */
static void join_remotes(void)
{
	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

/**
 * @brief Residual timer.
 */
static uint64_t residual = 0;

/**
 * @brief Callibrates the timer.
 */
static void timer_init(void)
{
	uint64_t t1, t2;

	t1 = sys_timer_get();
	t2 = sys_timer_get();

	residual = t2 - t1;
}

/**
 * @brief Computes the difference between two timers.
 */
static inline uint64_t timer_diff(uint64_t t1, uint64_t t2)
{
	return (t2 - t1 - residual);
}

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

/**
 * @brief Opens output portals.
 *
 * @param outportals Location to store IDs of output portals.
 */
static void open_portals(int *outportals)
{
	/* Open output portales. */
	for (int i = 0; i < nclusters; i++)
		assert((outportals[i] = sys_portal_open(i)) >= 0);
}

/**
 * @brief Closes output portals.
 *
 * @param outportals IDs of target output portals.
 */
static void close_portals(const int *outportals)
{
	/* Close output portals. */
	for (int i = 0; i < nclusters; i++)
		assert((sys_portal_close(outportals[i])) == 0);
}

/**
 * @brief Broadcast kernel.
 */
static void kernel_broadcast(void)
{
	int outportals[nclusters];

	/* Initialization. */
	open_portals(outportals);
	memset(buffer, 1, nclusters*bufsize);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		uint64_t t1, t2;
		uint64_t tkernel, tnetwork;

		tkernel = tnetwork = 0;

		for (int i = 0; i < nclusters; i++)
		{
			t1 = sys_timer_get();
				assert(sys_portal_write(
					outportals[i],
					&buffer[i*bufsize],
					bufsize) == bufsize
				);
			t2 = sys_timer_get();
			tkernel += timer_diff(t1, t2);

			assert(sys_portal_ioctl(outportals[i], PORTAL_IOCTL_GET_LATENCY, &t1) == 0);

			tnetwork += t1;
			tkernel -= t1;
		}

		/* Warmup. */
		if (((k == 0) || (k == (niterations + 1))))
			continue;

		printf("nanvix;portal;%s;%d;%d;%lf;%lf\n",
			kernel,
			bufsize,
			nclusters,
			tkernel/((double) sys_get_core_freq()),
			tnetwork/((double) sys_get_core_freq())
		);
	}

	/* House keeping. */
	close_portals(outportals);
}

/**
 * @brief Gather kernel.
 */
static void kernel_gather(void)
{
	int inportal;

	/* Initialization. */
	assert((inportal = get_inportal()) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		uint64_t tnetwork, tkernel;

		tkernel = tnetwork = 0;

		for (int i = 0; i < nclusters; i++)
		{
			uint64_t t1, t2;

			t1 = sys_timer_get();
				assert(sys_portal_allow(inportal, i) == 0);
			t2 = sys_timer_get();
			tkernel += timer_diff(t1, t2);

			t1 = sys_timer_get();
				assert(sys_portal_read(
					inportal,
					&buffer[i*bufsize],
					bufsize) == bufsize
				);
			t2 = sys_timer_get();
			tkernel += timer_diff(t1, t2);

			assert(sys_portal_ioctl(inportal, PORTAL_IOCTL_GET_LATENCY, &t1) == 0);

			tnetwork += t1;
			tkernel -= t1;
		}

		/* Warmup. */
		if (((k == 0) || (k == (niterations + 1))))
			continue;

		printf("nanvix;portal;%s;%d;%d;%lf;%lf\n",
			kernel,
			bufsize,
			nclusters,
			tkernel/((double) sys_get_core_freq()),
			tnetwork/((double) sys_get_core_freq())
		);
	}
}

/**
 * @brief Unnamed Portal microbenchmark.
 */
static void benchmark(void)
{
	/* Initialization. */
	timer_init();
	spawn_remotes();

	if (!strcmp(kernel, "broadcast"))
		kernel_broadcast();
	else if (!strcmp(kernel, "gather"))
		kernel_gather();
	
	/* House keeping. */
	join_remotes();
}

/*============================================================================*
 * Unnamed Portal Microbenchmark Driver                                       *
 *============================================================================*/

/**
 * @brief Unnamed Portal Microbenchmark Driver
 */
int main2(int argc, const char **argv)
{
	assert(argc == 5);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);
	niterations = atoi(argv[2]);
	bufsize = atoi(argv[3]);
	kernel = argv[4];

	/* Parameter checking. */
	assert(niterations > 0);
	assert((bufsize > 0) && (bufsize <= (BUFFER_SIZE_MAX)));
	assert((bufsize%2) == 0);

	benchmark();

	return (EXIT_SUCCESS);
}
