#!/usr/bin/env python3
"""Writes the pages of the Sphinx site into sphinx/generated from the files
of the repository, which stay the one source of the text: the head of
README.md becomes the home page and each of its sections a page, NEWS the
page of the release notes, and the relative links of the Markdown point to
the files on GitHub; then apiref.py writes the API reference into
sphinx/api from the XML of Doxygen. sphinx/conf.py runs it before Sphinx
reads the pages.
"""

import os
import re
import shutil

import apiref

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'sphinx', 'generated')
GITHUB = 'https://github.com/dmeoli/abLEMON/'


def slug(title):
    return re.sub(r'[^a-z0-9]+', '-', title.lower()).strip('-')


def links(text):
    """the relative links of the Markdown made absolute on GitHub"""
    def fix(m):
        target = m.group(2)
        if re.match(r'[a-z]+:', target) or target.startswith('#'):
            return m.group(0)
        kind = 'tree' if target.endswith('/') else 'blob'
        return '%s(%s%s/master/%s)' % (m.group(1), GITHUB, kind, target)
    return re.sub(r'(\])\(([^)\s]+)\)', fix, text)


def read(name):
    with open(os.path.join(ROOT, name), encoding='utf-8') as f:
        return f.read()


def write(page, text):
    with open(os.path.join(OUT, page), 'w', encoding='utf-8') as f:
        f.write(text)


def main(xml):
    """the pages, and those of the API reference from the XML in xml"""
    if os.path.isdir(OUT):
        shutil.rmtree(OUT)
    os.makedirs(OUT)

    # the badges of the repository stay on GitHub
    readme = re.sub(r'^\[!\[.*\n', '', read('README.md'), flags=re.M)
    parts = re.split(r'^## ', links(readme), flags=re.M)
    write('home.md', parts[0])
    pages = []
    for part in parts[1:]:
        title, _, body = part.partition('\n')
        pages.append(slug(title))
        write(pages[-1] + '.md', '# %s\n%s' % (title, body))
    # the table of contents of those pages, which index.rst includes
    write('readme.inc', '.. toctree::\n   :maxdepth: 1\n'
          '   :caption: Introduction\n\n'
          + ''.join('   generated/%s\n' % p for p in pages))

    write('news.md', '# Release notes\n\n```text\n%s```\n' % read('NEWS'))

    apiref.main(xml)


if __name__ == '__main__':
    main(os.path.join(ROOT, 'build', 'doc', 'xml'))
