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

libedbg = library(
            'edbg',
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
            dependencies : [libcpp, libyaml, libpdbg, libipl],
            link_with : libecmd,
            version: meson.project_version(),
            install: true)

import('pkgconfig').generate(
  name: 'libedbg',
  description: 'EDBG [ecmd-pdbg] library',
  version: meson.project_version(),
  libraries: libedbg)
