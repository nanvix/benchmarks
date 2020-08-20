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
 * @brief Number of iterations for the benchmark.
 */
#define NITERATIONS 1

static barrier_t barrier;
static int nodes[2];

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
 * @brief Receives messages from worker.
 */
static void do_leader(void)
{
	int inbox, outbox;
	uint64_t ping_latency = 0ULL;
	uint64_t pong_latency = 0ULL;

	/* Establish connection. */
	KASSERT((inbox = kmailbox_create(PROCESSOR_NODENUM_LEADER, PORT_NUM)) >= 0);
	KASSERT((outbox = kmailbox_open(PROCESSOR_NODENUM_LEADER + 1, PORT_NUM)) >= 0);

	barrier_wait(barrier);

	for (int i = 1; i <= NITERATIONS; i++)
	{
		uint64_t ping_latency_old = ping_latency;
		uint64_t pong_latency_old = pong_latency;

		KASSERT(kmailbox_read(inbox, msg, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);
		KASSERT(kmailbox_write(outbox, msg, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);

		KASSERT(kmailbox_ioctl(inbox, KMAILBOX_IOCTL_GET_LATENCY, &ping_latency) == 0);
		KASSERT(kmailbox_ioctl(outbox, KMAILBOX_IOCTL_GET_LATENCY, &pong_latency) == 0);

		/* Dump statistics. */
#ifndef NDEBUG
		uprintf("[benchmarks][mail-variable-size] it=%d size=%l read=%l write=%l",
#else
		uprintf("[benchmarks][mail-variable-size] %d %l %l %l",
#endif
			i,
			(uint64_t) KMAILBOX_MESSAGE_SIZE,
			(ping_latency - ping_latency_old),
			(pong_latency - pong_latency_old)
		);
	}

	barrier_wait(barrier);

	/* House keeping. */
	KASSERT(kmailbox_close(outbox) == 0);
	KASSERT(kmailbox_unlink(inbox) == 0);
}

/**
 * @brief Sends menssages to leader.
 */
static void do_worker(void)
{
	int outbox, inbox;

	/* Establish connection. */
	KASSERT((inbox = kmailbox_create(PROCESSOR_NODENUM_LEADER + 1, PORT_NUM)) >= 0);
	KASSERT((outbox = kmailbox_open(PROCESSOR_NODENUM_LEADER, PORT_NUM)) >= 0);

	barrier_wait(barrier);

	for (int i = 1; i <= NITERATIONS; i++)
	{
		KASSERT(kmailbox_write(outbox, msg, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);
		KASSERT(kmailbox_read(inbox, msg,  KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);
	}

	barrier_wait(barrier);

	/* House keeping. */
	KASSERT(kmailbox_close(outbox) == 0);
	KASSERT(kmailbox_unlink(inbox) == 0);
}

/**
 * @brief Benchmarks pingpong communication with mailboxes.
 */
static void benchmark_mail_variable_size(void)
{
	void (*fn)(void);

	fn = (knode_get_num() == PROCESSOR_NODENUM_LEADER) ?
		do_leader : do_worker;

	/* Build list of nodes. */
	for (int i = 0; i < 2; i++)
		nodes[i] = PROCESSOR_NODENUM_LEADER + i;

	barrier = barrier_create(nodes, 2);
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

	benchmark_mail_variable_size();

	return (0);
}

