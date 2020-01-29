#!/bin/bash
source pkg/automate/common.sh

# Check MAKEPKG_DIR
echo "MAKEPKG_DIR=$MAKEPKG_DIR"

# Get the package repo
git clone ssh://aur@aur.archlinux.org/${AUR_NAME}.git aur
cd aur

create_src test_src

echo "==== START .SRCINFO ===="
cat test_src
echo "====  END  .SRCINFO ===="