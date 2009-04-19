Memcached Example
=================

This is a very simple example of how one might use scribl with memcached. The
patch illustrates adding a counter to memcached that counts how often each key
has an update operation performed on it. This is probably not super useful in
real life (more likely you'd want to count key prefixes, or something else less
unique than the full key name), but it illustrates the simplicity of adding
scribl to an external codebase.

This example consists of two files:

- `memcached-set-counter.patch` a patch against `memcached.c`
- `scribl-memcached_set-1240120191.log` an example log generated from a
  memcached that was using the aforementioned patch.

The key thing to take away from this example is how minimal the patch against
memcached really is. For only six additional lines of code we're able to count
statistics about update operations in memcached. Additionally,

* this is fully threadsafe, running memcached in threaded mode works fine
* memcached commands are still O(1), since the scribl increment operation is
  O(1)
* logging/serialization is automatically happening in separate threads, which
  means that these operations don't block memcached (even when memcached itself
  is only running a single thread)
