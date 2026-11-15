"""The configuration of Sphinx for the API documentation of abLEMON, next to
the Doxygen site and to the user manual: the pages that sphinx/pages.py
writes from README.md and NEWS (read by MyST), and the API reference, whose
pages sphinx/apiref.py writes from the XML that Doxygen writes with the
target html of CMake, rendered by Breathe. The XML is read from
build/doc/xml, or from the directory in LEMON_DOXYGEN_XML; the workflow
Docs builds the site with

    cmake --build build --target html
    pip install -r sphinx/requirements.txt
    sphinx-build -b html -d build/doctrees sphinx build/doc/html/sphinx

The theme is that of Read the Docs, set as the PDF of the manual by
_static/ablemon.css with the Latin Modern fonts of manual/build/fonts.
"""

import html
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
XML = os.environ.get('LEMON_DOXYGEN_XML',
                     os.path.join(ROOT, 'build', 'doc', 'xml'))


def derive_version():
    """the version as CMakeLists.txt derives it: the tag of git, without
    the r or v before its number, else unknown"""
    try:
        raw = subprocess.run(['git', 'describe', '--tags', '--always'],
                             cwd=ROOT, capture_output=True, text=True,
                             check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        raw = ''
    return re.sub(r'^[rv](\d)', r'\1', raw) or 'unknown'


project = 'abLEMON'
author = 'EGRES and the authors of abLEMON'
# the copyright of LICENSE, the changes of abLEMON having that of their
# authors
copyright = '2003-2012, EGRES'
release = derive_version()
version = re.match(r'[\d.]*', release).group(0) or release

extensions = [
    'breathe',              # the Doxygen XML in Sphinx
    'myst_parser',          # the pages in Markdown
    'sphinx.ext.mathjax',   # the formulae of the comments
    'sphinx_copybutton',    # a button that copies a block of code
]

# a signature longer than this goes one parameter per line, instead of
# being cut at the margin or wrapped at random
maximum_signature_line_length = 90

# the prompts of the shells are not copied with the commands
copybutton_prompt_text = r'\$ '
copybutton_prompt_is_regexp = True

source_suffix = {'.rst': 'restructuredtext', '.md': 'markdown'}
myst_enable_extensions = ['dollarmath']
myst_heading_anchors = 3
exclude_patterns = ['_build']

# the macros that declare the constants of the headers, which the C++
# domain takes for attributes
cpp_id_attributes = ['LEMON_CONSTANT', 'LEMON_DATA']

# the verbatim blocks of the comments (files of the LGF format, commands of
# the shell) are lexed as C++ and fail, then shown as they are
suppress_warnings = ['misc.highlighting_failure']

primary_domain = 'cpp'
highlight_language = 'cpp'

# Breathe reads a copy of the XML, mended by mend_xml() below
MENDED = os.path.join(os.path.dirname(XML), 'xml-sphinx')
breathe_projects = {'abLEMON': MENDED}
breathe_default_project = 'abLEMON'

VARIABLE = re.compile(r'<memberdef kind="variable".*?</memberdef>\s*',
                      re.S)
MEMBER = re.compile(r'<memberdef .*?</memberdef>\s*', re.S)
INNERCLASS = re.compile(r'\s*<innerclass [^>]*>[^<]*</innerclass>')
# the images of the comments for the outputs other than HTML (the EPS of
# LaTeX), which Breathe would show next to the PNG
IMAGE = re.compile(r'<image type="(?!html")[^"]*"[^>]*?(?:/>|>.*?</image>)',
                   re.S)
BRIEF = re.compile(r'(<briefdescription>)(.*?)(</briefdescription>\s*'
                   r'<detaileddescription>)(.*?)(?=</detaileddescription>)',
                   re.S)


def plain(xml):
    """the text of a description, without markup and spaces"""
    return re.sub(r'\s+', '', html.unescape(re.sub(r'<[^>]*>', '', xml)))


def mend_brief(m):
    """the brief of a compound or member is dropped when its detailed
    description begins with the same sentence, which the page would show
    twice"""
    brief = plain(m.group(2)).rstrip('.')
    detail = plain(m.group(4))
    if brief and detail.startswith(brief) and \
       detail[len(brief):len(brief) + 1] in ('', '.'):
        return m.group(1) + m.group(3) + m.group(4)
    return m.group(0)


def mend_member(m):
    """a variable as the C++ domain of Sphinx parses it: the anonymous
    union of a class is dropped (its members are in the class), and the
    arguments of a constant built by a constructor, which Doxygen gives
    as those of a function, become its initializer"""
    text = m.group(0)
    if '<name></name>' in text:
        return ''
    args = re.search(r'<argsstring>(\(.*?\))</argsstring>', text)
    if args:
        kind = re.search(r'<type>(?:const )?([^<]*)</type>', text).group(1)
        text = text.replace(args.group(0), '<argsstring></argsstring>'
                            '<initializer>= %s%s</initializer>'
                            % (kind.split()[-1], args.group(1)))
    return text


def mend_xml():
    """the copy of the XML that Breathe reads: the classes of a group, and
    the members of classes that it lists, are dropped from it, since each
    class has its page and the page of the group would render them a
    second time, the images for LaTeX are dropped, the briefs repeated by
    the detailed descriptions are dropped by mend_brief(), and the
    variables are mended by mend_member()"""
    with open(os.path.join(XML, 'index.xml'), encoding='utf-8') as f:
        classes = set(re.findall(r'kind="(?:class|struct|union)"><name>'
                                 r'([^<]*)</name>', f.read()))

    def member_of_group(m):
        name = re.search(r'<qualifiedname>(.*)::[^:<]*</qualifiedname>',
                         m.group(0))
        return '' if name and name.group(1) in classes else m.group(0)

    os.makedirs(MENDED, exist_ok=True)
    for name in os.listdir(XML):
        with open(os.path.join(XML, name), 'rb') as f:
            data = f.read()
        if name.endswith('.xml'):
            text = VARIABLE.sub(mend_member, data.decode('utf-8'))
            text = IMAGE.sub('', text)
            text = BRIEF.sub(mend_brief, text)
            if name.startswith('group__'):
                text = INNERCLASS.sub('', text)
                text = MEMBER.sub(member_of_group, text)
            data = text.encode('utf-8')
        with open(os.path.join(MENDED, name), 'wb') as f:
            f.write(data)


sys.path.insert(0, HERE)
import pages  # noqa: E402

pages.main(XML)
if os.path.isdir(XML):
    mend_xml()

html_theme = 'sphinx_rtd_theme'
html_title = 'abLEMON %s' % release
html_logo = os.path.join(ROOT, 'doc', 'theme', 'lemon-logo.png')
# the fonts are copied from the manual at every build, next to the css
html_static_path = ['_static', os.path.join(ROOT, 'manual', 'build',
                                            'fonts')]
html_css_files = ['ablemon.css']
html_theme_options = {'navigation_depth': 4, 'collapse_navigation': True,
                      'sticky_navigation': True,
                      'style_nav_header_background': '#ffffff'}


def codestyle(app, exception):
    """the template arguments and the parameter lists of the pages spaced as
    in the code [see doc/codestyle_html.py]"""
    if exception is None:
        import sys
        sys.path.insert(0, os.path.join(ROOT, 'doc'))
        import codestyle_html
        codestyle_html.main([app.outdir])


def setup(app):
    app.connect('build-finished', codestyle)
