# Simple SGen Client

This is a minimal client and test program for SGen, the garbage
collector of the Mono project.

## Requirements

Everything needed to configure Mono, and GLib.

## Compiling

This requires Mono as a submodule, so first make sure it's there.
Then configure Mono.  It's not necessary to build it, but the
`config.h` file needs to be there and work for your system.  Then
`make`.
