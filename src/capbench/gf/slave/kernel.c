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

#include "../gf.h"

/**
 * @brief Kernel Data
 */
/**@{*/
static float mask[PROBLEM_MASKSIZE2];               /* Mask         */
static unsigned char chunk[CHUNK_WITH_HALO_SIZE2];  /* Input Chunk  */
static unsigned char newchunk[PROBLEM_CHUNK_SIZE2]; /* Output Chunk */
/**@}*/

/**
 * @brief Gaussian Filter kernel.
 */
void gauss_filter(void)
{
	perf_start(0, PERF_CYCLES);

	for (int chunkI = 0; chunkI < PROBLEM_CHUNK_SIZE; chunkI++)
	{
		for (int chunkJ = 0; chunkJ < PROBLEM_CHUNK_SIZE; chunkJ++)
		{
			float pixel = 0.0;

			for (int maskI = 0; maskI < PROBLEM_MASKSIZE; maskI++)
			{
				for (int maskJ = 0; maskJ < PROBLEM_MASKSIZE; maskJ++)
					pixel += CHUNK(chunkI + maskI, chunkJ + maskJ)*MASK(maskI, maskJ);
			}

			NEWCHUNK(chunkI, chunkJ) = (pixel > 255) ? 255 : (unsigned char) pixel;
		}
	}

	total += perf_read(0);
}

/**
 * @brief Kernel wrapper.
 */
void do_kernel(void)
{
	/* Receive kernel parameters. */
	data_receive(0, mask, sizeof(float)*PROBLEM_MASKSIZE2);

	while (1)
	{
		int msg = 0;

		data_receive(0, &msg, sizeof(int));

		if (msg == MSG_DIE)
			break;

		data_receive(0, chunk, CHUNK_WITH_HALO_SIZE2*sizeof(unsigned char));

		gauss_filter();

		data_send(0, &newchunk, PROBLEM_CHUNK_SIZE2*sizeof(unsigned char));
	}

	/* Send back statistics. */
	data_send(0, &total, sizeof(uint64_t));
}
