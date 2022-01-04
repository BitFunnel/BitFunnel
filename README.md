# BitFunnel

This repo contains the code for the BitFunnel index used by [Bing's](http://www.bing.com) super-fresh, news, and media indexes. The algorithm is described in [BitFunnel: Revisiting Signatures for Search](https://dl.acm.org/doi/pdf/10.1145/3077136.3080789), a paper presented at SIGIR 2017. This [video](https://www.youtube.com/watch?v=1-Xoy5w5ydM) gives a good overview of the algorithm.

The codebase here was published to allow the research community to replicate results from the SIGIR paper. The documentation is pretty thin, but we encourage you to look at the following :

* [How to index Wikipedia](http://bitfunnel.org/index-build-tools/)
* [How to play with our query parser](http://bitfunnel.org/a-small-query-language/)
* [BitFunnel engineering diary](http://bitfunnel.org/blog-archive/)



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
