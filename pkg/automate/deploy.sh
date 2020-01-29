#!/bin/bash
source pkg/automate/common.sh

set -ex

# Get the repo
git clone ssh://aur@aur.archlinux.org/${AUR_NAME}.git aur

# Remove old package version
rm aur/*.tar.gz

# Update it
cp "${TRAVIS_BUILD_DIR}/pkg/PKGBUILD" aur
cp "tvolnoti-${TRAVIS_TAG:1}.tar.gz" aur
cd aur

create_src ".SRCINFO"

# Commit
git add -A
git config user.email "deploy@mihalea.ro"
git config user.name "mihalea-deploy"
git commit -m "Release $TRAVIS_TAG"

# Deploy to AUR
git push origin master
