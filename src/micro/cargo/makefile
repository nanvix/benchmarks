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
all: all-pingpong all-broadcast all-gather

# Cleans up build objects.
clean: clean-pingpong clean-broadcast clean-gather

# Cleans up everything.
distclean: distclean-pingpong distclean-broadcast distclean-gather

#===============================================================================
# Ping-Pong
#===============================================================================

# Builds benchmark.
all-pingpong:
	$(MAKE) -C pingpong all

# Cleans up build object.
clean-pingpong:
	$(MAKE) -C pingpong clean

# Cleans up everything.
distclean-pingpong:
	$(MAKE) -C pingpong distclean

#===============================================================================
# Broadcast
#===============================================================================

# Builds benchmark.
all-broadcast:
	$(MAKE) -C broadcast all

# Cleans up build object.
clean-broadcast:
	$(MAKE) -C broadcast clean

# Cleans up everything.
distclean-broadcast:
	$(MAKE) -C broadcast distclean

#===============================================================================
# Gather
#===============================================================================

# Builds benchmark.
all-gather:
	$(MAKE) -C gather all

# Cleans up build object.
clean-gather:
	$(MAKE) -C gather clean

# Cleans up everything.
distclean-gather:
	$(MAKE) -C gather distclean
