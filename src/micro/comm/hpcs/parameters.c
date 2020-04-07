/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
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

#include "parameters.h"

#if (__TARGET_HAS_PORTAL && __TARGET_HAS_MAILBOX)

/**
 * @name Configuration of the routines (Involved clusters)
 */
/**@{*/
#if __DO_COMM_SATURATION
	#define _NUM_NODES 17
	const int _nodenums[_NUM_NODES] ALIGN(sizeof(uint64_t)) = {
	#ifdef __mppa256__
			0, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
	#else
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
	#endif
		};
#else
	#define _NUM_NODES 2
	const int _nodenums[_NUM_NODES] ALIGN(sizeof(uint64_t)) = {
	#ifdef __mppa256__
			0, 8
	#else
			0, 1
	#endif
		};
#endif
/**@}*/

/**
 * @name Configuration exported
 */
/**@{*/
const int NUM_NODES = _NUM_NODES;
const int MESSAGE_SIZE = (1 * KB);
const int * nodenums = &_nodenums[0];
/**@}*/

/**
 * @name Collective communication routines functions
 */
/**@{*/
extern int do_throughput(const int *, int, int, int);
extern int do_latency(const int *, int, int, int);
extern int do_remote_operations(const int *, int, int, int);
extern int do_comm_saturation(const int *, int, int, int);
/**@}*/

/**
 * @brief Executable routine
 */
int (*enabled_routine_fn)(const int *, int, int, int) =
#if __DO_COMM_SATURATION
	&do_comm_saturation;
#elif __DO_REMOTE_OPERATIONS
	&do_remote_operations;
#elif __DO_THROUGHPUT
	&do_throughput;
#else
	&do_latency;
#endif

uint64_t portal_sizes[NUM_PORTAL_SIZE] = {
	1*KB, 2*KB, 4*KB, 8*KB, 16*KB, 32*KB, 64*KB, 128*KB, 256*KB
};

uint64_t mailbox_sizes[NUM_MAILBOX_SIZE] = {64, 128, 256, 512, 1024};

#endif /* #if __TARGET_HAS_PORTAL && __TARGET_HAS_MAILBOX */
