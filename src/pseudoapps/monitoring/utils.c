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

#include "monitoring.h"

/*============================================================================*
 * Delay                                                                      *
 *============================================================================*/

/**
 * @brief Forces a platform-independent delay.
 */
void delay(int times, uint64_t cycles)
{
	uint64_t t0, t1;

	for (int i = 0; i < times; ++i)
	{
		kclock(&t0);

		do
			kclock(&t1);
		while ((t1 - t0) < cycles);
	}
}

/*============================================================================*
 * Dump infos                                                                 *
 *============================================================================*/

/**
 * @brief Dump a info structure.
 */
void dump_infos(struct infos * infos)
{
	uprintf("------------------------------------------------------------------");
	uprintf("Monitoring Node %d:",                        infos->nodenum);
	uprintf("\tRule:                                 %s", infos->rule);
	uprintf("\tTimestamp:                            %l", infos->timestamp);
	uprintf("\tThreads:");
	uprintf("\t\tCurrent number of threads:    %l",       infos->curr_nthreads);
	uprintf("\t\tTotal of threads created:     %l",       infos->total_nthreads);
	uprintf("\tMailbox:");
	uprintf("\t\tNumber of connectors created: %l",       infos->mcreates);
	uprintf("\t\tNumber of connectors opened:  %l",       infos->mopens);
	uprintf("\t\tAmount of data received       %l bytes", infos->mreceived);
	uprintf("\t\tAmount of data transferred    %l bytes", infos->mtransferred);
	uprintf("\tPortal:");
	uprintf("\t\tNumber of connectors created: %l",       infos->pcreates);
	uprintf("\t\tNumber of connectors opened:  %l",       infos->popens);
	uprintf("\t\tAmount of data received       %l bytes", infos->preceived);
	uprintf("\t\tAmount of data transferred    %l bytes", infos->ptransferred);
	uprintf("------------------------------------------------------------------");
	uprintf("");
}

/*============================================================================*
 * kthread_get_curr_nthreads                                                  *
 *============================================================================*/

/**
 * @brief Workaround to get the number of active threads.
 */
extern int nthreads;
uint64_t kthread_get_curr_nthreads(void)
{
	dcache_invalidate();
	return (nthreads);
}

/*============================================================================*
 * kthread_get_total_nthreads                                                 *
 *============================================================================*/

/**
 * @brief Workaround to get the number of active threads.
 */
extern int next_tid;
uint64_t kthread_get_total_nthreads(void)
{
	dcache_invalidate();
	return (next_tid);
}
