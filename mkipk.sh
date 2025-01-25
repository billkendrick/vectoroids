#!/bin/sh

# mkipk.sh
# generates an ipkg for embedded Vectoroids

# Bill Kendrick
# bill@newbreedsoftware.com

# 2002.Apr.13 - 2002.Apr.20


VER=1.1.0


PACKAGE=vectoroids
TMPDIR=tmp
CONTROL=$TMPDIR/CONTROL/control
ARCH=arm
RM=rm

echo "SETTING UP"
mkdir $TMPDIR
mkdir $TMPDIR/CONTROL


echo
echo "MAKING SURE BINARY EXISTS"
make clean
make embedded

echo 
echo "CREATING CONTROL FILE"

echo "Package: $PACKAGE" > $CONTROL
echo "Priority: optional" >> $CONTROL
echo "Version: $VER" >> $CONTROL
echo "Section: games" >> $CONTROL
echo "Architecture: $ARCH" >> $CONTROL
echo "Maintainer: Bill Kendrick (bill@newbreedsoftware.com)" >> $CONTROL
echo "Description: An asteroids game" >> $CONTROL

echo
echo "COPYING DATA FILES"

mkdir -p $TMPDIR/opt/QtPalmtop/share/vectoroids
cp -R data/* $TMPDIR/opt/QtPalmtop/share/vectoroids

echo
echo "CREATING BINARIES"

mkdir -p $TMPDIR/opt/QtPalmtop/bin/
echo "vectoroids" > $TMPDIR/opt/QtPalmtop/bin/vectoroids.sh
cp vectoroids $TMPDIR/opt/QtPalmtop/bin/


echo "CREATING ICON AND DESKTOP FILE"

mkdir -p $TMPDIR/opt/QtPalmtop/pics/
cp data/images/icon.png $TMPDIR/opt/QtPalmtop/pics/vectoroids.png

mkdir -p $TMPDIR/opt/QtPalmtop/apps/Games/
DESKTOP=$TMPDIR/opt/QtPalmtop/apps/Games/vectoroids.desktop
echo "[Desktop Entry]" > $DESKTOP
echo "Comment=Asteroids game" >> $DESKTOP
echo "Exec=vectoroids.sh" >> $DESKTOP
echo "Icon=vectoroids" >> $DESKTOP
echo "Type=Application" >> $DESKTOP
echo "Name=Vectoroids" >> $DESKTOP


echo
echo "CREATING IPK..."

ipkg-build $TMPDIR

echo
echo "CLEANING UP"

$RM -r $TMPDIR

echo

