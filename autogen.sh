#!/bin/bash

CWD=`pwd`
ARCH=`uname -m`

export EXT_CMD=""
AUTOGEN_PATH="${CWD}/ecmd/src_"$ARCH

export SRCPATH=$AUTOGEN_PATH
echo "$SRCPATH"

MAKE_DLL="${CWD}/ecmd/mkscripts/makedll.pl"
MAKE_EXT="${CWD}/ecmd/mkscripts/makeext.py"
CREATE_ECMD_HELP="${CWD}/ecmd/ecmd-core/cmd/createEcmdHelp.pl"
ECMD_EXT="${CWD}/ecmd/ecmd-core/capi"
CIP_EXT="${CWD}/ecmd/ecmd-core/ext/cip/capi"

declare -A comp
comp=([ecmd]=${ECMD_EXT} [cip]=${CIP_EXT})

declare -a genFiles=(ecmdClientCapiFunc.C ecmdDllCapi.H ecmdClientEnums.H cipClientCapiFunc.C cipDllCapi.H cipClientEnums.H)

for file in "${genFiles[@]}"
do
    for component in "${!comp[@]}"
    do
       if [[ "${file}" =~ ^${component}.* ]]; then
           cd ${comp[${component}]}
           echo "Generating ${file}"
           ${MAKE_DLL} ${component} ${file}
       fi
    done
done

cd ${CWD}/ecmd/ecmd-core/cmd
echo "Generating ecmdExtInterpreter.C"
${MAKE_EXT} cmd

ecmdhelp_path="help/ecmd.htxt"
echo "Generating ecmd.htxt"
${CREATE_ECMD_HELP} "${ecmdhelp_path}"

