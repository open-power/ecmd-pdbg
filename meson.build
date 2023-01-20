project(
    'ecmd-pdbg',
    ['cpp','c'],
    default_options: [
        'cpp_std=c++17',
        'default_library=shared',
    ],
    meson_version : '>= 0.51',
    license: 'Apache-2.0',
    version: '0.1',
)

conf = configuration_data()
conf.set('PROJECT_VERSION',        meson.project_version())

substs = configuration_data()
substs.set('PROJECT_URL',          'https://github.com/open-power/ecmd-pdbg')
substs.set('PROJECT_VERSION',      meson.project_version())

# This is to be used instead of meson.source_root(), as the latter will return
# the wrong result when ecmd-pdbg is being built as a meson subproject
project_source_root = meson.current_source_dir()
relative_source_path = run_command('realpath',
                                   '--relative-to=@0@'.format(meson.current_build_dir()),
                                   project_source_root).stdout().strip()
conf.set_quoted('RELATIVE_SOURCE_PATH', relative_source_path)
cpp = meson.get_compiler('cpp')

possible_cpp_flags = [
        '-Wextra',
        '-Wmissing-include-dirs',
        '-Wold-style-definition',
        '-Wsuggest-attribute=noreturn',
        '-Werror=implicit-function-declaration',
        '-Werror=return-type',
        '-Werror=incompatible-pointer-types',
        '-Wredundant-decls',
        '-Wmissing-noreturn',
        '-Wshadow',
        '-Wnested-externs',

        # Wno-psabi reduces the number of "Note:" messages when cross-compiling some STL
        # stuff for ARM.
        # See https://stackoverflow.com/questions/48149323/strange-gcc-warning-when-compiling-qt-project
        # Basically, gcc 6 and gcc 7 are not ABI compatible, but since the whole OpenBMC
        # project uses the same compiler, we can safely ignmore these info notes.
        # negative arguments are correctly detected starting with meson 0.46.
        '-Wno-psabi',
]

add_project_arguments(cpp.get_supported_arguments(possible_cpp_flags), language : 'cpp')

#Checking if cross compilation works
if cpp.compiles('''
   #include <time.h>
   #include <inttypes.h>
   typedef uint64_t usec_t;
   usec_t now(clockid_t clock);
   int main(void) {
           struct timespec now;
           return 0;
   }
''', args: '-Werror=shadow', name : '-Werror=shadow with local shadowing')
        add_project_arguments('-Werror=shadow', language : 'cpp')
endif

conf.set('_GNU_SOURCE', true)
conf.set('__SANE_USERSPACE_TYPES__', true)

#dependency check
git     = find_program('git', required : true)
perl    = find_program('perl', required : true)
python  = find_program('python3', required : true)
libcpp  = meson.get_compiler('cpp').find_library('stdc++')
libfs   = meson.get_compiler('cpp').find_library('stdc++fs')
libz    = meson.get_compiler('cpp').find_library('z')
libdl   = meson.get_compiler('cpp').find_library('dl')
libyaml = meson.get_compiler('cpp').find_library('yaml')

message('git submodule init & update of ecmd')
rc = run_command(git, 'submodule', 'init', 'ecmd')
if rc.returncode() != 0
  error('git submodule init of ecmd : FAILED')
endif

message('git submodule init of ecmd : SUCCESS')

rc = run_command(git, 'submodule', 'update', 'ecmd')
if rc.returncode() != 0
  error('git submodule update of ecmd : FAILED')
endif

message('git submodule update of ecmd : SUCCESS')

build_arch = build_machine.cpu()
host_arch = host_machine.cpu()
message('Build target: ' + build_arch)
message('Host target: ' + host_arch)
conf.set_quoted('BUILD_ARCH_TYPE', build_arch)
conf.set_quoted('HOST_ARCH_TYPE', host_arch)

#This is the path in project root where auto generated files will be created
autogen_path = 'ecmd/src_' + build_arch
message('Auto generated path: ' + autogen_path)

rootprefixdir = get_option('rootprefix')
rootprefix_default = '/usr'
if rootprefixdir == ''
    rootprefixdir = rootprefix_default
endif

helpdir = join_paths(rootprefixdir, 'help')

#Removing the dir before we create dir for auto generated files
rc = run_command('rm', '-rf', autogen_path)
if rc.returncode() != 0
  error('remove directory failed, error is: ' + rc.stderr().strip())
endif

#Create the directory
rc = run_command('mkdir', autogen_path)
if rc.returncode() != 0
  error('create directory for auto generate code failed, error is: ' +
        rc.stderr().strip())
endif

# autogen.sh script internally uses makedll.pl and makext.py script to
# auto generate required C and header codes
message('Auto generating required C and header files')
rc = run_command(join_paths(project_source_root, 'autogen.sh'))
if rc.returncode() != 0
  error('Autogeneration failed, error is: ' + rc.stderr().strip())
endif

#Get build date and time
rc = run_command('date','+"%Y-%m-%d\ %H:%M:%S\ %Z"')
if rc.returncode() != 0
  error('date and time command failed')
endif
build_date_time = rc.stdout().strip()
message('Build Date & time: ' +build_date_time)

#Get GIT commit revision
if git.found()
    rc = run_command('git', '--work-tree=.', '--git-dir=./.git', 'describe',
                     '--always', '--long', '--dirty')
    if rc.returncode() != 0
      error('git commit revision fetch failed')
    endif
endif
git_commit_rev = rc.stdout().strip()
message('Git commit rev: ' +git_commit_rev)
conf.set_quoted('GIT_COMMIT_REV', git_commit_rev)

# We let compile time know plugin lib path which is always fixed to avoid 
# runtime envirorment variable setting
plugin_lib = '/usr/lib/libedbg.so.' + meson.project_version()
conf.set_quoted('ECMD_DLL_FILE', plugin_lib)

foreach compileflags :['-DBUILD_DATE=' + build_date_time,
                       '-DGIT_COMMIT_REV=' + conf.get('GIT_COMMIT_REV'),
                       '-DEDBG_BUILD_EBMC',
                       '-DEDBG_ISTEP_CTRL_FUNCTIONS',
                       '-DECMD_DLL_FILE_FIXED=' + conf.get('ECMD_DLL_FILE')]

    add_global_arguments(compileflags, language : 'cpp')
    add_global_arguments(compileflags, language : 'c')
endforeach

libecmd_headers = include_directories('ecmd/ecmd-core/capi',
                              'ecmd/ecmd-core/capi/sedcScomdef/',
                              autogen_path)

cip_headers = include_directories('ecmd/ecmd-core/ext/cip/capi/')

# Create static lib "ecmdClientCapi"
libecmdClientCapi = static_library(
                      'ecmdClientCapi',
                      autogen_path + '/ecmdClientCapiFunc.C',
                      'ecmd/ecmd-core/capi/ecmdClientCapi.C',
                      'ecmd/ecmd-core/capi/ecmdUtils.C',
                      'ecmd/ecmd-core/capi/sedcScomdef/sedcScomdefParser.C',
                      'ecmd/ecmd-core/capi/sedcScomdef/sedcScomdefClasses.C',
                      'ecmd/ecmd-core/capi/sedcScomdef/sedcCommonParser.C',
                      'ecmd/ecmd-core/capi/sedcScomdef/sedcCommonClasses.C',
                      include_directories : libecmd_headers,
                      install: true)

# Create static lib "cipClientCapi"
libcipClientCapi = static_library(
                      'cipClientCapi',
                      autogen_path + '/cipClientCapiFunc.C',
                      autogen_path + '/cipClientCapi.C',
                      include_directories : [cip_headers,libecmd_headers],
                      install: true)

# Create shared lib "libecmd"
libecmd = library(
            'ecmd',
            'ecmd/ecmd-core/capi/ecmdDataBufferBase.C',
            'ecmd/ecmd-core/capi/ecmdDataBuffer.C',
            'ecmd/ecmd-core/capi/ecmdStructs.C',
            'ecmd/ecmd-core/capi/ecmdSharedUtils.C',
            'ecmd/ecmd-core/capi/ecmdParseTokens.C',
            'ecmd/ecmd-core/capi/ecmdChipTargetCompare.C',
            'ecmd/ecmd-core/capi/ecmdWriteTarget.C',
            dependencies : [libdl,libz],
            link_with : libecmdClientCapi,
            include_directories : libecmd_headers,
            version: meson.project_version(),
            install: true)

import('pkgconfig').generate(
  name: 'libecmd',
  description: 'ECMD library',
  version: meson.project_version(),
  libraries: libecmd)

# Create ecmd executable
# This is the main executable where all the HW access tools are liked to
ecmd_headers = include_directories('src/common',
                                   'ecmd/ecmd-core/cmd',
                                   autogen_path,
                                   'ecmd/ecmd-core/capi',
                                   'ecmd/ecmd-core/ext/cip/capi/',
                                   'ecmd/ecmd-core/ext/cip/cmd/')

edbg = executable(
         'edbg',
         'ecmd/ecmd-core/cmd/ecmdInterpreter.C',
         autogen_path + '/ecmdExtInterpreter.C',
         'ecmd/ecmd-core/cmd/ecmdCommandUtils.C',
         'ecmd/ecmd-core/cmd/ecmdScomUser.C',
         'ecmd/ecmd-core/cmd/ecmdRingUser.C',
	 'ecmd/ecmd-core/cmd/ecmdQueryUser.C',
	 'ecmd/ecmd-core/cmd/ecmdSpyUser.C',
	 'ecmd/ecmd-core/cmd/ecmdArrayUser.C',
	 'ecmd/ecmd-core/cmd/ecmdJtagUser.C',
	 'ecmd/ecmd-core/cmd/ecmdMemUser.C',
	 'ecmd/ecmd-core/cmd/ecmdIstepUser.C',
	 'ecmd/ecmd-core/cmd/ecmdMiscUser.C',
	 'ecmd/ecmd-core/cmd/ecmdVpdUser.C',
	 'ecmd/ecmd-core/cmd/ecmdI2cGpioUser.C',
	 'ecmd/ecmd-core/cmd/ecmdProcUser.C',
         'ecmd/ecmd-core/ext/cip/cmd/cipInterpreter.C',
         'ecmd/ecmd-core/ext/cip/cmd/cipProcUser.C',
	 'ecmd/ecmd-core/cmd/ecmdMain.C',
	 include_directories : ecmd_headers,
	 dependencies : [libcpp,libdl,libz],
	 link_with : [libecmd,libecmdClientCapi,libcipClientCapi],
	 install: true,
	 install_dir: get_option('bindir'))

# Install edbgBmcWrapper.sh to /usr/bin
install_data('bin/edbgBmcWrapper.sh', install_dir : get_option('bindir'))

# Create a soft link to edbgBmcWrapper.sh
link_cmd = find_program('ln')
foreach ecmdtool : ['ecmdquery', 'getcfam', 'putcfam', 'getscom', 'putscom',
                    'getmemproc', 'putmemproc', 'stopclocks','startclocks',
                    'istep','cipinstruct', 'cipgetmemproc', 'cipputmemproc',
                    'getmempba', 'putmempba', 'getgpr', 'putgpr', 'getspr',
                    'putspr']
    infile = 'bin/edbgBmcWrapper.sh'
    outfile = ecmdtool

    link_target = custom_target(ecmdtool,
      output : outfile,
      input : infile,
      command : [link_cmd, '@INPUT@', '@OUTPUT@'],
      install : true,
      install_dir : get_option('bindir'))
endforeach

# Installing the ecmd help text(htxt) files to '/usr/help'
cp_cmd = find_program('cp')
helptext_path = 'ecmd/ecmd-core/cmd/help/'
foreach helptext : ['getscom.htxt', 'putscom.htxt', 'getcfam.htxt',
                    'putcfam.htxt', 'putmemproc.htxt', 'getmemproc.htxt',
                    'stopclocks.htxt', 'startclocks.htxt', 'ecmdquery.htxt',
                    'getvpdkeyword.htxt','putvpdkeyword.htxt','ecmd.htxt',
                    'istep.htxt','getmempba.htxt', 'putmempba.htxt','getgpr.htxt',
                    'putgpr.htxt', 'getspr.htxt', 'putspr.htxt']
    infile  = helptext_path + helptext
    outfile = helptext

    cp_target = custom_target(helptext,
      output : outfile,
      input : infile,
      command : [cp_cmd, '@INPUT@', '@OUTPUT@'],
      install : true,
      install_dir : helpdir)
endforeach

# Installing the cip extension help text(htxt) files to '/usr/help'
helptext_path = 'ecmd/ecmd-core/ext/cip/cmd/help/'
foreach helptext : ['cipinstruct.htxt', 'cipgetmemproc.htxt', 'cipputmemproc.htxt']
    infile  = helptext_path + helptext
    outfile = helptext

    cp_target = custom_target(helptext,
      output : outfile,
      input : infile,
      command : [cp_cmd, '@INPUT@', '@OUTPUT@'],
      install : true,
      install_dir : helpdir)
endforeach

conf.set('JOURNAL', get_option('journal').enabled(),
              description : 'Use journal to log traces'
             )
configure_file(output: 'config.h', configuration: conf)
subdir('src')
