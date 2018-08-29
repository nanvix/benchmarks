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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>

/**
 * @brief Global benchmark parameters.
 */
/**@{*/
static int niterations = 0; /**< Number of benchmark parameters. */
/**@}*/

/**
 * @brief Underlying NoC node ID.
 */
static int nodenum;

/*============================================================================*
 * Lookup Kernel                                                              *
 *============================================================================*/

/**
 * @brief Lookup kernel. 
 */
static void kernel_lookup(void)
{
	char pathname[NANVIX_PROC_NAME_MAX];

	sprintf(pathname, "cluster%d", nodenum);
	assert(name_link(nodenum, pathname) == 0);

	/* Benchmark. */
	for (int k = 0; k < niterations; k++)
		assert(name_lookup(pathname) >= 0);

	/* House keeping. */
	assert(name_unlink(pathname) == 0);
}

/*============================================================================*
 * Unnamed Mailbox Microbenchmark Driver                                      *
 *============================================================================*/

/**
 * @brief Unnamed Mailbox microbenchmark.
 *
 * @param kernel Name of the target kernel.
 */
static void benchmark(const char *kernel)
{
	nodenum = sys_get_node_num();

	/* Run kernel. */
	if (!strcmp(kernel, "lookup"))
		kernel_lookup();
}

/**
 * @brief Unnamed Mailbox Microbenchmark Driver
 */
int main2(int argc, const char **argv)
{
	/* Retrieve kernel parameters. */
	assert(argc == 3);
	niterations = atoi(argv[1]);

	benchmark(argv[2]);

	return (EXIT_SUCCESS);
}
