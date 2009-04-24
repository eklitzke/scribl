Scribl
========

Scribl is my idea for a system for profiling web servies. My initial hope is to
be able to use it with memcached; if that's successful it can be used in more
web-stack services.

You are strongly recommended to look in the `examples` directory, which gives a
concrete example demonstrating how one would use scribl in a real program, and
why it's useful.

Currently scribl is in a pre-alpha state, and is only useful for building
trivial demonstration programs.

Goals for 0.01
--------------

* Minimal dependencies -- just glib
* Full thread-safety
* Only floating point (double precision) counters are supported
* Time-based threaded serialization
* Performance is *not* a goal

Compiling & Dependencies
------------------------

Scribl depends only on [glib](http://www.gtk.org/), a low-level cross-platform C
library. Since scribl relies heavily on glib to implement platform specific
routines, it should be pretty portable. In particular, it should compile and run
on OS X and Windows (although the developer only has access to Linux for now).

To build scribl you'll also want to have
[pkg-config](http://pkg-config.freedesktop.org/) installed, although strictly
speaking this is not necessary. On Fedora, you'll want the following packages:

* gcc
* glib2-devel
* make
* pkgconfig

On Ubuntu (as of 9.04), the only package you should need to install is
`libglib2.0-dev`. Everything else should be installed as part of the regular
desktop installation.

To compile and build scribl, just invoke `make`.

Known Bugs & TODO Items
-----------------------

These are currently being tracked in GitHub, see
<http://github.com/eklitzke/scribl/issues>
