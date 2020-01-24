#!/bin/bash

CWD=`pwd`
ARCH=`uname -m`
echo "$ARCH"

#set EXT_CMD and EXT_${EXT_CMD}_PATH required by makeext.py script to
#auto generate ecmdExtInterpreter.C
export EXT_CMD="cip"
VAR1="EXT_"
VAR2="${EXT_CMD}"
VAR3="_PATH"
EXT_PATH="$VAR1$VAR2$VAR3"
export ${EXT_PATH}=${CWD}/ecmd/ecmd-core/ext/cip

AUTOGEN_PATH="${CWD}/ecmd/src_"$ARCH
export SRCPATH=$AUTOGEN_PATH
echo "$SRCPATH"

MAKE_DLL="${CWD}/ecmd/mkscripts/makedll.pl"
MAKE_EXT="${CWD}/ecmd/mkscripts/makeext.py"
CREATE_ECMD_HELP="${CWD}/ecmd/ecmd-core/cmd/createEcmdHelp.pl"
ECMD_EXT="${CWD}/ecmd/ecmd-core/capi"
CIP_EXT="${CWD}/ecmd/ecmd-core/ext/cip/capi"
TEMPLATE_EXT="${CWD}/ecmd/ecmd-core/ext/template/capi"

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

#Generation of cip extension source files
comp=([cip]=${TEMPLATE_EXT})

declare -a templateFiles=(templateClientCapi.C templateDllCapi.C)

#replace template with cip, Template with Cip and TEMPLATE with CIP
#in template files
EXTENSION_NAME_u=$(echo ${component} | tr 'a-z' 'A-Z')
EXTENSION_NAME_u1=$(perl -e 'printf(ucfirst('${component}'))')

#Using sed to replace the patterns in the template files with component name
for infile in "${templateFiles[@]}"
do
    for component in "${!comp[@]}"
    do
        cd ${comp[${component}]}
        cp -f ${infile} ${SRCPATH}
        cd ${SRCPATH}
        sed -i "s/template/${component}/g" ${infile}
        sed -i "s/TEMPLATE/${EXTENSION_NAME_u}/g" ${infile}
        sed -i "s/Template/${EXTENSION_NAME_u1}/g" ${infile}
    done
done

declare -a cipExtFiles=(cipClientCapi.C cipDllCapi.C)

#Renaming template with cip in file name
for (( i=0; i<${#templateFiles[@]}; i++ ))
do
    echo "Generating ${cipExtFiles[i]}"
    mv ${templateFiles[i]} ${cipExtFiles[i]}
done
