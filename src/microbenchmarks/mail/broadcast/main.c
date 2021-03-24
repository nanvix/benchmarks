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

/**
 * @brief Number of processes.
 */
#ifndef NUM_PROCS
#define NUM_PROCS NANVIX_PROC_MAX
#endif

/**
 * @brief Number of iterations for the benchmark.
 */
#define NITERATIONS 1

static barrier_t barrier;
static int nodes[NUM_PROCS];

/*============================================================================*
 * Benchmark Kernel                                                           *
 *============================================================================*/

/**
 * @brief Port number used in the benchmark.
 */
#define PORT_NUM 10

/**
 * @brief Dummy message.
 */
static char msg[KMAILBOX_MESSAGE_SIZE];

/**
 * @brief Sends messages to worker.
 */
static void do_leader(void)
{
	int outboxes[NUM_PROCS - 1];
	uint64_t old_latency[NUM_PROCS - 1];
	uint64_t new_latency[NUM_PROCS - 1];

	/* Establish connections. */
	for (int i = 1; i < NUM_PROCS; i++)
	{
		uassert((outboxes[i - 1] = kmailbox_open(PROCESSOR_NODENUM_LEADER + i, PORT_NUM)) >= 0);
		new_latency[i - 1] = 0;
	}

	uassert(barrier_wait(barrier) == 0);

	/* Broadcast messages. */
	for (int k = 1; k <= NITERATIONS; k++)
	{
		uint64_t total_latency = 0;

		for (int i = 1; i < NUM_PROCS; i++)
		{
			uassert(
				kmailbox_write(
					outboxes[i - 1],
					msg,
					KMAILBOX_MESSAGE_SIZE
				) == KMAILBOX_MESSAGE_SIZE
			);
		}

		for (int i = 1; i < NUM_PROCS; i++)
		{
			old_latency[i - 1] = new_latency[i - 1];
			uassert(kmailbox_ioctl(outboxes[i - 1], KMAILBOX_IOCTL_GET_LATENCY, &new_latency[i - 1]) == 0);

			total_latency += new_latency[i - 1] - old_latency[i - 1];
		}

		/* Dump statistics. */
#ifndef NDEBUG
		uprintf("[benchmarks][mail-broadcast] it=%d latency=%l",
#else
		uprintf("[benchmarks][mail-broadcast] %d %l",
#endif
			k, total_latency
		);
	}

	uassert(barrier_wait(barrier) == 0);

	/* House keeping. */
	for (int i = 1; i < NUM_PROCS; i++)
		uassert(kmailbox_close(outboxes[i - 1]) == 0);
}

/**
 * @brief Sends menssages to leader.
 */
static void do_worker(void)
{
	int inbox;

	/* Establish connection. */
	uassert((inbox = kmailbox_create(knode_get_num(), PORT_NUM)) >= 0);

	uassert(barrier_wait(barrier) == 0);

	for (int i = 1; i <= NITERATIONS; i++)
	{
		uassert(
			kmailbox_read(
				inbox,
				msg,
				KMAILBOX_MESSAGE_SIZE
			) == KMAILBOX_MESSAGE_SIZE
		);
	}

	uassert(barrier_wait(barrier) == 0);

	/* House keeping. */
	uassert(kmailbox_unlink(inbox) == 0);
}

/**
 * @brief Benchmarks broadcast communication with mailboxes.
 */
static void benchmark_mail_broadcast(void)
{
	void (*fn)(void);

	fn = (knode_get_num() == PROCESSOR_NODENUM_LEADER) ?
		do_leader : do_worker;

	/* Build list of nodes. */
	for (int i = 0; i < NUM_PROCS; i++)
		nodes[i] = PROCESSOR_NODENUM_LEADER + i;

	barrier = barrier_create(nodes, NUM_PROCS);
	uassert(BARRIER_IS_VALID(barrier));

		fn();

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

	benchmark_mail_broadcast();

	return (0);
}
