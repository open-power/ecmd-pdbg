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
python  = find_program('python', required : true)
libcpp  = meson.get_compiler('cpp').find_library('libstdc++')
libz    = meson.get_compiler('cpp').find_library('libz')
libdl   = meson.get_compiler('cpp').find_library('libdl')
libyaml = meson.get_compiler('cpp').find_library('libyaml')

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

'''
# Following piece of code to auto generate the files using custom_target doesn't
# seem to fly.
env = environment()
env.set('SRCPATH', join_paths(project_source_root,'src'))
rc = run_command(join_paths(project_source_root, 'autogen.sh'),
                    :env: env)

if rc.returncode() != 0
  message('ERROR=' + rc.stderr().strip())
endif

rc = run_command('mkdir', join_paths(project_source_root,'ecmd/src'))
if rc.returncode() != 0
  error('create directory for autogenerate failed')
endif

message('Autogenerating of ecmd & client CAPI code')
makedll_cmd = find_program('ecmd/mkscripts/makedll.pl')
foreach capi_gen : ['ecmdClientCapiFunc.C', 'ecmdDllCapi.H', 'ecmdClientEnums.H']
   rc = run_command('cd', join_paths(project_source_root,'ecmd/ecmd-core/capi'))
   if rc.returncode() != 0
     error('change directory to ecmd/ecmd-core/capi failed')
   endif
   autogen_target = custom_target(
     capi_gen,
     output: capi_gen,
     input:'makedll.pl',
     command: [perl, '@INPUT@','ecmd','@OUTPUT@'])
endforeach
'''

build_arch = build_machine.cpu_family()
host_arch = host_machine.cpu_family()
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

foreach compileflags :['-DBUILD_DATE=' + build_date_time,
                       '-DGIT_COMMIT_REV=' + conf.get('GIT_COMMIT_REV'),
                       '-DECMD_STRIP_DEBUG',
                       '-DECMD_REMOVE_SEDC_SUPPORT',
                       '-DECMD_REMOVE_LATCH_FUNCTIONS',
                       '-DECMD_REMOVE_ARRAY_FUNCTIONS',
                       '-DECMD_REMOVE_SPY_FUNCTIONS',
                       '-DECMD_REMOVE_REFCLOCK_FUNCTIONS',
                       '-DECMD_REMOVE_PROCESSOR_FUNCTIONS',
                       '-DECMD_REMOVE_GPIO_FUNCTIONS',
                       '-DECMD_REMOVE_I2C_FUNCTIONS',
                       '-DECMD_REMOVE_POWER_FUNCTIONS',
                       '-DECMD_REMOVE_ADAL_FUNCTIONS',
                       '-DECMD_REMOVE_JTAG_FUNCTIONS',
                       '-DECMD_REMOVE_INIT_FUNCTIONS',
                       '-DECMD_REMOVE_TRACEARRAY_FUNCTIONS ',
                       '-DECMD_REMOVE_SENSOR_FUNCTIONS',
                       '-DECMD_REMOVE_BLOCK_FUNCTIONS',
                       '-DECMD_REMOVE_MPIPL_FUNCTIONS',
                       '-DECMD_REMOVE_PNOR_FUNCTIONS',
                       '-DECMD_REMOVE_SP_FUNCTIONS',
                       '-DECMD_REMOVE_UNITID_FUNCTIONS',
                       '-DCIP_REMOVE_INSTRUCTION_FUNCTIONS',
                       '-DCIP_REMOVE_BREAKPOINT_FUNCTIONS',
                       '-DCIP_REMOVE_VR_FUNCTIONS',
                       '-DCIP_REMOVE_VSR_FUNCTIONS',
                       '-DCIP_REMOVE_PORE_FUNCTIONS',
                       '-DCIP_REMOVE_RW_FUNCTIONS',
                       '-DCIP_REMOVE_MBOX_FUNCTIONS',
                       '-DCIP_REMOVE_PMC_VOLTAGE_FUNCTIONS']

    add_global_arguments(compileflags, language : 'cpp')
    add_global_arguments(compileflags, language : 'c')
endforeach

libecmd_headers = include_directories('ecmd/ecmd-core/capi',
                              'ecmd/ecmd-core/capi/sedcScomdef/',
                              autogen_path)

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

# Create shared lib "libstub"
libstub_headers = include_directories('ecmd/dllStub',
                                      autogen_path,
                                      'ecmd/ecmd-core/dll',
                                      'ecmd/ecmd-core/capi')

libstub = library(
            'stub',
            'ecmd/dllStub/ecmdDllStub.C',
            'ecmd/dllStub/ecmdDllStubInfo.C',
            include_directories : libstub_headers,
            dependencies : [libcpp, libdl],
            link_with : libecmd,
            version: meson.project_version(),
            install: true)

import('pkgconfig').generate(
  name: 'libstub',
  description: 'ECMD Stub library',
  version: meson.project_version(),
  libraries: libstub)


# Create ecmd executable
# This is the main executable where all the HW access tools are liked to
ecmd_headers = include_directories('ecmd/ecmd-core/cmd',
                                   autogen_path,
                                   'ecmd/ecmd-core/capi')

edbg = executable(
         'edbg',
         'ecmd/ecmd-core/cmd/ecmdInterpreter.C',
         autogen_path + '/ecmdExtInterpreter.C',
         'ecmd/ecmd-core/cmd/ecmdCommandUtils.C',
         'ecmd/ecmd-core/cmd/ecmdScomUser.C',
         'ecmd/ecmd-core/cmd/ecmdRingUser.C',
	 'ecmd/ecmd-core/cmd/ecmdQueryUser.C',
	 'ecmd/ecmd-core/cmd/ecmdSpyUser.C',
	 'ecmd/ecmd-core/cmd/ecmdSimUser.C',
	 'ecmd/ecmd-core/cmd/ecmdArrayUser.C',
	 'ecmd/ecmd-core/cmd/ecmdJtagUser.C',
	 'ecmd/ecmd-core/cmd/ecmdProcUser.C',
	 'ecmd/ecmd-core/cmd/ecmdMemUser.C',
	 'ecmd/ecmd-core/cmd/ecmdIstepUser.C',
	 'ecmd/ecmd-core/cmd/ecmdMiscUser.C',
	 'ecmd/ecmd-core/cmd/ecmdVpdUser.C',
	 'ecmd/ecmd-core/cmd/ecmdI2cGpioUser.C',
	 'ecmd/ecmd-core/cmd/ecmdPowerUser.C',
	 'ecmd/ecmd-core/cmd/ecmdPsiUser.C',
	 'ecmd/ecmd-core/cmd/ecmdPnorUser.C',
	 'ecmd/ecmd-core/cmd/ecmdMain.C',
	 include_directories : ecmd_headers,
	 dependencies : [libcpp,libdl],
	 link_with : [libecmd,libecmdClientCapi],
	 install: true,
	 install_dir: get_option('bindir'))

# Install edbgWrapper.sh to /usr/bin
install_data('bin/edbgWrapper.sh', install_dir : get_option('bindir'))

# Create a soft link to edbgWrapper.sh
link_cmd = find_program('ln')
foreach ecmdtool : ['ecmdquery', 'getcfam', 'putcfam', 'getscom', 'putscom',
                'getmemproc', 'putmemproc', 'stopclocks','startclocks']
    infile = 'bin/edbgWrapper.sh'
    outfile = ecmdtool

    link_target = custom_target(ecmdtool,
      output : outfile,
      input : infile,
      command : [link_cmd, '@INPUT@', '@OUTPUT@'],
      install : true,
      install_dir : get_option('bindir'))
endforeach


# Installing the help text(htxt) files to '/usr/help'
cp_cmd = find_program('cp')
helptext_path = 'ecmd/ecmd-core/cmd/help/'
foreach helptext : ['getscom.htxt', 'putscom.htxt', 'getcfam.htxt',
                   'putcfam.htxt', 'putmemproc.htxt', 'getmemproc.htxt',
                   'stopclocks.htxt', 'startclocks.htxt', 'ecmdquery.htxt',
                   'getvpdkeyword.htxt','putvpdkeyword.htxt','ecmd.htxt']
    infile  = helptext_path + helptext
    outfile = helptext

    cp_target = custom_target(helptext,
      output : outfile,
      input : infile,
      command : [cp_cmd, '@INPUT@', '@OUTPUT@'],
      install : true,
      install_dir : helpdir)
endforeach

configure_file(output: 'config.h', configuration: conf)
subdir('src')
