#
# MIT License
#
# Copyright(c) 2011-2019 The Maintainers of Nanvix
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
all:
	$(MAKE) -C mailbox all
	$(MAKE) -C portal all
	$(MAKE) -C stress all
	$(MAKE) -C hpcs all

# Builds multibinary image.
image:
	$(MAKE) -C mailbox image
	$(MAKE) -C portal image
	$(MAKE) -C stress image
	$(MAKE) -C hpcs image

# Cleans All Object Files
clean:
	$(MAKE) -C mailbox clean
	$(MAKE) -C portal clean
	$(MAKE) -C stress clean
	$(MAKE) -C hpcs clean

# Cleans Everything
distclean: clean
	$(MAKE) -C mailbox distclean
	$(MAKE) -C portal distclean
	$(MAKE) -C stress distclean
	$(MAKE) -C hpcs distclean

