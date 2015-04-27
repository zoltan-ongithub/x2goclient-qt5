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

GIT_VER=""

if [ "$(git rev-parse --is-inside-git-dir)" = "true" ] || [ "$(git rev-parse --is-inside-work-tree)" = "true" ]; then
  GIT_SHA="$(git rev-parse --short=12 HEAD)"
  GIT_DESCRIPTION="$(git describe)"

  TMP_REGEX='^.+-[0-9]+-g[0-9A-Fa-f]+$'
  if [[ "${GIT_DESCRIPTION}" =~ ${TMP_REGEX} ]]; then
    GIT_VER=" git-${GIT_SHA} $(date +'%Y%m%d')"
  fi
fi

# Substitute FULL_VERSION in Info.plist file.
${SED} -i '' -e "s/@FULL_VERSION@/${VERSION}${GIT_VER}/g" -e "s/@TARGET@/${TARGET}/g" "${INFO_PLIST_OUT_PATH}"

# Copy icon.
${COPY_CMD} "${ICON_PATH}" "${ICON_OUT_DIR}"

exit 0
