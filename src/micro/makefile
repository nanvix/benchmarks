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

# Builds everything.
all: all-hello all-pgfault all-memalloc all-memread all-memwrite all-memset \
	all-mail

# Cleans up build objects.
clean: clean-hello clean-pgfault clean-memalloc clean-memread clean-memwrite \
	clean-memset clean-mail

# Cleans up everything.
distclean: distclean-hello distclean-pgfault distclean-memalloc \
	distclean-memread distclean-memwrite distclean-memset distclean-mail

#===============================================================================
# Hello Benchmark
#===============================================================================

# Builds benchmark.
all-hello:
	$(MAKE) -C hello all

# Cleans up build objects.
clean-hello:
	$(MAKE) -C hello clean

# Cleans up everything.
distclean-hello:
	$(MAKE) -C hello distclean

#===============================================================================
# Page Fault Benchmark
#===============================================================================

# Builds benchmark.
all-pgfault:
	$(MAKE) -C pgfault all

# Cleans up build objects.
clean-pgfault:
	$(MAKE) -C pgfault clean

# Cleans up everything.
distclean-pgfault:
	$(MAKE) -C pgfault distclean

#===============================================================================
# Memory Allocation
#===============================================================================

# Builds benchmark.
all-memalloc:
	$(MAKE) -C memalloc all

# Cleans up build object.
clean-memalloc:
	$(MAKE) -C memalloc clean

# Cleans up everything.
distclean-memalloc:
	$(MAKE) -C memalloc distclean

#===============================================================================
# Memory Read
#===============================================================================

# Builds benchmark.
all-memread:
	$(MAKE) -C memread all

# Cleans up build object.
clean-memread:
	$(MAKE) -C memread clean

# Cleans up everything.
distclean-memread:
	$(MAKE) -C memread distclean

#===============================================================================
# Memory Write
#===============================================================================

# Builds benchmark.
all-memwrite:
	$(MAKE) -C memwrite all

# Cleans up build object.
clean-memwrite:
	$(MAKE) -C memwrite clean

# Cleans up everything.
distclean-memwrite:
	$(MAKE) -C memwrite distclean

#===============================================================================
# Memory Set
#===============================================================================

# Builds benchmark.
all-memset:
	$(MAKE) -C memset all

# Cleans up build object.
clean-memset:
	$(MAKE) -C memset clean

# Cleans up everything.
distclean-memset:
	$(MAKE) -C memset distclean

#===============================================================================
# Mail
#===============================================================================

# Builds benchmark.
all-mail:
	$(MAKE) -C mail all

# Cleans up build object.
clean-mail:
	$(MAKE) -C mail clean

# Cleans up everything.
distclean-mail:
	$(MAKE) -C mail distclean
