#!/usr/bin/env python3
# --------------------------------------------------------------------------- #
#                            codestyle_html.py                                #
#                                                                             #
# Spaces the template arguments and the parameter lists in the HTML that      #
# Doxygen and Sphinx write as the code of the library writes them: one space  #
# inside the brackets, before and after a comma and around & and *, e.g.,     #
# NetworkSimplex< GR , V , C > and run( Node s , Node t ) instead of          #
# NetworkSimplex< GR, V, C > and run(Node s, Node t). The tags are left as    #
# they are and only the text between them changes; a bracket opens a list     #
# only right after a name, a parenthesis only if it holds a declaration (so   #
# neither #include <lemon/core.h>, a < b nor node(s) is touched), and         #
# scripts and styles are skipped; in the scripts of the navigation tree and   #
# of the search only the strings change. Usage: codestyle_html.py <folder>    #
# --------------------------------------------------------------------------- #

import os
import re
import sys

TOKEN = re.compile(r'(<script\b.*?</script>|<style\b.*?</style>|<[^>]*>)',
                   re.S | re.I)
# the string literals of the scripts of the navigation tree and the search
STRING = re.compile(r"(['\"])((?:(?!\1)[^\\\n]|\\.)*)\1")
ENTITY = re.compile(r'&(lt|gt|amp|nbsp|#\d+|#x[0-9a-fA-F]+|[a-zA-Z]+);')


def chars(text):
    """the visible characters of a text, each with its source string"""
    out, i = [], 0
    while i < len(text):
        m = ENTITY.match(text, i)
        if m:
            out.append(m.group(0))
            i = m.end()
        else:
            out.append(text[i])
            i += 1
    return out


def visible(c):
    return {'&lt;': '<', '&gt;': '>', '&amp;': '&', '&nbsp;': ' ',
            '&#160;': ' '}.get(c, c)


def space(html):
    parts = TOKEN.split(html)
    # the visible characters of all the texts, with their part
    seq = []
    for k, p in enumerate(parts):
        if k % 2 == 0:
            seq += [(k, c) for c in chars(p)]
    v = [visible(c) for _, c in seq]
    before, after = set(), set()

    def name_before(i):
        j = i
        while j > 0 and (v[j - 1].isalnum() or v[j - 1] == '_'):
            j -= 1
        return ''.join(v[j:i])

    def close_of(i):
        d = 0
        for j in range(i, len(v)):
            if v[j] == '(':
                d += 1
            elif v[j] == ')':
                d -= 1
                if d == 0:
                    return j
            elif v[j] in ';{}\n':
                return -1
        return -1

    stack = []  # '<' a template list, '(' a parameter list, '.' other
    for i, c in enumerate(v):
        spaced = bool(stack) and stack[-1] in '<('
        name = name_before(i)
        if not name and c == '(':  # operator<(, operator()(, ...
            m = re.search(r'operator(\(\)|[^\w\s()]{1,3})$',
                          ''.join(v[max(0, i - 12):i]))
            name = m.group(0) if m else ''
        if c == '<' and name and not name.startswith('operator'):
            stack.append('<')
            if i + 1 < len(v) and v[i + 1] not in ' >':
                after.add(i)
        elif c == '>' and stack and stack[-1] == '<':
            if v[i - 1] not in ' <':
                before.add(i)
            stack.pop()
        elif c == '(':
            j = close_of(i)
            inner = ''.join(v[i + 1:j]) if j > 0 else ''
            if name and j > 0 and re.search(r'\w[\s&*]+\w|[&*]\s*\w', inner):
                stack.append('(')
                if v[i + 1] != ' ':
                    after.add(i)
                if v[j - 1] != ' ':
                    before.add(j)
            else:
                stack.append('.')
        elif c == ')':
            if stack:
                stack.pop()
        elif c == ',' and spaced:
            if v[i - 1] != ' ':
                before.add(i)
            if i + 1 < len(v) and v[i + 1] != ' ':
                after.add(i)
        elif c in '&*' and stack and stack[-1] == '(':
            if v[i - 1] not in ' &*(':
                before.add(i)
            if i + 1 < len(v) and v[i + 1] not in ' &*),':
                after.add(i)
        elif (c in '&*' and not stack and i > 1 and v[i - 1] == ' ' and
              (v[i - 2].isalnum() or v[i - 2] in '_>') and
              i + 1 < len(v) and (v[i + 1].isalpha() or v[i + 1] == '_')):
            after.add(i)  # the return type: GraphReader & attributes(
        elif c in ';{}' or c == '\n':
            stack = []
    if not before and not after:
        return html
    texts = {}
    for i, (k, c) in enumerate(seq):
        texts.setdefault(k, []).append(
            (' ' if i in before else '') + c + (' ' if i in after else ''))
    for k, cs in texts.items():
        parts[k] = ''.join(cs)
    return ''.join(parts)


def main(folders):
    for folder in folders:
        for root, _, files in os.walk(folder):
            for f in files:
                if f.endswith(('.html', '.js')):
                    p = os.path.join(root, f)
                    with open(p, encoding='utf-8') as h:
                        s = h.read()
                    t = space(s) if f.endswith('.html') else STRING.sub(
                        lambda m: m.group(1) + space(m.group(2)) + m.group(1),
                        s)
                    if t != s:
                        with open(p, 'w', encoding='utf-8') as h:
                            h.write(t)


if __name__ == '__main__':
    main(sys.argv[1:])
