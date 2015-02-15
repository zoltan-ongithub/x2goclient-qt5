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

: SDK="${SDK:-"/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk"}"
: MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-"10.7"}"
: DEBUG="${DEBUG:-"0"}"

case "${DEBUG}" in
	("0"|"no"|""|"No"|"nO"|"NO") BUILD_MODE="release";;
	(*) BUILD_MODE="debug";;
esac

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
rm -rf "${APPBUNDLE}"
rm -rf "${DMGFILE}"

[ -e "${BUILD_DIR}" ] && rm -rf "${BUILD_DIR}"

mkdir "${BUILD_DIR}"
pushd "${BUILD_DIR}"

phase "Running lrelease"
lrelease "${PROJECT}"

phase "Running qmake"
qmake -config "${BUILD_MODE}" -spec macx-g++ "${PROJECT}" \
	CONFIG+="x86_64" \
	QMAKE_MAC_SDK="${SDK}" \
	QMAKE_MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET}"

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
	--copy "${TOP_DIR}/res/img/png/macinstaller_background.png:/.background" \
	--copy "${TOP_DIR}/res/osxbundle/macdmg.DS_Store:/.DS_Store" \
	--copy "${TOP_DIR}/LICENSE" \
	--copy "${TOP_DIR}/COPYING" \
	--symlink "/Applications: " \
	--icon "${TOP_DIR}/res/img/icons/x2go-mac.icns" \
	--format "UDBZ"

popd
