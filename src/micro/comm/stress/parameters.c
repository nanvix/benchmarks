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

#include "parameters.h"

#if __TARGET_HAS_PORTAL

/**
 * @name Configuration of the routines (Involved clusters)
 */
/**@{*/
#define _NUM_NODES 2
const int _nodenums[_NUM_NODES] ALIGN(sizeof(uint64_t)) = {
#ifdef __mppa256__
	0, 8
#else
	0, 1
#endif
};
/**@}*/

/**
 * @name Configuration exported
 */
/**@{*/
const int NUM_NODES    = _NUM_NODES;
const int MESSAGE_SIZE = MAX_MESSAGE_SIZE;
const int * nodenums   = &_nodenums[0];
/**@}*/

/**
 * @name Collective communication routines functions
 */
/**@{*/
extern int do_mailbox_simple(const int *, int, int, int);
extern int do_mailbox_complex(const int *, int, int, int);
extern int do_portal_simple(const int *, int, int, int);
extern int do_portal_complex(const int *, int, int, int);
extern int do_ikc_simple(const int *, int, int, int);
extern int do_ikc_complex(const int *, int, int, int);
/**@}*/

/**
 * @brief Executable routine
 */
int (*enabled_routine_fn)(const int *, int, int, int) = &do_mailbox_simple;

#endif /* __TARGET_HAS_PORTAL */