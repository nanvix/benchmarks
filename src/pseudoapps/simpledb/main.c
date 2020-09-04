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

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/barrier.h>
#include <nanvix/sys/perf.h>
#include <nanvix/limits.h>
#include <nanvix/ulib.h>

#include <posix/errno.h>
#include <posix/sys/stat.h>
#include <posix/sys/ipc.h>
#include <posix/errno.h>

/**
 * @brief Number of messages to send.
 */
#define NUM_ROWS (NANVIX_SHM_MAX/(2*sizeof(unsigned)))

/**
 * @brief Shared Table.
 */
struct table
{
	struct
	{
		unsigned key;
		unsigned value;
	} rows [NUM_ROWS];
} table;

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Number of operations.
 */
#ifndef __NUM_QUERIES
#define __NUM_QUERIES 8
#endif

/**
 * @brief Number of writers.
 */
#ifndef __NUM_WRITERS
#define __NUM_WRITERS 1
#endif

/**
 * @brief Number of readers.
 */
#define NUM_READERS (NANVIX_PROC_MAX - __NUM_WRITERS)

/**
 * @group IPC
 */
/**@{*/
#define SEM_KEY_MUTEX 100        /* Mutex            */
const char *dbname = "simpledb"; /* Name of Database */
/**@}*/

/**
 * @brief Performs an up operation in a semaphore.
 *
 * @param semid ID of the target semaphore.
 * @param x     Operation value.
 */
static void up(int semid, int x)
{
	struct nanvix_sembuf sembuf;

	sembuf.sem_op = x;
	sembuf.sem_flg = 0;
	uassert(__nanvix_semop(semid, &sembuf, 1) == 0);
}

/**
 * @brief Performs an down operation in a semaphore.
 *
 * @param semid ID of the target semaphore.
 * @param x     Operation value.
 */
static void down(int semid, int x)
{
	struct nanvix_sembuf sembuf;

	sembuf.sem_op = -x;
	sembuf.sem_flg = 0;
	uassert(__nanvix_semop(semid, &sembuf, 1) == 0);
}

/**
 * @brief Writer.
 */
static void writer(void)
{
	int sem_readers;
	int shmid;
	uint64_t t;
	barrier_t barrier;
	int nodes[NANVIX_PROC_MAX];

	/* Build list of nodes. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i] = PROCESSOR_NODENUM_LEADER + i;

	barrier = barrier_create(nodes, NANVIX_PROC_MAX);
	uassert(BARRIER_IS_VALID(barrier));

	/* IPC initialization. */
	uassert((sem_readers = __nanvix_semget(SEM_KEY_MUTEX, IPC_CREAT)) >= 0);
	if (knode_get_num() == PROCESSOR_NODENUM_LEADER)
	{
		uassert((shmid = __nanvix_shm_open(dbname, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR)) >= 0);
		uassert(__nanvix_shm_ftruncate(shmid, sizeof(struct table)) == 0);
		up(sem_readers, NUM_READERS);
		uassert(barrier_wait(barrier) == 0);
	}
	else
	{
		uassert(barrier_wait(barrier) == 0);
		uassert((shmid = __nanvix_shm_open(dbname, O_RDWR, 0)) >= 0);
	}

	uassert(barrier_wait(barrier) == 0);

	perf_start(0, PERF_CYCLES);

		/* Read from the db. */
		for (int i = 0; i < __NUM_QUERIES; i++)
		{
			down(sem_readers, NUM_READERS);

			/* Write data. */
			uassert(__nanvix_shm_read(shmid, &table, sizeof(struct table), 0) == sizeof(struct table));
			uassert(__nanvix_shm_write(shmid, &table, sizeof(struct table), 0) == sizeof(struct table));

			up(sem_readers, NUM_READERS);
		}

		uassert(barrier_wait(barrier) == 0);

	perf_stop(0);
	t = perf_read(0);

	uassert(barrier_wait(barrier) == 0);

	/* House keeping. */
	uassert(__nanvix_shm_close(shmid) == 0);
	if (knode_get_num() == PROCESSOR_NODENUM_LEADER)
		uassert(__nanvix_shm_unlink(dbname) == 0);
	uassert(__nanvix_sem_close(sem_readers) == 0);
	uassert(barrier_destroy(barrier) == 0);

	if (knode_get_num() == PROCESSOR_NODENUM_LEADER)
	{
#ifdef NDEBUG
		uprintf("[benchmarks][simpledb] queries %d, time %l",
#else
		uprintf("[benchmarks][simpledb] %d %l",
#endif
			__NUM_QUERIES,
			t
		);
	}
}

/**
 * @brief Reader.
 */
static void reader(void)
{
	int sem_readers;
	int shmid;
	barrier_t barrier;
	int nodes[NANVIX_PROC_MAX];

	/* Build list of nodes. */
	for (int i = 0; i < NANVIX_PROC_MAX; i++)
		nodes[i] = PROCESSOR_NODENUM_LEADER + i;

	/* IPC initialization. */
	barrier = barrier_create(nodes, NANVIX_PROC_MAX);
	uassert(BARRIER_IS_VALID(barrier));

	uassert(barrier_wait(barrier) == 0);

	uassert((sem_readers = __nanvix_semget(SEM_KEY_MUTEX, IPC_CREAT)) >= 0);
	uassert((shmid = __nanvix_shm_open(dbname, O_RDWR, 0)) >= 0);

	uassert(barrier_wait(barrier) == 0);

	/* Write to the db. */
	for (int i = 0; i < __NUM_QUERIES; i++)
	{
		down(sem_readers, 1);

		/* Read data. */
		uassert(__nanvix_shm_read(shmid, &table, sizeof(struct table), 0) == sizeof(struct table));

		up(sem_readers, 1);
	}

	uassert(barrier_wait(barrier) == 0);

	/* House keeping. */
	uassert(__nanvix_shm_close(shmid) == 0);
	uassert(__nanvix_sem_close(sem_readers) == 0);
	uassert(barrier_wait(barrier) == 0);
	uassert(barrier_destroy(barrier) == 0);
}

/**
 * @brief Benchmarks System V Shared Memory Regions.
 */
static void benchmark_simpledb(void)
{
	void (*fn)(void);

	fn = (knode_get_num() < (PROCESSOR_NODENUM_LEADER + __NUM_WRITERS)) ?
		writer : reader;

	fn();
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Launches a benchmark.
 */
int __main3(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	benchmark_simpledb();

	return (0);
}
