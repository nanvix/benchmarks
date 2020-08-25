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

#include <nanvix/ulib.h>

#include <posix/stdlib.h>

#define min3(a,b,c) \
	((a) < (b) && (a) < (c)) ? (a) : ((b) < (a) && (b) < (c)) ? (b) : (c)

#define D(i, j) d[(i)*(lt + 1) + (j)]

/**
 * @brief Computes the differences between two strings.
 */
int diff(const char *s, const char *t)
{
	int dist;
	int ls = ustrlen(s);
	int lt = ustrlen(t);
	int *d = nanvix_malloc((ls + 1)*(lt + 1)*sizeof(int));
	uassert(d != NULL);

	for (int i = 0; i < ls + 1; i++)
	{
		for (int j = 0; j < lt + 1; j++)
			D(i, j) = -1;
	}

	for (int i = 1; i <= ls; i++)
	{
		for (int j = 1; j <= lt; j++)
		{
			int cost = (s[i - 1] == t[j - 1]) ? 0 : 1;

			int del = D(i - 1, j) + 1;
			int ins = D(i, j - 1) + 1;
			int subst = D(i - 1, j - 1) + cost;

			D(i,j) = min3(del, ins, subst);
		}
	}

	dist = D(ls,lt) + 1;

	nanvix_free(d);

	return dist;
}
