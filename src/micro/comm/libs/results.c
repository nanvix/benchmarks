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

#include <posix/stddef.h>
#include <nanvix/sys/noc.h>
#include <nanvix/sys/mailbox.h>
#include <nanvix/runtime/stdikc.h>

#include "../comm.h"

#if (__TARGET_HAS_PORTAL || __TARGET_HAS_MAILBOX)

void result_write(struct saturation_result * result)
{
	int outbox;

	/* Needs to be a slave. */
	KASSERT((cluster_get_num() != PROCESSOR_CLUSTERNUM_MASTER));

	KASSERT((outbox = kmailbox_open(PROCESSOR_NODENUM_MASTER, 0)) >= 0);

	barrier_nodes();

		KASSERT(
			kmailbox_write(
				outbox,
				result,
				sizeof(struct saturation_result)
			) == sizeof(struct saturation_result)
		);

	barrier_nodes();

	KASSERT(kmailbox_close(outbox) == 0);
}

void result_read(int nslaves, struct saturation_result * result)
{
	int inbox;
	uint64_t opening;
	uint64_t requesting;
	uint64_t communication;
	uint64_t closing;
	uint64_t message_size;
	uint64_t worst_sum;

	/* Needs to be the master. */
	KASSERT((cluster_get_num() == PROCESSOR_CLUSTERNUM_MASTER));

	opening       = ~0ULL;
	requesting    = ~0ULL;
	communication = ~0ULL;
	closing       = ~0ULL;
	message_size  = ~0ULL;
	worst_sum     = 0ULL;

	__stdmailbox_setup();

		barrier_nodes();

		KASSERT((inbox = stdinbox_get()) >= 0);

		for (int i = 0; i < nslaves; ++i)
		{
			KASSERT(
				kmailbox_read(
					inbox,
					result,
					sizeof(struct saturation_result)
				) == sizeof(struct saturation_result)
			);

			uint64_t sum = result->opening
				+ result->requesting
				+ result->communication
				+ result->closing;

			if (worst_sum < sum)
			{
				opening       = result->opening;
				requesting    = result->requesting;
				communication = result->communication;
				closing       = result->closing;
				message_size  = result->message_size;
			}
		}

		barrier_nodes();

	__stdmailbox_cleanup();

	result->opening       = opening;
	result->requesting    = requesting;
	result->communication = communication;
	result->closing       = closing;
	result->message_size  = message_size;
}

#endif /* __TARGET_HAS_PORTAL || __TARGET_HAS_MAILBOX */