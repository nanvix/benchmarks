/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/barrier.h>
#include <nanvix/sys/perf.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>

#include <posix/errno.h>
#include <posix/sys/ipc.h>

#include "monitoring.h"

static barrier_t barrier;

/*============================================================================*
 * RPC: Request function                                                      *
 *============================================================================*/

static void get_rule(int nodenum, char * s)
{
	if (nodenum == TEST_STANDALONE_NUM)
		usprintf(s, "standalone %d", nodenum);
	else if (nodenum == TEST_WORKER_NUM)
		usprintf(s , "master worker %d", (nodenum - TEST_WORKER_NUM));
	else
		usprintf(s, "slave worker %d", (nodenum - TEST_WORKER_NUM));
}

/**
 * @brief Get cluster information and send to requester.
 */
static int request(
	int source,
	int mailbox_port,
	int portal_port,
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4,
	word_t arg5
)
{
	int mfd;
	int pfd;
	int nodenum;
	struct infos infos;

	/* Sanity checks. */
	uassert(arg0 == 0);
	uassert(arg1 == 1);
	uassert(arg2 == 2);
	uassert(arg3 == 3);
	uassert(arg4 == 4);
	uassert(arg5 == 5);

	/* Gets node information. */
	nodenum = knode_get_num();

	uprintf("- Executing RPC on Node %d", nodenum);

	/* Opens a transfer channels. */
	uassert((mfd = kmailbox_open(source, mailbox_port)) >= 0);
	uassert((pfd = kportal_open(nodenum, source, portal_port)) >= 0);

		/* Gets node information. */
		infos.nodenum = nodenum; 
		get_rule(nodenum, infos.rule);

		/* Gets clock information. */
		kclock(&infos.timestamp);

		/* Gets thread information. */
		infos.curr_nthreads  = kthread_get_curr_nthreads();
		infos.total_nthreads = kthread_get_total_nthreads();

		/* Gets mailbox information. */
		uassert(kmailbox_ioctl(mfd, KMAILBOX_IOCTL_GET_NCREATES,         &infos.mcreates)     == 0);
		uassert(kmailbox_ioctl(mfd, KMAILBOX_IOCTL_GET_NOPENS,           &infos.mopens)       == 0);

		/* Gets portal information. */
		uassert(kportal_ioctl(pfd,  KPORTAL_IOCTL_GET_NCREATES,          &infos.pcreates)     == 0);
		uassert(kportal_ioctl(pfd,  KPORTAL_IOCTL_GET_NOPENS,            &infos.popens)       == 0);

	/* Sends information. */
	uassert(kmailbox_write(mfd, &infos, TEST_INFOS_SIZE) >= 0);

	/* Closes transfer channels. */
	uassert(kportal_close(pfd) == 0);
	uassert(kmailbox_close(mfd) == 0);

	return (0);
}

/*============================================================================*
 * RPC: Response function                                                     *
 *============================================================================*/

/**
 * @brief Waits the information from target cluster.
 */
static int response(
	int target,
	int mailbox_port,
	int portal_port,
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4,
	word_t arg5
)
{
	int fd;
	struct infos infos;

	/**
	 * Unused arguments because we do not need to send any
	 * data to the target.
	 */
	UNUSED(target);
	UNUSED(mailbox_port);
	UNUSED(portal_port);

	/* Sanity checks. */
	uassert(arg0 == 6);
	uassert(arg1 == 7);
	uassert(arg2 == 8);
	uassert(arg3 == 9);
	uassert(arg4 == 10);
	uassert(arg5 == 11);

	/* Gets the standard reciever mailbox. */
	uassert((fd = stdinbox_get()) >= 0);

	/* Reads information. */
	uassert(kmailbox_read(fd, &infos, TEST_INFOS_SIZE) >= 0);

	/* Prints information. */
	dump_infos(&infos);

	return (0);
}

/*============================================================================*
 * Cluster function                                                           *
 *============================================================================*/

/**
 * @brief Client.
 */
static void do_monitor(void)
{
	int target;
	char tname[NANVIX_PROC_NAME_MAX];

	while (true)
	{
		for (int i = 1; i < TEST_RPC_NUM_PROCS; i++)
		{
			target = (PROCESSOR_NODENUM_LEADER + i);

			/* Create the target cluster name. */
			usprintf(tname, "cluster%d", target);

			uprintf("+ Requesting RPC from Node %d", target);

			/* Request an RPC. */
			uassert(nanvix_rpc_request(
				tname,                  //! Node name
				TEST_RPC_ID,            //! RPC Identification
				RPC_ONE_WAY,            //! Mode
				0, 1, 2, 3, 4, 5        //! Arguments
			) == 0);

			uprintf("+ Waiting informations from Node %d", target);

			/* Wait a response. */
			uassert(nanvix_rpc_response(
				tname,                  //! Node name
				TEST_RPC_ID,            //! RPC Identification
				6, 7, 8, 9, 10, 11      //! Arguments
			) == 0);

			delay(2, CLUSTER_FREQ);
		}
	}
}

/*============================================================================*
 * Benchmark function                                                         *
 *============================================================================*/

/**
 * @brief Benchmarks Sysytem V Message Queues.
 */
static void benchmark_monitoring(void)
{
	int nodenum;
	void (*fn)(void);
	int nodes[TEST_RPC_NUM_PROCS];

	nodenum = knode_get_num();

	uprintf("Booting Node %d", nodenum);

	if (nodenum == TEST_MONIDOR_NUM)
		fn = do_monitor;
	else if (nodenum == TEST_STANDALONE_NUM)
		fn = do_standalone;
	else
		fn = do_worker;

	/* Build list of nodes. */
	for (int i = 0; i < TEST_RPC_NUM_PROCS; i++)
		nodes[i] = PROCESSOR_NODENUM_LEADER + i;

	barrier = barrier_create(nodes, TEST_RPC_NUM_PROCS);
	uassert(BARRIER_IS_VALID(barrier));

		/* Register RPC. */
		nanvix_rpc_create(TEST_RPC_ID, request, response);

		uassert(barrier_wait(barrier) == 0);

			fn();

		uassert(barrier_wait(barrier) == 0);

	uassert(barrier_destroy(barrier) == 0);
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Launches a benchmark.
 */
int __main3(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	benchmark_monitoring();

	return (0);
}

