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
#include <inttypes.h>
#include <string.h>

#include <mppaipc.h>
#include <mppa/osconfig.h>
#include <HAL/hal/core/mp.h>
#include <HAL/hal/core/diagnostic.h>

#include "../kernel.h"

/**
 * @brief Number of benchmark interations.
 */
static int niterations = 0;

/**
 * @brief Buffer size.
 */
static int bufsize = 0;

/**
 * @brief Buffer.
 */
static char buffer[BUFFER_SIZE_MAX];

/*============================================================================*
 * Gather Kernel                                                              *
 *============================================================================*/

/**
 * @brief Gather kernel. 
 */
static void kernel_gather(void)
{
	int sync_fd;
	int outportal;

	/* Open output portal. */
	assert((outportal = mppa_open(PORTAL_MASTER, O_WRONLY)) != -1);

	/* Open sync. */
	assert((sync_fd = mppa_open(SYNC_SLAVES, O_RDONLY)) != -1);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		uint64_t mask;

		/* Wait for master. */
		mask = ~(1 << __k1_get_cluster_id());
		assert(mppa_ioctl(sync_fd, MPPA_RX_SET_MATCH, mask) != -1);
		assert(mppa_read(sync_fd, &mask, sizeof(uint64_t)) != -1);

		/* Send data. */
		assert(mppa_pwrite(outportal, buffer, bufsize, 0) == bufsize);
	}

	/* House keeping. */
	assert(mppa_close(sync_fd) != -1);
	assert(mppa_close(outportal) != -1);
}

/*============================================================================*
 * Broadcast Kernel                                                           *
 *============================================================================*/

/**
 * @brief Broadcast kernel. 
 */
static void kernel_broadcast(void)
{
	int sync_fd;
	int inportal;

	assert((inportal = mppa_open(PORTAL_SLAVES, O_RDONLY)) != -1);

	/* Open sync. */
	assert((sync_fd = mppa_open(SYNC_MASTER, O_WRONLY)) != -1);

	/* Benchmark. */
	for (int k = 0; k <= (niterations + 1); k++)
	{
		uint64_t mask;
		mppa_aiocb_t aiocb;

		/* Setup read operation. */
		mppa_aiocb_ctor(&aiocb, inportal, buffer, bufsize);
		assert(mppa_aio_read(&aiocb) != -1);

		/* Unblock master. */
		mask = 1 << __k1_get_cluster_id();
		assert(mppa_write(sync_fd, &mask, sizeof(uint64_t)) != -1);

		/* Wait read operation to complete. */
		assert(mppa_aio_wait(&aiocb) == bufsize);
	}

	/* House keeping. */
	assert(mppa_close(sync_fd) != -1);
	assert(mppa_close(inportal) != -1);
}


/*============================================================================*
 * HAL Portal Microbenchmark Driver                                           *
 *============================================================================*/

/**
 * @brief HAL Portal Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	const char *kernel;

	/* Retrieve kernel parameters. */
	assert(argc == 4);
	niterations = atoi(argv[1]);
	bufsize = atoi(argv[2]);
	kernel = argv[3];

	/* Run kernel. */
	if (!strcmp(kernel, "gather"))
		kernel_gather();
	else if (!strcmp(kernel, "broadcast"))
		kernel_broadcast();

	return (EXIT_SUCCESS);
}
