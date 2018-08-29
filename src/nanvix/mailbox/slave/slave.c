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

#include <nanvix/syscalls.h>
#include <nanvix/pm.h>

#include "../kernel.h"

/**
 * @brief Global benchmark parameters.
 */
/**@{*/
static int first_cluster;   /**< ID of first cluster.            */
static int last_cluster;    /**< ID of last cluster.             */
static int niterations = 0; /**< Number of benchmark parameters. */
/**@}*/

/**
 * @brief Input mailbox.
 */
static int inbox;

/**
 * @brief Global sync.
 */
static int sync;

/**
 * @brief Underlying NoC node ID.
 */
static int nodenum;

/**
 * @brief Master node NoC ID.
 */
static int masternode;

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

	msg.nodenum = nodenum;

	/* Open output box. */
	assert((outbox = sys_mailbox_open(masternode)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		/* Unblock master. */
		assert(sys_sync_signal(sync) == 0);

		/* Ping-pong. */
		assert(sys_mailbox_write(outbox, &msg, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
		assert(sys_mailbox_read(inbox, &msg, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
	}

	/* House keeping. */
	assert(sys_mailbox_close(outbox) == 0);
}

/*============================================================================*
 * Unnamed Mailbox Microbenchmark Driver                                      *
 *============================================================================*/

/**
 * @brief Unnamed Mailbox microbenchmark.
 *
 * @param kernel Name of the target kernel.
 */
static void benchmark(const char *kernel)
{
	const int nclusters = last_cluster - first_cluster;
	int nodes[nclusters + 1];

	nodenum = sys_get_node_num();

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = first_cluster + i;

	/* Initialization. */
	assert((inbox = get_inbox()) >= 0);
	assert((sync = sys_sync_open(nodes, nclusters + 1, SYNC_ALL_TO_ONE)) >= 0);

	/* Run kernel. */
	if (!strcmp(kernel, "pingpong"))
		kernel_pingpong();
	
	/* House keeping. */
	assert(sys_sync_close(sync) == 0);
}

/**
 * @brief Unnamed Mailbox Microbenchmark Driver
 */
int main2(int argc, const char **argv)
{
	/* Retrieve kernel parameters. */
	assert(argc == 6);
	masternode = atoi(argv[1]);
	first_cluster = atoi(argv[2]);
	last_cluster = atoi(argv[3]);
	niterations = atoi(argv[4]);

	benchmark(argv[5]);

	return (EXIT_SUCCESS);
}
