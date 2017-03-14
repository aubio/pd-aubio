pd-aubio
========

Various externals for PureData using the aubio library

For more information, see:
  - aubio homepage: https://aubio.org/pd-aubio
  - puredata homepage: https://puredata.info

Building
--------

To configure and build pd-aubio, run:

    $ make

pd-aubio uses waf. To find out more information about build options:

    $ ./scripts/get_waf.sh
    $ ./waf configure --help

Once configured, the external can be built with:

    $ ./waf configure build

If you have aubio installed in an unusual location, you might need to set
`PKG_CONFIG_PATH` accordingly.

For instance, to build against aubio installed in `/var/tmp/aubio`:

    $ export PKG_CONFIG_PATH=/var/tmp/aubio/lib/pkgconfig
    $ waf distclean configure build

Installing
----------

To install pd-aubio:

    # ./waf install

Depending on the installation location, you might need to run this command with
sudo (or as the user root), or to copy the files manually.

To install it in a different location, you could also try:

    $ ./waf configure --prefix=/my/favorite/place build
    # ./waf install

Running
-------

To load the external, start pd as follows

    $ pd -lib aubio

Alternatively, you can create an object [aubio] to initialize the external.

If you want to use the external without installing it, the above command should
work as expected from the pd-aubio source directory. If needed, set your
`LD_LIBRARY_PATH` to include the path to your libaubio installation.
