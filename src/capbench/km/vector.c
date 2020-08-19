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

#include "km.h"

/*============================================================================*
 * vector_distance()                                                          *
 *============================================================================*/

/*
 * Calculates the distance between two points.
 */
float vector_distance(float *a, float *b)
{
	float distance = 0;

	for (int i = 0; i < DIMENSION_MAX; i++)
		distance += (a[i] - b[i])*(a[i] - b[i]);

	return (distance);
}

/*============================================================================*
 * vector_add()                                                               *
 *============================================================================*/

/*
 * Adds two vectors.
 */
float *vector_add(float *v1, const float *v2)
{
	for (int i = 0; i < DIMENSION_MAX; i++)
		v1[i] += v2[i];

	return (v1);
}

/*============================================================================*
 * vector_mult()                                                              *
 *============================================================================*/

/*
 * Multiplies a vector by a scalar.
 */
float *vector_mult(float *v, float scalar)
{
	for (int i = 0; i < DIMENSION_MAX; i++)
		v[i] *= scalar;

	return (v);
}

/*============================================================================*
 * vector_assign()                                                            *
 *============================================================================*/

/*
 * Assigns a vector to another.
 */
float *vector_assign(float *v1, const float *v2)
{
	for (int i = 0; i < DIMENSION_MAX; i++)
		v1[i] = v2[i];

	return (v1);
}

/*============================================================================*
 * vector_equal()                                                             *
 *============================================================================*/

/*
 * Tests if two vectors are equal.
 */
int vector_equal(const float *v1, const float *v2)
{
	for (int i = 0; i < DIMENSION_MAX; i++)
	{
		float val = (v1[i] - v2[i]);
		if (val < 0)
			val = -1;

		if (val <= 0.0000001)
			return (0);
	}

	return (1);
}
