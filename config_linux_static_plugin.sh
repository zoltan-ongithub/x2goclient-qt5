#!/bin/bash

make distclean

X2GO_CLIENT_TARGET=plugin /usr/local/Trolltech/Qt-4.7.1/bin/qmake -config release -spec linux-g++
