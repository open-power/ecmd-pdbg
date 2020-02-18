#!/bin/sh

# ***************************************************************************
# help files will be searched under $EDBG_HOME/help/
# ***************************************************************************
export EDBG_HOME=/usr/
PLAT_DTB_DIR=/etc/pdata

#Set default environment value to none
EDBG_DTB=none

if [ -n "$PDBG_DTB" ] ; then
	EDBG_DTB="$PDBG_DTB"
else
	for entry in "$PLAT_DTB_DIR"/*.dtb ; do
		if [ -f "$entry" ] ; then
			EDBG_DTB="$entry"
		else
			echo "No DTB files found in the path"
			exit 1
		fi
	done
fi
export EDBG_DTB

# ***************************************************************************
# Execute the edbg using relative path
# ***************************************************************************
exec /usr/bin/edbg ${0##*/} "$@"
