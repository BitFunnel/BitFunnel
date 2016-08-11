# BitFunnel

This is an experiment in text search/retrieval. It doesn't work (yet). We're making this available in the spirit of doing development out in the open, but nothing here has been cleaned up for public consumption. The documentation is non-existent and the code is in an incomplete state.

[![Build status](https://ci.appveyor.com/api/projects/status/b65lb8wn2r7ux7d2/branch/master?svg=true)](https://ci.appveyor.com/project/danluu/bitfunnel/branch/master)

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

### Windows

Install the following tools:

- Visual Studio 2015 with C++ compiler
- CMake ([http://www.cmake.org/download/](http://www.cmake.org/download/))

You can get [the free version of Visual Studio here](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx).
Note that if you're installing Visual Studio for the first time and select the
default install options, you won't get a C++ compiler. To force the install of
the C++ compiler, you need to either create a new C++ project or open an
existing C++ project.

In order to configure solution for Visual Studio 2015 run the following
commands from the root `BitFunnel` directory:

~~~
.\Configure_MSVC.bat
~~~

From now on you can use the generated solution `build-msvc\BitFunnel.sln` from Visual Studio
or build from command line using `cmake`.
