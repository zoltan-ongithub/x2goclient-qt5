#!/bin/bash

make distclean

lrelease x2goclient.pro

X2GO_CLIENT_TARGET=plugin qmake-qt4
