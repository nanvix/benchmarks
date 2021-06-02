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

#ifndef _ALL_H_
#define _ALL_H_

	#include "../common.h"

/*============================================================================*
 * Parameters                                                                 *
 *============================================================================*/

	#define PROBLEM_ITERATIONS                    (100)
	#define PROBLEM_SIZE_MIN                      (128)
	#define PROBLEM_SIZE_MAX                (32 * 1024)
	#define PROBLEM_SIZE_STEP (previous) (2 * previous)

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

	EXTERN void do_work(int rank, size_t size);
	EXTERN void all_barrier_setup(int rank);
	EXTERN void all_barrier(void);
	EXTERN void all_barrier_destroy(int rank);

#endif /* _ALL_H_ */

