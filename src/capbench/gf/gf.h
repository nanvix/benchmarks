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

#ifndef _GF_H_
#define _GF_H_

	#include "../common.h"

/*============================================================================*
 * Parameters                                                                 *
 *============================================================================*/

	#define PROBLEM_SEED 0

	#define PROBLEM_MASKSIZE                                              (7)
	#define PROBLEM_CHUNK_SIZE                                           (32)
	#define PROBLEM_IMGSIZE     (PROBLEM_IMGDIMENSION + PROBLEM_MASKSIZE - 1)

	#define PROBLEM_IMGDIMENSION 256

	#define PROBLEM_IMGSIZE2           (PROBLEM_IMGSIZE * PROBLEM_IMGSIZE)
	#define PROBLEM_CHUNK_SIZE2  (PROBLEM_CHUNK_SIZE * PROBLEM_CHUNK_SIZE)
	#define PROBLEM_MASKSIZE2        (PROBLEM_MASKSIZE * PROBLEM_MASKSIZE)

	#define FPS     5
	#define SECONDS 3

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

	/* Type of messages. */
	#define MSG_CHUNK 2
	#define MSG_DIE   1

	/**
	 * @brief Standard Deviation for Mask
	 */
	#define SD 0.8

	#define CHUNK_WITH_HALO_SIZE    (PROBLEM_CHUNK_SIZE + PROBLEM_MASKSIZE - 1)
	#define CHUNK_WITH_HALO_SIZE2 (CHUNK_WITH_HALO_SIZE * CHUNK_WITH_HALO_SIZE)

	extern void do_master(void);
	extern void do_slave(void);

#endif /* _GF_H_ */

