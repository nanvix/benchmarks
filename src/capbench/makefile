#
# MIT License
#
# Copyright(c) 2011-2020 The Maintainers of Nanvix
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

#===============================================================================
# C Source Files
#===============================================================================

export SRC = $(wildcard $(CURDIR)/common/*.c)

#===============================================================================

# Builds Everything
all: all-km all-fn all-gf

# Cleans Build Objects
clean: clean-km clean-fn clean-gf

# Cleans Everything
distclean: distclean-km distclean-fn distclean-gf

#===============================================================================
# KM Benchmark
#===============================================================================

all-km:
	$(MAKE) -C km all

clean-km:
	$(MAKE) -C km clean

distclean-km:
	$(MAKE) -C km distclean

#===============================================================================
# FN Benchmark
#===============================================================================

all-fn:
	$(MAKE) -C fn all

clean-fn:
	$(MAKE) -C fn clean

distclean-fn:
	$(MAKE) -C fn distclean

#===============================================================================
# GF Benchmark
#===============================================================================

all-gf:
	$(MAKE) -C gf all

clean-gf:
	$(MAKE) -C gf clean

distclean-gf:
	$(MAKE) -C gf distclean
