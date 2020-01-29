#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

key_name=$(cat pkg/encrypted)

# Decrypt private key
echo "Using key ${key_name}"
key="${key_name}_key"
iv="${key_name}_iv"
openssl aes-256-cbc -K ${!key} -iv ${!iv}  -in pkg/private_key.enc -out /tmp/private_key -d
chmod 600 /tmp/private_key
echo "Decrypted and permissioned the deployment key"

# Set up to run makepkg
wget https://www.archlinux.org/packages/core/x86_64/pacman/download/ -O pacman.pkg.tar.xz
tar -Jxf pacman.pkg.tar.xz
export MAKEPKG_DIR="$(pwd)/usr/bin"
export PATH="$MAKEPKG_DIR:$PATH"
export LIBRARY="$(pwd)/usr/share/makepkg"
export MAKEPKG_CONF="$(pwd)/etc/makepkg.conf"
echo "Installed makepkg"

# Package version 
LATEST_TAG=$(git describe --long | sed -rn 's/^(.*)-.*-.*$/\1/p')
VERSION=$(git describe --long | sed 's/\([^-]*-g\)/r\1/;s/-/./g')

export LATEST_TAG
export VERSION

echo "VERSION=${VERSION}"
echo "LATEST_TAG=${LATEST_TAG}"
echo "TRAVIS_TAG=${TRAVIS_TAG}"

# Set up git to use the private key and skip host checking
git config --global --add core.sshCommand "ssh -o StrictHostKeyChecking=false -i /tmp/private_key"

export AUR_NAME=tvolnoti

execute package.sh
MD5=$(md5sum tvolnoti-*.tar.gz)
export MD5

echo "MD5=${MD5}"