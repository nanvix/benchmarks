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

# C Source Files
SRC = $(wildcard *.c)

# Object Files
OBJ = $(SRC:.c=.o)

# Binary Files
ELFBIN = $(BINDIR)/mppa256-async-master

# Compiler Options
CFLAGS += -march=k1b -mos=bare -mcluster=ioddr

# Toolchain Configuration
LDFLAGS = -march=k1b -mos=bare -mcluster=ioddr

# Libraries.
LIBS  = -mhypervisor -lutask -lmppa_async -lmppa_request_engine
LIBS += -lmppapower -lmppanoc -lmpparouting -lpcie_queue 

#===============================================================================

# Builds the kernel.
all: $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) $(OBJ) -o $(ELFBIN) $(LIBS)

# Builds object files.
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# Cleans compilation files.
clean:
	rm -rf $(OBJ)
	rm -rf $(ELFBIN)

