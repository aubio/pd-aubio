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
        ctx.env.CFLAGS += ['-I/Applications/Pd-0.47-1.app/Contents/Resources/src']
        ctx.env.CFLAGS += ['-I/Applications/Pd-0.47-1-64bit.app/Contents/Resources/src']
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
        ctx.start_msg("Checking for platform")
        ctx.end_msg("no idea how to build for %s yet, assuming linux"
                % ctx.env['DEST_OS'], 'YELLOW')
        ctx.env.cshlib_PATTERN = '%s.pd_linux'
        ctx.env.LINKFLAGS_cshlib += ['--export_dynamic']

    # check for puredata header
    ctx.check(header_name='m_pd.h')

    ctx.check(header_name='pthread.h', mandatory = False)
    needs_pthread = ctx.get_define("HAVE_PTHREAD") is not None
    if needs_pthread:
        ctx.check_cc(lib="pthread", uselib_store="PTHREAD", mandatory=needs_pthread)

    # required dependencies
    ctx.check_cfg(package = 'aubio', atleast_version = '0.4.0',
            args = '--cflags --libs')

def build(bld):

    bld(features = 'c cshlib',
        source = bld.path.ant_glob('src/*.c'),
        uselib = ['AUBIO'],
        target = 'aubio',
        defines = ['PD', 'PACKAGE_VERSION=\"'+repr(VERSION)+"\""],
        install_path = '${PREFIX}/lib/pd/extra/aubio')

    bld.install_files('${PREFIX}/lib/pd/extra/aubio',
            bld.path.ant_glob('help/**.pd'))

    bld.install_files('${PREFIX}/lib/pd/extra/aubio/examples',
            bld.path.ant_glob('examples/**.pd'))
