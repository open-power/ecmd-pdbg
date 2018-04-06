#!/bin/sh

# ***************************************************************************
# Get rid of any path information from the command that came in
# ***************************************************************************
cmd=${0##*/}

# ***************************************************************************
# Figure out what the user's ecmd executable is set to, if they have one.
# ***************************************************************************
if [ ! -x "$ECMD_EXE" ]
  then
    echo "*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****"
    echo "The eCMD executable '$ECMD_EXE' does NOT exist or is not executable"
    echo "Please modify your ECMD_EXE variable to point to a valid eCMD executable before running"
    echo "*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****ERROR*****"
    exit 1
fi

# ***************************************************************************
# put all of the arguments from the command line together and run it
# ***************************************************************************
$ECMD_EXE $cmd "$@"
