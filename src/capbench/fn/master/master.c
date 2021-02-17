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

/* Timing statistics. */
uint64_t master = 0;                 /* Time spent on master.       */
uint64_t spawn = 0;                  /* Time spent spawning slaves. */
uint64_t slave[PROBLEM_NUM_WORKERS]; /* Time spent on slaves.       */

/* FN Data. */
static struct item tasks[PROBLEM_SIZE];
static int tasksize[PROBLEM_NUM_WORKERS]; /* Tasks size. */
static int friendlyNumbers = 0;

static void init(void)
{
	int aux = PROBLEM_START_NUM;
	int avgtasksize = PROBLEM_SIZE/PROBLEM_NUM_WORKERS;

	for (int i = 0; i < PROBLEM_SIZE; i++)
		tasks[i].number = aux++;

	for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
		tasksize[i] = (i + 1 < PROBLEM_NUM_WORKERS)?avgtasksize:PROBLEM_SIZE-i*avgtasksize;
}

static void send_work(void)
{
	for (int i = 0, offset = 0; i < PROBLEM_NUM_WORKERS; i++)
	{
#if VERBOSE
		uprintf("Sending work to slave %d...", (i+1));
#endif /* VERBOSE */

		data_send(i + 1, &tasksize[i], sizeof(int));

#if VERBOSE
		uprintf("Sent first message...");
#endif /* VERBOSE */

		data_send(i + 1, &tasks[offset], tasksize[i]*sizeof(struct item));
		offset += tasksize[i];

#if VERBOSE
		uprintf("Sent!");
#endif /* VERBOSE */
	}
}

static void receive_result(void)
{
	for (int i = 0, offset = 0; i < PROBLEM_NUM_WORKERS; i++)
	{
#if VERBOSE
		uprintf("Waiting to receive results from slave %d...", (i+1));
#endif /* VERBOSE */

		data_receive(i + 1, &tasks[offset], tasksize[i]*sizeof(struct item));

#if VERBOSE
		uprintf("Received first message...");
#endif /* VERBOSE */

		data_receive(i + 1, &slave[i], sizeof(uint64_t));
		offset += tasksize[i];

#if VERBOSE
		uprintf("Received");
#endif /* VERBOSE */
	}
}

static void sum_friendly_numbers(void)
{
	perf_start(0, PERF_CYCLES);

	for (int i = 0; i < PROBLEM_SIZE; i++)
	{
		for (int j = i + 1; j < PROBLEM_SIZE; j++)
		{
			if (tasks[i].num == tasks[j].num && tasks[i].den == tasks[j].den)
				friendlyNumbers++;
		}
	}
	master += perf_read(0);
}

void do_master(void)
{
#if VERBOSE
	uprintf("Master process initializing data structures...");
#endif /* VERBOSE */

	init();

#if VERBOSE
	uprintf("Master process sending work...");
#endif /* VERBOSE */

	send_work();

#if VERBOSE
	uprintf("Master process waiting for results...");
#endif /* VERBOSE */

	receive_result();

#if VERBOSE
	uprintf("Master process calculating final result...");
#endif /* VERBOSE */

	sum_friendly_numbers();

	update_total(master + communication());
}
