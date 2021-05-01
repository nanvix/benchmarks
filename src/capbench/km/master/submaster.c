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

#define CENTROID(i) \
	(&centroids[(i)*DIMENSION_MAX])

#define POINT(i) \
	(&points[(i)*DIMENSION_MAX])

#define PCENTROID(i, j) \
	(&pcentroids[(i)*PROBLEM_NUM_CENTROIDS*DIMENSION_MAX + (j)*DIMENSION_MAX])

#define PPOPULATION(i, j) \
	(&ppopulation[(i)*PROBLEM_NUM_CENTROIDS + (j)])

static int lnpoints[PROCS_PER_CLUSTER_MAX];                                         /* Local number of points.   */
static float points[PROBLEM_NUM_POINTS*DIMENSION_MAX/ACTIVE_CLUSTERS];         /* Data points.              */
static float centroids[PROBLEM_NUM_CENTROIDS*DIMENSION_MAX]; /* Data centroids.           */
static int map[PROBLEM_NUM_POINTS/ACTIVE_CLUSTERS];                            /* Map of clusters.          */
static int ppopulation[PROBLEM_NUM_CENTROIDS];               /* Partial population.       */
static float pcentroids[PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*PROCS_PER_CLUSTER_MAX]; /* Partial centroids.         */
static int has_changed[PROCS_PER_CLUSTER_MAX];                                      /* Has any centroid changed? */

static int my_rank;
static int slaves;
static int numpoints;

/*============================================================================*
 * generate_points()                                                          *
 *============================================================================*/

static void generate_points(void)
{
	int index;
	int cnt_amnt;
	int first_cnt;

	/* Unique seed per cluster. */
	srandnum(cluster_get_num());

	/* Initialize points. */
	for (int i = 0; i < PROBLEM_NUM_POINTS*DIMENSION_MAX/ACTIVE_CLUSTERS; i++)
		points[i] = randnum() & 0xffff;

	/* Initialize mapping. */
	for (int i = 0; i < PROBLEM_NUM_POINTS/ACTIVE_CLUSTERS; i++)
		map[i] = -1;

	index = cluster_get_num() - MPI_BASE_NODE;
	cnt_amnt = PROBLEM_NUM_CENTROIDS / ACTIVE_CLUSTERS;
	first_cnt = index * cnt_amnt;

	for (int i = first_cnt; i < first_cnt + cnt_amnt; ++i)
	{
		int j = randnum() % PROBLEM_NUM_POINTS/ACTIVE_CLUSTERS;
		umemcpy(POINT(j), CENTROID(i), DIMENSION_MAX*sizeof(float));
		map[j] = i;
	}

	/* Map unmapped data points. */
	for (int i = 0; i < PROBLEM_NUM_POINTS/ACTIVE_CLUSTERS; i++)
	{
		if (map[i] < 0)
			map[i] = randnum() % PROBLEM_NUM_CENTROIDS;
	}
}

/*============================================================================*
 * get_work()                                                                 *
 *============================================================================*/

static void get_work(void)
{
	data_receive(0, &slaves, sizeof(int));
	data_receive(0, (void *) &numpoints, sizeof(int));
	//data_receive(0, &points, numpoints*DIMENSION_MAX*sizeof(float));
	//data_receive(0, &map, numpoints*sizeof(int));
	data_receive(0, &centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
}

/*============================================================================*
 * send_work()                                                                *
 *============================================================================*/

static void send_work(void)
{
	int count;
	int rank;
	uint64_t time_elapsed = 0;

	perf_start(0, PERF_CYCLES);

	/* Distribute work among slaves. */
	for (int i = 0; i < slaves; i++)
	{
		// lnpoints[i] = ((i + 1) < PROBLEM_NUM_WORKERS) ?
		// 	PROBLEM_NUM_POINTS/PROBLEM_NUM_WORKERS :
		// 	PROBLEM_NUM_POINTS - i*(PROBLEM_NUM_POINTS/PROBLEM_NUM_WORKERS);
		lnpoints[i] = PROBLEM_LNPOINTS;
	}

	time_elapsed += perf_read(0);
	update_total(time_elapsed);

	count = lnpoints[0];

	for (int i = 1; i < slaves; i++)
	{
		rank = my_rank + i;
#if DEBUG
	uprintf("submaster sending work to rank %d...", (i+1));
#endif /* DEBUG */

		/* Util information for the problem. */
		data_send(rank, &my_rank, sizeof(int));
		data_send(rank, &lnpoints[i], sizeof(int));

		/* Actual initial tasks. */
		data_send(rank, &points[count*DIMENSION_MAX], lnpoints[i]*DIMENSION_MAX*sizeof(float));
		data_send(rank, &map[count], lnpoints[i]*sizeof(int));
		data_send(rank, &centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));

		count += lnpoints[i];
	}
}

static void populate(void)
{
	int init_map;   /* Point initial mapping. */
	float tmp_dist; /* Temporary distance.    */
	float distance; /* Distance.              */
	uint64_t time_elapsed;

	perf_start(0, PERF_CYCLES);

	/* Reset variables for new calculation. */
	umemset(ppopulation, 0, PROBLEM_NUM_CENTROIDS*sizeof(int));
	has_changed[0] = 0;

	/* Iterate over data points. */
	for (int i = 0; i < lnpoints[0]; i++)
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
			has_changed[0] = 1;
	}

	time_elapsed = perf_read(0);
	update_total(time_elapsed);
}

/*============================================================================*
 * compute_centroids()                                                        *
 *============================================================================*/

static void compute_centroids(void)
{
	uint64_t time_elapsed;

	perf_start(0, PERF_CYCLES);

	/* Compute means. */
	umemset(CENTROID(0), 0, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	for (int i = 0; i < lnpoints[0]; i++)
		vector_add(CENTROID(map[i]), POINT(i));

	time_elapsed = perf_read(0);
	update_total(time_elapsed);
}

/*============================================================================*
 * sync()                                                                     *
 *============================================================================*/

static int sync(void)
{
	int rank;
	int changed = 0;
	int again = 0;

	for (int i = 1; i < slaves; i++)
	{
		rank = my_rank + i;

#if DEBUG
		uprintf("Submaster Syncing rank %d...", rank);
#endif /* DEBUG */

		data_receive(rank, PCENTROID(i,0), PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
		data_receive(rank, PPOPULATION(i,0), PROBLEM_NUM_CENTROIDS*sizeof(int));
		data_receive(rank, &has_changed[i], sizeof(int));
	}

	/* Has changed? */
	for (int i = 0; i < slaves; ++i)
	{
		if (has_changed[i])
		{
			changed = 1;
			break;
		}
	}

#if DEBUG
	uprintf("submaster sending data back to the master!");
#endif /* DEBUG */

	data_send(0, &centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	data_send(0, &ppopulation, PROBLEM_NUM_CENTROIDS*sizeof(int));
	data_send(0, &changed, sizeof(int));

#if DEBUG
	uprintf("Submaster Waiting to receive again confirmation...");
#endif
	data_receive(0, &again, sizeof(int));

	if (again == 1)
	{
#if DEBUG
	uprintf("Submaster Waiting to receive recalculated centroids...");
#endif
		data_receive(0, &centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	}

	for (int i = 1; i < slaves; i++)
	{
		rank = my_rank + i;

#if DEBUG
		uprintf("Submaster Signalizing slave %d...", rank);
#endif

		data_send(rank, &again, sizeof(int));
		if (again == 1)
			data_send(rank, &centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	}

#if DEBUG
	uprintf("Submaster completed sync...");
#endif

	return again;
}

/*============================================================================*
 * get_results()                                                              *
 *============================================================================*/

static void get_results(void)
{
	int counter = lnpoints[0]; /* Points counter. */
	int rank;

	for (int i = 1; i < slaves; i++)
	{
		rank = my_rank + i;

		data_receive(rank, &map[counter], lnpoints[i]*sizeof(int));
		counter += lnpoints[i];
	}
}

/*============================================================================*
 * send_results()                                                             *
 *============================================================================*/

static void send_results(void)
{
	data_send(0, &map, numpoints*sizeof(int));
}

/*============================================================================*
 * send_statistics()                                                          *
 *============================================================================*/

static void send_statistics(void)
{
	uint64_t total_time;

	total_time = total();

	data_send(0, &total_time, sizeof(uint64_t));
}

/*============================================================================*
 * do_kmeans()                                                                *
 *============================================================================*/

void do_submaster(void)
{
#if DEBUG
	int i = 0;
#endif

	runtime_get_rank(&my_rank);

#if VERBOSE
	uprintf("submaster %d getting work from master...", my_rank);
#endif /* VERBOSE */

	get_work();

	#if VERBOSE
		uprintf("submaster %d generating cloud of points...", my_rank);
	#endif /* VERBOSE */

		generate_points();

	#if VERBOSE
		uprintf("submaster %d sending work to slaves...", my_rank);
	#endif /* VERBOSE */

		send_work();

	#if VERBOSE
		uprintf("submaster %d clustering data...", my_rank);
	#endif /* VERBOSE */

	do
	{
#if DEBUG
		uprintf("submaster %d populating iteration %d...", my_rank, ++i);
#endif /* VERBOSE */
		populate();
#if DEBUG
		uprintf("submaster %d computing centroids...", my_rank);
#endif /* VERBOSE */
		compute_centroids();
#if DEBUG
		uprintf("submaster %d syncing...", my_rank);
#endif /* VERBOSE */
	} while (sync());

#if VERBOSE
	uprintf("submaster %d getting results...", my_rank);
#endif /* DEBUG */

	get_results();

#if VERBOSE
	uprintf("submaster %d sending results back...", my_rank);
#endif /* DEBUG */

	//send_results();

	update_total(communication());

#if VERBOSE
	uprintf("submaster %d sending statistics...", my_rank);
#endif /* DEBUG */

	send_statistics();
}

