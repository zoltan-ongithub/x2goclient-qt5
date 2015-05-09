rmdir /s /q client_build
rmdir /s /q plugin_build

mingw32-make clean

mkdir client_build

cd client_build

lrelease ..\x2goclient.pro

set X2GO_CLIENT_TARGET=

set config=release
if [%1]==[debug] set config=debug console
qmake ..\x2goclient.pro CONFIG+="%config%"

cd ..
cd x2gohelper

mingw32-make clean

cd ..
