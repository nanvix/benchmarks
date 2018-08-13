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

#include <mppa/osconfig.h>
#include <mppaipc.h>

#include <nanvix/syscalls.h>
#include <nanvix/limits.h>

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
static char buffer[BUFFER_SIZE_MAX];

/*============================================================================*
 * Utility                                                                    *
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
		"/benchmark/hal-portal-slave",
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
	double total;
	uint64_t t1, t2;
	int outportals[nclusters];

	/* Initialization. */
	open_portals(outportals);
	memset(buffer, 1, bufsize);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		t1 = sys_timer_get();
		for (int i = 0; i < nclusters; i++)
			assert(sys_portal_write(outportals[i], buffer, bufsize) == bufsize);
		t2 = sys_timer_get();

		total = sys_timer_diff(t1, t2)/((double) sys_get_core_freq());

		/* Warmup. */
		if (k == 0)
			continue;

		printf("nanvix;%s;%d;%d;%.2lf;%.2lf\n",
			kernel,
			bufsize,
			nclusters,
			(total*MEGA)/nclusters,
			(nclusters*bufsize)/total
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
	int nodenum;
	double total;
	uint64_t t1, t2;

	nodenum = sys_get_node_num();

	/* Initialization. */
	assert((inportal = sys_portal_create(nodenum)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		uint64_t t3 = 0;

		for (int i = 0; i < nclusters; i++)
		{
			assert(sys_portal_allow(inportal, i) == 0);
			t1 = sys_timer_get();
				assert(sys_portal_read(inportal, buffer, bufsize) == bufsize);
			t2 = sys_timer_get();
			t3 += sys_timer_diff(t1, t2);
		}

		total = t3/((double) sys_get_core_freq());

		/* Warmup. */
		if (k == 0)
			continue;

		printf("nanvix;%s;%d;%d;%.2lf;%.2lf\n",
			kernel,
			bufsize,
			nclusters,
			(total*MEGA)/nclusters,
			(nclusters*bufsize)/total
		);
	}

	/* House keeping. */
	assert(sys_portal_unlink(inportal) == 0);
}

/**
 * @brief Ping-Pong kernel.
 */
static void kernel_pingpong(void)
{
	int inportal;
	int nodenum;
	double total;
	uint64_t t1, t2;
	int outportals[nclusters];

	nodenum = sys_get_node_num();

	/* Initialization. */
	assert((inportal = sys_portal_create(nodenum)) >= 0);
	open_portals(outportals);

	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
	{
		t1 = sys_timer_get();
		for (int i = 0; i < nclusters; i++)
			assert(sys_portal_write(outportals[i], buffer, bufsize) == bufsize);
		for (int i = 0; i < nclusters; i++)
		{
			assert(sys_portal_allow(inportal, i) == 0);
			assert(sys_portal_read(inportal, buffer, bufsize) == bufsize);
		}
		t2 = sys_timer_get();

		total = sys_timer_diff(t1, t2)/((double) sys_get_core_freq());

		/* Warmup. */
		if (k == 0)
			continue;

		printf("nanvix;%s;%d;%d;%.2lf;%.2lf\n",
			kernel,
			bufsize,
			nclusters,
			(total*MEGA)/nclusters,
			2*(nclusters*bufsize)/total
		);
	}

	/* House keeping. */
	close_portals(outportals);
	assert(sys_portal_unlink(inportal) == 0);
}

/**
 * @brief HAL Portal microbenchmark.
 */
static void benchmark(void)
{
	/* Initialization. */
	kernel_setup();
	spawn_remotes();

	if (!strcmp(kernel, "broadcast"))
		kernel_broadcast();
	else if (!strcmp(kernel, "gather"))
		kernel_gather();
	else if (!strcmp(kernel, "pingpong"))
		kernel_pingpong();
	
	/* House keeping. */
	join_remotes();
	kernel_cleanup();
}

/*============================================================================*
 * HAL Portal Microbenchmark Driver                                           *
 *============================================================================*/

/**
 * @brief HAL Portal Microbenchmark Driver
 */
int main(int argc, const char **argv)
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
