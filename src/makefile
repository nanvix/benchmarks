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

# Builds Everything
# all: all-microbenchmarks all-pseudoapps all-utilities all-samos
all: all-microbenchmarks all-samos

# Cleans Build Objects
clean: clean-microbenchmarks clean-samos

# Cleans Everything
distclean: distclean-microbenchmarks distclean-samos

#===============================================================================
# Micro Benchmarks
#===============================================================================

all-microbenchmarks:
	$(MAKE) -C microbenchmarks all

clean-microbenchmarks:
	$(MAKE) -C microbenchmarks clean

distclean-microbenchmarks:
	$(MAKE) -C microbenchmarks distclean

#===============================================================================
# Pseudo Applications
#===============================================================================

all-pseudoapps:
	$(MAKE) -C pseudoapps all

# Cleans build objects.
clean-pseudoapps:
	$(MAKE) -C pseudoapps clean

# Cleans build.
distclean-pseudoapps:
	$(MAKE) -C pseudoapps distclean

#===============================================================================
# System V Abstractions
#===============================================================================

# Builds benchmarks.
all-utilities:
	$(MAKE) -C utilities all

# Cleans build objects.
clean-utilities:
	$(MAKE) -C utilities clean

# Cleans build.
distclean-utilities:
	$(MAKE) -C utilities distclean

#===============================================================================
# CAP Benchmarks
#===============================================================================

# Builds benchmarks.
all-capbench:
	$(MAKE) -C capbench all

# Cleans build objects.
clean-capbench:
	$(MAKE) -C capbench clean

# Cleans build.
distclean-capbench:
	$(MAKE) -C capbench distclean

#===============================================================================
# Samos Benchmarks
#===============================================================================

# Builds benchmarks.
all-samos:
	$(MAKE) -C samos all

# Cleans build objects.
clean-samos:
	$(MAKE) -C samos clean

# Cleans build.
distclean-samos:
	$(MAKE) -C samos distclean
