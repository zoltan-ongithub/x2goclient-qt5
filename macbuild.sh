#!/bin/bash
NAME="x2goclient"

TOP_DIR="$(dirname "$0")"
[[ "${TOP_DIR}" == /* ]] || TOP_DIR="${PWD}/${TOP_DIR#./}"
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

SDK="${SDK:-"/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk"}"
MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-"10.7"}"
DEBUG="${DEBUG:-"0"}"
BUNDLE="${BUNDLE:-"1"}"

case "${DEBUG}" in
	("0"|"no"|""|"No"|"nO"|"NO"|"false"|"FALSE") BUILD_MODE="release";;
	(*) BUILD_MODE="debug";;
esac

case "${BUNDLE}" in
	("0"|"no"|""|"No"|"nO"|"NO"|"false"|"FALSE") BUNDLE="0";;
	(*) BUNDLE="1";;
esac

SDK_MINOR_VERSION="$(/usr/bin/perl -pe 's#.*?10\.(\d+).*?\.sdk$#\1#' <<< "${SDK}")"

MATCH_NUMBERS='^[0-9]+$'
if [[ "${SDK_MINOR_VERSION}" =~ ${MATCH_NUMBERS} ]]; then
	STDLIB="libstdc++"
	[ "${SDK_MINOR_VERSION}" -gt "8" ] && STDLIB="libc++"
else
	echo "Unable to determine OS X version. Unknown value '${SDK_MINOR_VERSION}'." >&2
	exit 1
fi

set -e

function phase() {
	echo
	echo "***"
	echo "*** ${1}..."
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
	QMAKE_MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET}" \
	OSX_STDLIB="${STDLIB}"

phase "Running make"
make -j2

phase "Copying nxproxy"
mkdir -p "${APPBUNDLE}/Contents/exe"
cp "${NXPROXY}" "${APPBUNDLE}/Contents/exe"

if [ "${BUNDLE}" = "1" ]; then
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
fi

popd
