#!/bin/sh

export EDBG_DTB=/etc/pdata/rainier.dtb

# ***************************************************************************
# Execute the edbg using relative path
# ***************************************************************************
exec /usr/bin/edbg ${0##*/} "$@"
