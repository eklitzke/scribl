Scribl On-Disk Log Format
=========================

The scribl logs are simple ASCII logs. Each line consists of a key/value pair,
separated by a space. For example, you might see a log like:

    foo 1.6
    bar 9.7
    baz 0.0

The naming scheme of logs is simple. It follows the format:
    scribl-COUNTERNAME-TIMESTAMP.log
Here, `COUNTERNAME` corresponds to the name that you gave the counter in your
code, and `TIMESTAMP` is an integer representing the Unix time that the
serialization happened at. If you two counters have the same name at the same
time, it is likely that their log names will collide which will cause
problems. For this reason you should strive to give all counters in your program
unique names.

There are plans to add to a binary log format, once I figure out the best way to
do that in a platform neutral way.
