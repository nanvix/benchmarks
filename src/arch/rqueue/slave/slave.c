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
 * @brief Global benchmark parameters.
 */
/**@{*/
static int niterations = 0; /**< Number of benchmark parameters. */
/**@}*/

/**
 * @brief Input rqueue.
 */
static int inbox;

/**
 * @brief Global sync.
 */
static int sync;

/**
 * @brief Cluster ID.
 */
static int clusterid;

/*============================================================================*
 * Ping-Pong Kernel                                                           *
 *============================================================================*/

/**
 * @brief Ping-Pong kernel. 
 */
static void kernel_pingpong(void)
{
	int outbox;
	struct message msg;

	msg.clusterid = clusterid;

	/* Open output mailbox. */
	assert((outbox = mppa_open(RQUEUE_MASTER, O_WRONLY)) != -1);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		uint64_t mask;

		/* Unblock master. */
		mask = 1 << clusterid;
		assert(mppa_write(sync, &mask, sizeof(uint64_t)) != -1);


		/* Ping-pong. */
		assert(mppa_write(outbox, &msg, MSG_SIZE) == MSG_SIZE);
		assert(mppa_read(inbox, &msg, MSG_SIZE) == MSG_SIZE);
	}

	/* House keeping. */
	assert(mppa_close(outbox) != -1);
}

/*============================================================================*
 * MPPA-256 Rqueue Microbenchmark Driver                                      *
 *============================================================================*/

/**
 * @brief Rqueue microbenchmark.
 *
 * @param kernel Name of the target kernel.
 */
static void benchmark(const char *kernel)
{
	char pathname[128];

	clusterid = __k1_get_cluster_id();

	/* Initialization. */
	sprintf(pathname, RQUEUE_SLAVE, clusterid, 58 + clusterid, 59 + clusterid);
	assert((inbox = mppa_open(pathname, O_RDONLY)) != -1);
	assert((sync = mppa_open(SYNC_MASTER, O_WRONLY)) != -1);

	/* Run kernel. */
	if (!strcmp(kernel, "pingpong"))
		kernel_pingpong();

	/* House keeping. */
	assert(mppa_close(sync) != -1);
	assert(mppa_close(inbox) != -1);
}

/**
 * @brief Rqueue Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	/* Retrieve kernel parameters. */
	assert(argc == 3);
	niterations = atoi(argv[1]);

	benchmark(argv[2]);

	return (EXIT_SUCCESS);
}
