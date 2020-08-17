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
#define NUM_ITEMS 1000
#else
#define NUM_ITEMS 100
#endif

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Number of slots in the buffer.
 */
#define NUM_SLOTS 10

/**
 * @group Key for semaphores.
 */
/**@{*/
#define SEM_KEY_EMPTY 100 /**< Empty Semaphore */
#define SEM_KEY_FULL  101 /**< Full Semaphore  */
/**@}*/

/**
 * @brief Performs an UP operation in a semaphore.
 *
 * @param semid ID of the target semaphore.
 */
static void do_up(int semid)
{
	struct nanvix_sembuf sembuf;

	sembuf.sem_op = 1;
	sembuf.sem_flg = 0;
	uassert(__nanvix_semop(semid, &sembuf, 1) == 0);
}

/**
 * @brief Performs an DOWN operation in a semaphore.
 *
 * @param semid ID of the target semaphore.
 */
static void do_down(int semid)
{
	struct nanvix_sembuf sembuf;

	sembuf.sem_op = -1;
	sembuf.sem_flg = 0;
	uassert(__nanvix_semop(semid, &sembuf, 1) == 0);
}

/**
 * @brief Server.
 */
static void do_consumer(void)
{
	int full;   /**< Counts nuber of used slot in the buffer.        */
	int empty;  /**< Counts the number of empty slots in the buffer. */
	uint64_t t; /**< Used for time statistics.                       */

	/* Create server endpoint. */
	uassert((empty = __nanvix_semget(SEM_KEY_EMPTY, IPC_CREAT)) >= 0);
	uassert((full  = __nanvix_semget(SEM_KEY_FULL, IPC_CREAT)) >= 0);

	perf_start(0, PERF_CYCLES);

		/* Make some empty slots. */
		for (int i = 0; i < NUM_SLOTS; i++)
			do_up(empty);

		/* Receive messages. */
		for (int i = 0; i < NUM_ITEMS; i++)
		{
			do_down(full);
			do_up(empty);
		}

	t = perf_read(0);

	/* Close server endpoint. */
	uassert(__nanvix_sem_close(full) == 0);
	uassert(__nanvix_sem_close(empty) == 0);

#ifdef NDEBUG
	uprintf("[benchmarks][buffer] buffer takes %l",
#else
	uprintf("[benchmarks][buffer] %l",
#endif
		t
	);
}

/**
 * @brief Client.
 */
static void do_producer(void)
{
	int full;   /**< Counts nuber of used slot in the buffer.        */
	int empty;  /**< Counts the number of empty slots in the buffer. */

	/* Create server endpoint. */
	uassert((empty = __nanvix_semget(SEM_KEY_EMPTY, IPC_CREAT)) >= 0);
	uassert((full  = __nanvix_semget(SEM_KEY_FULL, IPC_CREAT)) >= 0);

	/* Receive messages. */
	for (int i = 0; i < NUM_ITEMS; i++)
	{
		do_down(empty);
		do_up(full);
	}

	/* Close server endpoint. */
	uassert(__nanvix_sem_close(full) == 0);
	uassert(__nanvix_sem_close(empty) == 0);
}

/**
 * @brief Benchmarks Sysytem V Message Queues.
 */
static void benchmark_buffer(void)
{
	/* TODO: implement. */
	void (*fn)(void);

	fn = (knode_get_num() == PROCESSOR_NODENUM_LEADER) ?
		do_consumer : do_producer;

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

	benchmark_buffer();

	return (0);
}

