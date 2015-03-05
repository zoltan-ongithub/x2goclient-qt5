rmdir /s /q plugin_build

mkdir plugin_build

set X2GO_CLIENT_TARGET=plugin
qmake ..\x2goclient.pro

cd ..
