#!/usr/bin/env python
"""Script to install CryptoLW from Git repo

CryptoLW by Rhys Weatherley 0.2.0 is unfortunately not in
the Arduino library collection, so this script installs it
manually from the Git repository (it's in a subdirectory).

Requires ``arduino-cli`` and ``git``.
"""
from tempfile import TemporaryDirectory
from shutil import make_archive
from subprocess import run
import contextlib
import json

# spell: ignore Rhys Weatherley worktree

# Crypto by Rhys Weatherley 0.4.0
run(['arduino-cli', 'lib', 'install', 'Crypto@0.4.0'], check=True)

rv = run(['arduino-cli', 'lib', 'list', '--json'],
         capture_output=True, check=True)
if rv.stderr:
    raise RuntimeError(repr(rv.stderr))
libs = [li for li in json.loads(rv.stdout)['installed_libraries']
        if li['library']['name'] == 'CryptoLW']
if len(libs) > 2:
    raise RuntimeError(repr(libs))

if len(libs):
    print(f"It appears CryptoLW {libs[0]['library']['version']} is already"
          f" installed at {libs[0]['library']['install_dir']}")
    print("You can try 'arduino-cli lib uninstall CryptoLW' to uninstall it")
else:
    with TemporaryDirectory() as td, contextlib.chdir(td):
        # efficiently fetch only the commit we want from the repo
        run(['git', 'init', '-q'], check=True)
        run(['git', 'config', '--worktree', 'advice.detachedHead',
             'false'], check=True)
        run(['git', 'remote', 'add', 'origin',
             'https://github.com/rweather/arduinolibs.git'], check=True)
        # 37a76b8 is the most recent commit from 2024-05-26, as of 2026-02
        run(['git', 'fetch', '-q', '--depth=1', 'origin',
             '37a76b8f7516568e1c575b6dc9268da1ccaac6b6'], check=True)
        run(['git', 'checkout', 'FETCH_HEAD'], check=True)
        # zip up the folder so arduino-cli can install it
        make_archive('CryptoLW', format='zip', root_dir='libraries',
                     base_dir='CryptoLW')
        # temporarily switch on "unsafe installs" to be able to install ZIP
        # see also `arduino-cli config dump --json`
        rv = run(['arduino-cli', 'config', 'get',
                  'library.enable_unsafe_install'],
                 capture_output=True, check=True)
        if rv.stderr:
            raise RuntimeError(repr(rv.stderr))
        prev_unsafe = rv.stdout.strip()
        run(['arduino-cli', 'config', 'set', 'library.enable_unsafe_install',
             'true'], check=True)
        run(['arduino-cli', 'lib', 'install', '--zip-path', 'CryptoLW.zip'],
            check=True)
        run(['arduino-cli', 'config', 'set', 'library.enable_unsafe_install',
             prev_unsafe], check=True)
