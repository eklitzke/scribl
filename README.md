Scribl
========

Scribl is my idea for a system for profiling web servies. My initial hope is to
be able to use it with memcached; if that's successful it can be used in more
web-stack services.

Currently scribl is in a pre-alpha state, and is only useful for building
trivial demonstration programs.

Goals for 0.01
--------------

* Minimal dependencies -- just glib
* Full thread-safety
* Only floating point (double precision) counters are supported
* Only two-level string counters are supported
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

To compile and build scribl, just execute `make`.
