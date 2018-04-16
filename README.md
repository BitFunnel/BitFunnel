# BitFunnel

This is an experiment in text search/retrieval. It doesn't work (yet). We're making this available in the spirit of doing development out in the open, but nothing here has been cleaned up for public consumption. The documentation is non-existent and the code is in an incomplete state.

If you really want to try things out, we have some instructions on [how to index Wikipedia](http://bitfunnel.org/index-build-tools/), and [how to play with our query parser](http://bitfunnel.org/a-small-query-language/). However, the system doesn't do anything useful to an end user -- these instructions are intended to allow contributors to build and run part of a system for the purpose of understand or debugging the system. If you're looking to play with an incomplete and likely buggy system, go ahead! If you want a working search index that you can use in production, check back later.

If you want a description of one of the core ideas, see [this StrangeLoop talk transcript](http://bitfunnel.org/strangeloop/). See our [engineering diary](http://bitfunnel.org/blog-archive/) for descriptions of other parts of the system. We seem to be adding new descriptions at least once a week, and we (sometimes) go back and update old descriptions as things change.

[![Build status](https://ci.appveyor.com/api/projects/status/b65lb8wn2r7ux7d2/branch/master?svg=true)](https://ci.appveyor.com/project/MichaelHopcroft/bitfunnel)
[![Build status](https://doozer.io/badge/mikehopcroft/BitFunnel/buildstatus/master)](https://doozer.io/user/mikehopcroft/BitFunnel/builds)

Dependencies
------------

In order to build BitFunnel you will need CMake (2.8.11+), and a modern C++
compiler (gcc 5+, clang 3.5+, or VC 2015+). You can run CMake directly to generate the appropriate build setup for your platform. Alternately, we have some scripts that have the defaults that we use available.

### *nix

For *nix platforms (including OS X),

~~~
./Configure_Make.sh
cd build-make
make
make test
~~~

Note that while these instructions are for a `make` build, it's also possible to build using `ninja` by changing the `cmake` command to create `ninja` files instead of `Makefiles`. These aren't listed in the instructions because `ninja` requires installing an extra dependency for some developers, but if you want to use `ninja` it's available via `apt-get`, `brew`, etc., and is susbtantially faster than `make`.

#### Ubuntu

If you're on Ubuntu 15+, you can install dependencies with:

~~~
sudo apt-get install clang cmake
~~~

On Ubuntu 14 and below, you'll need to install a newer version of CMake. To
install a new-enough CMake, see [this link](http://askubuntu.com/questions/610291/how-to-install-cmake-3-2-on-ubuntu-14-04).
If you're using gcc, you'll also need to make sure you have gcc-5 (`sudo apt-get install g++-5`).

To override the default compiler, set the `CXX` and `CC` environment variables.
For example, if you have clang-3.8 installed as `clang-3.8` and are using bash:

~~~
export CXX="clang++-3.8"
export CC="clang-3.8"
~~~

#### OS X

Install XCode and then run the following command to install required packages
using Homebrew ([http://brew.sh/](http://brew.sh/)):

~~~
brew install cmake
~~~

BitFunnel can be built on OS X using either standard \*nix makefiles or XCode.
In order to generate and build makefiles, in the root `BitFunnel` directory run:

If you want to create an Xcode project instead of using Makefiles, run:

~~~
./Configure_XCode.sh
~~~

If you use XCode, you'll have to either re-run `Configure_XCode` or run the `ZERO_CHECK` target when the `CMakeLists` changes, e.g., when source files are added or removed.

### Windows

You will need these tools:

- CMake ([http://www.cmake.org/download/](http://www.cmake.org/download/))
- Visual Studio 2017 with C++ compiler ([the free version](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx))

**Note**: If you install Visual Studio for the first time and select the
default install options, you won't get a C++ compiler. To force the install of
the C++ compiler, you need to either create a new C++ project or open an
existing C++ project.

Clone the BitFunnel repository and then run the following command in BitFunnel's root folder:

~~~
.\Configure_MSVC.bat
~~~

**Note**: You will need to modify the CMake -G option if you use a different version of Visual Studio.
Bitfunnel must be built as a 64-bit program, so 'Win64' must be part of the specified G option text.

At this point, you can open the generated solution `BitFunnel_CMake.sln` from Visual Studio and then build it.
Alternatively, you can build from the command line using `cmake --build build-MSVC`.
