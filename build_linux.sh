#!/bin/bash

make distclean
qmake-qt4
make -j4
