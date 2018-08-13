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
 * @brief Buffer.
 */
static char buffer[MAILBOX_MSG_SIZE];

/**
 * @brief Inbox for receiving messages.
 */
static int inbox;

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
		assert(sys_mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
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

	/* Open output box. */
	assert((outbox = sys_mailbox_open(masternode)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
		assert(sys_mailbox_write(outbox, buffer, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);

	/* House keeping. */
	assert(sys_mailbox_close(outbox) == 0);
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

	/* Open output box. */
	assert((outbox = sys_mailbox_open(masternode)) >= 0);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		assert(sys_mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
		assert(sys_mailbox_write(outbox, buffer, MAILBOX_MSG_SIZE) == MAILBOX_MSG_SIZE);
	}

	/* House keeping. */
	assert(sys_mailbox_close(outbox) == 0);
}

/*============================================================================*
 * Unnamed Mailbox Microbenchmark Driver                                      *
 *============================================================================*/

/**
 * @brief Syncs with remote master.
 *
 * @param first_remote First remote in the nodes list.
 * @param last_remote  Last remote in the nodes list.
 */
static void sync_master(int first_remote, int last_remote)
{
	const int nremotes = last_remote - first_remote;
	int syncid;
	int nodes[nremotes + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nremotes; i++)
		nodes[i + 1] = first_remote + i;

	/* Sync. */
	assert((syncid = sys_sync_open(nodes, nremotes + 1, SYNC_ALL_TO_ONE)) >= 0);
	assert(sys_sync_signal(syncid) == 0);
	assert(sys_sync_close(syncid) == 0);
}

/**
 * @brief Unnamed Mailbox Microbenchmark Driver
 */
int main2(int argc, const char **argv)
{
	const char *mode;
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
	mode = argv[5];

	assert((inbox = get_inbox()) >= 0);

	sync_master(first_remote, last_remote);

	/* Run kernel. */
	if (!strcmp(mode, "broadcast"))
		kernel_broadcast();
	else if (!strcmp(mode, "gather"))
		kernel_gather();
	else if (!strcmp(mode, "pingpong"))
		kernel_pingpong();

	return (EXIT_SUCCESS);
}
