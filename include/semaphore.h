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

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

	#include <fcntl.h>
	#include <time.h>

	/**
	 * @brief Semaphore error.
	 */
	#define SEM_FAILED NULL

	/**
	 * @brief Named semaphore descriptor.
	 */
	typedef int  sem_t;

	/* Forward definitions. */
	int sem_close(sem_t *);
	int sem_destroy(sem_t *);
	int sem_getvalue(sem_t *restrict, int *restrict);
	int sem_init(sem_t *, int, unsigned);
	sem_t *sem_open(const char *, int, ...);
	int sem_post(sem_t *);
#ifdef _POSIX_C_SOURCE 
#if (_POSIX_C_SOURCE >= 200112L)
		int sem_timedwait(sem_t *restrict, const struct timespec *restrict);
#endif
#endif
	int sem_trywait(sem_t *);
	int sem_unlink(const char *);
	int sem_wait(sem_t *);

#endif /* SEMAPHORE_H_ */
