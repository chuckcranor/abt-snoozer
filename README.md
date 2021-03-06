# abt-snoozer

abt-snoozer is a utility library for Argobots that can be used to configure
execution streams to sleep when idle.  It implements a user-defined pool and
scheduler that are linked together via a libev event loop for signalling
purposes.

See the following for more details about Argobots:

* https://collab.mcs.anl.gov/display/ARGOBOTS/Argobots+Home

##  Dependencies

* argobots (git -b dev-get-dev-basic clone https://github.com/carns/argobots.git)
* libev (e.g libev-dev package on Ubuntu or Debian)

Note that the Argobots repo named above is *not* the primary upstream
repository, but rather a fork that includes an additional feature needed by
abt-snoozer in the short term.  abt-snoozer will be updated to work with the
upstream repository without modifications once
https://github.com/pmodels/argobots/issues/26 is resolved.

## Building Argobots (dependency)

Example configuration:

    ../configure --prefix=/home/pcarns/working/install 

## Building

Example configuration:

    ../configure --prefix=/home/pcarns/working/install \
        PKG_CONFIG_PATH=/home/pcarns/working/install/lib/pkgconfig 

If libev is not in your system path, then you can add --with-libev=PATH to
the configure command line.

Example build:

    make
    make check
    make install

## Examples

The following example programs are provided in the examples subdirectory:

* basic-standard: Illustrates one execution stream waiting for completion of
  a thread running on a separate execution stream using the default Argobots
  scheduler.
* basic-snoozer: Same as the above, but using the abt-snoozer library to
  make sure that the idle execution stream sleeps while waiting.


