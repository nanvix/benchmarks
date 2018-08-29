#
# MIT License
#
# Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
# "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
# LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

# Import Platform-Dependent Stuff
ifeq ($(ARCH),$(filter $(ARCH),k1bdp k1bio))
	include $(MAKEDIR)/makefile.mppa256
endif

#===============================================================================
# Toolchain
#===============================================================================

# Compiler Options
export CFLAGS += -ansi -std=c99
export CFLAGS += -Wall -Wextra -Werror
export CFLAGS += -Winit-self -Wswitch-default -Wfloat-equal
export CFLAGS += -Wundef -Wshadow -Wuninitialized
export CFLAGS += -O3
export CFLAGS += -I $(INCDIR)

# Linker Options
export LDFLAGS += 
export ARFLAGS = rcs

#===============================================================================
# Binaries & Libraries
#===============================================================================

# Startup Routines
export CRT0 = $(LIBDIR)/crt0.o
export CRT1 = $(LIBDIR)/crt1.o

#===============================================================================

# Conflicts.
.PHONY: arch
.PHONY: mailbox
.PHONY: portal
.PHONY: rmem
.PHONY: sync
.PHONY: name

# Builds binaries.
all: arch mailbox portal rmem sync name

# Native microbenchmarks.
arch:
	cd arch && $(MAKE) all

# Unnamed mailbox microbenchmark.
mailbox:
	cd nanvix/mailbox && $(MAKE) all

# Unnamed portal microbenchmark.
portal:
	cd nanvix/portal && $(MAKE) all

# RMem microbenchmark.
rmem:
	cd nanvix/rmem && $(MAKE) all

# Synchronization point microbenchmark.
sync:
	cd nanvix/sync && $(MAKE) all

# Name Service microbenchmark.
name:
	cd nanvix/name && $(MAKE) all

# Cleans compilation files.
clean:
	cd arch && $(MAKE) clean
	cd nanvix/mailbox && $(MAKE) clean
	cd nanvix/portal && $(MAKE) clean
	cd nanvix/rmem && $(MAKE) clean
	cd nanvix/name && $(MAKE) clean
	cd nanvix/sync && $(MAKE) clean

