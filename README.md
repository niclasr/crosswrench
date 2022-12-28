# Crosswrench
A python wheel 1.0 (pep 427) installer designed for third party package managers, but can be used by others as well.
## TOC
- [background](#background)
- [usage](#usage)
- [build and install](#build-and-install)
## Background
Some users don't want to install [pip](https://pip.pypa.io/) in their systems, but still wants to be able to install
wheels.
## Usage
## Build and install
Building is done using CMake.
```
cmake
make
make install
```
out-of-source builds are recommended
```
mkdir <build-dir>
cd <build-dir>
cmake <source-dir>
make
make install
```
### Required dependencies
- working c++14 compiler environment
- [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) or [pkgconf](http://pkgconf.org/)
- [boost](https://www.boost.org/)
- [botan2](https://botan.randombit.net/)
- [cmake](https://cmake.org/)
- [libzip](https://libzip.org/)

These are always required, **crosswrench** bundles some dependencies that can used externally if choosen.
These are:
- [csv2](https://github.com/p-ranav/csv2)
- [cxxopts](https://github.com/jarro2783/cxxopts)
- [libzippp](https://github.com/ctabin/libzippp)
- [pstreams](http://pstreams.sourceforge.net/)
- [pystring](https://github.com/imageworks/pystring)

The CMake argument -DEXTERNAL_LIBS=ON makes them all external by default, -DEXTERNAL_#NAME#=ON where #NAME#
is the name in uppercase of the dependency listed above makes the individual dependency external.

**crosswrench** requires a python 3 interpreter with the
[sysconfig module](https://docs.python.org/3/library/sysconfig.html)
to install wheels, only [cpython](https://www.python.org/) has been tested.
