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
all:          \
	all-diff  \
	all-grep  \
	all-hello \
	all-qsort \
	all-sed

# Cleans up build objects.
clean:          \
	clean-diff  \
	clean-grep  \
	clean-hello \
	clean-qsort \
	clean-sed

# Cleans up everything.
distclean:          \
	distclean-diff  \
	distclean-grep  \
	distclean-hello \
	distclean-qsort \
	distclean-sed

#===============================================================================
# Diff
#===============================================================================

all-diff:
ifneq ($(TARGET), unix64)
	$(MAKE) -C diff all
endif

clean-diff:
ifneq ($(TARGET), unix64)
	$(MAKE) -C diff clean
endif

distclean-diff:
ifneq ($(TARGET), unix64)
	$(MAKE) -C diff distclean
endif

#===============================================================================
# Grep
#===============================================================================

all-grep:
ifneq ($(TARGET), unix64)
	$(MAKE) -C grep all
endif

clean-grep:
ifneq ($(TARGET), unix64)
	$(MAKE) -C grep clean
endif

distclean-grep:
ifneq ($(TARGET), unix64)
	$(MAKE) -C grep distclean
endif

#===============================================================================
# Hello World
#===============================================================================

all-hello:
	$(MAKE) -C hello all

clean-hello:
	$(MAKE) -C hello clean

distclean-hello:
	$(MAKE) -C hello distclean

#===============================================================================
# Quick Sort
#===============================================================================

all-qsort:
ifneq ($(TARGET), unix64)
	$(MAKE) -C qsort all
endif

clean-qsort:
ifneq ($(TARGET), unix64)
	$(MAKE) -C qsort clean
endif

distclean-qsort:
ifneq ($(TARGET), unix64)
	$(MAKE) -C qsort distclean
endif

#===============================================================================
# Sed
#===============================================================================

all-sed:
ifneq ($(TARGET), unix64)
	$(MAKE) -C sed all
endif

clean-sed:
ifneq ($(TARGET), unix64)
	$(MAKE) -C sed clean
endif

distclean-sed:
ifneq ($(TARGET), unix64)
	$(MAKE) -C sed distclean
endif
