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

	#include "../common.h"

/*============================================================================*
 * Parameters                                                                 *
 *============================================================================*/

	#define PROBLEM_SEED                                                   (0)
	#define DIMENSION_MAX                                                  (2)
	#define PROBLEM_NUM_CENTROIDS                                        (128)
	#define PROBLEM_NUM_POINTS                     ((13440) * ACTIVE_CLUSTERS)
	#define PROBLEM_LNPOINTS        (PROBLEM_NUM_POINTS / PROBLEM_NUM_WORKERS)

/*============================================================================*
 * Vector                                                                     *
 *============================================================================*/

	extern float vector_distance(float *a, float *b);
	extern float *vector_add(float *v1, const float *v2);
	extern float *vector_mult(float *v, float scalar);
	extern float *vector_assign(float *v1, const float *v2);
	extern int vector_equal(const float *v1, const float *v2);

#endif /* _KM_H_ */

