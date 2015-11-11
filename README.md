# abt-snoozer

abt-snoozer is a utility library for Argobots that can be used to configure
execution streams to sleep when idle.  It implements a user-defined pool and
scheduler that are linked together via a libev event loop for signalling
purposes.

See the following for more details about Argobots:

* https://collab.mcs.anl.gov/display/ARGOBOTS/Argobots+Home

##  Dependencies

* argobots (git://git.mcs.anl.gov/argo/argobots.git)
* libev (e.g libev-dev package on Ubuntu or Debian)

## Building

Example configuration:

    ../configure --prefix=/home/pcarns/working/install \
        PKG_CONFIG_PATH=/home/pcarns/working/install/lib/pkgconfig 

If libev is not in your system path, then you can add --with-libev=PATH to
the configure command line.

## To Do:

* Add autoconf detection of libev
* Add licensing (take into account that portions are based on modified code
  from Argobots)
