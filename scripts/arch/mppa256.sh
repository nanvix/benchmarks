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

export OUTDIR=.
export K1TOOLS_DIR="/usr/local/k1tools"

#
# Runs a multibinary file using a single IO Cluster.
#
function run1
{
	local multibin=$1
	local bin=$2
	local args=$3

	$K1TOOLS_DIR/bin/k1-jtag-runner     \
		--multibinary=$OUTDIR/$multibin \
		--exec-multibin=IODDR0:$bin     \
		-- $args

	local ret=$?
	if [ $ret == "0" ]
	then
		printf "[nanvix][test] %-30s \e[32m%s\e[0m\n" "$multibin" "passed"
	else
		printf "[nanvix][test] %-30s \e[91m%s\e[0m\n" "$multibin" "FAILED"
	fi
}

#
# Runs a multibinary file using two IO Clusters.
#
function run2
{
	local multibin=$1
	local bin1=$2
	local args=$3
	local bin2="/spawner1"

	$K1TOOLS_DIR/bin/k1-jtag-runner     \
		--multibinary=$OUTDIR/$multibin \
		--exec-multibin=IODDR0:$bin1    \
		--exec-multibin=IODDR1:$bin2    \
		-- $args

	local ret=$?
	if [ $ret == "0" ]
	then
		printf "[nanvix][test] %-30s \e[32m%s\e[0m\n" "$multibin" "passed"
	else
		printf "[nanvix][test] %-30s \e[91m%s\e[0m\n" "$multibin" "FAILED"
	fi
}

function build1
{
	local bindir=$1
	local bootbin=$2
	local nodebin=$3
	local multibin=$4

	$K1TOOLS_DIR/bin/k1-create-multibinary -f \
		--remove-prefix $bindir               \
		--boot $bootbin                       \
		--clusters $nodebin                   \
		-T $multibin
}

function build2
{
	local bindir=$1
	local bootbin=$2
	local nodebin=$3
	local multibin=$4
	local iobin=$1/spawner1

	$K1TOOLS_DIR/bin/k1-create-multibinary -f \
		--remove-prefix $bindir               \
		--boot $bootbin                       \
		--ios $iobin                          \
		--clusters $nodebin                   \
		-T $multibin
}
