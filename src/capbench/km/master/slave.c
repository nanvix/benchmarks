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

#define CENTROID(data, i) \
	(&data->centroids[(i)*DIMENSION_MAX])

#define POINT(data, i) \
	(&data->points[(i)*DIMENSION_MAX])

#define PCENTROID(data, i, j) \
	(&data->pcentroids[(i)*PROBLEM_NUM_CENTROIDS*DIMENSION_MAX + (j)*DIMENSION_MAX])

#define PPOPULATION(data, i, j) \
	(&data->ppopulation[(i)*PROBLEM_NUM_CENTROIDS + (j)])

/* K-means Data */
PRIVATE struct local_data
{
	int lnpoints;                                         /* Local number of points.   */
	float points[PROBLEM_LNPOINTS*DIMENSION_MAX];         /* Data points.              */
	float centroids[PROBLEM_NUM_CENTROIDS*DIMENSION_MAX]; /* Data centroids.           */
	int map[PROBLEM_LNPOINTS];                            /* Map of clusters.          */
	int ppopulation[PROBLEM_NUM_CENTROIDS];               /* Partial population.       */
	int has_changed;                                      /* Has any centroid changed? */
} _local_data[PROCS_PER_CLUSTER_MAX];

/*============================================================================*
 * populate()                                                                 *
 *============================================================================*/

PRIVATE void populate(struct local_data *data)
{
	int init_map;   /* Point initial mapping. */
	float tmp_dist; /* Temporary distance.    */
	float distance; /* Distance.              */
	uint64_t time_elapsed;

	perf_start(0, PERF_CYCLES);

	/* Reset variables for new calculation. */
	umemset(data->ppopulation, 0, PROBLEM_NUM_CENTROIDS*sizeof(int));
	data->has_changed = 0;

	/* Iterate over data points. */
	for (int i = 0; i < data->lnpoints; i++)
	{
		distance = vector_distance(CENTROID(data, data->map[i]), POINT(data, i));
		init_map = data->map[i];

		/* Looking for closest cluster. */
		for (int j = 0; j < PROBLEM_NUM_CENTROIDS; j++) {
			/* Point is in this cluster. */
			if (j == data->map[i])
				continue;

			tmp_dist = vector_distance(CENTROID(data, j), POINT(data, i));

			/* Found. */
			if (tmp_dist < distance) {
				data->map[i] = j;
				distance = tmp_dist;
			}
		}

		data->ppopulation[data->map[i]]++;

		if (data->map[i] != init_map)
			data->has_changed = 1;
	}

	time_elapsed = perf_read(0);
	update_total(time_elapsed);
}

/*============================================================================*
 * compute_centroids()                                                        *
 *============================================================================*/

static void compute_centroids(struct local_data *data)
{
	uint64_t time_elapsed;

	perf_start(0, PERF_CYCLES);

	/* Compute means. */
	umemset(CENTROID(data, 0), 0, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	for (int i = 0; i < data->lnpoints; i++)
		vector_add(CENTROID(data, data->map[i]), POINT(data, i));

	time_elapsed = perf_read(0);
	update_total(time_elapsed);
}

/*============================================================================*
 * sync()                                                                     *
 *============================================================================*/

static int sync(struct local_data *data)
{
	int again = 0;
	data_send(0, data->centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	data_send(0, data->ppopulation, PROBLEM_NUM_CENTROIDS*sizeof(int));
	data_send(0, &data->has_changed, sizeof(int));

#if DEBUG
	uprintf("Waiting to receive again confirmation...");
#endif
	data_receive(0, &again, sizeof(int));

	if (again == 1)
	{
#if DEBUG
	uprintf("Waiting to receive recalculated centroids...");
#endif
		data_receive(0, data->centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	}

	return (again);
}

/*============================================================================*
 * get_work()                                                                 *
 *============================================================================*/

static void get_work(struct local_data *data)
{
	data_receive(0, (void *) &data->lnpoints, sizeof(int));
	data_receive(0, data->points, data->lnpoints*DIMENSION_MAX*sizeof(float));
	data_receive(0, data->map, data->lnpoints*sizeof(int));
	data_receive(0, data->centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
}

/*============================================================================*
 * send_results()                                                             *
 *============================================================================*/

static void send_results(struct local_data *data)
{
	uint64_t total_time;

	total_time = total();

	data_send(0, data->map, data->lnpoints*sizeof(int));
	data_send(0, &total_time, sizeof(uint64_t));
}

/*============================================================================*
 * do_kmeans()                                                                *
 *============================================================================*/

void do_slave(void)
{
	int local_index;

#if DEBUG
	int iteration = 0;
#endif

#if VERBOSE
	int rank;
	runtime_get_rank(&rank);
#endif /* VERBOSE */

	local_index = runtime_get_index();

#if VERBOSE
	uprintf("rank %d getting work...", rank);
#endif /* VERBOSE */

	get_work(&_local_data[local_index]);

#if VERBOSE
	uprintf("rank %d clustering data...", rank);
#endif /* VERBOSE */

	/* Cluster data. */
	do
	{
#if DEBUG
		uprintf("rank %d populating iteration %d...", rank, ++iteration);
#endif /* VERBOSE */
		populate(&_local_data[local_index]);
#if DEBUG
		uprintf("rank %d computing centroids...", rank);
#endif /* VERBOSE */
		compute_centroids(&_local_data[local_index]);
#if DEBUG
		uprintf("rank %d syncing...", rank);
#endif /* VERBOSE */
	} while (sync(&_local_data[local_index]));

#if VERBOSE
	uprintf("rank %d sending results back...", rank);
#endif /* VERBOSE */

	send_results(&_local_data[local_index]);
}

