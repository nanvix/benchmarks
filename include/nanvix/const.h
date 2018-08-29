/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef NANVIX_CONST_H_
#define NANVIX_CONST_H_

	#define __NEED_HAL_CONST_
	#include <nanvix/hal.h>

	/**
	 * @brief Number of NoC nodes.
	 */
	#define NANVIX_NR_NODES HAL_NR_NOC_NODES

	/**
	 * @brief Spawner Server NoC node number.
	 */
	#define SPAWNER_SERVER_NODE 16

	/**
	 * @brief Shared Memory Region Server NoC node number.
	 */
	#define SHM_SERVER_NODE 17

	/**
	 * @brief Spawner Server NoC node number.
	 */
	#define SPAWNER1_SERVER_NODE 20

	/**
	 * @brief Name Server NoC node number.
	 */
	#define NAME_SERVER_NODE 21

	/**
	 * @brief Remote Memory Server NoC node number.
	 */
	#define RMEM_SERVER_NODE 22

	/**
	 * @brief Semaphores Server NoC node number.
	 */
	#define SEMAPHORE_SERVER_NODE 23

	/**
	 * @brief Startup delay (in seconds).
	 */
	#define STARTUP_DELAY 5

#endif /* CONST_H_ */

