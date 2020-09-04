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
static int lnpoints;                                         /* Local number of points.   */
static float points[PROBLEM_LNPOINTS*DIMENSION_MAX];         /* Data points.              */
static float centroids[PROBLEM_NUM_CENTROIDS*DIMENSION_MAX]; /* Data centroids.           */
static int map[PROBLEM_LNPOINTS];                            /* Map of clusters.          */
static int ppopulation[PROBLEM_NUM_CENTROIDS];               /* Partial population.       */
static int has_changed;                                      /* Has any centroid changed? */

const int _dimension = DIMENSION_MAX;

/*============================================================================*
 * populate()                                                                 *
 *============================================================================*/

static void populate(void )
{
	int init_map;   /* Point initial mapping. */
	float tmp_dist; /* Temporary distance.    */
	float distance; /* Distance.              */

	perf_start(0, PERF_CYCLES);

	/* Reset variables for new calculation. */
	umemset(ppopulation, 0, PROBLEM_NUM_CENTROIDS*sizeof(int));
	has_changed = 0;

	/* Iterate over data points. */
	for (int i = 0; i < lnpoints; i++)
	{
		distance = vector_distance(CENTROID(map[i]), POINT(i));
		init_map = map[i];

		/* Looking for closest cluster. */
		for (int j = 0; j < PROBLEM_NUM_CENTROIDS; j++) {
			/* Point is in this cluster. */
			if (j == map[i])
				continue;

			tmp_dist = vector_distance(CENTROID(j), POINT(i));

			/* Found. */
			if (tmp_dist < distance) {
				map[i] = j;
				distance = tmp_dist;
			}
		}

		ppopulation[map[i]]++;

		if (map[i] != init_map)
			has_changed = 1;
	}

	total += perf_read(0);
}

/*============================================================================*
 * compute_centroids()                                                        *
 *============================================================================*/

static void compute_centroids(void)
{
	perf_start(0, PERF_CYCLES);

	/* Compute means. */
	umemset(CENTROID(0), 0, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	for (int i = 0; i < lnpoints; i++)
			vector_add(CENTROID(map[i]), POINT(i));

	total += perf_read(0);
}

/*============================================================================*
 * sync()                                                                     *
 *============================================================================*/

static int sync(void)
{
	int again = 0;

	data_send(0, centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	data_send(0, ppopulation, PROBLEM_NUM_CENTROIDS*sizeof(int));
	data_send(0, &has_changed, sizeof(int));

	data_receive(0, &again, sizeof(int));

	if (again == 1)
		data_receive(0, centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));

	return (again);
}

/*============================================================================*
 * get_work()                                                                 *
 *============================================================================*/

static void get_work(void)
{
	data_receive(0, &lnpoints, sizeof(int));
	data_receive(0, points, lnpoints*DIMENSION_MAX*sizeof(float));
	data_receive(0, map, lnpoints*sizeof(int));
	data_receive(0, centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
}

/*============================================================================*
 * send_results()                                                             *
 *============================================================================*/

static void send_results(void)
{
	data_send(0, map, lnpoints*sizeof(int));
	data_send(0, &total, sizeof(uint64_t));
}

/*============================================================================*
 * do_kmeans()                                                                *
 *============================================================================*/

void do_kernel(void)
{
	get_work();

	/* Cluster data. */
	do
	{
		populate();
		compute_centroids();
	} while (sync());

	send_results();
}
