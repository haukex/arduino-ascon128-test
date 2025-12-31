#!/bin/bash
set -euxo pipefail

# This script requires `arduino-cli`, `git`, and `zip`

# Crypto by Rhys Weatherley 0.4.0
arduino-cli lib install Crypto@0.4.0

# CryptoLW by Rhys Weatherley 0.2.0 is unfortunately not in
# the Arduino library collection, so install it manually:

temp_dir="$( mktemp --directory )"
trap 'set +ex; popd >/dev/null; rm -rf "$temp_dir"' EXIT
pushd "$temp_dir" >/dev/null

# efficiently fetch only the commit we want from the repo
git init
git config --worktree advice.detachedHead false
git remote add origin https://github.com/rweather/arduinolibs.git
git fetch --depth=1 origin 37a76b8f7516568e1c575b6dc9268da1ccaac6b6
git checkout FETCH_HEAD

# zip up the folder so arduino-cli can install it
cd libraries
zip -qr CryptoLW.zip CryptoLW

# temporarily switch on "unsafe installs" to be able to install from ZIP
prev_unsafe="$( arduino-cli config get library.enable_unsafe_install )"
arduino-cli config set library.enable_unsafe_install true

arduino-cli lib install --zip-path CryptoLW.zip

arduino-cli config set library.enable_unsafe_install "$prev_unsafe"

# spell: ignore Rhys Weatherley worktree