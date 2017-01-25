# GMLib Demo

The demo application illustraces basic usage of the GMLib `screen` architecture to render a frame with an example UI interface. The application demostrates creation of ui components in runtime and from JSON description files.

## Building

Once you have GMLib and all of the dependencies built, either as shared or static libraries, you can proceed with building demo app. The `Makefile` uses the same `flags.make` settings as GMLib itself. Build demo app with:

    $ make demo

## Running

Run `demo` binary with

    $ ./demo demo.conf

You may want to `ln -s` shared libraries into your demo folder in you're using relative paths or modified linker paths.

##### Mac OS X

Make sure you have your libraries installed into expected `install_path`-s or tuned to use `@rpath`. 