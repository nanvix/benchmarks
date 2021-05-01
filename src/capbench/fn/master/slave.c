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

/**
 * @brief Kernel Data
 */
/**@{*/
PRIVATE struct local_data
{
	struct item task[PROBLEM_SIZE/ACTIVE_CLUSTERS/PROCS_PER_CLUSTER_MAX*2];
} _local_data[PROCS_PER_CLUSTER_MAX-1];

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

static void send_result(struct item *data, int tasksize, int submaster_rank)
{
	uint64_t total_time;

#if DEBUG
	int rank;
	runtime_get_rank(&rank);
#endif /* DEBUG */

	total_time = total();

#if DEBUG
	uprintf("Slave %d Sending result to submaster...", rank);
#endif /* DEBUG */

	data_send(submaster_rank, data, tasksize*sizeof(struct item));

#if DEBUG
	uprintf("Slave %d Sending statistics back for master...", rank);
#endif /* DEBUG */

	data_send(0, &total_time, sizeof(uint64_t));
}

void do_slave(void)
{
	//struct item *task;
	int tasksize;
	int submaster_rank;
	uint64_t time_passed;
	int local_index;
	struct local_data *data;

	local_index = runtime_get_index() - 1;
	data = &_local_data[local_index];

#if VERBOSE
	int rank;
	runtime_get_rank(&rank);
#endif /* VERBOSE */

#if VERBOSE
	uprintf("Slave %d waiting to receive work...", rank);
#endif /* VERBOSE */

	/* Here we make a little trick to avoid problems with nanvix IPC. */
	submaster_rank = (cluster_get_num() - MPI_PROCESSES_COMPENSATION) * PROCS_PER_CLUSTER_MAX;

	if (submaster_rank == 0)
		submaster_rank++;

	/* Receive rank of submaster. */
	data_receive(submaster_rank, &submaster_rank, sizeof(int));

	/* Receive tasksize. */
	data_receive(submaster_rank, &tasksize, sizeof(int));

	/* Allocates data structure to hold the task items. */
	// if ((task = (struct item *) umalloc(sizeof(struct item) * tasksize)) == NULL)
	// 	upanic("Failed when allocating task structure");

	data_receive(submaster_rank, &data->task, tasksize*sizeof(struct item));

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

	send_result(data->task, tasksize, submaster_rank);

	/* Frees the allocated data structure. */
	// ufree(task);
}

