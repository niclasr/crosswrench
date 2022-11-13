# crosswrench
A python wheel 1.0 (pep 427) installer designed for third party package managers, but can be used by others as well.
## TOC
- [background](#background)
- [usage](#usage)
- [build and install](#build-and-install)
## background
Some users don't want to install [pip](https://pip.pypa.io/) in their systems, but still wants to be able to install
wheels.
## usage
## build and install
Building is done using CMake.
```
cmake
make
make install
```
out-of-source build is recommended
```
mkdir <build-dir>
cd <build-dir>
cmake <source-dir>
make
make install
```
### required dependencies
- a working c++17 compiler environment
- [botan2](https://botan.randombit.net/)
- [cmake](https://cmake.org/)
- [libzip](https://libzip.org/)

These three are always required, **crosswrench** bundles some dependencies that can used externally if choosen.
These are:
- [csv2](https://github.com/p-ranav/csv2)
- [cxxopts](https://github.com/jarro2783/cxxopts)
- [libzippp](https://github.com/ctabin/libzippp)
- [pstreams](http://pstreams.sourceforge.net/)
- [pystring](https://github.com/imageworks/pystring)

The CMake argument -DEXTERNAL_LIBS=ON makes them all external by default, -DEXTERNAL_#NAME#=ON where #NAME#
is the name of the dependency listed above makes the individual dependency external.

**crosswrench** requires a python interpreter to install wheels, only [cpython](https://www.python.org/) has been tested.
