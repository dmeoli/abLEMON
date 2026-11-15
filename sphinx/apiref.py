#!/usr/bin/env python3
"""Writes the API reference of the Sphinx site into sphinx/api from the XML
that Doxygen writes with the target html of CMake (into build/doc/xml), as
the tree of the groups of doc/groups.dox that the Topics of the Doxygen
site show: a page with the groups of the first level (the data structures,
the algorithms, the tools, ...), a page per group, which holds its
description, the tables of its subgroups and of its classes and the
functions and the other members of the group, and a page per class, all
rendered by Breathe. A class that is in no group (e.g., the traits and the
wizard of an algorithm) goes to the group of its header, else to the group
of the other classes of its header, else to a page of the classes in no
group; the specializations of a template go to the page of the template,
and the nested classes to the page of the class that holds them. The
internals of the headers (the classes of lemon/bits, of the namespaces bits
and of those whose name begins with an underscore) have no page. pages.py
calls it.
"""

import collections
import glob
import os
import re
import shutil
import xml.etree.ElementTree as ET

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'sphinx', 'api')
NAMESPACE = 'lemon::'
OTHERS = 'others'

# where the pages of the Doxygen site are, seen from a page of the
# reference, which lies in sphinx/api on a site served in the folder
# sphinx/ of the Doxygen one
DOXYGEN = '../../'


# ----- reading the XML ----------------------------------------------------- #

def text(node):
    """the text of a node of the XML, on one line"""
    if node is None:
        return ''
    return ' '.join(''.join(node.itertext()).split())


def description(cd):
    """the brief description of a compound, else the first sentence of the
    first paragraph of its detailed one"""
    brief = text(cd.find('briefdescription'))
    if brief:
        return brief
    for para in cd.findall('detaileddescription/para'):
        line = text(para)
        if line:
            m = re.search(r'(?<!\be\.g)(?<!\bi\.e)\.(\s|$)', line)
            return line[:m.start() + 1] if m else line
    return ''


class Compound:
    def __init__(self, cd):
        self.refid = cd.get('id')
        self.kind = cd.get('kind')
        self.name = cd.findtext('compoundname')
        self.title = cd.findtext('title') or self.name
        loc = cd.find('location')
        self.file = loc.get('file') if loc is not None else ''
        self.line = int(loc.get('line') or 0) if loc is not None else 0
        self.brief = description(cd)
        self.groups = [i.get('refid') for i in cd.findall('innergroup')]
        self.classes = [i.get('refid') for i in cd.findall('innerclass')]
        self.files = [i.get('refid') for i in cd.findall('innerfile')]
        self.members = [m.findtext('qualifiedname') or ''
                        for m in cd.findall('sectiondef/memberdef')]
        self.short = (self.name[len(NAMESPACE):]
                      if self.name.startswith(NAMESPACE) else self.name)


def read_xml(xml):
    compounds = {}
    for path in glob.glob(os.path.join(xml, '*.xml')):
        if os.path.basename(path) in ('index.xml', 'Doxyfile.xml'):
            continue
        for cd in ET.parse(path).getroot().findall('compounddef'):
            c = Compound(cd)
            compounds[c.refid] = c
    return compounds


# ----- the tree of the groups ---------------------------------------------- #

def is_class(c):
    return c.kind in ('class', 'struct', 'union')


def internal(c):
    """a class of the internals of the headers, or of no header of the
    library (the forward declarations of the solvers, the classes of the
    tools)"""
    return (not c.name.startswith(NAMESPACE)
            or not c.file.startswith('lemon/')
            or c.file.startswith('lemon/bits/')
            or any(w.startswith('_') or w == 'bits'
                   for w in c.name.split('::')))


def template(name):
    """the name of the template of a specialization, else None"""
    return name.split('<', 1)[0].rstrip() if '<' in name else None


def place(compounds):
    """the classes with a page, each with the group it goes to, and the
    specializations that go to the page of each of them"""
    group_of = {}
    for g in compounds.values():
        if g.kind == 'group':
            for r in g.classes:
                group_of.setdefault(r, g.refid)
    file_group = {}
    for g in compounds.values():
        if g.kind == 'group':
            for r in g.files:
                if r in compounds:
                    file_group.setdefault(compounds[r].file, g.refid)
    nested = {r for c in compounds.values() if is_class(c) for r in c.classes}
    classes = [c for c in compounds.values() if is_class(c)
               and c.refid not in nested
               and (c.refid in group_of or not internal(c))]
    by_name = {c.name: c for c in classes if not template(c.name)}
    special = collections.defaultdict(list)
    pages = {}
    for c in classes:
        base = template(c.name)
        if base in by_name:
            special[by_name[base].refid].append(c)
        else:
            pages[c.refid] = c
    # the group of the classes of a header that are in a group
    header_group = {}
    for c in sorted(pages.values(), key=lambda c: c.line):
        if c.refid in group_of:
            header_group.setdefault(c.file, group_of[c.refid])
    for c in pages.values():
        c.grouped = c.refid in group_of
        c.group = (group_of.get(c.refid) or file_group.get(c.file)
                   or header_group.get(c.file) or OTHERS)
    return pages, special


# ----- the pages ----------------------------------------------------------- #

def escape(line):
    return re.sub(r'([*_|`\\\[\]<>])', r'\\\1', line)


def capital(line):
    return line[:1].upper() + line[1:]


def title(line):
    return '%s\n%s\n\n' % (line, '=' * len(line))


def rubric(line):
    # a heading that stays out of the table of contents, where the pages
    # below a page go and its sections would come between them
    return '.. rubric:: %s\n\n' % line


def toctree(entries):
    return ('.. toctree::\n   :hidden:\n\n'
            + ''.join('   %s\n' % e for e in entries) + '\n')


def table(rows, header):
    if not rows:
        return ''
    out = ('.. list-table::\n   :header-rows: 1\n   :widths: 30 70\n'
           '   :width: 100%\n\n')
    for row in [header] + rows:
        out += '   * - %s\n     - %s\n' % (row[0], row[1] or ' ')
    return out + '\n'


def page_of(c):
    return 'class_' + re.sub(r'\W+', '_', c.short).strip('_')


def group_page(refid):
    return refid if refid != OTHERS else 'group_' + OTHERS


def link(name, page):
    return ':doc:`%s <%s>`' % (escape(name), page)


def class_page(c, special, compounds):
    kind = 'struct' if c.kind == 'struct' else 'class'
    if c.group == OTHERS:
        where = link('Classes in no group', group_page(OTHERS))
    else:
        g = compounds[c.group]
        where = link(g.title, group_page(g.refid))
    out = title(escape(c.short))
    out += ('Declared in ``%s``, in the group %s; the same %s on the '
            '`Doxygen site <%s%s.html>`__.\n\n'
            % (c.file, where, kind, DOXYGEN, c.refid))
    for s in [c] + sorted(special, key=lambda s: s.line):
        kind = 'struct' if s.kind == 'struct' else 'class'
        out += ('.. doxygen%s:: %s\n   :members:\n   :protected-members:\n'
                '   :undoc-members:\n\n' % (kind, s.name))
    return out


def group_text(g, compounds, pages, members):
    out = title(escape(g.title))
    out += ('The same group on the `Doxygen site <%s%s.html>`__.\n\n'
            % (DOXYGEN, g.refid))
    out += '.. doxygengroup:: %s\n   :desc-only:\n\n' % g.name
    subs = [compounds[r] for r in g.groups if r in compounds]
    if subs:
        out += rubric('Subgroups')
        out += table([(link(s.title, group_page(s.refid)),
                       escape(capital(s.brief))) for s in subs],
                     ('Group', 'Description'))
    for head, cs in (('Classes', [c for c in members if c.grouped]),
                     ('Other classes of its headers',
                      [c for c in members if not c.grouped])):
        if cs:
            out += rubric(head)
            out += table([(link(c.short, page_of(c)),
                           escape(capital(c.brief))) for c in cs],
                         ('Class', 'Description'))
    # the files of a group of programs (the tools and the demos), which
    # are documented only on the Doxygen site
    progs = [compounds[r] for r in g.files if r in compounds
             and not compounds[r].file.startswith('lemon/')]
    if progs:
        out += rubric('Programs')
        out += table([('`%s <%s%s.html>`__' % (escape(f.name), DOXYGEN,
                                                 f.refid),
                       escape(capital(f.brief))) for f in progs],
                     ('Program', 'Description'))
    # the members of the classes that a group lists are on the pages of
    # their classes, and Breathe reads them from an XML without them
    classes = {c.name for c in compounds.values() if is_class(c)}
    if any(m.rpartition('::')[0] not in classes for m in g.members):
        out += rubric('Functions and other members')
        out += ('.. doxygengroup:: %s\n   :content-only:\n   :members:\n\n'
                % g.name)
    out += toctree([group_page(s.refid) for s in subs]
                   + [page_of(c) for c in members])
    return out


def write(page, content):
    with open(os.path.join(OUT, page + '.rst'), 'w', encoding='utf-8') as f:
        f.write(content)


def main(xml):
    if os.path.isdir(OUT):
        shutil.rmtree(OUT)
    os.makedirs(OUT)
    if not os.path.isdir(xml):
        write('index', title('API reference')
              + 'Doxygen has not written its XML into ``%s``.\n' % xml)
        return
    compounds = read_xml(xml)
    pages, special = place(compounds)
    by_group = collections.defaultdict(list)
    # in a group, its own classes first, then those of its headers
    for c in sorted(pages.values(),
                    key=lambda c: (not c.grouped, c.file, c.line)):
        by_group[c.group].append(c)
    inner = {r for g in compounds.values() if g.kind == 'group'
             for r in g.groups}
    # the groups in the order of doc/groups.dox, where the XML gives no
    # place for them
    with open(os.path.join(ROOT, 'doc', 'groups.dox'), encoding='utf-8') as f:
        order = re.findall(r'[@\\]defgroup\s+(\w+)', f.read())
    groups = sorted((g for g in compounds.values() if g.kind == 'group'),
                    key=lambda g: (order.index(g.name) if g.name in order
                                   else len(order), g.name))
    top = [g for g in groups if g.refid not in inner]

    index = title('API reference')
    index += ('The classes, the functions and the other members of the '
              'library, gathered in the groups of what they do, as in the '
              'Topics of the `Doxygen site <%s>`__, which also lists all the '
              'classes, the files and the namespaces.\n\n' % DOXYGEN[3:])
    rows = [(link(g.title, group_page(g.refid)), escape(capital(g.brief)))
            for g in top]
    if by_group.get(OTHERS):
        rows.append((link('Classes in no group', group_page(OTHERS)),
                     'The classes of the library that no group holds.'))
    index += table(rows, ('Group', 'Description'))
    index += toctree([group_page(g.refid) for g in top]
                     + ([group_page(OTHERS)] if by_group.get(OTHERS) else []))
    write('index', index)

    for g in groups:
        write(group_page(g.refid),
              group_text(g, compounds, pages, by_group.get(g.refid, [])))
    if by_group.get(OTHERS):
        cs = by_group[OTHERS]
        write(group_page(OTHERS), title('Classes in no group')
              + table([(link(c.short, page_of(c)), escape(capital(c.brief)))
                       for c in cs], ('Class', 'Description'))
              + toctree([page_of(c) for c in cs]))
    for c in pages.values():
        write(page_of(c), class_page(c, special.get(c.refid, []), compounds))


if __name__ == '__main__':
    main(os.path.join(ROOT, 'build', 'doc', 'xml'))
