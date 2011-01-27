#!/bin/bash

make distclean

rm x2goclientconfig.pri
ln -s x2goclientconfig.pri.plugin x2goclientconfig.pri
/usr/local/Trolltech/Qt-4.6.2/bin/qmake -config release -spec linux-g++


#cd plug_askpass
#make distclean
#/usr/local/Trolltech/Qt-4.6.2/bin/qmake -config release -spec linux-g++
#cd ..

