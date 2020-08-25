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

/**
 * @brief Size of Alphabet
 */
#define ALPHABET_SIZE 256

/**
 * @brief Table of jumps;
 */
static int jmptable[ALPHABET_SIZE];

/**
 * @brief Greps a pattern.
 */
const char *grep(const char *text, const char *pattern)
{
	int textlen = ustrlen(text);
	int patternlen = ustrlen(pattern);

	/* Initialize table of jumps. */
	for (int i = 0; i < ALPHABET_SIZE; i++)
		jmptable[i] = patternlen + 1;
	for (int i = 0; i < patternlen; i++)
		jmptable[(int)pattern[i]] = patternlen - i;

	/* Traverse text. */
	for (int i = 0, p = 0; (i + patternlen) <= textlen; /* noop */)
	{
		/* Search pattern backwards. */
		for (int j = patternlen - 1; j >= 0; j--)
		{
			/* Mismatch. */
			if (text[i + j] != pattern[j])
			{
				p = 0;
				i += jmptable[(int)text[i + patternlen]];
				break;
			}

			p++;
		}

		/* Found. */
		if (p == patternlen)
			return (&text[i]);
	}

	return (NULL);
}
