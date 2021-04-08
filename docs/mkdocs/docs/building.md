# Building

First steps for using MiniMC is downloading the source code and compile it. You will need to following tools

* git,
* g++,
* cmake.

## Dependencies
MiniMC is build on top of existing technologies and requires a few libraries to be installed on your machine:

* LLVM,
* CVC4,
* Graphviz (CGraph).
* boost

In addition to these, the automated build system will also download and compile:

* [Murmur Hash](https://github.com/kloetzl/libmurmurhash)
* [SMTLib](https://gitlab.com/dannybpoulsen/smtlib.git)
* [Base64](https://github.com/ReneNyffenegger/cpp-base64.git)



## Compiling
The initial steps for setting up an out-of-source build with MiniMC is: 


	git clone https://github.com/dannybpoulsen/minimc.git
	cd minimc
	git submodule init
	mkdir build
	cd build


After compilation the {{minimc}} binary is availble a `./bin/minimc` in the `build` directory

