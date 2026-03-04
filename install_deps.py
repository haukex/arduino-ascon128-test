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
from pathlib import Path
import contextlib
import json

# spell: ignore Rhys Weatherley worktree

# Crypto by Rhys Weatherley 0.4.0
run(['arduino-cli', 'lib', 'install', 'Crypto@0.4.0'], check=True)

rv = run(['arduino-cli', 'lib', 'list', '--json'],
         capture_output=True, check=True)
if rv.stderr:
    raise RuntimeError(repr(rv.stderr))
lib_clw = [li for li in json.loads(rv.stdout)['installed_libraries']
           if li['library']['name'] == 'CryptoLW']
lib_c = [li for li in json.loads(rv.stdout)['installed_libraries']
         if li['library']['name'] == 'Crypto']
if len(lib_c) != 1:
    raise RuntimeError(repr(lib_c))

if len(lib_clw):
    if len(lib_clw) > 2:
        raise RuntimeError(repr(lib_clw))
    print(f"It appears CryptoLW {lib_clw[0]['library']['version']} is already"
          f" installed at {lib_clw[0]['library']['install_dir']}")
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
        # The ESP8266 core also includes a "Crypto.h". To make sure that the
        # correct Crypto.h is included in "CryptoLW.h", copy it over.
        chs = Path(lib_c[0]['library']['install_dir'])/'src'/'Crypto.h'  # type: ignore[assignment]  # noqa: E501  # pylint: disable=line-too-long
        if not chs.is_file():
            raise FileNotFoundError(chs)
        chd = Path('libraries', 'CryptoLW', 'src')
        if not chd.is_dir():
            raise NotADirectoryError(chd)
        cht = chd/chs.name
        if cht.exists():
            raise FileExistsError(cht)
        chs.copy(cht)
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
