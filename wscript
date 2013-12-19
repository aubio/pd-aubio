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
    # check for puredata header
    ctx.check(header_name='m_pd.h')
    # required dependancies
    ctx.check_cfg(package = 'aubio', atleast_version = '0.4.0',
            args = '--cflags --libs')

def build(bld):

    uselib = ['AUBIO']
    bld(features = 'c cshlib',
        source = bld.path.ant_glob('*.c'),
        uselib = ['AUBIO'],
        target = 'aubio',
        defines = ['PD', 'PACKAGE_VERSION=\"'+repr(VERSION)+"\""],
        install_path = '${PREFIX}/lib/pd/extra')

    if bld.env['DEST_OS'] == 'win32':
        bld.env.cshlib_PATTERN = '%s.dll'
        # do not use -fPIC -DPIC on windows
        bld.env.cshlib_CCFLAGS.remove('-fPIC')
        bld.env.cshlib_CCFLAGS.remove('-DPIC')
        bld.env.append_unique('shlib_LINKFLAGS', ['-export_dynamic', '-lpd'])
    elif bld.env['DEST_OS'] == 'darwin':
        bld.env.cshlib_PATTERN = '%s.pd_darwin'
        bld.env.append_unique('shlib_LINKFLAGS',
            ['-bundle', '-undefined suppres', '-flat_namespace'])
    else: #if bld.env['DEST_OS'] == 'linux':
        bld.env.cshlib_PATTERN = '%s.pd_linux'
        bld.env.append_unique('shlib_LINKFLAGS', ['--export_dynamic'])

    # do not rename the shlib at all
    bld.env.shlib_PATTERN = '%s'

    bld.install_files('${PREFIX}/lib/pd/doc/5.reference',
            bld.path.ant_glob('help/**.pd'))

    bld.install_files('${PREFIX}/lib/pd/doc/aubio',
            bld.path.ant_glob('examples/**.pd'))
