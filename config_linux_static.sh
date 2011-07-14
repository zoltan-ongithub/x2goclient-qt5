#!/bin/bash

make distclean
X2GO_LINUX_STATIC=x2go_linux_static /usr/local/Trolltech/Qt-4.7.1/bin/qmake -config release -spec linux-g++

