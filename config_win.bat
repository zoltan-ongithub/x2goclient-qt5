mingw32-make distclean
lrelease x2goclient.pro
set X2GO_CLIENT_TARGET=
qmake
cd x2gohelper
mingw32-make clean
cd ..
