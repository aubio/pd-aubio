#! /usr/bin/python

# build puredata external

top = '.'
out = 'build'
APPNAME = 'pd-aubio'

# source VERSION
for l in open('VERSION').readlines(): exec (l.strip())
VERSION = '.'.join ([str(x) for x in [
    PD_AUBIO_MAJOR_VERSION,
    PD_AUBIO_MINOR_VERSION,
    ]]) + PD_AUBIO_VERSION_STATUS

def options(ctx):
    ctx.load('compiler_c')

def configure(ctx):
    ctx.load('compiler_c')

    if ctx.env['DEST_OS'] == 'linux':
        ctx.env.cshlib_PATTERN = '%s.pd_linux'
    elif ctx.env['DEST_OS'] == 'darwin':
        ctx.env.cshlib_PATTERN = '%s.pd_darwin'
        # add default include path for both pd and pd-extended
        # set CFLAGS for custom location
        ctx.env.CFLAGS += ['-I/Applications/Pd-0.45-3.app/Contents/Resources/src']
        ctx.env.CFLAGS += ['-I/Applications/Pd-extended.app/Contents/Resources/include']
        ctx.env.CFLAGS += ['-arch', 'i386', '-arch', 'x86_64']
        ctx.env.LINKFLAGS += ['-arch', 'i386', '-arch', 'x86_64']
        ctx.env.LINKFLAGS_cshlib = ['-bundle', '-undefined', 'suppress', '-flat_namespace']
    elif ctx.env['DEST_OS'] in ['win32', 'win64']:
        ctx.env.cshlib_PATTERN = '%s.dll'
        # do not use -fPIC -DPIC on windows
        ctx.env.CFLAGS_cshlib = []
        ctx.env.LINKFLAGS_cshlib +=  ['-export_dynamic', '-lpd']
    else:
        ctx.fatal("Sorry, i don't know how to build for %s yet" % ctx.env['DEST_OS'])

    # check for puredata header
    ctx.check(header_name='m_pd.h')

    # required dependencies
    ctx.check_cfg(package = 'aubio', atleast_version = '0.4.0',
            args = '--cflags --libs')

def build(bld):

    bld(features = 'c cshlib',
        source = bld.path.ant_glob('src/*.c'),
        uselib = ['AUBIO'],
        target = 'aubio',
        defines = ['PD', 'PACKAGE_VERSION=\"'+repr(VERSION)+"\""],
        install_path = '${PREFIX}/lib/pd/extra')

    bld.install_files('${PREFIX}/lib/pd/doc/5.reference',
            bld.path.ant_glob('help/**.pd'))

    bld.install_files('${PREFIX}/lib/pd/doc/aubio',
            bld.path.ant_glob('examples/**.pd'))
