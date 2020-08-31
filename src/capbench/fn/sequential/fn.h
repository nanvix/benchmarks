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

#ifndef _FN2_H_
#define _FN2_H_

	#include <nanvix/sys/perf.h>
	#include <nanvix/hal.h>
	#include <nanvix/ulib.h>
	#include <nanvix/config.h>
	#include <nanvix/limits.h>

	#include <posix/stdint.h>
	#include <posix/stddef.h>

/*============================================================================*
 * Parameters                                                                 *
 *============================================================================*/

	#define PROBLEM_SIZE                                    28
	#define PROBLEM_START_NUM                          1000001
	#define PROBLEM_END_NUM      (PROBLEM_START + PROBLEM_SIZE)

/*============================================================================*
 * Statistics                                                                 *
 *============================================================================*/

	extern uint64_t total_time;

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

	extern void do_kernel(void);

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

	extern void srandnum(int seed);
	extern unsigned randnum(void);

#endif
