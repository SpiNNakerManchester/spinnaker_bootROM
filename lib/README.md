This is the host-side code for testing the flood fill system used by the
second stage of booting a SpiNNaker system, which is the part that uploads a
SCAMP image and boots it on one core of every chip. (After that has happened,
the boot ROM has finished its job; SCAMP has taken over entirely.)

* `hostpcfloodfillserver.c` is the implementation for POSIX (Linux, macOS, etc).

* `hostpcfloodfillserver-ms.c` is the implementation for Windows.

This code has been long obsoleted by SpiNNMan and JavaSpiNNaker.
