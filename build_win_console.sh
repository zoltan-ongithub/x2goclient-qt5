#!/bin/bash


make distclean

rm x2goclientconfig.pri
ln -s x2goclientconfig.pri.client x2goclientconfig.pri




/usr/local/Trolltech/Qt-4.5.3/bin/qmake -spec win32-x-g++-console
