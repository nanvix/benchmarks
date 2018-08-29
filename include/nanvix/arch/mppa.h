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

#ifndef NANVIX_ARCH_MPPA256
#define NANVIX_ARCH_MPPA256

	#ifndef _KALRAY_MPPA256
		#error "bad target"
	#endif

/*============================================================================*
 * Core Interface                                                             *
 *============================================================================*/

#if (defined(__NEED_HAL_CORE_) || defined(__NEED_HAL_CONST_))

	/**
	 * @brief Number of user cores.
	 */
	#define HAL_NR_CORES_USER 256

	/**
	 * @brief Number of resource management cores.
	 */
	#define HAL_NR_CORES_RMAN 16

	/**
	 * @brief Number of system cores.
	 */
	#define HAL_NR_CORES_SYSTEM 16

	/**
	 * @brief Number of IO Clusters in the platform.
	 */
	#define HAL_NR_IOCLUSTERS 4

	/**
	 * @brief Number of Compute Clusters in the platform.
	 */
	#define HAL_NR_CCLUSTERS 16

	/**
	 * @brief Number of clusters in the platform.
	 */
	#define HAL_NR_CLUSTERS (NR_IOCLUSTER + NR_CCLUSTER)

#endif /* __NEED_HAL_CONST_ */

/*============================================================================*
 * NoC Interface                                                              *
 *============================================================================*/

#if (defined(__NEED_HAL_NOC_) || defined(__NEED_HAL_CONST_))

	/**
	 * @brief Number of NoC nodes attached to an IO device.
	 */
	#define HAL_NR_NOC_IONODES 8

	/**
	 * @define Number of NoC nodes not attached to an IO device.
	 */
	#define HAL_NR_NOC_CNODES 16

	/**
	 * @brief Number of NoC nodes.
	 */
	#define HAL_NR_NOC_NODES (HAL_NR_NOC_IONODES + HAL_NR_NOC_CNODES)

#endif /* __NEED_HAL_CONST_ */

/*============================================================================*
 * Mailbox Interface                                                          *
 *============================================================================*/

#if (defined(__NEED_HAL_MAILBOX_) || defined(__NEED_HAL_CONST_))

	/**
	 * @brief Number of mailboxes.
	 */
	#ifdef _KALRAY_MPPA_CCLUSTER_
		#define HAL_NR_MAILBOX HAL_NR_NOC_NODES
	#else
		#define HAL_NR_MAILBOX (4 + HAL_NR_NOC_NODES)
	#endif

	/**
	 * @brief Size (in bytes) of a mailbox message.
	 */
	#define HAL_MAILBOX_MSG_SIZE 120

#endif /* __NEED_HAL_CONST_ */

/*============================================================================*
 * Portal Interface                                                           *
 *============================================================================*/

#if (defined(__NEED_HAL_PORTAL_) || defined(__NEED_HAL_CONST_))

	/**
	 * @brief Number of unnamed portals.
	 */
	#ifdef _KALRAY_MPPA_CCLUSTER_
		#define HAL_NR_PORTAL HAL_NR_NOC_NODES
	#else
		#define HAL_NR_PORTAL (4*HAL_NR_NOC_NODES)
	#endif

	/**
	 * @brief Maximum size for portal data transfers (in bytes).
	 */
	#define HAL_PORTAL_MAX_SIZE (1024*1024)

#endif /* __NEED_HAL_CONST_ */

/*============================================================================*
 * Synchronization Point Interface                                            *
 *============================================================================*/

#if (defined(__NEED_HAL_SYNC_) || defined(__NEED_HAL_CONST_))

	/**
	 * @brief Number of mailboxes.
	 */
	#ifdef _KALRAY_MPPA_CCLUSTER_
		#define HAL_NR_SYNC HAL_NR_NOC_NODES
	#else
		#define HAL_NR_SYNC (4*HAL_NR_NOC_NODES)
	#endif

#endif /* __NEED_HAL_CONST_ */

#endif /* NANVIX_ARCH_MPPA256 */
