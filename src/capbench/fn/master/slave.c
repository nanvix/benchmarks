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
PRIVATE struct local_data
{
	struct item task[CHUNK_MAX_SIZE];
	int tasksize;
} _local_data[MPI_PROCS_PER_CLUSTER_MAX];

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

static void get_work(struct local_data *data)
{
	data_receive(0, &data->tasksize, sizeof(int));
	data_receive(0, &data->task, data->tasksize*sizeof(struct item));
}

static void send_result(struct local_data *data)
{
	uint64_t total_time;

	total_time = total();

	data_send(0, &data->task, data->tasksize*sizeof(struct item));
	data_send(0, &total_time, sizeof(uint64_t));
}

void do_slave(void)
{
	struct local_data *data;
	uint64_t time_passed;

#if VERBOSE
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif /* VERBOSE */

	data = &_local_data[curr_mpi_proc_index()];

#if VERBOSE
	uprintf("Slave %d waiting to receive work...", rank);
#endif /* VERBOSE */

	get_work(data);

#if VERBOSE
	uprintf("Slave %d starting computation...", rank);
#endif /* VERBOSE */

	perf_start(0, PERF_CYCLES);

	/* Compute abundances. */
	for (int i = 0; i < data->tasksize; i++)
	{
		int n;

		data->task[i].num = sumdiv(data->task[i].number);
		data->task[i].den = data->task[i].number;

		n = gcd(data->task[i].num, data->task[i].den);

		if (n != 0)
		{
			struct division result1 = divide(data->task[i].num, n);
			struct division result2 = divide(data->task[i].den, n);
			data->task[i].num = result1.quotient;
			data->task[i].den = result2.quotient;
		}
	}

	time_passed = perf_read(0);
	update_total(time_passed);

#if VERBOSE
	uprintf("Slave %d preparing to send result back...", rank);
#endif /* VERBOSE */

	send_result(data);
}
