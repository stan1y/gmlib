# GMLib library

GMLib is a C++ utilities toolkit around SDL and satellite projects 
SDL_ttf and SDL_img. Together with these libraries and Boost, GMLib depends on
several other smaller libraries providing platform independent features.

The goal of GMLib is to provide comprehensive one stop SDL wrapper utility
with most of the tools you need to build a sprites based and/or OpenGL application.

GMLib provides both native C++ interface for building applications but
allows to fully extend the application with Python scripts. GMLib provides
native support for C++ data classes into python objects serialization.

## Dependencies

GMLib uses the following libraries:

* `SDL`, `SDL_ttf` and `SDL_img` - Simple Direct Layer libraries
* `boost` - Most of the cross platform needs
* `jansson` - Fast JSON parser
* `freetype` - Used by SDL_ttf

## Components

The overview list of GMLib features:
* Components (of a screen) architecture.
* Generic resources caches with dynamic and static update.
* Sprites, sprite sheets and animations.
* Textures, drawing and pixel access.
* Comprehensive GUI manager and controls library
* Native JSON as internal and external structured data format
* Embedded python runtime and data marshaling support

There is a brief [documentation of GMLib architecture and classes](https://github.com/stan1y/gmlib/blob/master/Documentation.md) and a separate [UI controls reference](https://github.com/stan1y/gmlib/blob/master/GMUI_Reference.md). Also there are a [getting started guide](/) and a few [examples](/).

## Building

GMLib support build on Windows, Linux and Mac OS X

### Building on Windows

### Building on Linux

### Building on Mac OS X