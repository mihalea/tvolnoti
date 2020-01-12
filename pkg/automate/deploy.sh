#!/bin/bash
source pkg/automate/common.sh

set -ex
cd "$TRAVIS_BUILD_DIR/pkg"

# Get the repo
git clone ssh://aur@aur.archlinux.org/${AUR_NAME}.git aur

# Update it
cp PKGBUILD aur
cd aur

create_src ".SRCINFO"

# Commit
git add PKGBUILD .SRCINFO
git config user.email "deploy@mihalea.ro"
git config user.name "mihalea-deploy"
git commit -m "Release $TRAVIS_TAG"

# Deploy to AUR
git push origin master
