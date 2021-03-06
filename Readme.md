# GMLib library

GMLib is a C++ utilities toolkit around SDL and satellite projects 
SDL_ttf and SDL_image. Together with these libraries and Boost, GMLib depends on
several other smaller libraries providing platform independent features.

The goal of GMLib is to provide comprehensive one stop SDL wrapper utility
with most of the tools you need to build a sprites based and/or OpenGL application.

GMLib provides both native C++ interface for building applications but
allows to fully extend the application with Python scripts. GMLib provides
native support for C++ data classes into python objects serialization.

## Dependencies

GMLib uses the following libraries:

* `SDL`, `SDL_ttf` and `SDL_image` - Simple Direct Layer libraries
* `freetype` - Used by SDL_ttf
* `boost` - Most of the cross platform needs
* `https://github.com/nlohmann/json` - JSON for Modern C++

## Components

The overview list of GMLib features:
* Components (of a screen) architecture.
* Generic in-memory resources cache.
* Sprites, sprite sheets and animations.
* Textures, drawing and pixel access.
* Comprehensive GUI manager and controls library
* Native JSON as internal and external structured data format
* Embedded python (Python 3) runtime and data marshaling support

### Planned

This planned to be implemented at some point:
* Resources based on [PhysicsFS](https://icculus.org/physfs/).
* Generic 2d grids with rectangular and [hexagon](https://www.redblobgames.com/grids/hexagons) cells.

There is a brief [documentation of GMLib architecture and classes](https://github.com/stan1y/gmlib/blob/master/Documentation.md) and a separate [UI controls reference](https://github.com/stan1y/gmlib/blob/master/GMUI_Reference.md). Also there are a [getting started guide](/) and a [demo application](https://github.com/stan1y/gmlib/tree/master/src/demo).

## Building

GMLib support build on Windows, Linux and Mac OS X. There are several environmental variables used by GMLib to determine paths to dependencies, refer to `Makefile` for details.

### Prerequisites

You need to build Boost or get a precompiled version for your target platform before building anything else. Refer to Boost documentation on how to build a shared libraries version for your platform.

Also you need to build or get SDL, SDL_ttf, SDL_image libraries. Refer to SDL documentation on how to build it for your platform, however it is usually just a CMake.

### Building GMLib

GMLib builds on Windows, Linux and Mac OS X.

#### Windows

To build on Windows use `GMLib.vcproj` file. Make sure you modify it accoring to your environment.

#### Linux & Mac OS X

Refer to `flags.make` for configurable build settings recognized by GMLib makefiles. There is a example build environment file `buildenv.example' which should be modified to suit your environment.

	$ cp buildenv.example buildenv
	$ vi buildenv
	$ source buildenv
    $ make static
    $ make shared

To build demo application use:

	$ make demo

To install GMLib into `PREFIX` use:

    $ sudo make install

##### Mac OS X App bundles

To include GMLib and dependencies into your app package on Mac, you should build Python, SDL, SDL_ttf and SDL_image as static libraries. Optionally you can use static boost libraries too.

In order to include shared libraries, such as freetype or boost, make sure you tune them with `-install_name` relative to the executable or package, sush as:

	$ install_name_tool -id @rpath/libfreetype.dylib libfreetype.dylib

