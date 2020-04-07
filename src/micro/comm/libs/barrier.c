/*
 * MIT License
 *
 * Copyright(c) 2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
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

#include <posix/errno.h>
#include <nanvix/ulib.h>
#include <nanvix/sys/sync.h>
#include <nanvix/sys/noc.h>

#if (__TARGET_HAS_SYNC)

/*----------------------------------------------------------------------------*
 * Nodes                                                                      *
 *----------------------------------------------------------------------------*/

static int _syncin         = -1;
static int _syncout        = -1;
static int _node_is_master = 0;

/**
 * @brief Barrier setup function.
 */
void barrier_nodes_setup(const int * nodes, int nnodes, int is_master)
{
	KASSERT((nodes != NULL) && (nnodes > 0));

	_node_is_master = is_master;

	if (_node_is_master)
	{
		_syncin  = ksync_create(nodes, nnodes, SYNC_ALL_TO_ONE);
		_syncout = ksync_open(nodes, nnodes, SYNC_ONE_TO_ALL);
	}
	else
	{
		_syncout = ksync_open(nodes, nnodes, SYNC_ALL_TO_ONE);
		_syncin  = ksync_create(nodes, nnodes, SYNC_ONE_TO_ALL);
	}

	KASSERT((_syncout >= 0) && (_syncin >= 0));
}

/**
 * @brief Barrier function.
 */
void barrier_nodes(void)
{
	if (_node_is_master)
	{
		ksync_wait(_syncin);
		ksync_signal(_syncout);
	}
	else
	{
		ksync_signal(_syncout);
		ksync_wait(_syncin);
	}
}

/**
 * @brief Barrier cleanup function.
 */
void barrier_nodes_cleanup(void)
{
	ksync_unlink(_syncin);
	ksync_close(_syncout);

	_syncin    = -1;
	_syncout   = -1;
	_node_is_master = 0;
}

#endif /* __TARGET_HAS_SYNC */

/*----------------------------------------------------------------------------*
 * Cores                                                                      *
 *----------------------------------------------------------------------------*/

/**
 * @brief A simple fence.
 */
struct fence
{
	int volatile ncores;   /**< Number of cores in the fence.            */
	int volatile nreached; /**< Number of cores that reached the fence.  */
	int volatile release;  /**< Wait condition.                          */
	spinlock_t lock;       /**< Lock.                                    */
};

static volatile int _exit  = 0;
static spinlock_t _lock    = SPINLOCK_UNLOCKED;
static struct fence _fence = { 0, 0, 0, SPINLOCK_UNLOCKED };

/**
 * @brief Initializes a fence.
 *
 * @param b      Target fence.
 * @param ncores Number of cores in the fence.
 */
PRIVATE void fence_init(struct fence *b, int ncores)
{
	b->ncores   = ncores;
	b->nreached = 0;
}

/**
 * @brief Waits in a fence.
 */
PRIVATE void fence(struct fence *b)
{
	int try;
	int exit;
	int local_release;

	/* Notifies thread reach. */
	spinlock_lock(&b->lock);

		local_release = !b->release;

		b->nreached++;

		if (b->nreached == b->ncores)
		{
			b->nreached = 0;
			b->release  = local_release;
		}

	spinlock_unlock(&b->lock);

	try = 0;
	exit = 0;
	while (!exit)
	{
		spinlock_lock(&b->lock);
			exit = (local_release == b->release);
		spinlock_unlock(&b->lock);

		if (++try == 100000)
			uprintf("I'm stuck on fence release");
	}
}

/**
 * @brief Barrier setup function.
 */
void barrier_cores_setup(int tid, int ncores)
{
	int try;
	int exit;

	if (tid == 0)
	{
		spinlock_lock(&_lock);
			fence_init(&_fence, ncores);
			_exit = 1;
		spinlock_unlock(&_lock);
	}
	else
	{
		try = 0;
		exit = 0;
		while (!exit)
		{
			spinlock_lock(&_lock);
				exit = _exit;
			spinlock_unlock(&_lock);

			if (++try == 100000)
				uprintf("I'm stuck on fence setup");
		}
	}

	fence(&_fence);
}

/**
 * @brief Barrier function.
 */
void barrier_cores(void)
{
	fence(&_fence);
}

/**
 * @brief Barrier cleanup function.
 */
void barrier_cores_cleanup(int tid)
{
	if (tid == 0)
		_exit = 0;

	fence(&_fence);	
}
