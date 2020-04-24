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
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>

/**
 * @brief Number of iterations for the benchmark.
 */
#ifndef NDEBUG
#define NITERATIONS 30
#else
#define NITERATIONS 1
#endif

/*============================================================================*
 * Benchmark Kernel                                                           *
 *============================================================================*/

/**
 * @brief Port number used in the benchmark.
 */
#define PORT_NUM 3

/**
 * @brief Dummy message.
 */
static char msg[KMAILBOX_MESSAGE_SIZE];

/**
 * @bbrief Receives messages from worker.
 */
static void do_leader(void)
{
	int inbox, outbox;
	uint64_t latency, volume;

	/* Establish connection. */
	uassert((inbox = kmailbox_create(knode_get_num(), PORT_NUM)) >= 0);
	uassert((outbox = kmailbox_open(PROCESSOR_CLUSTERNUM_LEADER + 1, PORT_NUM)) >= 0);

	for (int i = 1; i <= NITERATIONS; i++)
	{
		uassert(kmailbox_read(inbox, msg, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);
		uassert(kmailbox_write(outbox, msg, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);

		uassert(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_LATENCY, &latency) == 0);
		uassert(kmailbox_ioctl(inbox, MAILBOX_IOCTL_GET_VOLUME, &volume) == 0);

		/* Dump statistics. */
#ifndef NDEBUG
		uprintf("[benchmarks][mail][pingpong] it=%d latency=%l volume=%l",
#else
		uprintf("mailbox;pingpong;%d;%l;%l",
#endif
			i, latency, volume
		);
	}

	/* House keeping. */
	uassert(kmailbox_close(outbox) == 0);
	uassert(kmailbox_unlink(inbox) == 0);
}

/**
 * @brief Sends menssages to leader.
 */
static void do_worker(void)
{
	int outbox, inbox;

	/* Establish connection. */
	uassert((inbox = kmailbox_create(knode_get_num(), PORT_NUM)) >= 0);
	uassert((outbox = kmailbox_open(PROCESSOR_CLUSTERNUM_LEADER, PORT_NUM)) >= 0);

	for (int i = 1; i <= NITERATIONS; i++)
	{
		uassert(kmailbox_write(outbox, msg, KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);
		uassert(kmailbox_read(inbox, msg,  KMAILBOX_MESSAGE_SIZE) == KMAILBOX_MESSAGE_SIZE);
	}

	/* House keeping. */
	uassert(kmailbox_close(outbox) == 0);
	uassert(kmailbox_unlink(inbox) == 0);
}

/**
 * @brief Benchmarks pingpong communication with mailboxes.
 */
static void benchmark_mail_pingpong(void)
{
	void (*fn)(void);

	fn = (kcluster_get_num() == PROCESSOR_CLUSTERNUM_LEADER) ?
		do_leader : do_worker;

	fn();
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

	benchmark_mail_pingpong();

	return (0);
}
