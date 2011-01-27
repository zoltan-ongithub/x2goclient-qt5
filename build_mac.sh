#!/bin/bash

make distclean

rm x2goclientconfig.pri
ln -s x2goclientconfig.pri.client x2goclientconfig.pri


cd plug_askpass
make distclean
cd ..

qmake -spec macx-g++
