#!/bin/bash

make distclean

rm x2goclientconfig.pri
ln -s x2goclientconfig.pri.client x2goclientconfig.pri


cd plug_askpass
make distclean
cd ..

/usr/local/Trolltech/Qt-4.6.2/bin/qmake -config release -spec linux-g++

