/*
 * Steve Reid <steve@edmweb.com>
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SHA1_H
#define SHA1_H

	#include <posix/sys/types.h>
	#include <posix/stdint.h>

	struct sha1_ctx
	{
		uint32_t state[5];
		uint32_t count[2];
		unsigned char buffer[64];
	};

	void SHA1Transform(
		uint32_t state[5],
		const unsigned char buffer[64]
	);

	void SHA1Init(
		struct sha1_ctx *context
	);

	void SHA1Update(
		struct sha1_ctx *context,
		const unsigned char *data,
		uint32_t len
	);

	void SHA1Final(
		unsigned char digest[20],
		struct sha1_ctx *context
	);

	void SHA1(
		char *hash_out,
		const char *str,
		size_t len
	);

#endif /* SHA1_H */
