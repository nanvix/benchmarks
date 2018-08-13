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

#include <mppa_power.h>
#include <mppa_async.h>
#include <mppa_remote.h>
#include <utask.h>

#include "../kernel.h"

/**
 * @brief Benchmark parameters.
 */
/**@{*/
static int nclusters = 0;         /**< Number of remotes processes. */
static int bufsize = 0;           /**< Buffer size.                 */
static const char *kernel = NULL; /**< Benchmark kernel.            */
/**@}*/

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 */
static void spawn_remotes(void)
{
	char bufsize_str[10];
	const char *argv[] = {
		"/mppa256-async-slave",
		bufsize_str,
		NULL
	};

	/* Spawn remotes. */
	sprintf(bufsize_str, "%d", bufsize);
	for(int i = 0; i < nclusters; i++)
	{
		assert(mppa_power_base_spawn(
			i,
			argv[0],
			argv,
			NULL,
			MPPA_POWER_SHUFFLING_ENABLED) != -1
		);
	}
}

/**
 * @brief Wait for remote processes.
 */
static void join_remotes(void)
{
	for(int i = 0; i < nclusters; i++)
		assert(mppa_power_base_waitpid(i, NULL, 0) >= 0);
}

/*============================================================================*
 * MPPA-256 Async Microbenchmark Driver                                       *
 *============================================================================*/

/**
 * @brief Portal microbenchmark.
 */
static void benchmark(void)
{
	utask_t t;

	/* Initialization. */
	mppa_rpc_server_init(1, 0, nclusters);
	mppa_async_server_init();
	utask_create(&t, NULL, (void*)mppa_rpc_server_start, NULL);
	spawn_remotes();

	/* Run kernel. */
	if (!strcmp(kernel, "gather"))
		{ /* noop */ ; } ;
	
	/* House keeping. */
	join_remotes();
}

/**
 * @brief Async Microbenchmark Driver
 */
int main(int argc, const char **argv)
{
	assert(argc == 4);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);
	bufsize = atoi(argv[2]);
	kernel = argv[3];

	/* Parameter checking. */
	assert((nclusters > 0) && (nclusters <= NR_CCLUSTER));
	assert((bufsize > 0) && (bufsize <= (MAX_BUFFER_SIZE)));
	assert((bufsize%2) == 0);

	benchmark();

	return 0;
}
