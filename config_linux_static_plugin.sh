#!/bin/bash

make distclean

lrelease x2goclient.pro

export X2GO_LINUX_STATIC=x2go_linux_static
X2GO_CLIENT_TARGET=plugin /usr/local/Trolltech/Qt-4.8.5/bin/qmake -config release -spec linux-g++
