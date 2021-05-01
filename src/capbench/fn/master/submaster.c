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

/* FN Data. */
static struct item tasks[PROBLEM_SIZE/ACTIVE_CLUSTERS];
static int tasksize[PROCS_PER_CLUSTER_MAX]; /* Tasks size. */

static int my_rank;
static int slaves;
static int numbers_num;

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

static void receive_work(void)
{
	/* Receive util information. */
	data_receive(0, &slaves, sizeof(int));
	data_receive(0, &numbers_num, sizeof(int));

	/* Receive task. */
	data_receive(0, &tasks, numbers_num*sizeof(struct item));

	int avgtasksize = numbers_num/slaves;

	for (int i = 0; i < slaves; i++)
		tasksize[i] = (i + 1 < slaves)?avgtasksize:numbers_num-i*avgtasksize;
}

static void send_work(void)
{
	int rank;

	for (int i = 1, offset = tasksize[0]; i < slaves; i++)
	{
		rank = my_rank + i;

#if DEBUG
		uprintf("Submaster sending work to slave %d...", rank);
#endif /* DEBUG */

		/* Util information. */
		data_send(rank, &my_rank, sizeof(int));

		/* Sends task. */
		data_send(rank, &tasksize[i], sizeof(int));
		data_send(rank, &tasks[offset], tasksize[i]*sizeof(struct item));
		offset += tasksize[i];

#if DEBUG
		uprintf("Submaster Sent!");
#endif /* DEBUG */
	}
}

static void process_chunk(void)
{
	uint64_t time_passed;

#if VERBOSE
	uprintf("Submaster starting computation of %d numbers...", tasksize[0]);
#endif /* VERBOSE */

	perf_start(0, PERF_CYCLES);

	/* Compute abundances. */
	for (int i = 0; i < tasksize[0]; i++)
	{
		int n;

#if DEBUG
		uprintf("Submaster computing abundance of %dth number", i);
#endif

		tasks[i].num = sumdiv(tasks[i].number);
		tasks[i].den = tasks[i].number;

		n = gcd(tasks[i].num, tasks[i].den);

		if (n != 0)
		{
			struct division result1 = divide(tasks[i].num, n);
			struct division result2 = divide(tasks[i].den, n);
			tasks[i].num = result1.quotient;
			tasks[i].den = result2.quotient;
		}
	}

	time_passed = perf_read(0);
	update_total(time_passed);
}

static void receive_result(void)
{
	int rank;

	for (int i = 1, offset = tasksize[0]; i < slaves; i++)
	{
		rank = my_rank + i;

#if DEBUG
		uprintf("Submaster waiting to receive results from slave %d...", rank);
#endif /* DEBUG */

		data_receive(rank, &tasks[offset], tasksize[i]*sizeof(struct item));
		offset += tasksize[i];
	}
}

static void send_results(void)
{
	uint64_t total_time;

	total_time = total();

	data_send(0, &tasks, numbers_num*sizeof(struct item));
	data_send(0, &total_time, sizeof(uint64_t));
}

void do_submaster(void)
{
	runtime_get_rank(&my_rank);

#if VERBOSE
	uprintf("Submaster waiting to receive work from master...");
#endif /* VERBOSE */

	receive_work();

#if VERBOSE
	uprintf("Submaster sending work to slaves...");
#endif /* VERBOSE */

	send_work();

#if VERBOSE
	uprintf("Submaster calculating friendly numbers...");
#endif /* VERBOSE */

	process_chunk();

#if VERBOSE
	uprintf("Submaster waiting for results...");
#endif /* VERBOSE */

	receive_result();

#if VERBOSE
	uprintf("Submaster sending final results back to master...");
#endif /* VERBOSE */

	send_results();

	//update_total(master + communication());
}

