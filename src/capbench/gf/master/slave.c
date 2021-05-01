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

#define MASK(data, i, j) \
	data->mask[(i)*PROBLEM_MASKSIZE + (j)]

#define CHUNK(data, i, j) \
	data->chunk[(i)*(PROBLEM_CHUNK_SIZE + PROBLEM_MASKSIZE - 1) + (j)]

#define NEWCHUNK(data, i, j) \
	data->newchunk[(i)*PROBLEM_CHUNK_SIZE + (j)]

/**
 * @brief Kernel Data
 */
/**@{*/
PRIVATE struct local_data
{
	float mask[PROBLEM_MASKSIZE2];               /* Mask         */
	unsigned char chunk[CHUNK_WITH_HALO_SIZE2];  /* Input Chunk  */
	unsigned char newchunk[PROBLEM_CHUNK_SIZE2]; /* Output Chunk */
	int submaster_rank;
} _local_data[PROCS_PER_CLUSTER_MAX-1];
/**@}*/

static void init(struct local_data *data)
{
	/* Here we make a little trick to avoid problems with nanvix IPC. */
	int master_rank = (cluster_get_num() - MPI_PROCESSES_COMPENSATION) * PROCS_PER_CLUSTER_MAX;

	if (master_rank == 0)
		master_rank++;

	/* Receive rank of submaster. */
	data_receive(master_rank, &data->submaster_rank, sizeof(int));

	/* Receive mask. */
	data_receive(master_rank, &data->mask, sizeof(float)*PROBLEM_MASKSIZE2);
}

/**
 * @brief Gaussian Filter kernel.
 */
void gauss_filter(struct local_data *data)
{
	uint64_t time_elapsed;

	perf_start(0, PERF_CYCLES);

	for (int chunkI = 0; chunkI < PROBLEM_CHUNK_SIZE; chunkI++)
	{
		for (int chunkJ = 0; chunkJ < PROBLEM_CHUNK_SIZE; chunkJ++)
		{
			float pixel = 0.0;

			for (int maskI = 0; maskI < PROBLEM_MASKSIZE; maskI++)
			{
				for (int maskJ = 0; maskJ < PROBLEM_MASKSIZE; maskJ++)
					pixel += CHUNK(data, chunkI + maskI, chunkJ + maskJ)*MASK(data, maskI, maskJ);
			}

			NEWCHUNK(data, chunkI, chunkJ) = (pixel > 255) ? 255 : (unsigned char) pixel;
		}
	}

	time_elapsed = perf_read(0);
	update_total(time_elapsed);
}

/**
 * @brief Kernel wrapper.
 */
void do_slave(void)
{
	int local_index;
	uint64_t total_time;
	struct local_data *data;

#if DEBUG
	int repetition = 0;
	int rank;
	runtime_get_rank(&rank);
#endif /* DEBUG */

	local_index = runtime_get_index() - 1;
	data = &_local_data[local_index];

#if DEBUG
	uprintf("Rank %d receiving initialization...", rank);
#endif

	init(data);

	mpi_std_barrier();

	/* Applies the gaussian filter until a DIE message. */
	while (1)
	{
		int msg = 0;

#if DEBUG
		uprintf("Rank %d receiving control message...", rank);
#endif /* DEBUG */

		data_receive(data->submaster_rank, &msg, sizeof(int));

		if (msg == MSG_DIE)
			break;

#if DEBUG
		uprintf("Rank %d receiving new chunk...", rank);
#endif /* DEBUG */

		data_receive(data->submaster_rank, data->chunk, CHUNK_WITH_HALO_SIZE2*sizeof(unsigned char));

#if DEBUG
		uprintf("Rank %d applying filter for image %d...", rank, ++repetition);
#endif /* DEBUG */

		gauss_filter(data);

#if DEBUG
		uprintf("Rank %d sending chunk %d back...", rank, repetition);
#endif /* DEBUG */

		data_send(data->submaster_rank, &data->newchunk, PROBLEM_CHUNK_SIZE2*sizeof(unsigned char));
	}

#if DEBUG
	uprintf("Rank %d received DIE message...", rank);
#endif /* DEBUG */

	/* Send back statistics. */
	total_time = total();
	data_send(0, &total_time, sizeof(uint64_t));
}

