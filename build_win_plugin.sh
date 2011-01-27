#!/bin/bash

make distclean

rm x2goclientconfig.pri
ln -s x2goclientconfig.pri.plugin x2goclientconfig.pri
/usr/local/Trolltech/Qt-4.5.3/bin/qmake -config release -spec win32-x-g++

cd plug_askpass
make distclean
/usr/local/Trolltech/Qt-4.5.3/bin/qmake -config release -spec win32-x-g++

cd ..

