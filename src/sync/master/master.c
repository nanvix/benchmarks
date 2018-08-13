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
#include <pthread.h>

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
static const char *kernel = NULL; /**< Benchmark kernel.               */
/**@}*/

/**
 * @brief ID of slave processes.
 */
static int pids[NANVIX_PROC_MAX];

/**
 * @brief Underlying NoC node ID.
 */
static int nodenum;

/**
 * @brief Input synchronization point.
 */
static int insync;

/*============================================================================*
 * Utility                                                                    *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 */
static void spawn_remotes(void)
{
	char master_node[4];
	char first_remote[4];
	char last_remote[4];
	char niterations_str[4];
	const char *argv[] = {
		"/benchmark/hal-sync-slave",
		master_node,
		first_remote,
		last_remote,
		niterations_str,
		kernel,
		NULL
	};

	/* Spawn remotes. */
	sprintf(master_node, "%d", nodenum);
	sprintf(first_remote, "%d", 0);
	sprintf(last_remote, "%d", nclusters);
	sprintf(niterations_str, "%d", niterations);
	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);

	/* Sync. */
	assert(sys_sync_wait(insync) == 0);
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
 * @brief Barrier kernel.
 */
static void kernel_barrier(void)
{
	int syncid;
	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Open synchronization point. */
	assert((syncid = sys_sync_open(nodes, nclusters + 1, SYNC_ONE_TO_ALL)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		double total;
		uint64_t t1, t2;

		t1 = sys_timer_get();
			assert(sys_sync_signal(syncid) == 0);
			assert(sys_sync_wait(insync) == 0);
		t2 = sys_timer_get();

		total = sys_timer_diff(t1, t2)/((double) sys_get_core_freq());

		/* Warmup. */
		if (((k == 0) || (k == (niterations + 1))))
			continue;

		printf("nanvix;%s;%d;%.2lf\n",
			kernel,
			nclusters,
			(total*MEGA)/nclusters
		);
	}
	
	/* House keeping. */
	assert(sys_sync_close(syncid) == 0);
}


/**
 * @brief HAL Sync microbenchmark.
 */
static void benchmark(void)
{
	int nodes[nclusters + 1];

	/* Initialization. */
	kernel_setup();
	nodenum = sys_get_node_num();

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create synchronization point. */
	assert((insync = sys_sync_create(nodes, nclusters + 1, SYNC_ALL_TO_ONE)) >= 0);

	spawn_remotes();

	if (!strcmp(kernel, "barrier"))
		kernel_barrier();

	/* House keeping. */
	assert(sys_sync_unlink(insync) == 0);
	
	/* House keeping. */
	join_remotes();
	kernel_cleanup();
}

/*============================================================================*
 * HAL Sync Microbenchmark Driver                                             *
 *============================================================================*/

/**
 * @brief HAL Sync Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	assert(argc == 4);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);
	niterations = atoi(argv[2]);
	kernel = argv[3];

	/* Parameter checking. */
	assert(niterations > 0);

	benchmark();

	return (EXIT_SUCCESS);
}
