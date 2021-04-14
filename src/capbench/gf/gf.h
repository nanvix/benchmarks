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

#ifndef _GF_H_
#define _GF_H_

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
	 * @brief Enables benchmark verbose mode.
	 */
	#define VERBOSE (DEBUG || 0)

	#define DEBUG 0

/*============================================================================*
 * Parameters                                                                 *
 *============================================================================*/

	#define PROBLEM_SEED 0

	#define PROBLEM_MASKSIZE                              7
	#define PROBLEM_CHUNK_SIZE                           32
	#define PROBLEM_IMGSIZE     (PROBLEM_IMGDIMENSION + PROBLEM_MASKSIZE - 1)

	#define PROBLEM_IMGDIMENSION 256

	#define PROBLEM_NUM_WORKERS (MPI_PROCESSES_NR - 1)

	#define PROBLEM_IMGSIZE2           (PROBLEM_IMGSIZE*PROBLEM_IMGSIZE)
	#define PROBLEM_CHUNK_SIZE2  (PROBLEM_CHUNK_SIZE*PROBLEM_CHUNK_SIZE)
	#define PROBLEM_MASKSIZE2        (PROBLEM_MASKSIZE*PROBLEM_MASKSIZE)

	#define FPS     5
	#define SECONDS 3

/*============================================================================*
 * Communication                                                              *
 *============================================================================*/

	extern uint64_t data_send(int outfd, void *data, size_t n);
	extern uint64_t data_receive(int infd, void *data, size_t n);

/*============================================================================*
 * Statistics                                                                 *
 *============================================================================*/

	/**
	 * @brief Timing Statistics
	 */
	/**@{*/
	extern uint64_t master;
	extern uint64_t slave[PROBLEM_NUM_WORKERS];
	/**@}*/

	extern uint64_t total();
	extern void update_total(uint64_t amnt);
	extern uint64_t communication();
	extern void update_communication(uint64_t amnt);
	extern size_t data_sent();
	extern size_t data_received();
	extern unsigned nsend();
	extern unsigned nreceive();

/*============================================================================*
 * Math                                                                       *
 *============================================================================*/

	/**
	 * @brief Math Constants
	 */
	/**@{*/
	#define PI 3.14159265359    /* pi */
	#define E 2.71828182845904  /* e */
	/**@}*/

	/**
	 * @brief Resulft for Integer Division
	 */
	struct division
	{
		int quotient;
		int remainder;
	};

	#ifdef __mppa256__
	#define __fdiv(a, b) __builtin_k1_fsdiv(a, b)
	#else
	#define __fdiv(a, b) ((a)/(b))
	#endif

	extern struct division divide(int a, int b);
	extern float power(float base, float ex);

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

	/* Type of messages. */
	#define MSG_CHUNK 2
	#define MSG_DIE   1

	/**
	 * @brief Standard Deviation for Mask
	 */
	#define SD 0.8

	#define CHUNK_WITH_HALO_SIZE (PROBLEM_CHUNK_SIZE + PROBLEM_MASKSIZE - 1)
	#define CHUNK_WITH_HALO_SIZE2 (CHUNK_WITH_HALO_SIZE*CHUNK_WITH_HALO_SIZE)

	extern void do_master(void);
	extern void do_slave(void);

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

	extern void srandnum(int seed);
	extern unsigned randnum(void);

#endif
