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

#include <posix/stdlib.h>

#include "gf.h"

/**
 * @brief Kernel Data
 */
/**@{*/
static unsigned char *img;    /* Input Image.  */
static unsigned char *newimg; /* Output Image. */
static double *mask;           /* Mask.         */
/**@}*/

/*============================================================================*
 * Statistics                                                                 *
 *============================================================================*/

uint64_t total_time = 0;
unsigned char line[PROBLEM_IMGSIZE + 1];

/*============================================================================*
 * Initialization                                                             *
 *============================================================================*/

/**
 * @brief Generates the special mask.
 */
static void generate_mask(void)
{
    double sec;
    double first;
    double total_aux;

    total_aux = 0;
    first     = __fdiv(1.0,(2.0 * PI * SD * SD));

    for (int i = -HALF; i <= HALF; i++)
    {
        for (int j = -HALF; j <= HALF; j++)
        {
            sec = -((i * i + j * j) * __fdiv(1.0/2.0) * (SD * SD));
            sec = power(E,sec);

            MASK(i + (HALF), j + (HALF)) = first * sec;
            total_aux += MASK(i + (HALF), j + (HALF));
        }
    }

    for (int i = 0; i < PROBLEM_MASKSIZE; i++)
    {
        for (int j = 0; j < PROBLEM_MASKSIZE; j++)
            MASK(i, j) = __fdiv(MASK(i, j), total_aux);
    }
}

/**
 * @brief Initializes the image matrix.
 */
static void init(void)
{
	srandnum(PROBLEM_SEED);
    for (int i = 0; i < PROBLEM_IMGSIZE2; i++)
        img[i] = randnum() & 0xff;

    generate_mask();
}

/*
 * Gaussian filter.
 */
static void gauss_filter(void)
{
    double pixel;

    for (int imgI = HALF; imgI < (PROBLEM_IMGSIZE - HALF); imgI++)
    {
        for (int imgJ = HALF; imgJ < (PROBLEM_IMGSIZE - HALF); imgJ++)
        {
            pixel = 0.0;

            for (int maskI = 0; maskI < PROBLEM_MASKSIZE; maskI++)
            {
                for (int maskJ = 0; maskJ < PROBLEM_MASKSIZE; maskJ++)
                    pixel += IMAGE(imgI + maskI - HALF, imgJ + maskJ - HALF) *
                             MASK(maskI, maskJ);
            }

            NEWIMAGE(imgI, imgJ) = (pixel > 255) ? 255 : (int)pixel;
        }
    }
}

/*============================================================================*
 * Kernel                                                                     *
 *============================================================================*/

void do_kernel(void)
{
	/* Allocates memory for original image. */
	if ((img = (unsigned char *) nanvix_malloc(sizeof(unsigned char) * PROBLEM_IMGSIZE2)) == NULL)
	{
		upanic("Error in image memory allocation.");
		return;
	}

	/* Allocates memory to the gaussian mask. */
	if ((mask = (double *) nanvix_malloc(sizeof(double) * PROBLEM_MASKSIZE2)) == NULL)
	{
		upanic("Error in mask allocation.");
		return;
	}

	/* Allocates memory for the new image. */
	if ((newimg = (unsigned char *) nanvix_malloc(sizeof(unsigned char) * PROBLEM_IMGSIZE2)) == NULL)
	{
		upanic("Error in output image memory allocation.");
		return;
	}

    uprintf("initializing...\n");

	init();

	/* Apply filter. */
	uprintf("applying filter...\n");

	/* Starts to count the solution total time. */
	perf_start(0, PERF_CYCLES);

	gauss_filter();

	total_time += perf_read(0);

	/* Frees the allocated memory. */
	nanvix_free((void *) newimg);
	nanvix_free((void *) mask);
	nanvix_free((void *) img);
}
