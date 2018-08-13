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
#include <nanvix/pm.h>
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
 * @brief Buffer.
 */
static char buffer[MAILBOX_MSG_SIZE];

/**
 * @brief Underlying NoC node ID.
 */
static int nodenum;

/**
 * @brief Inbox for receiving messages.
 */
static int inbox;

/*============================================================================*
 * Utility                                                                    *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 */
static void spawn_remotes(void)
{
	int syncid;
	char master_node[4];
	char first_remote[4];
	char last_remote[4];
	char niterations_str[4];
	int nodes[nclusters + 1];
	const char *argv[] = {
		"/mailbox-slave",
		master_node,
		first_remote,
		last_remote,
		niterations_str,
		kernel,
		NULL
	};

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
 * @brief Opens output mailboxes.
 *
 * @param outboxes Location to store IDs of output mailboxes.
 */
static void open_mailboxes(int *outboxes)
{
	/* Open output portales. */
	for (int i = 0; i < nclusters; i++)
		assert((outboxes[i] = sys_mailbox_open(i)) >= 0);
}

/**
 * @brief Closes output mailboxes.
 *
 * @param outboxes IDs of target output mailboxes.
 */
static void close_mailboxes(const int *outboxes)
{
	/* Close output mailboxes. */
	for (int i = 0; i < nclusters; i++)
		assert((sys_mailbox_close(outboxes[i])) == 0);
}

/*============================================================================*
 * Kernels                                                                    *
 *============================================================================*/

/**
 * @brief Broadcast kernel.
 */
static void kernel_broadcast(void)
{
	int outboxes[nclusters];

	/* Initialization. */
	open_mailboxes(outboxes);
	memset(buffer, 1, MAILBOX_MSG_SIZE);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		double total;
		uint64_t t1, t2;

		t1 = sys_timer_get();
		for (int i = 0; i < nclusters; i++)
			assert(sys_mailbox_write(outboxes[i], buffer, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
		t2 = sys_timer_get();

		total = sys_timer_diff(t1, t2)/((double) sys_get_core_freq());

		/* Warmup. */
		if (((k == 0) || (k == (niterations + 1))))
			continue;

		printf("nanvix;mailbox;%s;%d;%d;%lf;%lf\n",
			kernel,
			MAILBOX_MSG_SIZE,
			nclusters,
			total/nclusters,
			(nclusters*MAILBOX_MSG_SIZE)/total
		);
	}
	
	/* Close output mailboxes. */
	close_mailboxes(outboxes);
}

/**
 * @brief Gather kernel.
 */
static void kernel_gather(void)
{
	double total;
	uint64_t t1, t2;

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		t1 = sys_timer_get();
		for (int i = 0; i < nclusters; i++)
			assert(sys_mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
		t2 = sys_timer_get();

		total = sys_timer_diff(t1, t2)/((double) sys_get_core_freq());

		/* Warmup. */
		if (((k == 0) || (k == (niterations + 1))))
			continue;

		printf("nanvix;mailbox;%s;%d;%d;%lf;%lf\n",
			kernel,
			MAILBOX_MSG_SIZE,
			nclusters,
			total/nclusters,
			(nclusters*MAILBOX_MSG_SIZE)/total
		);
	}
}

/**
 * @brief Ping-Pong kernel.
 */
static void kernel_pingpong(void)
{
	double total;
	uint64_t t1, t2;
	int outboxes[nclusters];

	/* Initialization. */
	open_mailboxes(outboxes);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		t1 = sys_timer_get();
		for (int i = 0; i < nclusters; i++)
			assert(sys_mailbox_write(outboxes[i], buffer, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
		for (int i = 0; i < nclusters; i++)
			assert(sys_mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
		t2 = sys_timer_get();

		total = sys_timer_diff(t1, t2)/((double) sys_get_core_freq());

		/* Warmup. */
		if (((k == 0) || (k == (niterations + 1))))
			continue;

		printf("nanvix;mailbox;%s;%d;%d;%lf;%lf\n",
			kernel,
			MAILBOX_MSG_SIZE,
			nclusters,
			total/nclusters,
			2*(nclusters*MAILBOX_MSG_SIZE)/total
		);
	}

	/* House keeping. */
	close_mailboxes(outboxes);
}

/**
 * @brief Unnamed Mailbox microbenchmark.
 */
static void benchmark(void)
{
	/* Initialization. */
	nodenum = sys_get_node_num();
	assert((inbox = get_inbox()) >= 0);
	spawn_remotes();

	if (!strcmp(kernel, "broadcast"))
		kernel_broadcast();
	else if (!strcmp(kernel, "gather"))
		kernel_gather();
	else if (!strcmp(kernel, "pingpong"))
		kernel_pingpong();
	
	/* House keeping. */
	join_remotes();
}

/*============================================================================*
 * Unnamed Mailbox Microbenchmark Driver                                      *
 *============================================================================*/

/**
 * @brief Unnamed Mailbox Microbenchmark Driver
 */
int main2(int argc, const char **argv)
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
