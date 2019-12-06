#!/bin/sh

# ***************************************************************************
# Get rid of any path information from the command that came in
# ***************************************************************************
cmd=${0##*/}

ECMD_EXE="/usr/bin/edbg"
if [ -f $ECMD_EXE ]; then
    export ECMD_EXE
elif [ ! -x "$ECMD_EXE" ]
  then
    echo "*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****"
    echo "The eCMD executable '$ECMD_EXE' does NOT exist or is not executable"
    echo "Please modify your ECMD_EXE variable to point to a valid eCMD executable before running"
    echo "*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****"
    exit 1
fi

ECMD_DLL_FILE="/usr/lib/libedbg.so.0.1"
if [ -f $ECMD_DLL_FILE ]; then
    export ECMD_DLL_FILE
elif [ ! -x "$ECMD_DLL_FILE" ]
  then
    echo "*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****"
    echo "The eCMD dll '$ECMD_DLL_FILE' does NOT exist or is not executable"
    echo "Please modify your ECMD_DLL_FILE variable to point to a valid eCMD dll before running"
    echo "*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****"
    exit 1
fi

# ***************************************************************************
# put all of the arguments from the command line together and run it
# ***************************************************************************
$ECMD_EXE $cmd "$@"
