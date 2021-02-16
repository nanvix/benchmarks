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
all:                 \
	all-barrier      \
	all-cargo        \
	all-fork-join    \
	all-heartbeat    \
	all-kcall-local  \
	all-kcall-remote \
	all-lookup       \
	all-pgfetch      \
	all-pginval      \
	all-mail         \
	all-noise        \
	all-rstream      \
	all-signal       \
	all-stream

# Cleans up build objects.
clean:                 \
	clean-barrier      \
	clean-cargo        \
	clean-fork-join    \
	clean-heartbeat    \
	clean-kcall-local  \
	clean-kcall-remote \
	clean-lookup       \
	clean-pgfetch      \
	clean-pginval      \
	clean-mail         \
	clean-noise        \
	clean-rstream      \
	clean-signal       \
	clean-stream

# Cleans up everything.
distclean:                 \
	distclean-barrier      \
	distclean-cargo        \
	distclean-fork-join    \
	distclean-heartbeat    \
	distclean-kcall-local  \
	distclean-kcall-remote \
	distclean-lookup       \
	distclean-pgfetch      \
	distclean-pginval      \
	distclean-mail         \
	distclean-noise        \
	distclean-rstream      \
	distclean-signal       \
	distclean-stream

#===============================================================================
# Barrier
#===============================================================================

all-barrier:
	$(MAKE) -C barrier all

clean-barrier:
	$(MAKE) -C barrier clean

distclean-barrier:
	$(MAKE) -C barrier distclean

#===============================================================================
# Cargo
#===============================================================================

all-cargo:
	$(MAKE) -C cargo all

clean-cargo:
	$(MAKE) -C cargo clean

distclean-cargo:
	$(MAKE) -C cargo distclean

#===============================================================================
# Fork-Join
#===============================================================================

all-fork-join:
	$(MAKE) -C fork-join all

clean-fork-join:
	$(MAKE) -C fork-join clean

distclean-fork-join:
	$(MAKE) -C fork-join distclean

#===============================================================================
# Heart Beat
#===============================================================================

all-heartbeat:
	$(MAKE) -C heartbeat all

clean-heartbeat:
	$(MAKE) -C heartbeat clean

distclean-heartbeat:
	$(MAKE) -C heartbeat distclean

#===============================================================================
# Local Kernel Call
#===============================================================================

all-kcall-local:
	$(MAKE) -C kcall-local all

clean-kcall-local:
	$(MAKE) -C kcall-local clean

distclean-kcall-local:
	$(MAKE) -C kcall-local distclean

#===============================================================================
# Remote Kernel Call
#===============================================================================

all-kcall-remote:
	$(MAKE) -C kcall-remote all

clean-kcall-remote:
	$(MAKE) -C kcall-remote clean

distclean-kcall-remote:
	$(MAKE) -C kcall-remote distclean

#===============================================================================
# Lookup
#===============================================================================

all-lookup:
	$(MAKE) -C lookup all

clean-lookup:
	$(MAKE) -C lookup clean

distclean-lookup:
	$(MAKE) -C lookup distclean

#===============================================================================
# Mail
#===============================================================================

all-mail:
	$(MAKE) -C mail all

clean-mail:
	$(MAKE) -C mail clean

distclean-mail:
	$(MAKE) -C mail distclean

#===============================================================================
# Memory Read
#===============================================================================

all-rstream:
	$(MAKE) -C rstream all

clean-rstream:
	$(MAKE) -C rstream clean

distclean-rstream:
	$(MAKE) -C rstream distclean

#===============================================================================
# Noise
#===============================================================================

all-noise:
	$(MAKE) -C noise all

clean-noise:
	$(MAKE) -C noise clean

distclean-noise:
	$(MAKE) -C noise distclean

#===============================================================================
# Page Fetch
#===============================================================================

all-pgfetch:
	$(MAKE) -C pgfetch all

clean-pgfetch:
	$(MAKE) -C pgfetch clean

distclean-pgfetch:
	$(MAKE) -C pgfetch distclean

#===============================================================================
# Page Invalidate
#===============================================================================

all-pginval:
	$(MAKE) -C pginval all

clean-pginval:
	$(MAKE) -C pginval clean

distclean-pginval:
	$(MAKE) -C pginval distclean

#===============================================================================
# Signal
#===============================================================================

all-signal:
	$(MAKE) -C signal all

clean-signal:
	$(MAKE) -C signal clean

distclean-signal:
	$(MAKE) -C signal distclean

#===============================================================================
# Stream
#===============================================================================

all-stream:
	$(MAKE) -C stream all

clean-stream:
	$(MAKE) -C stream clean

distclean-stream:
	$(MAKE) -C stream distclean

#===============================================================================
# Task
#===============================================================================

all-task:
	$(MAKE) -C task all

clean-task:
	$(MAKE) -C task clean

distclean-task:
	$(MAKE) -C task distclean
