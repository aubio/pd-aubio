#! /usr/bin/python
# build puredata external

srcdir = '.'
blddir = 'build'

def set_options(opt):
  opt.tool_options('compiler_cc')

def configure(conf):
  conf.check_tool('compiler_cc')
  # required dependancies
  conf.check_cfg(package = 'aubio', atleast_version = '0.4.0',
    args = '--cflags --libs')

def build(bld):
    aubio_pd = bld.new_task_gen(
        features = 'cc cshlib',
        uselib = ['AUBIO'],
        defines = ['PD'],
        install_path = '${PREFIX}/lib/pd/extra')

    if bld.env['DEST_OS'] == 'win32':
        aubio_pd.target = 'aubio.dll'
        # do not use -fPIC -DPIC on windows
        aubio_pd.env.shlib_CCFLAGS.remove('-fPIC')
        aubio_pd.env.shlib_CCFLAGS.remove('-DPIC')
        aubio_pd.env.append_unique('shlib_LINKFLAGS', ['-export_dynamic', '-lpd'])
    elif bld.env['DEST_OS'] == 'darwin':
        aubio_pd.target = 'aubio.pd_darwin'
        aubio_pd.env.append_unique('shlib_LINKFLAGS', 
            ['-bundle', '-undefined suppres', '-flat_namespace'])
    else: #if bld.env['DEST_OS'] == 'linux':
        aubio_pd.target = 'aubio.pd_linux'
        aubio_pd.env.append_unique('shlib_LINKFLAGS', ['--export_dynamic'])

    # do not rename the shlib at all
    aubio_pd.env.shlib_PATTERN = '%s'
    # get the source files
    aubio_pd.find_sources_in_dirs('.')

    bld.install_files('${PREFIX}/lib/pd/doc/5.reference', 'help/*.pd')

    bld.install_files('${PREFIX}/lib/pd/doc/aubio', 'examples/*.pd')
