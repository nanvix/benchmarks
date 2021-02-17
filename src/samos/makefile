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
all:                            \
	all-fork-join               \
	all-dispatch-wait           \
	all-master-core-usage

# Cleans Build Objects
clean:                          \
	clean-fork-join             \
	clean-dispatch-wait         \
	clean-master-core-usage

# Cleans Everything
distclean:                      \
	distclean-fork-join         \
	distclean-dispatch-wait     \
	distclean-master-core-usage

#===============================================================================
# Fork-Join Benchmark
#===============================================================================

all-fork-join:
	$(MAKE) -C fork-join all

clean-fork-join:
	$(MAKE) -C fork-join clean

distclean-fork-join:
	$(MAKE) -C fork-join distclean

#===============================================================================
# Dispatch-Wait Benchmark
#===============================================================================

all-dispatch-wait:
	$(MAKE) -C dispatch-wait all

clean-dispatch-wait:
	$(MAKE) -C dispatch-wait clean

distclean-dispatch-wait:
	$(MAKE) -C dispatch-wait distclean

#===============================================================================
# Master Core Usage Benchmark
#===============================================================================

all-master-core-usage:
	$(MAKE) -C master-core-usage all

clean-master-core-usage:
	$(MAKE) -C master-core-usage clean

distclean-master-core-usage:
	$(MAKE) -C master-core-usage distclean
