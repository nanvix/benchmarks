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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <HAL/hal/core/timer.h> 
#include <mppa_power.h>
#include <mppa_async.h>
#include <mppa_remote.h>
#include <utask.h>

#include "../kernel.h"

static char buffer[MAX_BUFFER_SIZE];

int main(int argc, const char **argv)
{
	long t[2];
	int size;
	int clusterid;
	off64_t offset;
	long total_time;

	assert(argc == 3);
	assert((size = atoi(argv[2])) <= MAX_BUFFER_SIZE);

	mppa_rpc_client_init();
	mppa_async_init();

	clusterid = __k1_get_cluster_id();

	assert(mppa_async_malloc(MPPA_ASYNC_DDR_0,  NR_CCLUSTER*size, &offset, NULL) == 0);

	for (int i = 0; i < NITERATIONS; i++)
	{
		mppa_rpc_barrier_all();
		t[0] = __k1_read_dsu_timestamp();

		assert(mppa_async_put(buffer,
				MPPA_ASYNC_DDR_0,
				offset + clusterid*size,
				size,
				NULL) == 0
		);
		assert(mppa_async_fence(MPPA_ASYNC_DDR_0, NULL) == 0);

		mppa_rpc_barrier_all();
		t[1] = __k1_read_dsu_timestamp();

		if (i == 0)
			continue;

		total_time = t[1] - t[0];
		printf("%s;%d;%d;%ld\n",
			"write",
			clusterid,
			size,
			total_time
		);
	}

	assert(mppa_async_free(MPPA_ASYNC_DDR_0, offset, NULL) == 0);

	mppa_async_final();

	return (EXIT_SUCCESS);
}
