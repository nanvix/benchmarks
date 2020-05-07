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

#include <nanvix/servers/message.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>

/*============================================================================*
 * Barrier                                                                    *
 *============================================================================*/

/**
 * @brief Port number for barrier.
 */
#define BARRIER_PORT 1

/**
 * @brief Number of processes in the barier.
 */
#define PROCS_NUM PROCESSOR_CCLUSTERS_NUM

/**
 * @brief Name Server message.
 */
struct message
{
	message_header header;
};

/**
 * @brief Startup barrier
 */
static struct
{
	int mailboxes[PROCS_NUM];
} slow_barrier = {
	.mailboxes = {
		[0 ... (PROCS_NUM - 1)] = -1
	}
};

/*
 * Build a list of the node IDs
 *
 * @param nodes
 * @param nioclusters
 * @param ncclusters
 *
 * @author João Vicente Souto
 */
static void build_node_list(int *nodes, int ncclusters)
{
	int base;
	int step;
	int index;
	int max_clusters;

	index = 0;

	/* Build node IDs of the Compute Clusters. */
	base         = PROCESSOR_IOCLUSTERS_NUM * (PROCESSOR_NOC_IONODES_NUM / PROCESSOR_IOCLUSTERS_NUM);
	max_clusters = PROCESSOR_CCLUSTERS_NUM;
	step         = (PROCESSOR_NOC_CNODES_NUM / PROCESSOR_CCLUSTERS_NUM);
	for (int i = 0; i < max_clusters && i < ncclusters; i++, index++)
		nodes[index] = base + (i * step);
}

/**
 * @brief Initializes the spawn barrier.
 */
void slow_barrier_setup(void)
{
	int procs[PROCS_NUM];

	build_node_list(procs, PROCS_NUM);

	/* Leader. */
	if (kcluster_get_num() == PROCESSOR_CLUSTERNUM_LEADER)
	{
		for (int i = 1 ; i < PROCS_NUM; i++)
		{
			uassert((slow_barrier.mailboxes[i] = kmailbox_open(
				procs[i], BARRIER_PORT)
			) >= 0);
		}
	}

	/* Follower. */
	else
	{
		uassert((slow_barrier.mailboxes[0] = kmailbox_open(
			procs[0], BARRIER_PORT)
		) >= 0);
	}
}

/**
 * @brief Shutdowns the spawn barrier.
 */
void slow_barrier_cleanup(void)
{
	/* Leader. */
	if (kcluster_get_num() == PROCESSOR_CLUSTERNUM_LEADER)
	{
		for (int i = 1 ; i < PROCS_NUM; i++)
			uassert(kmailbox_close(slow_barrier.mailboxes[i]) == 0);
	}

	/* Follower. */
	else
		uassert(kmailbox_close(slow_barrier.mailboxes[0]) == 0);
}

/**
 * @brief Waits on the startup barrier
 */
void slow_barrier_wait(void)
{
	struct message msg;

	/* Leader */
	if (kcluster_get_num() == PROCESSOR_CLUSTERNUM_LEADER)
	{
		for (int i = 1 ; i < PROCS_NUM; i++)
		{
			uassert(kmailbox_read(
				stdinbox_get(), &msg, sizeof(struct message)
			) == sizeof(struct message));
		}
		for (int i = 1 ; i < PROCS_NUM; i++)
		{
			uassert(kmailbox_write(
				slow_barrier.mailboxes[i], &msg, sizeof(struct message)
			) == sizeof(struct message));
		}
	}

	/* Follower. */
	else
	{
		uassert(kmailbox_write(
			slow_barrier.mailboxes[0], &msg, sizeof(struct message)
		) == sizeof(struct message));
		uassert(kmailbox_read(
			stdinbox_get(), &msg, sizeof(struct message)
		) == sizeof(struct message));
	}
}

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Benchmarks all-to-all synchronization.
 */
static void benchmark_slow_barrier(void)
{
	uint64_t time_slow_barrier;

	slow_barrier_setup();

	perf_start(0, PERF_CYCLES);
	slow_barrier_wait();
	perf_stop(0);
	time_slow_barrier = perf_read(0);

	slow_barrier_cleanup();

#ifndef NDEBUG
	uprintf("[benchmarks][slow_barrier] %l",
#else
	uprintf("[benchmarks][slow_barrier] %l",
#endif
		time_slow_barrier
	);
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

	benchmark_slow_barrier();

	return (0);
}

