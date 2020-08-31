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

#include <posix/stdlib.h>

#include "fn.h"

static int _friendly_numbers = 0; /* Number of friendly numbers. */

static struct item *task;         /* Array of items (task).      */

/*============================================================================*
 * Statistics                                                                 *
 *============================================================================*/

uint64_t total_time = 0;

/*============================================================================*
 * Functions                                                                  *
 *============================================================================*/

/**
 * @brief Initializes the task array.
 */
static void init_task()
{
	int aux = PROBLEM_START_NUM;

	for (int i = 0; i < PROBLEM_SIZE; ++i)
		task[i].number = aux++;
}

/**
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b)
{
	struct division result;
	int mod;

	while (b != 0)
	{
		result = divide(a, b);
		mod = result.remainder;
		a = b;
		b = mod;
	}

	return (a);
}

static int sumdiv(int n)
{
	int sum;    /* Sum of divisors.     */
	int factor; /* Working factor.      */
	int maxD; 	/* Max divisor before n */
	struct division result;

	maxD = (int)n/2;

	sum = (n == 1) ? 1 : 1 + n;

	/* Compute sum of divisors. */
	for (factor = 2; factor <= maxD; factor++)
	{
		result = divide(n, factor);
		if (result.remainder == 0)
			sum += factor;
	}

	return (sum);
}

/**
 * @brief Compute abundances for task array.
 */
static void compute_abundances()
{
	int n;

	for (int i = 0; i < PROBLEM_SIZE; ++i)
	{
		task[i].num = sumdiv(task[i].number);
		task[i].den = task[i].number;

		n = gcd(task[i].num, task[i].den);

		if (n != 0)
		{
			struct division result1 = divide(task[i].num, n);
			struct division result2 = divide(task[i].den, n);
			task[i].num = result1.quotient;
			task[i].den = result2.quotient;
		}
	}
}

/**
 * @brief Counts the friendly numbers.
 *
 * @returns Returns the number of friendly numbers in @p task.
 */
static void sum_friendly_numbers()
{
	for (int i = 0; i < PROBLEM_SIZE; i++)
	{
		for (int j = (i + 1); j < PROBLEM_SIZE; j++)
		{
			if ((task[i].num == task[j].num) && (task[i].den == task[j].den))
				_friendly_numbers++;
		}
	}
}

/**
 * @brief Friendly Numbers sequential solution.
 */
void do_kernel(void)
{
	/* Allocates task array. */
	if ((task = (struct item *) nanvix_malloc(sizeof(struct item) * PROBLEM_SIZE)) == NULL)
	{
		upanic("Error in task allocation.");
		return;
	}

	/* Initializes task array. */
	init_task();

	/* Starts to count total time. */
	perf_start(0, PERF_CYCLES);

	/* Compute abundances. */
	compute_abundances();

	/* Gets the number of friendly numbers. */
	sum_friendly_numbers();

	total_time += perf_read(0);

	/* Frees the allocated memory. */
	nanvix_free((void *) task);

	/* Prints the result. */
	uprintf("result: %d", _friendly_numbers);
}
