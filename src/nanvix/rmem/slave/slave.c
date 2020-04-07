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
#include <stdlib.h>
#include <string.h>

#include <nanvix/syscalls.h>
#include <nanvix/pm.h>
#include <nanvix/mm.h>

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
 * @brief Buffer size.
 */
static int bufsize = 0;

/**
 * @brief Barrier for global synchronization.
 */
static int barrier;

/**
 * @brief Buffer.
 */
static char buffer[BUFFER_SIZE_MAX];

/*============================================================================*
 * Read Kernel                                                                *
 *============================================================================*/

/**
 * @brief Read kernel.
 */
static void kernel_read(void)
{
	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
		assert(memread(nodenum*bufsize, buffer, bufsize) == 0);
}

/**
 * @brief Write kernel.
 */
static void kernel_write(void)
{
	/* Benchmark. */
	for (int k = 0; k <= niterations; k++)
		assert(memwrite(nodenum*bufsize, buffer, bufsize) == 0);
}

/**
 * @brief RMem microbenchmark.
 *
 * @param kernel    Benchmark kernel.
 * @param nclusters Number of clusters.
 */
static void benchmark(const char *kernel, int nclusters)
{
	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	assert((barrier = barrier_create(nodes, nclusters + 1)) >= 0);

	if (!strcmp(kernel, "read"))
		kernel_read();
	else if (!strcmp(kernel, "write"))
		kernel_write();

	assert(barrier_wait(barrier) == 0);

	/* House keeping. */
	assert(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * RMem Microbenchmark Driver                                                 *
 *============================================================================*/

/**
 * @brief RMem Microbenchmark Driver
 */
int main2(int argc, const char **argv)
{
	int nclusters;
	const char *kernel;
	
	/* Initialization. */
	nodenum = sys_get_node_num();

	/* Retrieve kernel parameters. */
	assert(argc == 6);
	masternode = atoi(argv[1]);
	nclusters = atoi(argv[2]);
	niterations = atoi(argv[3]);
	bufsize = atoi(argv[4]);
	kernel = argv[5];

	benchmark(kernel, nclusters);

	return (EXIT_SUCCESS);
}
