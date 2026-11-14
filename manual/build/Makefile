# Makefile of the abLEMON User Manual (one source, a PDF and a web site).
# Run from manual/build/.
#
#   make pdf       # PDF via Pandoc + pdflatex     -> ../abLEMON-manual.pdf
#   make html      # static MkDocs site                 -> ../../manual-site/
#   make serve     # live MkDocs preview at 127.0.0.1:8000
#   make figures   # the .pdf and .svg of each changed TikZ figure
#   make check     # citations, anchors and headers covered (see check.py)
#   make clean     # remove what the build writes
#
# Prerequisites: pandoc, a LaTeX distribution (pdflatex), python3 with
# mkdocs, mkdocs-material and pymdown-extensions, poppler-utils (pdftocairo);
# the figures also need the lamalunga package (variable LAMALUNGA).

MANUAL    := ..
FIGDIR    := $(MANUAL)/figures
PDF_OUT   := $(MANUAL)/abLEMON-manual.pdf
LAMALUNGA ?= $(HOME)/GDrive/lamalunga
FIGTEX    := $(filter-out $(FIGDIR)/tikz-ablemon.tex,\
               $(wildcard $(FIGDIR)/*.tex))

.PHONY: pdf html serve figures check clean

pdf:
	pandoc --defaults=pandoc.yaml -o $(PDF_OUT)
	@echo "PDF written to $(PDF_OUT)"

html:
	mkdocs build -f mkdocs.yml
	@echo "Static site written to ../../manual-site/"

serve:
	mkdocs serve -f mkdocs.yml

figures: $(FIGTEX:.tex=.pdf) $(FIGTEX:.tex=.svg)

.PRECIOUS: $(FIGDIR)/%.pdf

# SOURCE_DATE_EPOCH and FORCE_SOURCE_DATE make the PDF of a figure the same
# bytes at each build, so that rebuilding an unchanged figure changes no file
$(FIGDIR)/%.pdf: $(FIGDIR)/%.tex $(FIGDIR)/tikz-ablemon.tex
	cd $(FIGDIR) && SOURCE_DATE_EPOCH=0 FORCE_SOURCE_DATE=1 \
	  TEXINPUTS=$(LAMALUNGA)//: pdflatex -interaction=nonstopmode -halt-on-error \
	  $*.tex > /dev/null && rm -f $*.aux $*.log

$(FIGDIR)/%.svg: $(FIGDIR)/%.pdf
	pdftocairo -svg $< $@

check:
	python3 check.py $(MANUAL)/..

clean:
	rm -rf ../../manual-site
	rm -f $(PDF_OUT)
