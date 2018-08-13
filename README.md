Nanvix Benchmarks
=================

What is This Repository About?
-------------------------------

In this repository you will find some microbenchmarks for the Nanvix
operating system. All benchmarks are meant to run in the `Nanvix
Multikernel v1.0-beta.3` release. Currently, the following
microbenchmarks are featured:

* rmem-benchmark: reads/writes data sequentially from/to the remote memory.

Build Instructions
----------------------

1. Clone this Repository

	`git clone https://github.com/nanvix/benchmarks.git ~/nanvix-benchmarks && cd ~/nanvix-benchmarks`
	
3. Set the Target Platform

	`export TARGET=mppa256`

Note that the multikernel is only available for the Kalray MPPA-256 target.

2. Unpack the Developement Libraries and Headers of Nanvix

	`bash contrib/setup-contrib.sh`

3. Build the Binaries

	`make`

4. Clean Compilation Files (Optional)

	`make clean`

Run Instructions
----------------------

1. Run the Microbenchmarks

	`bash scripts/run.sh`

License and Maintainers
------------------------

Nanvix is a free operating system that is distributed under the MIT
License. It wasy initially designed by Pedro Henrique, but now it is
mainted by several people. If you have any questions or suggestions,
fell free to contact any of the contributors that are listed
here: https://github.com/nanvix/nanvix/blob/dev/CREDITS.
