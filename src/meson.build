headers = include_directories('common',
                              'vpd',
                              'istep',
                              'regaccess',
                              'dll',
                              'p9',
                              'p9/ekb',
                              'p10',
                              'p10/ekb',
                              '../ecmd/ecmd-core/capi',
                              '../ecmd/ecmd-core/cmd',
                              '../' + autogen_path,
                              '../ecmd/ecmd-core/ext/cip/capi',
                              '..')

libpdbg = meson.get_compiler('cpp').find_library('pdbg')
libipl = meson.get_compiler('cpp').find_library('ipl')
libekb = meson.get_compiler('cpp').find_library('ekb')

if get_option('journal').enabled()
    libsystemd_pkg = dependency('libsystemd')
endif

libedbg = library(
            'edbg',
            'common/edbgCommon.C',
            'p9/p9_edbgEcmdDllScom.C',
            'p10/p10_edbgEcmdDllScom.C',
            'dll/edbgEcmdDll.C',
            'dll/edbgEcmdDllInfo.C',
            'istep/edbgIstep.C',
            'regaccess/ecmdMapSpr2Str.C',
            'common/edbgOutput.C',
            'p9/ekb/p9_scominfo.C',
            'p10/ekb/p10_scominfo.C',
            'p10/ekb/p10_scom_addr.C',
            'p10/ekb/p10_spr_name_map.C',
            'vpd/lhtVpd.C',
            'vpd/lhtVpdFile.C',
            'vpd/lhtVpdDevice.C',
            'p9/p9_edbgCipDllInstrCtrl.C',
            'p10/p10_edbgCipDllInstrCtrl.C',
            'dll/edbgCipDll.C',
            '../ecmd/ecmd-core/dll/ecmdDllCapi.C',
            '../' + autogen_path + '/cipDllCapi.C',
            include_directories : headers,
            dependencies : [libcpp,libfs, libyaml, libpdbg, libipl, libekb, get_option('journal').enabled() ? libsystemd_pkg :[]],
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
