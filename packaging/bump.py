#!/usr/bin/env python3
"""Brings the recipes of packaging/ to a released version of lemon-ng: the
version, the checksums of the archive of its tag on GitHub (SHA-256 for
conda, Homebrew and Conan, SHA-512 for vcpkg) and the build number, which
starts again from 0.

    packaging/bump.py VERSION [--archive FILE]

The archive is downloaded, unless it is given; a recipe that does not
change is an error, since every one of them names the version.
"""

import argparse
import hashlib
import os
import re
import sys
import urllib.request

URL = 'https://github.com/dmeoli/lemon-ng/archive/%s.tar.gz'
HERE = os.path.dirname(os.path.abspath(__file__))


def sub(path, changes):
    """Applies to the file the pairs (pattern, replacement), each of which
    must match."""
    full = os.path.join(HERE, path)
    with open(full) as f:
        text = f.read()
    for pattern, repl in changes:
        text, n = re.subn(pattern, repl, text, flags=re.M)
        if n == 0:
            sys.exit('bump.py: %s: no match for %s' % (path, pattern))
    with open(full, 'w') as f:
        f.write(text)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    parser.add_argument('version', help='the version, e.g., 1.4.2')
    parser.add_argument('--archive', help='the archive of the tag, '
                        'instead of downloading it')
    args = parser.parse_args()
    v = args.version
    if not re.fullmatch(r'\d+\.\d+\.\d+', v):
        sys.exit('bump.py: %s is not a version X.Y.Z' % v)
    if args.archive:
        with open(args.archive, 'rb') as f:
            data = f.read()
    else:
        with urllib.request.urlopen(URL % v) as r:
            data = r.read()
    sha256 = hashlib.sha256(data).hexdigest()
    sha512 = hashlib.sha512(data).hexdigest()

    sub('conda/recipe.yaml',
        [(r'^(  version: )"[^"]*"', r'\g<1>"%s"' % v),
         (r'^(  sha256: )[0-9a-f]{64}', r'\g<1>' + sha256),
         (r'^(  number: )\d+', r'\g<1>0')])
    sub('homebrew/lemon-ng.rb',
        [(r'(archive/refs/tags/)[^/"]*(\.tar\.gz")', r'\g<1>%s\2' % v),
         (r'^(  sha256 ")[0-9a-f]{64}', r'\g<1>' + sha256)])
    sub('conan/config.yml', [(r'^  "[^"]*":', '  "%s":' % v)])
    sub('conan/all/conandata.yml',
        [(r'^  "[^"]*":', '  "%s":' % v),
         (r'(archive/)[^/"]*(\.tar\.gz")', r'\g<1>%s\2' % v),
         (r'^(    sha256: ")[0-9a-f]{64}', r'\g<1>' + sha256)])
    sub('vcpkg/lemon-ng/vcpkg.json',
        [(r'^(  "version": )"[^"]*"', r'\g<1>"%s"' % v)])
    full = os.path.join(HERE, 'vcpkg/lemon-ng/vcpkg.json')
    with open(full) as f:
        text = f.read()
    with open(full, 'w') as f:
        f.write(re.sub(r'^  "port-version": \d+,\n', '', text, flags=re.M))
    sub('vcpkg/lemon-ng/portfile.cmake',
        [(r'^(    SHA512 )[0-9a-f]{128}', r'\g<1>' + sha512)])
    print('%s: sha256 %s' % (v, sha256))
    print('%s: sha512 %s' % (v, sha512))
    return 0


if __name__ == '__main__':
    sys.exit(main())
