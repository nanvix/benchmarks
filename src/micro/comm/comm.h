/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
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

#ifndef _COMM_H_
#define _COMM_H_

	#include <nanvix/runtime/stdikc.h>
	#include <nanvix/sys/dev.h>
    #include <nanvix/ulib.h>
    #include <kbench.h>

    /**
     * @brief Executable routine
     */
    extern int (*enabled_routine_fn)(const int *, int, int, int);

    /**
     * @name Configuration of the routines
     */
    /**@{*/
    extern const int NUM_NODES;
    extern const int MESSAGE_SIZE;
    extern const int * nodenums;
    /**@}*/

    /**
     * @brief Portal message size
     */
    #define MAX_MESSAGE_SIZE (512)

    /**
     * @brief Portal message size
     */
    #define MAX_NUM_NODES 18

    /**
     * @name Barrier functions
     */
    /**@{*/
    extern void barrier_nodes_setup(const int * nodes, int nnodes, int is_master);
    extern void barrier_nodes(void);
    extern void barrier_nodes_cleanup(void);
    extern void barrier_cores_setup(int tid, int ncores);
    extern void barrier_cores(void);
    extern void barrier_cores_cleanup(int tid);
    /**@}*/

    /**
     * @brief Structure to benchmark results
     */
    struct benchmark_result
	{
		size_t volume;
		uint64_t latency;
	};

    struct saturation_result
	{
		uint64_t opening;
        uint64_t requesting;
        uint64_t communication;
        uint64_t closing;
        uint64_t message_size;
	};

    /**
     * @name Result functions
     */
    /**@{*/
    extern void result_write(struct saturation_result * result);
    extern void result_read(int nslaves, struct saturation_result * result);
    /**@}*/

    /**
     * @brief Horizontal line.
     */
    extern const char *HLINE;

#endif
