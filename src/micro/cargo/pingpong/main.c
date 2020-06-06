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

/**
 * @brief Number of iterations for the benchmark.
 */
#ifdef NDEBUG
#define NITERATIONS 30
#else
#define NITERATIONS 1
#endif

/*============================================================================*
 * Benchmark Kernel                                                           *
 *============================================================================*/

/**
 * @brief Size of buffers (in bytes)
 */
#define BUFFER_SIZE 4096

/**
 * @brief Port number used in the benchmark.
 */
#define PORT_NUM 0

/**
 * @brief Dummy buffer.
 */
static char buf[BUFFER_SIZE];

/**
 * @bbrief Receives data from worker.
 */
static void do_leader(void)
{
	int inportal, outportal;
	uint64_t latency, volume;

	/* Establish connection. */
	uassert((inportal = kportal_create(knode_get_num(), PORT_NUM)) >= 0);
	uassert((outportal = kportal_open(knode_get_num(), PROCESSOR_NODENUM_LEADER + 1, PORT_NUM)) >= 0);

	for (int i = 1; i <= NITERATIONS; i++)
	{
		uassert(kportal_allow(inportal, PROCESSOR_NODENUM_LEADER + 1, PORT_NUM) == 0);
		uassert(kportal_read(inportal, buf, BUFFER_SIZE) == BUFFER_SIZE);
		uassert(kportal_write(outportal, buf, BUFFER_SIZE) == BUFFER_SIZE);

		uassert(kportal_ioctl(inportal, KPORTAL_IOCTL_GET_LATENCY, &latency) == 0);
		uassert(kportal_ioctl(inportal, KPORTAL_IOCTL_GET_VOLUME, &volume) == 0);

		/* Dump statistics. */
#ifndef NDEBUG
		uprintf("[benchmarks][cargo][pingpong] it=%d latency=%l volume=%l",
#else
		uprintf("cargo;pingpong;%d;%l;%l",
#endif
			i, latency, volume
		);
	}

	/* House keeping. */
	uassert(kportal_close(outportal) == 0);
	uassert(kportal_unlink(inportal) == 0);
}

/**
 * @brief Sends data to leader.
 */
static void do_worker(void)
{
	int outportal, inportal;

	/* Establish connection. */
	uassert((inportal = kportal_create(knode_get_num(), PORT_NUM)) >= 0);
	uassert((outportal = kportal_open(knode_get_num(), PROCESSOR_NODENUM_LEADER, PORT_NUM)) >= 0);

	for (int i = 1; i <= NITERATIONS; i++)
	{
		uassert(kportal_write(outportal, buf, BUFFER_SIZE) == BUFFER_SIZE);
		uassert(kportal_allow(inportal, PROCESSOR_NODENUM_LEADER, PORT_NUM) == 0);
		uassert(kportal_read(inportal, buf,  BUFFER_SIZE) == BUFFER_SIZE);
	}

	/* House keeping. */
	uassert(kportal_close(outportal) == 0);
	uassert(kportal_unlink(inportal) == 0);
}

/**
 * @brief Benchmarks pingpong communication with portals.
 */
static void benchmark_cargo_pingpong(void)
{
	void (*fn)(void);

	fn = (knode_get_num() == PROCESSOR_NODENUM_LEADER) ?
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

	benchmark_cargo_pingpong();

	return (0);
}
