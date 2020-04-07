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

#include "../comm.h"

#ifndef __HPCS__

/**
 * @brief Executable routine
 */
#define __DO_LATENCY            0
#define __DO_COMM_SATURATION    0
#define __DO_REMOTE_OPERATIONS  0
#define __DO_THROUGHPUT         1

#if ((__DO_LATENCY + __DO_COMM_SATURATION + __DO_REMOTE_OPERATIONS + __DO_THROUGHPUT) != 1)
#error "Set only one of executable routine (use 1 to setting)"
#endif

/**
 * @brief Maximum portal size
 */
#define HPCS_MAX_MSG_SIZE (256 * KB)
#define NUM_PORTAL_SIZE 9
#define NUM_MAILBOX_SIZE 5

extern uint64_t portal_sizes[NUM_PORTAL_SIZE];
extern uint64_t mailbox_sizes[NUM_MAILBOX_SIZE];

#endif
