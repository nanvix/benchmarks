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
all:                               \
	all-fork-join                  \
	all-dispatch-wait              \
	all-heartbeat                  \
	all-lookup                     \
	all-pginval                    \
	all-services-dispatcher        \
	all-services-thread            \
	all-services-user              \
	all-heartbeat-core-usage       \
	all-lookup-core-usage          \
	all-noise                      \
	all-syscall

# Cleans Build Objects
clean:                             \
	clean-fork-join                \
	clean-dispatch-wait            \
	clean-heartbeat                \
	clean-lookup                   \
	clean-pginval                  \
	clean-services-dispatcher      \
	clean-services-thread          \
	clean-services-user            \
	clean-heartbeat-core-usage     \
	clean-lookup-core-usage        \
	clean-noise                    \
	clean-syscall

# Cleans Everything
distclean:                         \
	distclean-fork-join            \
	distclean-dispatch-wait        \
	distclean-heartbeat            \
	distclean-lookup               \
	distclean-pginval              \
	distclean-services-dispatcher  \
	distclean-services-thread      \
	distclean-services-user        \
	distclean-heartbeat-core-usage \
	distclean-lookup-core-usage    \
	distclean-noise                \
	distclean-syscall

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
# Heartbeat
#===============================================================================

all-heartbeat:
	$(MAKE) -C heartbeat all

clean-heartbeat:
	$(MAKE) -C heartbeat clean

distclean-heartbeat:
	$(MAKE) -C heartbeat distclean

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
# Pginval
#===============================================================================

all-pginval:
	$(MAKE) -C pginval all

clean-pginval:
	$(MAKE) -C pginval clean

distclean-pginval:
	$(MAKE) -C pginval distclean

#===============================================================================
# services-dispatcher
#===============================================================================

all-services-dispatcher:
	$(MAKE) -C services-dispatcher all

clean-services-dispatcher:
	$(MAKE) -C services-dispatcher clean

distclean-services-dispatcher:
	$(MAKE) -C services-dispatcher distclean

#===============================================================================
# services-thread
#===============================================================================

all-services-thread:
	$(MAKE) -C services-thread all

clean-services-thread:
	$(MAKE) -C services-thread clean

distclean-services-thread:
	$(MAKE) -C services-thread distclean

#===============================================================================
# services-user
#===============================================================================

all-services-user:
	$(MAKE) -C services-user all

clean-services-user:
	$(MAKE) -C services-user clean

distclean-services-user:
	$(MAKE) -C services-user distclean

#===============================================================================
# Heartbeat Core Usage Benchmark
#===============================================================================

all-heartbeat-core-usage:
	$(MAKE) -C heartbeat-core-usage all

clean-heartbeat-core-usage:
	$(MAKE) -C heartbeat-core-usage clean

distclean-heartbeat-core-usage:
	$(MAKE) -C heartbeat-core-usage distclean

#===============================================================================
# Lookup Core Usage Benchmark
#===============================================================================

all-lookup-core-usage:
	$(MAKE) -C lookup-core-usage all

clean-lookup-core-usage:
	$(MAKE) -C lookup-core-usage clean

distclean-lookup-core-usage:
	$(MAKE) -C lookup-core-usage distclean

#===============================================================================
# Noise Benchmark
#===============================================================================

all-noise:
	$(MAKE) -C noise all

clean-noise:
	$(MAKE) -C noise clean

distclean-noise:
	$(MAKE) -C noise distclean

#===============================================================================
# Syscall Benchmark
#===============================================================================

all-syscall:
	$(MAKE) -C syscall all

clean-syscall:
	$(MAKE) -C syscall clean

distclean-syscall:
	$(MAKE) -C syscall distclean
