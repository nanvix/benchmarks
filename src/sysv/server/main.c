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
#include <nanvix/sys/perf.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/sys/ipc.h>

/**
 * @brief Number of messages to send.
 */
#ifdef NDEBUG
#define NUM_MESSAGES 100
#else
#define NUM_MESSAGES 10
#endif

/**
 * @brief Number of clients.
 */
#define NUM_CLIENTS (NANVIX_PROC_MAX - 1)

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Key for message queue.
 */
#define MQUEUE_KEY 100

/**
 * @brief Server.
 */
static void do_server(void)
{
	int ret;
	int msgid;
	uint64_t t;
	char msg[NANVIX_MSG_SIZE_MAX];

	/* Create server endpoint. */
	uassert((msgid = __nanvix_msg_get(MQUEUE_KEY, IPC_CREAT)) >= 0);

	/* Receive messages. */
	perf_start(0, PERF_CYCLES);
	for (int i = 0; i < NUM_CLIENTS*NUM_MESSAGES; i++)
	{
		do
		{
			ret = __nanvix_msg_receive(
				msgid,
				msg,
				NANVIX_MSG_SIZE_MAX,
				0,
				IPC_NOWAIT
			);
			uassert((ret == 0) || (ret == -ENOMSG));
		} while(ret == -ENOMSG);
	}
	t = perf_read(0);

	/* Close server endpoint. */
	uassert(__nanvix_msg_close(msgid) == 0);

#ifdef NDEBUG
	uprintf("[benchmarks][server] server takes %l",
#else
	uprintf("[benchmarks][server] %l",
#endif
		t
	);
}

/**
 * @brief Client.
 */
static void do_client(void)
{
	int ret;
	int msgid;
	char msg[NANVIX_MSG_SIZE_MAX];

	umemset(msg, 1, NANVIX_MSG_SIZE_MAX);

	/* Open conenction to server. */
	uassert((msgid = __nanvix_msg_get(MQUEUE_KEY, IPC_CREAT)) >= 0);

	/* Send messages. */
	for (int i = 0; i < NUM_MESSAGES; i++)
	{
		do
		{
			ret = __nanvix_msg_send(
				msgid,
				msg,
				NANVIX_MSG_SIZE_MAX,
				IPC_NOWAIT
			);
			uassert((ret == 0) || (ret == -EAGAIN));
		} while(ret == -EAGAIN);
	}


	/* Close connection. */
	uassert(__nanvix_msg_close(msgid) == 0);
}


/**
 * @brief Benchmarks Sysytem V Message Queues.
 */
static void benchmark_server(void)
{
	/* TODO: implement. */
	void (*fn)(void);

	fn = (knode_get_num() == PROCESSOR_NODENUM_LEADER) ?
		do_server : do_client;

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

	benchmark_server();

	return (0);
}
