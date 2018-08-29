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

#ifndef NANVIX_PM_H_
#define NANVIX_PM_H_

	#include <sys/types.h>
	#include <stddef.h>

/*============================================================================*
 * Barrier                                                                    *
 *============================================================================*/

	/* Forward definitions. */
	extern int barrier_create(int *, int);
	extern int barrier_wait(int);
	extern int barrier_unlink(int);

/*============================================================================*
 * Mailbox                                                                    *
 *============================================================================*/

	/* Forward definitions .*/
	extern int mailbox_setup(int);
	extern int mailbox_cleanup(void);
	extern int get_inbox(void);
	extern int get_named_inbox(void);
	extern int mailbox_create(char *);
	extern int mailbox_open(char *);
	extern int mailbox_read(int, void *, size_t);
	extern int mailbox_write(int, const void *, size_t);
	extern int mailbox_close(int);
	extern int mailbox_unlink(int);

/*============================================================================*
 * Portal                                                                     *
 *============================================================================*/

	/* Forward definitions .*/
	extern int portal_setup(int);
	extern int portal_cleanup(void);
	extern int get_inportal(void);
	extern int portal_allow(int, int);
	extern int portal_create(char *);
	extern int portal_open(char *);
	extern ssize_t portal_read(int, void *, size_t);
	extern ssize_t portal_write(int, const void *, size_t);
	extern int portal_close(int);
	extern int portal_unlink(int);

/*============================================================================*
 * Name                                                                       *
 *============================================================================*/

	/* Forward definitions. */
	extern int get_name(char *);

#endif /* NANVIX_PM_H_ */
