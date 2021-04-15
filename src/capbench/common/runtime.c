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

#include "../common.h"

#if !__NANVIX_USES_LWMPI

#include <nanvix/sys/thread.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/runtime/barrier.h>
#include <nanvix/runtime/fence.h>
#include <nanvix/runtime/runtime.h>
#include <nanvix/ulib.h>
#include <nanvix/pm.h>

PRIVATE int ipc_processes_nr = MPI_PROCESSES_NR;
PRIVATE int ipc_local_processes_nr;
PRIVATE int ipc_master_tid;

PRIVATE struct ipc_process
{
	int rank;
	kthread_t tid;
} ipc_processes[THREAD_MAX];

PRIVATE barrier_t ipc_barrier;
PRIVATE struct fence_t ipc_fence;

PRIVATE int spawn_processes(void);

#endif

/**
 * Initializes runtime.
 */
PUBLIC int runtime_init(int argc, char **argv)
{
#if __NANVIX_USES_LWMPI
	MPI_Init(&argc, &argv);

	return (0);
#else
	UNUSED(argc);
	UNUSED(argv);

	return (spawn_processes());
#endif
}

/**
 * Finalize runtime.
 */
PUBLIC void runtime_finalize(void)
{
#if __NANVIX_USES_LWMPI
	MPI_Finalize();
#else
	barrier_destroy(ipc_barrier);
#endif
}

/**
 * Gets process rank.
 */
PUBLIC void runtime_get_rank(int * rank)
{
#if __NANVIX_USES_LWMPI
	MPI_Comm_rank(MPI_COMM_WORLD, rank);
#else

	int index;

	uassert((index = runtime_get_index()) >= 0);

	*rank = ipc_processes[index].rank;

#endif
}

/**
 * Gets process index.
 */
PUBLIC int runtime_get_index(void)
{
#if __NANVIX_USES_LWMPI
	return (curr_mpi_proc_index());
#else

	kthread_t tid = kthread_self();

	for (int i = 0; i < THREAD_MAX; ++i)
		if (ipc_processes[i].tid == tid)
			return (i);

	return (-1);

#endif
}

/**
 * Waits on a fence.
 */
PUBLIC void runtime_fence(void)
{
#if !__NANVIX_USES_LWMPI
	fence(&ipc_fence);
#endif
}

/**
 * Waits on a barrier.
 */
PUBLIC void runtime_barrier(void)
{
#if !__NANVIX_USES_LWMPI
	barrier_wait(ipc_barrier);
#endif
}

#if !__NANVIX_USES_LWMPI

PRIVATE void * wrapper_do_slave(void * arg)
{
	int rank;

	UNUSED(arg);

	uprintf("[cluster %d][rank %d][index %d][run] Running %d.",
		kthread_self()
	);

	runtime_fence();

	__stdmailbox_setup();
	__stdportal_setup();

	runtime_get_rank(&rank);
	uprintf("[cluster %d][rank %d][index %d][run] Running (port %d : %d).",
		kcluster_get_num(),
		rank,
		runtime_get_index(),
		stdinbox_get_port(),
		stdinportal_get_port()
	);

	return (NULL);
}

PRIVATE int spawn_processes(void)
{
	int first_pid;       /* First Local process ID. */
	int local_processes; /* Test verification.      */
	kthread_t tid;     /* Created thread ID.      */
	int nodes[MPI_NODES_NR];

	/* Checks if the current node will be active. */
	if (knode_get_num() >= (MPI_BASE_NODE + MPI_NODES_NR))
	{
		uprintf("Unactive node returning...");
		return (1);
	}

	/* Initializes the std_barrier if more than a single cluster is active. */
#if (MPI_NODES_NR > 1)

	/* Initializes list of nodes for stdbarrier. */
	for (int i = 0; i < MPI_NODES_NR; ++i)
		nodes[i] = MPI_BASE_NODE + i;

	ipc_barrier = barrier_create(nodes, MPI_NODES_NR);

	uassert(BARRIER_IS_VALID(ipc_barrier));

	uprintf("[barrier] %d nodes, begun %d", MPI_NODES_NR, MPI_BASE_NODE);

#else
	ipc_barrier = BARRIER_NULL;
#endif

	/* Calculates the lowest local MPI process number. */
#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)

	first_pid = kcluster_get_num() - MPI_PROCESSES_COMPENSATION;

#elif (__LWMPI_PROC_MAP == MPI_PROCESS_COMPACT)

	first_pid = (kcluster_get_num() - MPI_PROCESSES_COMPENSATION) * MPI_PROCS_PER_CLUSTER_MAX;

#endif

	/* Specifies the current thread as the master of the current cluster. */
	ipc_master_tid = kthread_self();

	ipc_processes[0].tid  = ipc_master_tid;
	ipc_processes[0].rank = first_pid;

	/* Calculates how many processes will be locally initialized. */
#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)

	ipc_local_processes_nr = ((int) (MPI_PROCESSES_NR / MPI_NODES_NR)) +
				 ((first_pid < (MPI_PROCESSES_NR % MPI_NODES_NR)) ? 1 : 0
			      );

#elif (__LWMPI_PROC_MAP == MPI_PROCESS_COMPACT)

	if ((kcluster_get_num() - MPI_PROCESSES_COMPENSATION) < (MPI_NODES_NR - 1))
		ipc_local_processes_nr = MPI_PROCS_PER_CLUSTER_MAX;
	else
	{
		ipc_local_processes_nr = MPI_PROCESSES_NR % MPI_PROCS_PER_CLUSTER_MAX;

		/* Checks if the last cluster is also full. */
		if (ipc_local_processes_nr == 0)
			ipc_local_processes_nr = MPI_PROCS_PER_CLUSTER_MAX;
	}

#endif

#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)
	uprintf("Spawning processes in SCATTER mode...");
#elif (__LWMPI_PROC_MAP == MPI_PROCESS_COMPACT)
	uprintf("Spawning processes in COMPACT mode...");
#endif

#if DEBUG
	#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)
	uprintf("Active nodes: %d", MPI_NODES_NR);
	uprintf("Number of processes in the current cluster: %d", ipc_local_processes_nr);
	#elif (__LWMPI_PROC_MAP == MPI_PROCESS_COMPACT)
	uprintf("Active nodes: %d", MPI_NODES_NR);
	uprintf("Number of processes in the current cluster: %d", ipc_local_processes_nr);
	#endif
#endif

	/* Checks if more than a single process will be spawned in the current cluster. */
	if (ipc_local_processes_nr > 1)
	{
		/* THREADS ARE NECESSARY TO EMULATE PROCESSES. */
		local_processes = 1;

		/* Initializes the std_fence. */
		fence_init(&ipc_fence, ipc_local_processes_nr);

		uprintf("[fence] %d processes", ipc_local_processes_nr);

		/* Initializes the local processes list and create the threads that will run them. */
#if (__LWMPI_PROC_MAP == MPI_PROCESS_SCATTER)
		for (int i = 1, id = (first_pid + MPI_NODES_NR); id < ipc_processes_nr; id += MPI_NODES_NR, ++i)
#elif (__LWMPI_PROC_MAP == MPI_PROCESS_COMPACT)
		for (int i = 1, id = (first_pid + 1); i < ipc_local_processes_nr; i++, id++)
#endif
		{
#if DEBUG
			uprintf("Spawning mpi-process-%d", id);
#endif /* DEBUG */

			uassert(i < THREAD_MAX);

			/* Creates a thread to emulate an MPI process. */
			uassert(kthread_create(&tid, &wrapper_do_slave, NULL) == 0);

			ipc_processes[i].rank = id;
			ipc_processes[i].tid  = tid;

#if DEBUG
			uprintf("mpi-process-%d TID: %d", id, tid);
#endif /* DEBUG */

			local_processes++;
		}

		uassert(local_processes == ipc_local_processes_nr);
	}

	/* Fence. */
	int cid = kcluster_get_num();
	uprintf("[cluster %d][rank %d][index 0] Barrier among nodes.", cid, first_pid);
	runtime_barrier();

	if (ipc_local_processes_nr > 1)
	{
		uprintf("[cluster %d][rank %d][index 0] Fence among processes.", cid, first_pid);
		runtime_fence();
	}
	uprintf("[cluster %d][rank %d][index 0] Correct initialization.", cid, first_pid);

	int rank;
	runtime_get_rank(&rank);
	uprintf("[cluster %d][rank %d][index %d] Running (port %d : %d).",
		kcluster_get_num(),
		rank,
		runtime_get_index(),
		stdinbox_get_port(),
		stdinportal_get_port()
	);

	return (0);
}

#endif

