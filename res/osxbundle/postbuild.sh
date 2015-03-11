#!/bin/bash

set -ex

TARGET="${1:?"no TARGET name given"}"
VERSION="${2:?"no version given"}"
INFO_PLIST_OUT_PATH="${3:?"no output directory given"}"
COPY_CMD="${4:?"no copy command given"}"
ICON_PATH="${5:?"no icon path given"}"
ICON_OUT_DIR="${6:?"no destination icon path given"}"

# Do NOT change this to gnused. The inplace options for bsdsed and gnused differ.
SED="/usr/bin/sed"

# Substitute FULL_VERSION in Info.plist file.
${SED} -i '' -e "s/@FULL_VERSION@/${VERSION}/g" -e "s/@TARGET@/${TARGET}/g" "${INFO_PLIST_OUT_PATH}"

# Copy icon.
${COPY_CMD} "${ICON_PATH}" "${ICON_OUT_DIR}"

exit 0
