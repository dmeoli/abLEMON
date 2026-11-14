#!/usr/bin/env python3
"""Checks of the abLEMON manual, run by `make check` with the root of the
repository as argument:

  1. every link to a file of the manual and an anchor resolves (the anchors
     are the {#id} of the headings and the <span id=...> of the items);
  2. every --8<-- line names a file that exists;
  3. every work that the headers and doc/*.dox cite with \\cite is in the
     bibliography, as the anchor ref-<key> (the key of doc/references.bib);
  4. every public header of lemon/ and every class or struct it declares at
     namespace scope is named somewhere in the chapters.

It prints one line per problem and exits nonzero if there is any.
"""
import os
import re
import sys

root = sys.argv[1] if len(sys.argv) > 1 else '../..'
man = os.path.join(root, 'manual')
chapdir = os.path.join(man, 'chapters')
files = {'index.md': os.path.join(man, 'index.md')}
for f in sorted(os.listdir(chapdir)):
    if f.endswith('.md'):
        files[f] = os.path.join(chapdir, f)
text = {f: open(p, encoding='utf-8').read() for f, p in files.items()}
bad = []

anchors = {}
for f, t in text.items():
    ids = set(re.findall(r'\{#([\w-]+)[^}]*\}', t))
    ids |= set(re.findall(r'<span id="([\w-]+)"', t))
    anchors[f] = ids

for f, t in text.items():
    for target in re.findall(r'\]\(([\w./-]+\.md)(?:#([\w-]+))?\)', t):
        fn, a = target
        fn = os.path.basename(fn)
        if fn not in anchors:
            bad.append(f'{f}: link to missing file {fn}')
        elif a and a not in anchors[fn]:
            bad.append(f'{f}: link to missing anchor {fn}#{a}')
    for p in re.findall(r'--8<--\s+"([^"]+)"', t):
        if not os.path.exists(os.path.join(man, p)):
            bad.append(f'{f}: --8<-- of missing file {p}')

bib = text.get('Z-bibliography.md', '')
have = set(re.findall(r'<span id="ref-([\w:-]+)"', bib))
keys = set()
for d in ('lemon', 'doc'):
    for dp, _, fs in os.walk(os.path.join(root, d)):
        for fn in fs:
            if fn.endswith(('.h', '.dox')):
                src = open(os.path.join(dp, fn), encoding='latin-1').read()
                keys |= set(re.findall(r'\\cite\s+(\w[\w:-]*\w)', src))
for k in sorted(keys):
    if k.strip() not in have:
        bad.append(f'bibliography: no entry ref-{k.strip()}')

alltext = '\n'.join(t for f, t in text.items())
skip = {'config.h', 'lemon.cppm'}
for h in sorted(os.listdir(os.path.join(root, 'lemon'))):
    if not h.endswith('.h') or h in skip:
        continue
    if h not in alltext:
        bad.append(f'coverage: header lemon/{h} not named')
    src = open(os.path.join(root, 'lemon', h), encoding='latin-1').read()
    src = re.sub(r'/\*.*?\*/', '', src, flags=re.S)
    src = re.sub(r'//[^\n]*', '', src)
    # the classes declared directly in a public namespace, i.e., not inside
    # a class nor in a namespace whose name starts with an underscore or is
    # bits
    stack, names = [], set()
    tok = re.compile(r'namespace\s+(\w*)\s*\{|'
                     r'(?:class|struct)\s+([A-Z]\w*)\s*(?:final\s*)?[:{]|'
                     r'[{}]')
    for t in tok.finditer(src):
        g = t.group(0)
        if g.startswith('namespace'):
            inner = t.group(1).startswith('_') or t.group(1) == 'bits'
            stack.append('_' if inner else 'ns')
        elif g == '{':
            stack.append('x')
        elif g == '}':
            if stack:
                stack.pop()
        else:
            if all(k == 'ns' for k in stack):
                names.add(t.group(2))
            if g.endswith('{'):
                stack.append('x')
    for n in sorted(names):
        if n.endswith(('Traits', 'Base')) or n.startswith('_'):
            continue
        if not re.search(r'\b' + n + r'\b', alltext):
            bad.append(f'coverage: {n} of lemon/{h} not named')

for b in bad:
    print(b)
sys.exit(1 if bad else 0)
