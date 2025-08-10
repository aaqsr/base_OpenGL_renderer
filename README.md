
# How to Build This?

We use `vcpkg`.
Ensure that `vcpkg` is installed and the `VCPKG_ROOT` environment variable is set.

## Generating Build Files

Skip the first step if you have already built the project once and have not added/removed source or header files nor edited cmake build files.
In this instance, you don't need new build files.

If you have not built the project yet, or if you add/remove source or header files, first run,
```sh
cmake --preset debug   # for debug mode
cmake --preset release # for release mode
```

This should install the relevant packages and should just work...

## Building

Then to actually build the project run,
```sh
# for everyone else
cmake --build --preset debug -j 8   # for debug build
cmake --build --preset release -j 8 # for release build
```
