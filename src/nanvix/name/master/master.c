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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mppaipc.h>

#include <nanvix/syscalls.h>
#include <nanvix/pm.h>
#include <nanvix/limits.h>

/**
 * @brief Global benchmark parameters.
 */
/**@{*/
static int nclusters = 0;         /**< Number of remotes processes.    */
static int niterations = 0;       /**< Number of benchmark parameters. */
static const char *kernel = NULL; /**< Name of the target kernel.      */
/**@}*/

/**
 * @brief ID of slave processes.
 */
static int pids[NANVIX_PROC_MAX];

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief Spawns remote processes.
 */
static void spawn_remotes(void)
{
	char niterations_str[8];
	const char *argv[] = {
		"/name-slave",
		niterations_str,
		kernel,
		NULL
	};

	/* Spawn remotes. */
	sprintf(niterations_str, "%d", niterations);
	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);
}

/**
 * @brief Wait for remote processes.
 */
static void join_remotes(void)
{
	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

/*============================================================================*
 * Name Service Microbenchmark Driver                                         *
 *============================================================================*/

/**
 * @brief Name Service Microbenchmark.
 */
static void benchmark(void)
{
	/* Initialization. */
	spawn_remotes();
	
	/* House keeping. */
	join_remotes();
}

/**
 * @brief Name Service Microbenchmark Driver
 */
int main2(int argc, const char **argv)
{
	assert(argc == 4);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[1]);
	niterations = atoi(argv[2]);
	kernel = argv[3];

	/* Parameter checking. */
	assert(niterations > 0);

	benchmark();

	return (EXIT_SUCCESS);
}
