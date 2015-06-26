# Simple SGen Clients

These are two simple clients for SGen, the garbage collector of the [Mono project](https://github.com/mono/mono/).

`test-sgen.c` is a minimal client program.  It's not very useful but shows how to initialize SGen and allocate memory.

A more interesting case is `scheme.c`, which is an adapted version of an [example Scheme interpreter](https://github.com/Ravenbrook/mps-temporary/tree/master/example/scheme) included in [Ravenbrook's MPS](http://www.ravenbrook.com/project/mps).

## Requirements

Everything needed to configure Mono, and GLib.

## Compiling

This requires Mono as a submodule, so first make sure it's there.  Then configure Mono.  It's not necessary to build it, but the `config.h` file needs to be there and work for your system.  Then `make`.
