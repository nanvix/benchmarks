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
#include <nanvix/sys/perf.h>
#include <nanvix/ulib.h>
#include <posix/unistd.h>

/**
 * @brief File Offset for Tests
 */
#define TEST_FILE_OFFSET (8*1024)

/*============================================================================*
 * Benchmark                                                                  *
 *============================================================================*/

/**
 * @brief Size of Read/Write Buffer
 */
#define BUFFER_SIZE 1024

/**
 * @brief Dummy buffer used for tests.
 */
static char buffer[RMEM_BLOCK_SIZE];

/**
 * @brief Benchmarks file write.
 */
static void benchmark_fwrite(void)
{
	int fd;
	const char *filename = "disk";
	uint64_t time_fwrite;

	perf_start(0, PERF_CYCLES);

		uassert((fd = nanvix_vfs_open(filename, O_RDWR)) >= 0);

			umemset(buffer, 1, BUFFER_SIZE);
			uassert(nanvix_vfs_seek(fd, TEST_FILE_OFFSET, SEEK_SET) >= 0);
			uassert(nanvix_vfs_write(fd, buffer, BUFFER_SIZE) == BUFFER_SIZE);

		uassert(nanvix_vfs_close(fd) == 0);

	perf_stop(0);
	time_fwrite = perf_read(0);


#ifndef NDEBUG
	uprintf("[benchmarks][fwrite] %l",
#else
	uprintf("[benchmarks][fwrite] %l",
#endif
		time_fwrite
	);
}

/*============================================================================*
 * Benchmark Driver                                                           *
 *============================================================================*/

/**
 * @brief Launches a benchmark.
 */
int __main3(int argc, const char *argv[])
{
	((void) argc);
	((void) argv);

	benchmark_fwrite();

	return (0);
}
