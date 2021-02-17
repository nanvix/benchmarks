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

#ifndef _FN_H_
#define _FN_H_

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
	#define VERBOSE 0

/*============================================================================*
 * Parameters                                                                 *
 *============================================================================*/

	#define PROBLEM_SIZE                                   128
	#define PROBLEM_START_NUM                          1000001
	#define PROBLEM_END_NUM      (PROBLEM_START + PROBLEM_SIZE)
	#define PROBLEM_NUM_WORKERS          (MPI_PROCESSES_NR - 1)

	#define CHUNK_MAX_SIZE ((PROBLEM_SIZE / PROBLEM_NUM_WORKERS) * 2)

/*============================================================================*
 * Communication                                                              *
 *============================================================================*/

	extern uint64_t data_send(int outfd, void *data, size_t n);
	extern uint64_t data_receive(int infd, void *data, size_t n);

/*============================================================================*
 * Statistics                                                                 *
 *============================================================================*/

	extern uint64_t slave[PROBLEM_NUM_WORKERS];
	extern uint64_t master;

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

	struct division
	{
		int quotient;
		int remainder;
	};

	extern struct division divide(int a, int b);
	extern float power(float base, float ex);

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

	struct item
	{
		int number;
		int num;
		int den;
	};

	extern void do_master(void);
	extern void do_slave(void);

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

	extern void srandnum(int seed);
	extern unsigned randnum(void);

#endif
