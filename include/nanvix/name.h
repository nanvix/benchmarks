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

#ifndef NANVIX_NAME_H_
#define NANVIX_NAME_H_

	#include <stdint.h>
    
	#include <nanvix/limits.h>
	#include <nanvix/message.h>

	/**
	 * @brief Operation types for name server.
	 */
	/**@{*/
	#define NAME_EXIT    0 /**< Exit request.            */
	#define NAME_LOOKUP  1 /**< lookup a name.           */
	#define NAME_LINK    2 /**< Add a new name.          */
	#define NAME_UNLINK  3 /**< Remove a name.           */
	#define NAME_SUCCESS 4 /**< Success acknowledgement. */
	#define NAME_FAIL    5 /**< Failure acknowledgement. */
	/**@}*/

	/**
	 * @brief Name server message.
	 */
	struct name_message
	{
		message_header header;           /**< Message header. */
		int32_t nodenum;                 /**< NoC node.       */
		char name[NANVIX_PROC_NAME_MAX]; /**< Portal name.    */
	};

	/* Forward definitions. */
	extern int name_init(void);
	extern int name_lookup(char *);
	extern int name_link(int, const char *);
	extern int name_unlink(const char *);

#endif /* _NAME_H_ */
