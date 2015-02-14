#!/bin/sh
NAME="x2goclient"
TOP_DIR="${PWD}"
BUILD_DIR="${TOP_DIR}/build_client"
APPBUNDLE="${BUILD_DIR}/${NAME}.app"
DMGFILE="${BUILD_DIR}/${NAME}.dmg"
PROJECT="${TOP_DIR}/${NAME}.pro"
PKG_DMG="${TOP_DIR}/pkg-dmg"

NXPROXY="$(which nxproxy)"
LIBXCOMP="libXcomp.3.dylib"
LIBPNG="libpng15.15.dylib"
LIBJPEG="libjpeg.9.dylib"
LIBZ="libz.1.dylib"

set -e

function phase() {
	echo
	echo "***"
	echo "*** ${1}…"
	echo "***"
	echo
}

phase "Cleaning"
make clean
rm -rf "$APPBUNDLE"
rm -rf "$DMGFILE"

phase "Running lrelease"
lrelease "$PROJECT"

phase "Running qmake"
qmake -config release \
	CONFIG+=x86_64 \
	QMAKE_MAC_SDK=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk \
	QMAKE_MACOSX_DEPLOYMENT_TARGET=10.7

phase "Running make"
make -j2

phase "Copying nxproxy"
mkdir -p "${APPBUNDLE}/Contents/exe"
cp "${NXPROXY}" "${APPBUNDLE}/Contents/exe"
dylibbundler \
	--fix-file "${APPBUNDLE}/Contents/exe/nxproxy" \
	--bundle-deps \
	--dest-dir "${APPBUNDLE}/Contents/Frameworks" \
	--install-path "@executable_path/../Frameworks/" \
	--create-dir

phase "Bundling up using macdeployqt"
macdeployqt "${APPBUNDLE}" -verbose=2

phase "Creating DMG"
${PKG_DMG} \
	--source "${APPBUNDLE}" \
	--sourcefile \
	--target "${DMGFILE}" \
	--volname "x2goclient" \
	--verbosity 2 \
	--mkdir "/.background" \
	--copy "./res/img/png/macinstaller_background.png:/.background" \
	--copy "./macdmg.DS_Store:/.DS_Store" \
	--copy "./LICENSE" \
	--copy "./COPYING" \
	--symlink "/Applications: " \
	--icon "./res/img/icons/x2go-mac.icns" \
	--format "UDBZ"
