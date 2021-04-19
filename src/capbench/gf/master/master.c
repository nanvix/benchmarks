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

/* Timing statistics. */
uint64_t master = 0;                 /* Time spent on master.       */
uint64_t spawn = 0;                  /* Time spent spawning slaves. */
uint64_t slave[PROBLEM_NUM_WORKERS]; /* Time spent on slaves.       */

static int current_chunk = 0;

/**
 * @brief Kernel Data
 */
/**@{*/
static unsigned char img[PROBLEM_IMGSIZE2];        /* Input Image   */
static unsigned char newimg[PROBLEM_IMGSIZE2];     /* Output Image  */
static float mask[PROBLEM_MASKSIZE2];              /* Mask          */
static unsigned char chunk[CHUNK_WITH_HALO_SIZE2]; /* Working Chunk */
/**@}*/

#define HALF (PROBLEM_MASKSIZE/2)

#define CHUNKS_PER_IMAGE (PROBLEM_IMGDIMENSION2 / PROBLEM_CHUNK_SIZE2)
#define TOTAL_CHUNKS                (CHUNKS_PER_IMAGE * FPS * SECONDS)

static void generate_image(void)
{
	for (int i = 0; i < PROBLEM_IMGSIZE2; i++)
		img[i] = randnum() & 0xff;
}

static void send_work(void)
{
	int ii;
	int jj;
	int msg = MSG_CHUNK;
	int ck;
	int last_chunk;

	while (current_chunk < TOTAL_CHUNKS)
	{
#if DEBUG
		uprintf("Generating new image");
#endif
		/* Generates a new image to be processed. */
		perf_start(0, PERF_CYCLES);
			generate_image();
		master += perf_read(0);

		ii = 0;
		jj = 0;

		for (int i = HALF; i < PROBLEM_IMGSIZE - HALF; i += PROBLEM_CHUNK_SIZE)
		{
			for (int j = HALF; j < PROBLEM_IMGSIZE - HALF; j += PROBLEM_CHUNK_SIZE)
			{
				ck = current_chunk % PROBLEM_NUM_WORKERS;

				if (current_chunk >= PROBLEM_NUM_WORKERS)
				{
#if DEBUG
					uprintf("Receiving data from rank %d", ck+1);
#endif
					/* Receives a chunk from the slave. */
					data_receive(ck + 1, chunk, PROBLEM_CHUNK_SIZE2*sizeof(unsigned char));
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

					master += perf_read(0);
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

				master += perf_read(0);

#if DEBUG
				uprintf("Sending chunk to rank %d", ck+1);
#endif
				/* Sending chunk to slave. */
				data_send(ck + 1, &msg, sizeof(int));
				data_send(ck + 1, chunk, CHUNK_WITH_HALO_SIZE2*sizeof(unsigned char));

				/* Remaining chunks of this image need a receive before. */
				current_chunk++;

				if (current_chunk >= TOTAL_CHUNKS)
					last_chunk = ck;
			}
		}
	}

	/* Get remaining chunks. */
	for (int i = last_chunk + 1; i != last_chunk; i++)
	{
		if (i == PROBLEM_NUM_WORKERS)
			i = 0;

#if DEBUG
		uprintf("Receiving last chunk from rank %d", i+1);
#endif

		data_receive(i + 1, chunk, PROBLEM_CHUNK_SIZE2*sizeof(unsigned char));

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

		master += perf_read(0);

		jj += PROBLEM_CHUNK_SIZE;
		if ((jj+PROBLEM_MASKSIZE-1) == PROBLEM_IMGSIZE)
		{
			jj = 0;
			ii += PROBLEM_CHUNK_SIZE;
		}

		if ((ii+PROBLEM_MASKSIZE-1) == PROBLEM_IMGSIZE)
			ii = 0;
	}

#if DEBUG
	uprintf("Receiving final chunk from rank %d", last_chunk + 1);
#endif

	data_receive(last_chunk + 1, chunk, PROBLEM_CHUNK_SIZE2*sizeof(unsigned char));

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

	master += perf_read(0);
}

static void process_chunks(void)
{
	int ii = 0;
	int jj = 0;
	int nchunks = 0;
	int msg = MSG_CHUNK;

	for (int i = HALF; i < PROBLEM_IMGSIZE - HALF; i += PROBLEM_CHUNK_SIZE)
	{
		for (int j = HALF; j < PROBLEM_IMGSIZE - HALF; j += PROBLEM_CHUNK_SIZE)
		{
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

			master += perf_read(0);

			/* Sending chunk to slave. */
			data_send(nchunks + 1, &msg, sizeof(int));
			data_send(nchunks + 1, chunk, CHUNK_WITH_HALO_SIZE2*sizeof(unsigned char));

			/* Receives chunk without halo. */
			if (++nchunks == PROBLEM_NUM_WORKERS)
			{
				for (int ck = 0; ck < nchunks; ck++)
				{
					data_receive(ck + 1, chunk, PROBLEM_CHUNK_SIZE2*sizeof(unsigned char));
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

					master += perf_read(0);
				}

				nchunks = 0;
			}
		}
	}

	/* Get remaining chunks. */
	for (int ck = 0; ck < nchunks; ck++)
	{
		data_receive(ck + 1, chunk, PROBLEM_CHUNK_SIZE2*sizeof(unsigned char));

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

		master += perf_read(0);

		jj += PROBLEM_CHUNK_SIZE;
		if ((jj+PROBLEM_MASKSIZE-1) == PROBLEM_IMGSIZE)
		{
			jj = 0;
			ii += PROBLEM_CHUNK_SIZE;
		}
	}
}

/* Gaussian filter. */
static void gauss_filter(void)
{
	send_work();

	/* Processing the chunks. */
	//process_chunks();
}

static void generate_mask(void)
{
	float sec;
	float first;
	float total_aux;

	total_aux = 0;
	first = __fdiv(1.0, (2.0*PI*SD*SD));

	for (int i = -HALF; i <= HALF; i++)
	{
		for (int j = -HALF; j <= HALF; j++)
		{
			sec = -((i*i + j*j))*__fdiv(1.0, (2.0*SD*SD));
			MASK(i + HALF, j + HALF) = first*sec;
			total_aux += MASK(i + HALF, j + HALF);
		}
	}

	for (int i = 0 ; i < PROBLEM_MASKSIZE; i++)
	{
		for (int j = 0; j < PROBLEM_MASKSIZE; j++)
			MASK(i, j) = __fdiv(MASK(i, j), total_aux);
	}
}

static void init(void)
{
	//generate_image();

	generate_mask();

	/* Send mask to slaves. */
	for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
		data_send(i + 1, mask, sizeof(float)*PROBLEM_MASKSIZE2);
}

static void update_image(void)
{
	for (int i = 0; i < PROBLEM_IMGSIZE2; i++)
		img[i] = newimg[i];
}

static void finalize(void)
{
	int msg;

	/* Releasing slaves. */
	msg = MSG_DIE;
	for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
		data_send(i + 1, &msg, sizeof(int));

	/* Collect slave statistics. */
	for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
		data_receive(i + 1, &slave[i], sizeof(uint64_t));
}

void do_master(void)
{
#if VERBOSE
	uprintf("initializing...\n");
#endif /* VERBOSE */

	init();

#if VERBOSE
	uprintf("applying filter...\n");
#endif /* VERBOSE */

	/* Apply filter. */
	gauss_filter();

#if VERBOSE
		uprintf("preparing to collect statistics...\n");
#endif /* VERBOSE */

	/* Release slaves and collect statistics. */
	finalize();

	update_total(master + communication());
}

