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
#include <nanvix/mm.h>

#include "../kernel.h"

/**
 * @brief Benchmark parameters.
 */
/**@{*/
static int nclusters = 0;             /**< Number of remotes processes.    */
static int niterations = 0;           /**< Number of benchmark parameters. */
static int bufsize = 0;               /**< Buffer size.                    */
static const char *kernelname = NULL; /**< Benchmark kernel.               */
/**@}*/

/**
 * @brief ID of slave processes.
 */
static int pids[NANVIX_PROC_MAX];

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief Barrier for global synchronization.
 */
static int barrier;

/**
 * @brief Spawns remote processes.
 */
static void spawn_remotes(void)
{
	int nodenum;
	char master_node[4];
	char nclusters_str[4];
	char niterations_str[4];
	char bufsize_str[20];
	int nodes[nclusters + 1];
	const char *argv[] = {
		"/rmem-slave",
		master_node,
		nclusters_str,
		niterations_str,
		bufsize_str,
		kernelname,
		NULL
	};

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	nodes[0] = nodenum;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create global barrier. */
	assert((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	/* Spawn remotes. */
	sprintf(master_node, "%d", nodenum);
	sprintf(nclusters_str, "%d", nclusters);
	sprintf(niterations_str, "%d", niterations);
	sprintf(bufsize_str, "%d", bufsize);
	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);
}

/**
 * @brief Wait for remote processes.
 */
static void join_remotes(void)
{
	/* Sync. */
	assert(barrier_wait(barrier) == 0);

	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);

	/* House keeping. */
	assert(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

/**
 * @brief RMem microbenchmark.
 */
static void benchmark(void)
{
	/* Initialization. */
	spawn_remotes();
	
	/* House keeping. */
	join_remotes();
}

/*============================================================================*
 * RMem Microbenchmark Driver                                                 *
 *============================================================================*/

/**
 * @brief RMem Microbenchmark Driver
 */
int main2(int argc, const char **argv)
{
	assert(argc == 5);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);
	niterations = atoi(argv[2]);
	bufsize = atoi(argv[3]);
	kernelname = argv[4];

	/* Parameter checking. */
	assert(niterations > 0);
	assert((bufsize > 0) && (bufsize <= (BUFFER_SIZE_MAX)));
	assert((bufsize%2) == 0);

	benchmark();

	return (EXIT_SUCCESS);
}
