#!/bin/sh

function create_src {
    TARGET="$1"

    # Change pkgver to current version
    sed -i -r "s/(@VERSION@)/${VERSION}/g" PKGBUILD
    sed -i -r "s/(@MD%@)/${MD5}/g" PKGBUILD

    # Create .SRCINFO
    /bin/bash "$MAKEPKG_DIR/makepkg" --config="${MAKEPKG_CONF}" --printsrcinfo > ${TARGET}
}