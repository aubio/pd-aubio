pd-aubio
========

Various externals for PureData using the aubio library

For more information, see:
  - aubio homepage: https://aubio.org/pd-aubio
  - puredata homepage: https://puredata.info

Building
--------

pd-aubio uses waf. To configure and build it, run:

    $ ./waf configure build

If you have aubio installed in an unusual location, just set your
`PKG_CONFIG_PATH` accordingly. For instance, to build against aubio installed
in /var/tmp/aubio:

    $ PKG_CONFIG_PATH=/var/tmp/aubio/lib/pkgconfig waf \
        distclean configure build install --prefix=/var/tmp/aubio

Installing
----------

To install the aubio external for puredata on your system, run the following as
root:

    # make install

Running
-------

To load the external, start pd as follows

    $ pd -lib aubio

Alternatively, you can create an object [aubio] to initialize the external.

If you want to use the external without installing it, the above command should
work as expected from the pd-aubio source directory. If needed, set your
`LD_LIBRARY_PATH` to include the path to the aubio library.
