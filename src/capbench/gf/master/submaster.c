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

#define MASK(i, j) \
	mask[(i)*PROBLEM_MASKSIZE + (j)]

#define CHUNK(i, j) \
	chunk[(i)*(PROBLEM_CHUNK_SIZE + PROBLEM_MASKSIZE - 1) + (j)]

#define NEWCHUNK(i, j) \
	newchunk[(i)*PROBLEM_CHUNK_SIZE + (j)]

#define SLAVE_RANK(i) (my_rank + i + 1)

/**
 * @brief Kernel Data
 */
/**@{*/
static unsigned char img[PROBLEM_IMGSIZE2];        /* Input Image   */
static unsigned char newimg[PROBLEM_IMGSIZE2];     /* Output Image  */
static float mask[PROBLEM_MASKSIZE2];              /* Mask          */
static unsigned char chunk[CHUNK_WITH_HALO_SIZE2]; /* Working Chunk */
/**@}*/

static int slaves;
static int my_rank;

#define HALF (PROBLEM_MASKSIZE/2)

#define CHUNKS_PER_IMAGE (PROBLEM_IMGDIMENSION2 / PROBLEM_CHUNK_SIZE2)

static void init(void)
{
	int rank;

	runtime_get_rank(&my_rank);

	/* Receive mask and num of slaves from master. */
	data_receive(0, &slaves, sizeof(int));
	data_receive(0, &mask, sizeof(float)*PROBLEM_MASKSIZE2);

	/* Broadcasts initialization messages for slaves. */
	for (int i = 0; i < slaves; ++i)
	{
		rank = SLAVE_RANK(i);

		/* Util information for the problem. */
		data_send(rank, &my_rank, sizeof(int));

		/* Sends mask for slaves. */
		data_send(rank, &mask, sizeof(float)*PROBLEM_MASKSIZE2);
	}
}

static void process_chunks(void)
{
	int ii;
	int jj;
	int ck;
	int msg;
	int rank;
	uint64_t elapsed_time;
	int current_chunk, received_chunks;

	msg = MSG_CHUNK;
	current_chunk = 0;
	received_chunks = 0;
	elapsed_time = 0;

	while (current_chunk < CHUNKS_PER_IMAGE)
	{
		ii = 0;
		jj = 0;

		for (int i = HALF; i < PROBLEM_IMGSIZE - HALF; i += PROBLEM_CHUNK_SIZE)
		{
			for (int j = HALF; j < PROBLEM_IMGSIZE - HALF; j += PROBLEM_CHUNK_SIZE)
			{
				ck = current_chunk % slaves;

				rank = SLAVE_RANK(ck);

				if (current_chunk >= slaves)
				{
#if DEBUG
					uprintf("Submaster Receiving chunk %d from rank %d", received_chunks, rank);
#endif
					/* Receives a chunk from the slave. */
					data_receive(rank, &chunk, PROBLEM_CHUNK_SIZE2*sizeof(unsigned char));
					
					perf_start(0, PERF_CYCLES);

						for (int k = 0; k < PROBLEM_CHUNK_SIZE; k++)
						{
							umemcpy(
								&newimg[(ii + k)*PROBLEM_IMGSIZE + jj],
								&chunk[k*PROBLEM_CHUNK_SIZE],
								PROBLEM_CHUNK_SIZE*sizeof(unsigned char)
							);
						}

						jj += PROBLEM_CHUNK_SIZE;
						if ((jj + PROBLEM_MASKSIZE - 1) == PROBLEM_IMGSIZE)
						{
							jj = 0;
							ii += PROBLEM_CHUNK_SIZE;
						}

					elapsed_time += perf_read(0);

					received_chunks++;
				}

				perf_start(0, PERF_CYCLES);

					/* Build chunk. */
					for (int k = 0; k < CHUNK_WITH_HALO_SIZE; k++)
					{
						umemcpy(
							&chunk[k*CHUNK_WITH_HALO_SIZE],
							&img[(i - HALF + k)*PROBLEM_IMGSIZE + j - HALF],
							CHUNK_WITH_HALO_SIZE*sizeof(unsigned char)
						);
					}

				elapsed_time += perf_read(0);

#if DEBUG
				uprintf("Submaster sending chunk %d to rank %d", current_chunk, rank);
#endif
				/* Sending chunk to slave. */
				data_send(rank, &msg, sizeof(int));
				data_send(rank, chunk, CHUNK_WITH_HALO_SIZE2*sizeof(unsigned char));

				/* Remaining chunks of this image need a receive before. */
				current_chunk++;
			}
		}
	}

	/* Get remaining chunks. */
	while (received_chunks < CHUNKS_PER_IMAGE)
	{
		ck = ++received_chunks % slaves;

		rank = SLAVE_RANK(ck);

#if DEBUG
		uprintf("Submaster Receiving last chunk (%d) from rank %d", received_chunks-1, rank);
#endif

		data_receive(rank, &chunk, PROBLEM_CHUNK_SIZE2*sizeof(unsigned char));

		perf_start(0, PERF_CYCLES);

		/* Build chunk. */
		for (int k = 0; k < PROBLEM_CHUNK_SIZE; k++)
		{
			umemcpy(
				&newimg[(ii + k)*PROBLEM_IMGSIZE + jj],
				&chunk[k*PROBLEM_CHUNK_SIZE],
				PROBLEM_CHUNK_SIZE*sizeof(unsigned char)
			);
		}

		elapsed_time += perf_read(0);

		jj += PROBLEM_CHUNK_SIZE;
		if ((jj+PROBLEM_MASKSIZE-1) == PROBLEM_IMGSIZE)
		{
			jj = 0;
			ii += PROBLEM_CHUNK_SIZE;
		}

		if ((ii+PROBLEM_MASKSIZE-1) == PROBLEM_IMGSIZE)
			ii = 0;
	}

	update_total(elapsed_time);
}

/* Gaussian filter. */
static void gauss_filter(void)
{
	int msg;

	/* Receives first image. */
	data_receive(0, &msg, sizeof(int));
	data_receive(0, &img, PROBLEM_IMGSIZE2);

	mpi_std_barrier();

	process_chunks();

	/* Sends processed image back to master. */
	data_send(0, &newimg, PROBLEM_IMGSIZE2);

	do
	{
#if DEBUG
		uprintf("Submaster receiving control message from master...");
#endif
		/* Receives control message from master. */
		data_receive(0, &msg, sizeof(int));

		/* Finalize execution? */
		if (msg == MSG_DIE)
			break;

#if DEBUG
		uprintf("Submaster receiving new image from master...");
#endif

		/* Receives new message from master. */
		data_receive(0, &img, PROBLEM_IMGSIZE2);

#if DEBUG
		uprintf("Submaster processing chunks...");
#endif

		process_chunks();

#if DEBUG
		uprintf("Submaster sending image back to master...");
#endif

		/* Sends processed image back to master. */
		data_send(0, &newimg, PROBLEM_IMGSIZE2);
	} while (1);
}

static void finalize(void)
{
	int msg;
	int rank;
	uint64_t total_time;

	/* Releasing slaves. */
	msg = MSG_DIE;
	for (int i = 0; i < slaves; i++)
	{
		rank = SLAVE_RANK(i);
		data_send(rank, &msg, sizeof(int));
	}

	/* Send statistics to master. */
	total_time = total();
	data_send(0, &total_time, sizeof(uint64_t));
}

void do_submaster(void)
{
#if VERBOSE
	uprintf("submaster initializing local structures...\n");
#endif /* VERBOSE */

	init();

#if VERBOSE
	uprintf("submaster applying filter...\n");
#endif /* VERBOSE */

	/* Apply filter. */
	gauss_filter();

#if VERBOSE
	uprintf("submaster finalizing...\n");
#endif /* VERBOSE */

	/* Release slaves and collect statistics. */
	finalize();
}

