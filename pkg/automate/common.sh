#!/bin/sh

# Exit immediately if a command exits with a non-zero status.
set -e

function create_src {
    local TARGET="$1"

    # Change pkgver to current version
    sed -i -r "s/(@VERSION@)/${VERSION}/g" PKGBUILD
    sed -i -r "s/(@MD%@)/${MD5}/g" PKGBUILD

    # Create .SRCINFO
    /bin/bash "$MAKEPKG_DIR/makepkg" --config="${MAKEPKG_CONF}" --printsrcinfo > ${TARGET}
}

function travis_fold() {
    local action="$1"
    local name="$2"
    local header="$3"
    echo -en "travis_fold:${action}:${name}\r"

    if [[ ! -z $header ]]; then
        echo "$header"
    fi
}