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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <nanvix/syscalls.h>

#include "../kernel.h"

/**
 * @brief Master node NoC ID.
 */
static int masternode;

/**
 * @brief Underlying NoC node ID.
 */
static int nodenum;

/**
 * @brief Number of benchmark interations.
 */
static int niterations = 0;

/**
 * @brief Number of clusters.
 */
static int nclusters = 0;

/**
 * @brief Output synchronization point.
 */
static int outsync;

/*============================================================================*
 * Barrier Kernel                                                              *
 *============================================================================*/

/**
 * @brief Barrier kernel. 
 */
static void kernel_barrier(void)
{
	int syncid;
	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create synchronization point. */
	assert((syncid = sys_sync_create(nodes, nclusters + 1, SYNC_ONE_TO_ALL)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		assert(sys_sync_wait(syncid) == 0);
		assert(sys_sync_signal(outsync) == 0);
	}

	/* House keeping. */
	assert(sys_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * Sync Microbenchmark Driver                                                 *
 *============================================================================*/

/**
 * @brief Syncs with remote master.
 */
static void sync_master(void)
{
	assert(sys_sync_signal(outsync) == 0);
}

/**
 * @brief Sync Microbenchmark Driver
 */
int main2(int argc, const char **argv)
{
	const char *kernel;
	int first_remote;
	int last_remote;
	
	/* Initialization. */
	nodenum = sys_get_node_num();

	/* Retrieve kernel parameters. */
	assert(argc == 6);
	masternode = atoi(argv[1]);
	first_remote = atoi(argv[2]);
	last_remote = atoi(argv[3]);
	niterations = atoi(argv[4]);
	kernel = argv[5];

	nclusters = last_remote - first_remote;

	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	/* Create synchronization point. */
	assert((outsync = sys_sync_open(nodes, nclusters + 1, SYNC_ALL_TO_ONE)) >= 0);

	sync_master();

	/* Run kernel. */
	if (!strcmp(kernel, "barrier"))
		kernel_barrier();

	assert(sys_sync_close(outsync) == 0);

	return (EXIT_SUCCESS);
}
