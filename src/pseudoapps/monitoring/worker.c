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

/**
 * @brief Number of processes.
 */
#define NUM_PROCS TEST_RPC_NUM_PROCS_COMM

/**
 * @brief Number of iterations for the benchmark.
 */
#define NITERATIONS 50

/*============================================================================*
 * Benchmark Kernel                                                           *
 *============================================================================*/

/**
 * @brief Size of buffers (in bytes)
 */
#define BUFFER_SIZE (4 * KB)

/**
 * @brief Port number used in the benchmark.
 */
#define PORT_NUM 0

/**
 * @brief Dummy buffer.
 */
static char buf[BUFFER_SIZE];

/**
 * @brief Sends data to worker.
 */
static void do_leader(void)
{
	int outportals[NUM_PROCS - 1];

	while (true)
	{
		/* Establish connection. */
		for (int i = 1; i < NUM_PROCS; i++)
		{
			uassert((
				outportals[i - 1] = kportal_open(
					knode_get_num(),
					TEST_WORKER_NUM + i,
					PORT_NUM)
				) >= 0
			);
		}

		/* Broadcast data. */
		for (int k = 1; k <= NITERATIONS; k++)
		{
			for (int i = 1; i < NUM_PROCS; i++)
			{
				uassert(
					kportal_write(
						outportals[i - 1],
						buf,
						BUFFER_SIZE
					) == BUFFER_SIZE
				);
			}
		}

		/* House keeping. */
		for (int i = 1; i < NUM_PROCS; i++)
			uassert(kportal_close(outportals[i - 1]) == 0);
	}
}

/**
 * @brief Receives data from leader.
 */
static void _do_worker(void)
{
	int inportal;

	while (true)
	{
		/* Establish connection. */
		uassert((inportal = kportal_create(knode_get_num(), PORT_NUM)) >= 0);

		for (int i = 1; i <= NITERATIONS; i++)
		{
			uassert(kportal_allow(inportal, TEST_WORKER_NUM, PORT_NUM) == 0);
			uassert(kportal_read(inportal, buf,  BUFFER_SIZE) == BUFFER_SIZE);
		}

		/* House keeping. */
		uassert(kportal_unlink(inportal) == 0);
	}
}

/**
 * @brief Benchmarks broadcast communication with portals.
 */
static void benchmark_cargo_broadcast(void)
{
	void (*fn)(void);

	fn = (knode_get_num() == TEST_WORKER_NUM) ?
		do_leader : _do_worker;

	fn();
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Launches a benchmark.
 */
void do_worker(void)
{
	benchmark_cargo_broadcast();
}

