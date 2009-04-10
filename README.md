Scribble
========
Scribble is my idea for a system for profiling web servies. My initial hope is
to be able to use it with memcached; if that's successful it can be used in more
web-stack services.

Goals for 0.01
--------------
* Minimal dependencies -- just glib and libevent
* Full thread-safety
* Only two-level string counters are supported
* Only integer and double counters are supported
* Event-based threaded serialization
* Performance is *not* a goal
