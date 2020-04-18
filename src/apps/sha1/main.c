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

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>

extern void sha1(char *hash_out, const char *str, size_t len);

/**
 * @brief NIST Secure Hash Algorithm (SHA1) Benchmark
 *
 * @param argc Argument counter.
 * @param argv Argument variables.
 */
int __main2(int argc, const char *argv[])
{

	char hash[21];
	const char *str = "hello world!";
	uint64_t time_sha1;

	((void) argc);
	((void) argv);

	__runtime_setup(0);

		/* Unblock spawner. */
		uassert(stdsync_fence() == 0);

		__runtime_setup(3);

		perf_start(0, PERF_CYCLES);
		sha1(hash, str, ustrlen(str));
		perf_stop(0);
		time_sha1 = perf_read(0);

#ifndef NDEBUG
		uprintf("[apps][sha1] time = %l",
#else
		uprintf("[apps][sha1] %l",
#endif
			time_sha1
		);

		nanvix_shutdown();

	__runtime_cleanup();

	return (0);
}
