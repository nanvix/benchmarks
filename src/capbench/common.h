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

#ifndef _COMMON_H_
#define _COMMON_H_

	#include <nanvix/sys/perf.h>
	#include <nanvix/hal.h>
	#include <nanvix/ulib.h>
	#include <nanvix/config.h>
	#include <nanvix/limits.h>

	#include <posix/stdint.h>
	#include <posix/stddef.h>

	#include <mputil/proc.h>
	#include <mpi.h>

/*============================================================================*
 * Constants                                                                  *
 *============================================================================*/

	/**
	 * @brief Enables debugging prints.
	 */
	#define DEBUG 0

	/**
	 * @brief Enables benchmark verbose mode.
	 */
	#define VERBOSE (DEBUG || 0)

/*============================================================================*
 * Runtime                                                                    *
 *============================================================================*/

	/**
	 * Number of processes per cluster.
	 */
	#define PROCESSES_NR MPI_PROCESSES_NR

	/**
	 * Number of processes per cluster.
	 */
	#define PROCS_PER_CLUSTER_MAX MPI_PROCS_PER_CLUSTER_MAX

	/**
	 * Initializes runtime.
	 */
	EXTERN int runtime_init(int argc, char **argv);

	/**
	 * Finalize runtime.
	 */
	EXTERN void runtime_finalize(void);

	/**
	 * Gets process rank.
	 */
	EXTERN void runtime_get_rank(int * rank);

	/**
	 * Gets process index.
	 */
	EXTERN int runtime_get_index(void);

	/**
	 * Waits on a fence.
	 */
	EXTERN void runtime_fence(void);

	/**
	 * Waits on a barrier.
	 */
	EXTERN void runtime_barrier(void);

#if !__NANVIX_USES_LWMPI

	/**
	 * Init request queues.
	 */
	EXTERN void data_init(void);

#endif

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

	/**
	 * Number of workers.
	 */
	#define PROBLEM_NUM_WORKERS (PROCESSES_NR - 1)

	extern void do_master(void);
	extern void do_slave(void);

/*============================================================================*
 * Communication                                                              *
 *============================================================================*/

#if !__NANVIX_USES_LWMPI
	EXTERN int node_from_rank(int rank);
	EXTERN int first_from_node(int node);
	EXTERN int index_from_rank(int rank);
	EXTERN int port_from_rank(int rank);
#endif

	EXTERN uint64_t data_send(int outfd, void *data, size_t n);
	EXTERN uint64_t data_receive(int infd, void *data, size_t n);

/*============================================================================*
 * Statistics                                                                 *
 *============================================================================*/

	EXTERN uint64_t slave[PROBLEM_NUM_WORKERS];
	EXTERN uint64_t master;

	EXTERN uint64_t total();
	EXTERN void update_total(uint64_t amnt);
	EXTERN uint64_t communication();
	EXTERN void update_communication(uint64_t amnt);
	EXTERN size_t data_sent();
	EXTERN size_t data_received();
	EXTERN unsigned nsend();
	EXTERN unsigned nreceive();

/*============================================================================*
 * Math                                                                       *
 *============================================================================*/

	/**
	 * @brief Math Constants
	 */
	/**@{*/
	#define PI    3.14159265359 /* pi */
	#define E  2.71828182845904 /* e  */
	/**@}*/

	/**
	 * @brief MPPA does not support explicity float division (a/b).
	 */
	/**@{*/
	#ifdef __mppa256__
		#define __fdiv(a, b) __builtin_k1_fsdiv(a, b)
	#else
		#define __fdiv(a, b) ((a)/(b))
	#endif
	/**@{*/

	struct division
	{
		int quotient;
		int remainder;
	};

	EXTERN struct division divide(int a, int b);
	EXTERN float power(float base, float ex);

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

	EXTERN void srandnum(int seed);
	EXTERN unsigned randnum(void);

#endif /* _COMMON_H_ */

