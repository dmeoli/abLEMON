# abLEMON User Manual — Maintenance Notes

*Internal document for whoever maintains this manual. It is excluded from
the MkDocs site and from the PDF build.*

---

## 1. What this is

A user manual for abLEMON, the C++ template library of graphs, maps,
heaps and network optimization algorithms, written family by family (the
plan is in `OUTLINE.md`). One source, Markdown under `manual/chapters/`,
built to a PDF (Pandoc and LaTeX, committed as `abLEMON-manual.pdf`) and
to a web site (MkDocs Material, published next to the Doxygen pages).

The manual mirrors the implementation exactly: every public class of
`lemon/` is described in the chapter of its family, and every claim on a
cost, a bound, a default, a parameter or a behavior is checked against
the header (and, when there is one, against its test) before it is
written. When the code changes, the chapter changes in the same commit.

---

## 2. Authoring conventions

- **Language:** scientific English, in the style of a methodological
  paper: long sentences built by subordination (mean around 30 words),
  punctuation doing the work (colons opening an explanation, semicolons
  for contrast and consequence, parentheses for the aside and for the
  reference to a formula or a section), `i.e.,` and `e.g.,` freely, hedges
  stated in the text ("Of course,", "We remark that"). No marketing words
  (novel, key, crucially, powerful, seamless, robust in a vague sense), no
  rhetorical antithesis ("it is not X, it is Y"), no bold for emphasis in
  the prose, no lists of takeaways; enumerations go inline with (i), (ii),
  (iii), and itemize only for truly parallel technical objects. "We" is
  the authors; "one" is the reader.
- **Only abLEMON.** The manual speaks of abLEMON and of the literature;
  it compares with LEMON 1.3.1 only where abLEMON differs from it, and it
  names no other library.
- **Section numbering** is written by hand in the headings
  (`# 13. Minimum cost flows {#ch-13}`, `## 13.2 The network simplex
  method {#sec-13-2}`, `### 13.2.1 ... {#sec-13-2-1}`); LaTeX numbering is
  suppressed in `build/preamble.tex`. Number new sections by hand, without
  gaps. The appendices are `# A. ... {#app-a}` and `## A.1 ... {#sec-a-1}`.
- **Labelled items** carry an anchor, `<span id="..."></span>`, right
  before their bold label, numbered by chapter: `alg-13-1` (Algorithm
  13.1), `thm-13-1`, `lem-13-1`, `tab-13-1`, `ex-13-1` (Exercise 13.1).
  Lemmas and theorems have separate counters. A figure takes its id on the
  image, in a paragraph of its own:
  `![Figure 13.1. caption](../figures/13-name.svg){#fig-13-1}`
  (a span in the same paragraph would stop Pandoc from making a figure).
  Equations are numbered with `\tag{13.1}` inside the display and cited as
  "(13.1)".
- **Cross-references** are Markdown links to `FILE.md#id`, from a chapter
  to a sibling file (`[§13.2](13-min-cost-flow.md#sec-13-2)`), from
  `index.md` with the `chapters/` prefix; `build/crossref-link.lua` turns
  them into `\hyperref` in the PDF.
- **Citations** link to the anchor of the entry in
  `chapters/Z-bibliography.md`:
  `[Ahuja, Magnanti and Orlin 1993, §9.5](Z-bibliography.md#ref-amo93networkflows)`.
  An entry is a paragraph `<span id="ref-KEY"></span> Authors. Title.
  Venue, volume(issue):pages, year. doi:...`, with the DOI whenever the
  work has one. KEY is the key of `doc/references.bib` when the work is
  there, else `firstauthorYEARword` in lowercase (`picard1980structure`).
  The bibliography holds every work that the headers and `doc/*.dox` cite
  (with `\cite` or in prose), every work the README and NEWS name, and every
  work the chapters cite; it holds no entry for other libraries (some keys
  of `doc/references.bib` are such, and are left out).
- **Code.** The chapters hold no programs and no extracts of the
  implementation: an algorithm is given by its pseudocode, and the use of a
  class by its interface in the prose, with real C++ only as signatures of
  the API and short examples of use (a few lines that show how a class is
  called), the same rule as the other manuals of the family. The programs of the manual live in
  `manual/examples/NN-name.cc`, each a complete program in the style of
  code-style, built and run by ctest as `manual_NN_name` (the hyphens
  become underscores), which checks its own results and returns nonzero if
  one is wrong; a chapter names the program that shows a class at work
  (and the exercises name the programs and tests that check them), so that
  what the manual states is checked by the tests without being printed.
  (`build/snippets.lua` and `pymdownx.snippets` still expand a line
  `--8<-- "path"`, should a file ever need to be included.)
- **Pseudocode** is written in place, as a numbered algorithm in a fenced
  block without a language, introduced by its label
  (`<span id="alg-13-1"></span>**Algorithm 13.1** (Network simplex).`),
  with the notation of the chapter and, after it, the line of the header
  that implements each step when that is not obvious.
- **Theory** is given to the extent that the user needs it to choose an
  algorithm and to read its results: the problem in formulas, the
  optimality conditions the algorithm maintains, its invariants, its
  complexity (worst case and, where the header or the literature states
  it, the behavior in practice), and what each variant of the library
  changes. Every claim of correctness, of termination, of optimality or of
  complexity is a lemma or a theorem, a paragraph `**Theorem 13.1.**
  *statement*`, followed by its proof (`*Proof.* ... $\square$`), complete
  and self-contained, or, for a result whose proof would take pages (e.g.,
  the bound of a data structure from the literature), a sketch of the
  argument with the precise citation of the full proof.
- **Exercises** close each chapter of Parts II to IV, under `## N.k
  Exercises {#sec-N-k}`, each one labelled and, when a test of `test/` or an
  example of `manual/examples/` solves it or checks it, naming that file.
- **Status admonitions**: the classes that abLEMON adds to LEMON or whose
  behavior differs from LEMON 1.3.1 carry a note
  `!!! note "abLEMON"` saying what is new; nothing else is marked. A note
  is one paragraph, indented by four spaces under its first line: MkDocs
  renders it as an admonition, and `build/admonition.lua` turns it into the
  framed environment `ablnote` of `build/preamble.tex` for the PDF.
- **Tables** are pipe tables. One wider than 60 characters takes the width
  of the text in the PDF (`columns: 60` in `pandoc.yaml`), with columns as
  wide as the dashes of its separator line; the widths that Pandoc derives
  are rounded to two decimals, so the dashes must be chosen so that the
  rounded fractions do not add up to more than 1 (otherwise the table is
  an `Overfull \hbox`).
- **Line width**: the prose is wrapped at 78 columns; only the lines that
  cannot be broken without changing what they render (images, display
  math, pseudocode, table rows) may be longer.
- **Math delimiters:** `$...$` and `$$...$$` only, never `\(...\)`; no
  `@{}` in array column specifications (KaTeX does not know it).
- **Doxygen links** go to `https://dmeoli.github.io/abLEMON/` and must be
  checked against a local Doxygen build (the file names of class pages are
  `a<number>.html` and are not guessable; link the group page or the header
  page when unsure).

---

## 3. Source of truth

The headers of `lemon/`, their Doxygen comments and groups
(`doc/groups.dox`, `doc/min_cost_flow.dox`), the tests of `test/`, and
NEWS for what abLEMON changed. A complexity or a default stated in a
chapter is the one in the code, not the one of the textbook, when the two
differ (and the difference is then worth a sentence).

---

## 4. Layout

```
manual/
  OUTLINE.md          # the plan: one chapter per family, with its headers
  MAINTENANCE.md      # this file
  chapters.txt        # the chapters in reading order (source of the lists)
  index.md            # front matter (title, abstract, conventions, ToC)
  abLEMON-manual.pdf  # built PDF (commit it)
  chapters/           # NN-<family>.md, appendices A, Y, Z
  examples/           # NN-<name>.cc, one complete program each, run by ctest
  figures/            # <NN>-<name>.tex (TikZ, standalone) + .pdf + .svg
    tikz-ablemon.tex  # the styles shared by the figures
  build/
    Makefile  pandoc.yaml  preamble.tex  mkdocs.yml  check.py
    snippets.lua  admonition.lua  figure-ext.lua  crossref-link.lua
    break-code.lua  katex-init.js
```

---

## 5. Building locally

From `manual/build/`:

```sh
make pdf       # -> ../abLEMON-manual.pdf    (Pandoc + pdflatex)
make html      # -> ../../manual-site/       (MkDocs Material)
make serve     # live preview at 127.0.0.1:8000
make figures   # the .pdf and .svg of each changed figure
make check     # links, anchors, snippets, bibliography and coverage
```

The examples are built and run with the tests of the library
(`ctest -R manual_`). PDF acceptance: zero `Overfull \hbox`, zero undefined
references, and `make check` silent. `pandoc.yaml` and the `nav` of
`mkdocs.yml` list the chapters in the order of `chapters.txt`: keep the
three, and the table of contents of `index.md`, in sync.

---

## 6. Adding or updating a figure

1. Write `figures/<NN>-<name>.tex` as a `standalone` TikZ file that loads
   `lmodern`, `tikz` and `lamalunga` and then `\input`s `tikz-ablemon`
   (see `03-digraph.tex`). The style is flat, one colour per role, with
   the palette lamalunga computes from the primary (`7030A0`): what the
   algorithm reads or follows is in the primary (`vertex-touched`,
   `arc-touched`, `edge-touched`), what it adds or chooses in the first
   colour of its triad (`vertex-added`, `arc-added`), the rest in golden
   tints of the primary (`vertex`, `arc`, `region`, `cell`); the data of an
   arc go on a `label`. No outlines, gradients or shadows, and no colour
   outside the palette. The Makefile finds lamalunga in `~/GDrive/lamalunga`
   (variable `LAMALUNGA`).
2. `make figures` writes the `.pdf` and the `.svg`.
3. Reference it as `![Figure N.M. caption](../figures/<NN>-<name>.svg){#fig-N-M}`,
   in a paragraph of its own.
4. Commit the `.tex`, the `.pdf` and the `.svg`.

---

## 7. Deployment

`.github/workflows/docs.yml` builds the site of the manual into the folder
`manual/` of the pages of the repository, next to the Doxygen pages, at
each push on master; `.github/workflows/release.yml` attaches
`manual/abLEMON-manual.pdf` to each release.

---

## 8. Updating for a new abLEMON version

1. Bump the version in `index.md`, in the subtitle of `build/pandoc.yaml`
   and in the `site_description` of `build/mkdocs.yml`.
2. For each entry of the new block of NEWS, update the chapter of the
   class it touches (and its `abLEMON` note).
3. Rebuild both outputs, run `make check` and `ctest -R manual_`.
4. Commit the refreshed `abLEMON-manual.pdf`.
