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
#include <nanvix/kernel/kernel.h>

/*
 * Computes the Greatest Common Divisor of two numbers.
 */
static int gcd(int a, int b)
{
	struct division result;
	int i = a;
	int mod;

	while (b != 0 && i > 0)
	{
		result = divide(a, b);
		mod = result.remainder;
		a = b;
		b = mod;
		i--;
	}

	return a;
}

static int sumdiv(int n)
{
	int sum;    /* Sum of divisors.     */
	int factor; /* Working factor.      */
	int maxD; 	/* Max divisor before n */

	/* Initialization to avoid problems with n == 0. */
	sum = 0;

	maxD = (int) (n / 2);
	sum = (n == 1) ? 1 : (1 + n);

	/* Compute sum of divisors. */
	for (factor = 2; factor <= maxD; factor++)
	{
		struct division result;

		result = divide(n, factor);

		if (result.remainder == 0)
			sum += factor;
	}

	return (sum);
}

static void send_result(struct item *data, int tasksize)
{
	uint64_t total_time;

#if DEBUG
	int rank;
	runtime_get_rank(&rank);
#endif /* DEBUG */

	total_time = total();

#if DEBUG
	uprintf("Slave %d Sending first message...", rank);
#endif /* DEBUG */

	data_send(0, data, tasksize*sizeof(struct item));

#if DEBUG
	uprintf("Slave %d Sending second message...", rank);
#endif /* DEBUG */

	data_send(0, &total_time, sizeof(uint64_t));

#if DEBUG
	uprintf("Slave %d Sent results!...", rank);
#endif /* DEBUG */
}

void do_slave(void)
{
	struct item *task;
	int tasksize;
	uint64_t time_passed;

#if VERBOSE
	int rank;
	runtime_get_rank(&rank);
#endif /* VERBOSE */

#if VERBOSE
	uprintf("Slave %d waiting to receive work...", rank);
#endif /* VERBOSE */

	/* Receive tasksize. */
	data_receive(0, &tasksize, sizeof(int));

	/* Allocates data structure to hold the task items. */
	if ((task = (struct item *) umalloc(sizeof(struct item) * tasksize)) == NULL)
		upanic("Failed when allocating task structure");

	data_receive(0, task, tasksize*sizeof(struct item));

#if VERBOSE
	uprintf("Slave %d starting computation of %d numbers...", rank, tasksize);
#endif /* VERBOSE */

	perf_start(0, PERF_CYCLES);

	/* Compute abundances. */
	for (int i = 0; i < tasksize; i++)
	{
		int n;

#if DEBUG
		uprintf("Slave %d computing abundance of %dth number", rank, i);
#endif

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

	time_passed = perf_read(0);
	update_total(time_passed);

#if VERBOSE
	uprintf("Slave %d preparing to send result back...", rank);
#endif /* VERBOSE */

	send_result(task, tasksize);

	/* Frees the allocated data structure. */
	ufree(task);
}

