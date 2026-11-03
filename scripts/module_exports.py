#!/usr/bin/env python3
"""Writes lemon/lemon.cppm, the interface of the C++20 module lemon: its
global module fragment includes the headers of lemon/, and it exports, with
a using-declaration, every name that they declare in the namespace lemon
and in its nested ones (but those whose name begins with an underscore,
which are the internals of the headers). The names are read from the AST
that clang dumps of the headers:

    scripts/module_exports.py [--clang clang++] [--check] -- -Ibuild

where the flags after -- are those that find lemon/config.h, i.e., the
binary directory of a build. The constants of namespace scope (e.g., PI of
lemon/math.h), which have internal linkage, cannot be exported: a unit that
uses them includes their header. The headers of the LP and MIP backends (lp.h,
glpk.h, clp.h, cbc.h, cplex.h, soplex.h), whose classes are in the library
only when it is built with their solvers, are left out, and so are the
macros, which a module does not export. To be run whenever a header gains
or loses a name; with --check the interface is not written, the exit status
is 1 if it would change.
"""

import argparse
import collections
import json
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'lemon', 'lemon.cppm')
LEFT_OUT = {'lp.h', 'glpk.h', 'clp.h', 'cbc.h', 'cplex.h', 'soplex.h',
            'config.h'}
# the declarations that a using-declaration can export; the others (e.g.,
# the definitions of members outside their class) come with their class
EXPORTED = {'CXXRecordDecl', 'ClassTemplateDecl', 'EnumDecl',
            'FunctionDecl', 'FunctionTemplateDecl', 'TypedefDecl',
            'TypeAliasDecl', 'TypeAliasTemplateDecl', 'VarDecl',
            'VarTemplateDecl'}


def headers():
    """The public headers of lemon/, as lemon/<name>.h."""
    return sorted('lemon/' + f for f in os.listdir(os.path.join(ROOT, 'lemon'))
                  if f.endswith('.h') and f not in LEFT_OUT)


def dump(clang, flags):
    """The JSON blocks of the namespace lemon in a translation unit that
    includes all the headers; the AST of the whole unit, the standard
    library included, would be far larger."""
    with tempfile.TemporaryDirectory() as tmp:
        tu = os.path.join(tmp, 'tu.cpp')
        with open(tu, 'w') as f:
            f.writelines('#include <%s>\n' % h for h in headers())
        out = subprocess.run([clang, '-std=c++20', '-fsyntax-only',
                              '-I' + ROOT, *flags, '-Xclang',
                              '-ast-dump=json', '-Xclang',
                              '-ast-dump-filter=lemon', tu],
                             capture_output=True)
    if out.returncode:
        sys.exit('module_exports.py: the headers do not compile:\n' +
                 out.stderr.decode()[:3000])
    text, dec, pos, blocks = out.stdout.decode(), json.JSONDecoder(), 0, []
    while True:
        while pos < len(text) and text[pos].isspace():
            pos += 1
        if pos == len(text):
            return blocks
        block, pos = dec.raw_decode(text, pos)
        blocks.append(block)


def collect(blocks):
    """{ namespace : set of names } of the names that the headers of lemon/
    declare at namespace scope, but the static ones, and the classes only
    declared in advance."""
    names = collections.defaultdict(set)
    declared, defined = set(), set()
    where = {'file': ''}
    prefix = os.path.join(ROOT, 'lemon') + os.sep

    def file_of(node):
        for key in ('loc', 'range'):
            loc = node.get(key, {})
            if key == 'range':
                loc = loc.get('begin', {})
            loc = loc.get('expansionLoc', loc)
            if 'file' in loc:
                where['file'] = os.path.realpath(loc['file'])
                return

    def walk(nodes, ns):
        for node in nodes:
            file_of(node)
            kind, name = node.get('kind'), node.get('name', '')
            if kind == 'NamespaceDecl':
                if name and not name.startswith('_'):
                    walk(node.get('inner', []), ns + [name])
                continue
            if (not ns or ns[0] != 'lemon' or kind not in EXPORTED or
                    node.get('isImplicit') or not name or
                    not where['file'].startswith(prefix)):
                continue
            decl = node
            if kind == 'FunctionTemplateDecl':
                decl = next((n for n in node.get('inner', [])
                             if n.get('kind') == 'FunctionDecl'), {})
            # static, a definition of a member outside its class, or a
            # constant, which C++ gives internal linkage unless extern
            if (decl.get('storageClass') == 'static' or
                    'parentDeclContextId' in node or
                    (kind == 'VarDecl' and
                     node.get('type', {}).get('qualType', '')
                     .startswith('const ') and
                     node.get('storageClass') != 'extern' and
                     not node.get('inline'))):
                continue
            key = '::'.join(ns)
            record = node
            if kind == 'ClassTemplateDecl':
                record = next((n for n in node.get('inner', [])
                               if n.get('kind') == 'CXXRecordDecl'), {})
            if kind in ('CXXRecordDecl', 'ClassTemplateDecl'):
                declared.add((key, name))
                if record.get('completeDefinition'):
                    defined.add((key, name))
            names[key].add(name)
            # the enumerators of an unscoped enum live in the namespace
            if kind == 'EnumDecl' and not node.get('scopedEnumTag'):
                names[key].update(e['name'] for e in node.get('inner', [])
                                  if e.get('kind') == 'EnumConstantDecl')

    for block in blocks:
        walk([block], [])
    for key, name in declared - defined:
        names[key].discard(name)
    return {k: v for k, v in names.items() if v}


def interface(names):
    out = ['// The interface of the C++20 module lemon, written by',
           '// scripts/module_exports.py, not by hand: a translation unit',
           '// can write "import lemon;" in place of the #include of the',
           '// headers, but those of the LP and MIP backends and those whose',
           '// macros it uses (e.g., DIGRAPH_TYPEDEFS of lemon/core.h).',
           '', 'module;', '']
    out += ['#include <%s>' % h for h in headers()]
    out += ['', 'export module lemon;', '']

    def emit(key, depth):
        ind = ' ' * depth
        inner = sorted(k for k in names if k.startswith(key + '::') and
                       k.count('::') == key.count('::') + 1)
        for n in sorted(names.get(key, ())):
            out.append('%susing ::%s::%s;' % (ind, key, n))
        for k in inner:
            out.append('%snamespace %s {' % (ind, k.rsplit('::', 1)[1]))
            emit(k, depth + 1)
            out.append('%s}' % ind)

    out.append('export namespace lemon {')
    emit('lemon', 1)
    out.append('}')
    return '\n'.join(out) + '\n'


def main():
    parser = argparse.ArgumentParser(
        description=__doc__.split('\n\n')[0].replace('\n', ' '))
    parser.add_argument('--clang', default='clang++')
    parser.add_argument('--check', action='store_true')
    parser.add_argument('flags', nargs='*')
    args = parser.parse_args()
    text = interface(collect(dump(args.clang, args.flags)))
    old = open(OUT).read() if os.path.exists(OUT) else ''
    if args.check:
        if text != old:
            print('module_exports.py: lemon/lemon.cppm is not up to date')
            return 1
        return 0
    with open(OUT, 'w') as f:
        f.write(text)
    return 0


if __name__ == '__main__':
    sys.exit(main())
