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
#include <inttypes.h>
#include <string.h>

#include <mppaipc.h>
#include <mppa/osconfig.h>
#include <HAL/hal/core/mp.h>
#include <HAL/hal/core/diagnostic.h>

#include "../kernel.h"

/**
 * @brief Number of benchmark interations.
 */
static int niterations = 0;

/**
 * @brief Input rqueue.
 */
static int inbox;

/**
 * @brief Buffer.
 */
static char buffer[MSG_SIZE];

/*============================================================================*
 * Broadcast Kernel                                                           *
 *============================================================================*/

/**
 * @brief Broadcast kernel. 
 */
static void kernel_broadcast(void)
{
	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
		assert(mppa_read(inbox, buffer, MSG_SIZE) == MSG_SIZE);
}

/*============================================================================*
 * Gather Kernel                                                              *
 *============================================================================*/

/**
 * @brief Gather kernel. 
 */
static void kernel_gather(void)
{
	int outbox;

	/* Open output portal. */
	assert((outbox = mppa_open(RQUEUE_MASTER, O_WRONLY)) != -1);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
		assert(mppa_write(outbox, buffer, MSG_SIZE) == MSG_SIZE);

	/* House keeping. */
	assert(mppa_close(outbox) != -1);
}

/*============================================================================*
 * Ping-Pong Kernel                                                           *
 *============================================================================*/

/**
 * @brief Ping-Pong kernel. 
 */
static void kernel_pingpong(void)
{
	int outbox;

	/* Open output portal. */
	assert((outbox = mppa_open(RQUEUE_MASTER, O_WRONLY)) != -1);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		assert(mppa_read(inbox, buffer, MSG_SIZE) == MSG_SIZE);
		assert(mppa_write(outbox, buffer, MSG_SIZE) == MSG_SIZE);
	}

	/* House keeping. */
	assert(mppa_close(outbox) != -1);
}

/*============================================================================*
 * HAL Rqueue Microbenchmark Driver                                           *
 *============================================================================*/

/**
 * @brief HAL Rqueue Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	int sync_fd;
	uint64_t mask;
	int clusterid;
	char pathname[128];
	const char *kernel;

	/* Retrieve kernel parameters. */
	assert(argc == 3);
	niterations = atoi(argv[1]);
	kernel = argv[2];

	clusterid = __k1_get_cluster_id();

	/* Initialization. */
	sprintf(pathname, RQUEUE_SLAVE, clusterid, 58 + clusterid, 59 + clusterid);
	assert((inbox = mppa_open(pathname, O_RDONLY)) != -1);
	assert((sync_fd = mppa_open(SYNC_MASTER, O_WRONLY)) != -1);

	/* Unblock master. */
	mask = 1 << clusterid;
	assert(mppa_write(sync_fd, &mask, sizeof(uint64_t)) != -1);

	/* Run kernel. */
	if (!strcmp(kernel, "broadcast"))
		kernel_broadcast();
	else if (!strcmp(kernel, "gather"))
		kernel_gather();
	else if (!strcmp(kernel, "pingpong"))
		kernel_pingpong();

	/* House keeping. */
	assert(mppa_close(sync_fd) != -1);
	assert(mppa_close(inbox) != -1);

	return (EXIT_SUCCESS);
}
