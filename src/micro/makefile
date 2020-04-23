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
all: all-hello all-memread all-memwrite all-mail all-cargo

# Cleans up build objects.
clean: clean-hello clean-memread clean-memwrite clean-mail clean-cargo

# Cleans up everything.
distclean: distclean-hello distclean-memread distclean-memwrite distclean-mail \
	distclean-cargo

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

#===============================================================================
# Cargo
#===============================================================================

# Builds benchmark.
all-cargo:
	$(MAKE) -C cargo all

# Cleans up build object.
clean-cargo:
	$(MAKE) -C cargo clean

# Cleans up everything.
distclean-cargo:
	$(MAKE) -C cargo distclean
