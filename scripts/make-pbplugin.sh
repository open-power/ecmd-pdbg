#!/bin/sh

# Do an arg check to make sure 1 arg was given
if [ "$#" != "1" ]
then
  echo "ERROR: This command only takes one argument (path to edbg install)!"
  echo "ERROR: Please review your command line entry and try again!"
  exit 1
fi

# Create a temp dir where the plugin will get packaged
pbdir=`mktemp -d`
echo Working in $pbdir
echo

# Create the directories we need
echo Creating our dirs
mkdir $pbdir/bin
mkdir -p $pbdir/edbg/bin
mkdir $pbdir/lib
mkdir -p $pbdir/usr/bin
mkdir -p $pbdir/usr/lib
ln -s -r $pbdir/lib $pbdir/lib64
mkdir -p $pbdir/edbg/bin
mkdir -p $pbdir/etc/preboot-plugins

# Load in the /bin
echo Loading in /bin
cp /bin/bash $pbdir/bin/.
cp /bin/cat $pbdir/bin/.
cp /bin/ls $pbdir/bin/.
cp /bin/readlink $pbdir/bin/.
ln -s -r $pbdir/bin/bash $pbdir/bin/sh

# Load in /usr/bin
echo Loading in /usr/bin
cp /usr/bin/dirname $pbdir/usr/bin/.
cp /usr/bin/id $pbdir/usr/bin/.

# Load in /lib
echo Loading in /lib
cp -P /lib/powerpc64le-linux-gnu/libtinfo.so.5* $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libdl* $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libc.so.6 $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libc-2.23.so $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/ld* $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libselinux.so.1* $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libpcre.so.3* $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libpthread* $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libz.so.1* $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libm.so.6* $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libm-2.23.so* $pbdir/lib/.
cp -P /usr/lib/powerpc64le-linux-gnu/libstdc++.so.6* $pbdir/lib/.
cp -P /lib/powerpc64le-linux-gnu/libgcc_s.so.1 $pbdir/lib/.

# Load in /usr/lib
# The files placed in /usr/lib are what you need if trying
# to use the tool outside of the chroot
echo Loading in /usr/lib
cp -P /usr/lib/powerpc64le-linux-gnu/libxml2.so* $pbdir/usr/lib/.
cp -P /usr/lib/powerpc64le-linux-gnu/libicuuc.so* $pbdir/usr/lib/.
cp -P /usr/lib/powerpc64le-linux-gnu/libicudata.so* $pbdir/usr/lib/.
cp -P /lib/powerpc64le-linux-gnu/liblzma.so.5* $pbdir/usr/lib/.

# Load in edbg stuff
echo Loading in edbg
cp $1/bin/edbg $pbdir/edbg/bin/.
cp $1/bin/edbgdetcnfg $pbdir/edbg/bin/.
cp $1/bin/updatemodel $pbdir/edbg/bin/.
cp $1/bin/updateserial $pbdir/edbg/bin/.

# Easier to copy everything and then remove the one thing
# we don't need
cp -P $1/lib/* $pbdir/usr/lib/.
rm $pbdir/usr/lib/edbg.dll

# Load in the info about our plugin
cp pb-plugin.conf $pbdir/etc/preboot-plugins/.

echo
echo Complete!  Now run \'pb-plugin create $pbdir\' to finish creation

# Cleanup at the end
#rm -rf $pbdir
