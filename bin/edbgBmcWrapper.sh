#!/bin/sh

# ***************************************************************************
# help files will be searched under $EDBG_HOME/help/
# ***************************************************************************
export EDBG_HOME=/usr/
PLAT_DTB_DIR=/etc/pdata

#Set default environment value to none
EDBG_DTB=none

for entry in "$PLAT_DTB_DIR"/*
do
  if [ -f "$entry" ];then
     if [[ "$entry" == *".dtb"* ]]; then
        EDBG_DTB=${entry}
     else
        echo "No DTB files found in the path"
        exit 1
     fi
  fi
done
export EDBG_DTB

# ***************************************************************************
# Execute the edbg using relative path
# ***************************************************************************
exec /usr/bin/edbg ${0##*/} "$@"
