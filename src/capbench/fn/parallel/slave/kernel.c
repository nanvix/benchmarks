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

#include "../fn.h"

/* Task sent by IO */
static struct item task[PROBLEM_SIZE];

/* Informations about the task */
static int tasksize;

/*
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b)
{
	while (b != 0)
	{
		struct division result = divide(a, b);
		int mod = result.remainder;
		a = b;
		b = mod;
	}

	return a;
}

static int sumdiv(int n)
{
	int sum;    /* Sum of divisors.     */
	int factor; /* Working factor.      */
	int maxD; 	/* Max divisor before n */

	maxD = (int)n/2;

	sum = (n == 1) ? 1 : 1 + n;

	/* Compute sum of divisors. */
	for (factor = 2; factor <= maxD; factor++)
	{
		struct division result = divide(n, factor);
		if (result.remainder == 0)
			sum += factor;
	}
	return (sum);
}

static void get_work(void)
{
    data_receive(0, &tasksize, sizeof(int));
	data_receive(0, &task, tasksize*sizeof(struct item));
}

static void send_result(void)
{
    data_send(0, &task, tasksize*sizeof(struct item));
	data_send(0, &total, sizeof(uint64_t));

}

void do_kernel(void)
{
	get_work();

	perf_start(0, PERF_CYCLES);

	/* Compute abundances. */
	for (int i = 0; i < tasksize; i++)
	{
		int n;

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

	total += perf_read(0);

	send_result();
}
