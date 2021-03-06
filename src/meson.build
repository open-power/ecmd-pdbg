headers = include_directories('common',
                              'vpd',
                              'istep',
                              'dll',
                              'p9',
                              'p9/ekb',
                              '../ecmd/ecmd-core/capi',
                              '../ecmd/ecmd-core/cmd',
                              '../' + autogen_path,
                              '../ecmd/ecmd-core/ext/cip/capi')

libpdbg = meson.get_compiler('cpp').find_library('pdbg')
libipl = meson.get_compiler('cpp').find_library('ipl')
libekb = meson.get_compiler('cpp').find_library('ekb')

libedbg = library(
            'edbg',
            'p9/p9_edbgEcmdDllScom.C',
            'dll/edbgEcmdDll.C',
            'dll/edbgEcmdDllInfo.C',
            'istep/edbgIstep.C',
            'common/edbgOutput.C',
            'p9/ekb/p9_scominfo.C',
            'vpd/lhtVpd.C',
            'vpd/lhtVpdFile.C',
            'vpd/lhtVpdDevice.C',
            'dll/edbgCipDll.C',
            '../ecmd/ecmd-core/dll/ecmdDllCapi.C',
            '../' + autogen_path + '/cipDllCapi.C',
            include_directories : headers,
            dependencies : [libcpp, libyaml, libpdbg, libipl, libekb],
            link_with : libecmd,
            version: meson.project_version(),
            install: true)

#Setting default to p10
helptext_path = 'istep/p10/help/'

#TODO:We will update this path once p11 comes in else condition
if get_option('chip').startswith('p10')
  helptext_path = 'istep/p10/help/'
endif

# Installing the istep extension help text(htxt) files to '/usr/help'
helptext = 'istep_list.htxt'
infile  = helptext_path + helptext
outfile = helptext
cp_target = custom_target(helptext,
            output : outfile,
            input : infile,
            command : [cp_cmd, '@INPUT@', '@OUTPUT@'],
            install : true,
            install_dir : helpdir)

import('pkgconfig').generate(
  name: 'libedbg',
  description: 'EDBG [ecmd-pdbg] library',
  version: meson.project_version(),
  libraries: libedbg)
