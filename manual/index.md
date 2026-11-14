# abLEMON — A User Manual {#front}

*For abLEMON 1.9.0 (November 2026).*

## Abstract {#abstract}

abLEMON is a C++ template library of graphs, maps, heaps and network
optimization algorithms, maintained as a fork of LEMON 1.3.1, whose headers
and namespace it keeps. This manual describes the library as it is
implemented, family by family: the graph structures and the adaptors that
present a graph as another, the maps through which data are attached to nodes,
arcs and edges, the heaps and the other auxiliary structures, and the
algorithms for graph search, shortest paths, spanning trees, maximum and
minimum cost flows (with the certificates of infeasibility and unboundedness
and the $k$ best flows that abLEMON adds), minimum cuts, minimum mean cycles,
matchings, connectivity, planarity and isomorphism, the heuristics for the
travelling salesman and the maximum clique, and the interface to LP and MIP
solvers. For each algorithm the manual states the problem it solves, the
conditions it maintains, its complexity as implemented and the choices it
leaves to the user, gives its pseudocode, and proves its correctness and its
complexity; programs that show each class at work are distributed with the
manual and run as tests of the library. Yet, it does not
replace the reference documentation, which lists every member with its exact
signature, and to which the chapters refer when a detail is not worth
repeating.

## Conventions {#conventions}

The headings carry the numbers of the manual, and sections, figures, tables,
algorithms, theorems, lemmas and exercises are numbered by chapter. The
programs named by the chapters are the files of `manual/examples/`, each of
which is a test of the library (`ctest -R manual_`). A note headed "abLEMON" marks what abLEMON adds to LEMON
1.3.1, or where its behavior differs from it; the rest of the library behaves
as in LEMON. Citations refer to the bibliography ([Appendix
Z](chapters/Z-bibliography.md#app-z)), and the terms defined in the chapters
are collected in the glossary ([Appendix Y](chapters/Y-glossary.md#app-y)).

## Contents {#contents}

**Part I: Foundations**

-  [1. Introduction](chapters/01-introduction.md#ch-1)
-  [2. Installation](chapters/02-installation.md#ch-2)
-  [3. Graphs, items and maps](chapters/03-mental-model.md#ch-3)

**Part II: Data structures**

-  [4. Graph structures](chapters/04-graphs.md#ch-4)
-  [5. Graph adaptors](chapters/05-adaptors.md#ch-5)
-  [6. Maps](chapters/06-maps.md#ch-6)
-  [7. Heaps](chapters/07-heaps.md#ch-7)
-  [8. Paths and auxiliary structures](chapters/08-auxiliary.md#ch-8)

**Part III: Algorithms**

-  [9. Graph search](chapters/09-search.md#ch-9)
-  [10. Shortest paths](chapters/10-shortest-paths.md#ch-10)
-  [11. Spanning trees and arborescences](chapters/11-spanning.md#ch-11)
-  [12. Maximum flows and circulations](chapters/12-max-flow.md#ch-12)
-  [13. Minimum cost flows](chapters/13-min-cost-flow.md#ch-13)
-  [14. Certificates and the k best flows](chapters/14-k-best-flows.md#ch-14)
-  [15. Minimum cuts](chapters/15-min-cut.md#ch-15)
-  [16. Minimum mean cycles](chapters/16-min-mean-cycle.md#ch-16)
-  [17. Matchings](chapters/17-matching.md#ch-17)
-  [18. Connectivity, planarity, isomorphism](chapters/18-properties.md#ch-18)
-  [19. Travelling salesman and maximum
  clique](chapters/19-tsp-clique.md#ch-19)
-  [20. The LP and MIP interface](chapters/20-lp.md#ch-20)

**Part IV: Input, output and tools**

-  [21. Input and output](chapters/21-io.md#ch-21)
-  [22. The tools](chapters/22-tools.md#ch-22)

**Part V: Appendices**

-  [A. Writing a graph, a map or a
  heap](chapters/A-writing-graph-map.md#app-a)
-  [Y. Glossary](chapters/Y-glossary.md#app-y)
-  [Z. Bibliography](chapters/Z-bibliography.md#app-z)
