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

#define SUBMASTERS_NUM (ACTIVE_CLUSTERS)

#define SUBMASTER_RANK(i) ((i == 0) ? 1 : (i * PROCS_PER_CLUSTER_MAX))

#define CENTROID(i) \
	(&centroids[(i)*DIMENSION_MAX])

#define POINT(i) \
	(&points[(i)*DIMENSION_MAX])

#define PCENTROID(i, j) \
	(&pcentroids[(i)*PROBLEM_NUM_CENTROIDS*DIMENSION_MAX + (j)*DIMENSION_MAX])

#define PPOPULATION(i, j) \
	(&ppopulation[(i)*PROBLEM_NUM_CENTROIDS + (j)])

/* Timing statistics. */
uint64_t master = 0;                 /* Time spent on master.       */
uint64_t spawn = 0;                  /* Time spent spawning slaves. */
uint64_t slave[PROBLEM_NUM_WORKERS]; /* Time spent on slaves.       */

/* Kernel Data. */
static float centroids[PROBLEM_NUM_CENTROIDS*DIMENSION_MAX];                      /* Data centroids.            */
//static int map[PROBLEM_NUM_POINTS];                                               /* Map of clusters.           */
static int population[PROBLEM_NUM_CENTROIDS];                                     /* Population of centroids.   */
static int ppopulation[PROBLEM_NUM_CENTROIDS];                                    /* Partial population.        */
static float pcentroids[PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*SUBMASTERS_NUM]; /* Partial centroids.         */
static int lnpoints[SUBMASTERS_NUM];                                         /* Local number of points.    */
static int has_changed[SUBMASTERS_NUM];                                      /* Has any centroid changed?  */
//static float points[PROBLEM_NUM_POINTS*DIMENSION_MAX];                            /* Data points.               */

/*============================================================================*
 * initialize_variables()                                                     *
 *============================================================================*/

static void initialize_variables()
{
	srandnum(PROBLEM_SEED);
	float point[DIMENSION_MAX];

	// /* Initialize points. */
	// for (int i = 0; i < PROBLEM_NUM_POINTS*DIMENSION_MAX; i++)
	// 	points[i] = randnum() & 0xffff;

	// /* Initialize mapping. */
	// for (int i = 0; i < PROBLEM_NUM_POINTS; i++)
	// 	map[i] = -1;

	/* Initialize centroids. */
	for (int i = 0; i < PROBLEM_NUM_CENTROIDS; i++)
	{
		/* Generate a new point. */
		for (int j = 0; j < DIMENSION_MAX; ++j)
			point[j] = randnum() & 0xffff;

		umemcpy(CENTROID(i), point, DIMENSION_MAX*sizeof(float));
	}

	/* Initialize centroids. */
	// for (int i = 0; i < PROBLEM_NUM_CENTROIDS; i++)
	// {
	// 	int j = randnum() % PROBLEM_NUM_POINTS;
	// 	umemcpy(CENTROID(i), POINT(j), DIMENSION_MAX*sizeof(float));
	// 	map[j] = i;
	// }

	// /* Map unmapped data points. */
	// for (int i = 0; i < PROBLEM_NUM_POINTS; i++)
	// {
	// 	if (map[i] < 0)
	// 		map[i] = randnum() % PROBLEM_NUM_CENTROIDS;
	// }
}

/*============================================================================*
 * send_work()                                                                *
 *============================================================================*/

static void send_work(void)
{
	int count = 0;
	int rank;
	int slaves;

	perf_start(0, PERF_CYCLES);

	/* Distribute work among clusters. */
	for (int i = 0; i < SUBMASTERS_NUM; i++)
	{
		// lnpoints[i] = ((i + 1) < PROBLEM_NUM_WORKERS) ?
		// 	PROBLEM_NUM_POINTS/PROBLEM_NUM_WORKERS :
		// 	PROBLEM_NUM_POINTS - i*(PROBLEM_NUM_POINTS/PROBLEM_NUM_WORKERS);
		lnpoints[i] = PROBLEM_NUM_POINTS/SUBMASTERS_NUM;
	}

	master += perf_read(0);

	for (int i = 0; i < SUBMASTERS_NUM; i++)
	{
		rank = SUBMASTER_RANK(i);

		if (rank == 1)
			slaves = CLUSTER_PROCESSES_NR(i) - 1;
		else
			slaves = CLUSTER_PROCESSES_NR(i);

		// if (rank == 1)
		// 	slaves = PROCS_PER_CLUSTER_MAX - 1;
		// else
		// 	slaves = PROCS_PER_CLUSTER_MAX;

#if DEBUG
	uprintf("Master sending work to rank %d...", rank);
#endif /* DEBUG */

		/* Util information for the problem. */
		data_send(rank, &slaves, sizeof(int));
		data_send(rank, &lnpoints[i], sizeof(int));

		/* Actual initial tasks. */
		//data_send(rank, &points[count*DIMENSION_MAX], lnpoints[i]*DIMENSION_MAX*sizeof(float));
		//data_send(rank, &map[count], lnpoints[i]*sizeof(int));
		data_send(rank, &centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));

		count += lnpoints[i];
	}
}

/*============================================================================*
 * sync()                                                                     *
 *============================================================================*/

static int _iterations = 0;

static int sync(void)
{
	int again = 0;
	int rank;

	for (int i = 0; i < SUBMASTERS_NUM; i++)
	{
		rank = SUBMASTER_RANK(i);
#if DEBUG
		uprintf("Master Syncing rank %d...", rank);
#endif /* DEBUG */

		data_receive(rank, PCENTROID(i,0), PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
		data_receive(rank, PPOPULATION(i,0), PROBLEM_NUM_CENTROIDS*sizeof(int));
		data_receive(rank, &has_changed[i], sizeof(int));
	}

	perf_start(0, PERF_CYCLES);

	/* Clear all centroids and population for recalculation. */
	umemset(centroids, 0, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	umemset(population, 0, PROBLEM_NUM_CENTROIDS*sizeof(int));

	for (int i = 0; i < PROBLEM_NUM_CENTROIDS; i++)
	{
		for (int j = 0; j < SUBMASTERS_NUM; j++)
		{
			vector_add(CENTROID(i), PCENTROID(j, i));
			population[i] += *PPOPULATION(j, i);
		}
		vector_mult(CENTROID(i), __fdiv(1.0f, population[i]));
	}

	/* Should be 131. */
	if ((++_iterations) < 1031)
	{
		// for (int i = 0; i < SUBMASTERS_NUM; i++)
		// {
		// 	if (has_changed[i])
		// 	{
		// 		again = 1;
		// 		break;
		// 	}
		// }
		again = 1;
	}

#if DEBUG
	uprintf("Master going to signalize submasters...");
#endif

	master += perf_read(0);

	for (int i = 0; i < SUBMASTERS_NUM; i++)
	{
		rank = SUBMASTER_RANK(i);

		data_send(rank, &again, sizeof(int));
		if (again == 1)
			data_send(rank, centroids, PROBLEM_NUM_CENTROIDS*DIMENSION_MAX*sizeof(float));
	}

	return again;
}

/*============================================================================*
 * get_results()                                                              *
 *============================================================================*/

static void get_results(void)
{
	int counter = 0; /* Points counter. */

	for (int i = 0; i < SUBMASTERS_NUM; i++)
	{
		//data_receive(SUBMASTER_RANK(i), &map[counter], lnpoints[i]*sizeof(int));
		counter += lnpoints[i];
	}
}

static void get_statistics(void)
{
	for (int i = 0; i < PROBLEM_NUM_WORKERS; ++i)
		data_receive(i + 1, &slave[i], sizeof(uint64_t));
}

/*============================================================================*
 * do_kmeans()                                                                *
 *============================================================================*/

void do_master(void)
{
	int i = 0;

#if VERBOSE
	uprintf("initializing...");
#endif /* VERBOSE */

	/* Benchmark initialization. */
	initialize_variables();

#if VERBOSE
	uprintf("sending work...");
#endif /* VERBOSE */

	send_work();

#if VERBOSE
	uprintf("clustering data...");
#endif /* VERBOSE */

	/* Cluster data. */
	do
	{
#if VERBOSE
		uprintf("iteration %d...", i);
#endif /* DEBUG */

		i++;
	} while (sync());

#if DEBUG
	uprintf("getting results...");
#endif /* DEBUG */

	//get_results();

#if DEBUG
	uprintf("getting statistics...");
#endif /* DEBUG */

	get_statistics();

	update_total(master + communication());

	uprintf("Realized %d iterations", i);
}

