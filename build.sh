#!/bin/sh

KERNEL=`uname -s`
LIBC=gnu          # no idea how to detect this
ARCH=`uname -m`
QT=`pkg-config --modversion Qt5Core | cut -d. -f1-2`

BINDING_PLATFORM=`echo $KERNEL-$LIBC-$ARCH-qt$QT | tr '[A-Z]' '[a-z]'`
echo Compiling qt5.cr for $BINDING_PLATFORM ...
env BINDING_PLATFORM=$BINDING_PLATFORM lib/bindgen/tool.sh qt.yml
