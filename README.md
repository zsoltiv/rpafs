# rpafs

Mount a Ren'Py archive as a FUSE filesystem.

# Usage

```sh
rpafs --archive=</path/to/archive.rpa> [-d] [-f] </path/to/mount/point>
```

`-d` enables FUSE-level debug information to be printed, and `-f` runs the
file system process in the foreground.

# Requirements

- a C99 compiler
- libfuse 3.x
- zlib

# License

GPLv3
