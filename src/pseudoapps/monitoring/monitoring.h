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

#ifndef _MONITOR_UTILS_H
#define _MONITOR_UTILS_H

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/barrier.h>
#include <nanvix/sys/perf.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>

#include <posix/errno.h>
#include <posix/sys/ipc.h>

/*============================================================================*
 * Parameters                                                                 *
 *============================================================================*/

#define TEST_RPC_ID              0
#define TEST_RPC_NUM_PROCS      16
#define TEST_RPC_NUM_PROCS_COMM 14
#define TEST_RPC_ITERATIONS     10
#define TEST_RPC_RULE_SIZE      20 
#define TEST_INFOS_SIZE         (sizeof(struct infos)) 

#define TEST_MONIDOR_NUM    (PROCESSOR_NODENUM_LEADER)     //! [0    ]
#define TEST_STANDALONE_NUM (PROCESSOR_NODENUM_LEADER + 1) //! [15   ]
#define TEST_WORKER_NUM     (PROCESSOR_NODENUM_LEADER + 2) //! [1,  7]

/*============================================================================*
 * Structure                                                                  *
 *============================================================================*/


struct infos
{
	char rule[TEST_RPC_RULE_SIZE];
	int nodenum;
	uint64_t timestamp;
	uint64_t curr_nthreads;
	uint64_t total_nthreads;
	uint64_t mcreates;
	uint64_t mopens;
	uint64_t mreceived;
	uint64_t mtransferred;
	uint64_t pcreates;
	uint64_t popens;
	uint64_t preceived;
	uint64_t ptransferred;
};

/*============================================================================*
 * Functions                                                                  *
 *============================================================================*/

extern void do_standalone(void);
extern void do_worker(void);

extern void delay(int times, uint64_t cycles);
extern void dump_infos(struct infos * infos);
extern uint64_t kthread_get_curr_nthreads(void);
extern uint64_t kthread_get_total_nthreads(void);

#endif /* _MONITOR_UTILS_H */

