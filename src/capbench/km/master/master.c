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

#include "../km.h"

/* K-means Data */
static float centroids[PROBLEM_NUM_CENTROIDS*DIMENSION_MAX];                      /* Data centroids.            */
static int map[PROBLEM_NUM_POINTS];                                               /* Map of clusters.           */
static int population[PROBLEM_NUM_CENTROIDS];                                     /* Population of centroids.   */
static int ppopulation[PROBLEM_NUM_CENTROIDS];                                    /* Partial population.        */
static float pcentroids[PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*PROBLEM_NUM_WORKERS]; /* Partial centroids.         */
static int lnpoints[PROBLEM_NUM_WORKERS];                                         /* Local number of points.    */
static int has_changed[PROBLEM_NUM_WORKERS];                                      /* Has any centroid changed?  */
static float points[PROBLEM_NUM_POINTS*DIMENSION_MAX];                            /* Data points.               */

/*============================================================================*
 * initialize_variables()                                                     *
 *============================================================================*/

static void initialize_variables()
{
	srandnum(PROBLEM_SEED);
	for (int i = 0; i < PROBLEM_NUM_POINTS*DIMENSION_MAX; i++)
		points[i] = randnum() & 0xffff;

	/* Initialize mapping. */
	for (int i = 0; i < PROBLEM_NUM_POINTS; i++)
		map[i] = -1;

	/* Initialize centroids. */
	for (int i = 0; i < PROBLEM_NUM_CENTROIDS; i++)
	{
		int j = randnum()%PROBLEM_NUM_POINTS;
		umemcpy(CENTROID(i), POINT(j), DIMENSION_MAX*sizeof(float));
		map[j] = i;
	}

	/* Map unmapped data points. */
	for (int i = 0; i < PROBLEM_NUM_POINTS; i++)
	{
		if (map[i] < 0)
			map[i] = randnum()%PROBLEM_NUM_CENTROIDS;
	}
}

/*============================================================================*
 * send_work()                                                                *
 *============================================================================*/

static void send_work(void)
{
	int count = 0;

	perf_start(0, PERF_CYCLES);

	/* Distribute work among clusters. */
	for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
	{
		lnpoints[i] = ((i + 1) < PROBLEM_NUM_WORKERS) ?
			PROBLEM_NUM_POINTS/PROBLEM_NUM_WORKERS :
			PROBLEM_NUM_POINTS - i*(PROBLEM_NUM_POINTS/PROBLEM_NUM_WORKERS);
	}

	master += perf_read(0);

	perf_start(0, PERF_CYCLES);

	for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
	{
		/* Util information for the problem. */
		data_send(i + 1, &lnpoints[i], sizeof(int));

		/* Actual initial tasks. */
		data_send(i + 1, &points[count*DIMENSION_MAX], lnpoints[i]*DIMENSION_MAX*sizeof(float));
		data_send(i + 1, &map[count], lnpoints[i]*sizeof(int));
		data_send(i + 1, centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));

		count += lnpoints[i];
	}

	communication += perf_read(0);
}

/*============================================================================*
 * sync()                                                                     *
 *============================================================================*/

static int _iterations = 0;

static int sync(void)
{
	int again = 0;

	perf_start(0, PERF_CYCLES);

	for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
	{
		data_receive(i + 1, PCENTROID(i,0), PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
		data_receive(i + 1, PPOPULATION(i,0), PROBLEM_NUM_CENTROIDS*sizeof(int));
		data_receive(i + 1, &has_changed[i], sizeof(int));
	}

	communication += perf_read(0);

	perf_start(0, PERF_CYCLES);

	/* Clear all centroids and population for recalculation. */
	umemset(centroids, 0, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	umemset(population, 0, PROBLEM_NUM_CENTROIDS*sizeof(int));

	for (int i = 0; i < PROBLEM_NUM_CENTROIDS; i++)
	{
		for (int j = 0; j < PROBLEM_NUM_WORKERS; j++)
		{
			vector_add(CENTROID(i), PCENTROID(j, i));
			population[i] += *PPOPULATION(j, i);
		}
		vector_mult(CENTROID(i), __fdiv(1.0f, population[i]));
	}

	if ((++_iterations) < 131)
	{
		for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
		{
			if (has_changed[i])
			{
				again = 1;
				break;
			}
		}
	}

	master += perf_read(0);

	perf_start(0, PERF_CYCLES);

	for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
	{
		data_send(i + 1, &again, sizeof(int));
		if (again == 1)
			data_send(i + 1, centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	}

	communication += perf_read(0);

	return again;
}

/*============================================================================*
 * get_results()                                                              *
 *============================================================================*/

static void get_results(void)
{
	int counter = 0; /* Points counter. */

	for (int i = 0; i < PROBLEM_NUM_WORKERS; i++)
	{
		data_receive(i + 1, &map[counter], lnpoints[i]*sizeof(int));
		data_receive(i + 1, &slave[i], sizeof(uint64_t));
		counter += lnpoints[i];
	}
}

/*============================================================================*
 * do_kmeans()                                                                *
 *============================================================================*/

void do_kernel(void)
{
#ifndef NDEBUG
	uprintf("initializing...");
#endif

	/* Benchmark initialization. */
	initialize_variables();

	/* Cluster data. */
#ifndef NDEBUG
	uprintf("clustering data...");
#endif

	send_work();

	/* Cluster data. */
	do
	{
		/* noop */
	} while (sync());

	get_results();

	total = master + communication;
}
