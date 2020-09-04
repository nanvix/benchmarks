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

#include <posix/stdlib.h>

#include "km.h"

/* K-means Data */
static float *points;    /* Data points.              */
static float *centroids; /* Data centroids.           */
static int *map;         /* Map of clusters.          */
static int *population;  /* Population of centroids.  */
static int has_changed;  /* Has any centroid changed? */

const int _dimension = DIMENSION_MAX;

uint64_t total_time = 0;

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
 * populate()                                                                 *
 *============================================================================*/

static void populate(void )
{
	int init_map;   /* Point initial mapping. */
	float tmp_dist; /* Temporary distance.    */
	float distance; /* Distance.              */

	/* Reset variables for new calculation. */
	for (int i = 0; i < PROBLEM_NUM_CENTROIDS; ++i)
		population[i] = 0;

	/* Iterate over data points. */
	for (int i = 0; i < PROBLEM_NUM_POINTS; i++)
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

		population[map[i]]++;

		if (map[i] != init_map)
			has_changed = 1;
	}
}

/*============================================================================*
 * compute_centroids()                                                        *
 *============================================================================*/

static void compute_centroids(void)
{
	/* Compute means. */
	umemset(CENTROID(0), 0, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	for (int i = 0; i < PROBLEM_NUM_POINTS; i++)
		vector_add(CENTROID(map[i]), POINT(i));

	for (int i = 0; i < PROBLEM_NUM_CENTROIDS; i++)
		vector_mult(CENTROID(i), __fdiv(1.0f, population[i]));
}

/*============================================================================*
 * do_kmeans()                                                                *
 *============================================================================*/

void do_kernel(void)
{
	int iterations;

	/* Allocates memory for points. */
	if ((points = (float *) nanvix_malloc(sizeof(float) * PROBLEM_NUM_POINTS*DIMENSION_MAX)) == NULL)
	{
		upanic("Error in points memory allocation.");
		return;
	}

	/* Allocates memory for centroids. */
	if ((centroids = (float *) nanvix_malloc(sizeof(float) * PROBLEM_NUM_CENTROIDS*DIMENSION_MAX)) == NULL)
	{
		upanic("Error in centroids memory allocation.");
		return;
	}

	/* Allocates memory for map. */
	if ((map = (int *) nanvix_malloc(sizeof(int) * PROBLEM_NUM_POINTS)) == NULL)
	{
		upanic("Error in map of points allocation.");
		return;
	}

	/* Allocates memory for population array. */
	if ((population = (int *) nanvix_malloc(sizeof(int) * PROBLEM_NUM_CENTROIDS)) == NULL)
	{
		upanic("Error in centroids population allocation.");
		return;
	}

	/* Benchmark initialization. */
	initialize_variables();

	/* Starts to count the solution total time. */
	perf_start(0, PERF_CYCLES);

	iterations = 0;

	/* Cluster data. */
	do
	{
		has_changed = 0;

		populate();
		compute_centroids();

		iterations++;
	} while (has_changed);

	total_time += perf_read(0);

	/* Frees the allocated memory. */
	nanvix_free((void *) population);
	nanvix_free((void *) map);
	nanvix_free((void *) centroids);
	nanvix_free((void *) points);
}
