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

#ifndef _KM_H_
#define _KM_H_

	#include <nanvix/sys/perf.h>
	#include <nanvix/hal.h>
	#include <nanvix/ulib.h>
	#include <nanvix/config.h>
	#include <nanvix/limits.h>

	#include <posix/stdint.h>
	#include <posix/stddef.h>

	#include <mputil/proc.h>
	#include <mpi/datatype.h>
	#include <mpi.h>

/*============================================================================*
 * Parameters                                                                 *
 *============================================================================*/

	#define PROBLEM_SEED                            0
	#define PROBLEM_LNPOINTS                 (15*2048)
	#define PROBLEM_NUM_CENTROIDS                  64
	#define DIMENSION_MAX                           4
	#define PROBLEM_NUM_POINTS      (PROBLEM_LNPOINTS)
	#define PROBLEM_NUM_WORKERS (MPI_PROCESSES_NR - 1)

/*============================================================================*
 * Communication                                                              *
 *============================================================================*/

	extern int rank;
	extern MPI_Group group;

	extern uint64_t data_send(int outfd, void *data, size_t n);
	extern uint64_t data_receive(int infd, void *data, size_t n);

/*============================================================================*
 * Statistics                                                                 *
 *============================================================================*/

	extern uint64_t slave[PROBLEM_NUM_WORKERS];
	extern size_t data_sent;
	extern unsigned nsend;
	extern size_t data_received;
	extern unsigned nreceive;
	extern uint64_t communication;
	extern uint64_t total;
	extern uint64_t master;

/*============================================================================*
 * Vector                                                                     *
 *============================================================================*/

	/**
	 * @brief Used for floating-point zero comparison.
	 */
	#define ZERO 0.0000001

	extern float vector_distance(float *a, float *b);
	extern float *vector_add(float *v1, const float *v2);
	extern float *vector_mult(float *v, float scalar);
	extern float *vector_assign(float *v1, const float *v2);
	extern int vector_equal(const float *v1, const float *v2);

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

	#define CENTROID(i) \
		(&centroids[(i)*DIMENSION_MAX])

	#define POINT(i) \
		(&points[(i)*DIMENSION_MAX])

	#define PCENTROID(i, j) \
		(&pcentroids[(i)*PROBLEM_NUM_CENTROIDS*DIMENSION_MAX + (j)*DIMENSION_MAX])

	#define PPOPULATION(i, j) \
		(&ppopulation[(i)*PROBLEM_NUM_CENTROIDS + (j)])

	extern void do_kernel();

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

	extern void srandnum(int seed);
	extern unsigned randnum(void);

#endif
