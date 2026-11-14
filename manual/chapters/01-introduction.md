# 1. Introduction {#ch-1}

We are concerned with abLEMON, a C++ template library of graphs and of the
algorithms of network optimization built on them. It provides the structures
that represent directed, undirected and bipartite graphs, the maps that attach
data to their nodes, arcs and edges, the heaps and the other auxiliary
structures that the algorithms need, and the algorithms themselves (searches,
shortest paths, spanning trees, maximum and minimum cost flows, minimum cuts
and mean cycles, matchings, connectivity, planarity and isomorphism,
heuristics for the travelling salesman and the maximum clique, and an
interface to LP and MIP solvers). abLEMON is a maintained fork of LEMON, the
Library for Efficient Modeling and Optimization in Networks of the Egerváry
Research Group of the Eötvös Loránd University of Budapest [(Dezső, Jüttner
and Kovács 2011)](Z-bibliography.md#ref-DezsoJuttnerKovacs11Lemon), which has
had no release since 1.3.1 (2014). It starts from the last revision of the
Mercurial repository of LEMON, whose history it keeps, and it keeps its
headers (`lemon/`) and its namespace (`lemon`); hence, a program written for
LEMON usually compiles with abLEMON unchanged.

This manual describes the library as it is implemented in abLEMON 1.9.0. Each
public class of `lemon/` is described in the chapter of its family, with the
theory that one needs to choose among the algorithms and to read their
results, and the statements on costs, defaults and behavior have been checked
against the code. The manual is a companion to the reference documentation
that Doxygen produces from the headers (<https://dmeoli.github.io/abLEMON/>),
which gives each member with its exact signature. We give instead the problem
that each class solves, the conditions it maintains, how the classes are meant
to be combined, and the pseudocode of each algorithm with the proofs of its
properties; the programs that use the classes are not printed in the chapters,
but kept in `manual/examples/`, where each is compiled and run as a test of
the library (cf. [chapter 2](02-installation.md#ch-2)) that checks what the
chapter states. We assume that the reader writes C++ and knows the basic
notions of graph theory and of linear programming. The network optimization
that a chapter needs is recalled in it, with references to the textbook of
[Ahuja, Magnanti and Orlin (1993)](Z-bibliography.md#ref-amo93networkflows)
and to the papers of each algorithm.

## 1.1 What abLEMON changes {#sec-1-1}

The differences from LEMON 1.3.1 fall into four groups, which NEWS lists
release by release and which the chapters mark with a note "abLEMON" where
they apply. The first group is portability: the library compiles without
warnings from C++11 to C++23 with GCC, Clang, AppleClang and MSVC. For
instance, the allocators go through `std::allocator_traits`, since C++20
removed the members of `std::allocator` that `ArrayMap` and `ListPath` called;
also, each header is self-contained, and the headers survive the macros `min`
and `max` of `<windows.h>`. The second group is correctness. Several defects
of the algorithms and of the data structures are fixed, most of them with a
test that failed before the fix (e.g., `RadixHeap` mixed the items of adjacent
boxes, `BinomialHeap` truncated fractional priorities, and `Circulation` read
past its storage on an empty digraph), and the items of the graphs are
`INVALID` when they are default-constructed.

The third group concerns the minimum cost flow algorithms ([chapter
13](13-min-cost-flow.md#ch-13) and [chapter 14](14-k-best-flows.md#ch-14)).
These solve problems with floating-point data, under tolerances relative to
the data, and they certify an infeasible or unbounded answer (with a set of
nodes whose supply exceeds what can leave it, or with a cycle of negative cost
and infinite capacity). Furthermore, they give the $k$ most violated such
sets, and two further classes give the $k$ best integer flows and the $k$ best
extreme flows; alternative optimal potentials are also available. The fourth
group is a set of new algorithms and structures, i.e., `HopcroftKarp` and
`MinCostAssignment` for bipartite matching ([chapter
17](17-matching.md#ch-17)), `BidirDijkstra` and `AStar` for the shortest path
between two nodes ([chapter 10](10-shortest-paths.md#ch-10)), `TomitaSekiMc`
for the exact maximum clique ([chapter 19](19-tsp-clique.md#ch-19)), and
`StaticGraph`, the undirected counterpart of `StaticDigraph` ([chapter
4](04-graphs.md#ch-4)). Also, with CMake 3.28 and Clang 16 or MSVC 19.34, the
library may be built as the C++20 module `lemon` (whose interface is
`lemon/lemon.cppm`, cf. [§2.3](02-installation.md#sec-2-3)).

## 1.2 How the manual is organized {#sec-1-2}

Part I gives what one needs before anything else: the installation ([chapter
2](02-installation.md#ch-2)) and the model of the library ([chapter
3](03-mental-model.md#ch-3)). In this model graphs have nodes and arcs that
are handles, data are attached to them by maps, concepts state what an
algorithm requires of the types it is given, and algorithms are classes that
are configured, run and then queried. Part II describes the data structures,
i.e., the graphs, the adaptors that present a graph as another one without
copying it, the maps, the heaps, the paths and the auxiliary structures. Part
III describes the algorithms, family by family: each chapter states the
problem, the optimality conditions and the complexity of each algorithm, gives
its pseudocode with the theorems on its correctness and its complexity,
compares the variants that the library offers, names the programs and the
tests that check it, and ends with exercises. Part IV covers the formats of
input and output (LGF, DIMACS, nauty, PostScript) and the tools of `tools/`.
Appendix A explains how a new graph, map or heap is written so that the
algorithms accept it, while Appendices Y and Z hold the glossary and the
bibliography.

A reader who only needs one algorithm may read [chapter
3](03-mental-model.md#ch-3) and then the chapter of that algorithm, since the
chapters of Part III are meant to be read independently of each other. They
refer to each other where one algorithm uses another one; e.g., the
certificates of [chapter 14](14-k-best-flows.md#ch-14) rest on the maximum
flows of [chapter 12](12-max-flow.md#ch-12). Table 1.1 gives the chapter where
each header of `lemon/` is described (the internal headers of `lemon/bits/`
only where a user meets them). In it, `core.h` holds the items of the graphs,
the basic utilities and the common definitions, and is included by all the
other headers; `config.h`, instead, is written by CMake when the library is
configured (cf. [§2.4](02-installation.md#sec-2-4)), and `lemon.cppm` is the
interface of the C++20 module (cf. [§2.3](02-installation.md#sec-2-3)).

<span id="tab-1-1"></span>**Table 1.1.** The headers of `lemon/` by chapter.

| Chapter | Headers |
|----------------------------|----------------------------|
| [2](02-installation.md#ch-2) | `config.h`, `lemon.cppm` |
| [3](03-mental-model.md#ch-3) | `core.h` (items), `concept_check.h`, `tolerance.h`, `math.h`, `bits/traits.h` |
| [4](04-graphs.md#ch-4) | `list_graph.h`, `smart_graph.h`, `static_graph.h`, `compact_graph.h`, `full_graph.h`, `grid_graph.h`, `hypercube_graph.h`, `edge_set.h`, `core.h` (utilities), `concepts/digraph.h`, `concepts/graph.h`, `concepts/bpgraph.h`, `concepts/graph_components.h` |
| [5](05-adaptors.md#ch-5) | `adaptors.h` |
| [6](06-maps.md#ch-6) | `maps.h`, `concepts/maps.h` |
| [7](07-heaps.md#ch-7) | `bin_heap.h`, `quad_heap.h`, `dheap.h`, `fib_heap.h`, `pairing_heap.h`, `binomial_heap.h`, `radix_heap.h`, `bucket_heap.h`, `concepts/heap.h` |
| [8](08-auxiliary.md#ch-8) | `path.h`, `concepts/path.h`, `unionfind.h`, `elevator.h`, `dim2.h`, `radix_sort.h`, `random.h`, `counter.h`, `time_measure.h`, `color.h`, `error.h`, `assert.h`, `arg_parser.h` |
| [9](09-search.md#ch-9) | `bfs.h`, `dfs.h`, `max_cardinality_search.h` |
| [10](10-shortest-paths.md#ch-10) | `dijkstra.h`, `bidir_dijkstra.h`, `astar.h`, `bellman_ford.h`, `suurballe.h` |
| [11](11-spanning.md#ch-11) | `kruskal.h`, `min_cost_arborescence.h` |
| [12](12-max-flow.md#ch-12) | `preflow.h`, `edmonds_karp.h`, `circulation.h`, `concepts/max_flow.h` |
| [13](13-min-cost-flow.md#ch-13) | `network_simplex.h`, `cost_scaling.h`, `capacity_scaling.h`, `cycle_canceling.h`, `concepts/min_cost_flow.h` |
| [14](14-k-best-flows.md#ch-14) | `k_best_flows.h`, `k_best_extreme_flows.h` |
| [15](15-min-cut.md#ch-15) | `hao_orlin.h`, `nagamochi_ibaraki.h`, `gomory_hu.h` |
| [16](16-min-mean-cycle.md#ch-16) | `karp_mmc.h`, `hartmann_orlin_mmc.h`, `howard_mmc.h`, `concepts/min_mean_cycle.h` |
| [17](17-matching.md#ch-17) | `matching.h`, `bipartite_matching.h`, `fractional_matching.h` |
| [18](18-properties.md#ch-18) | `connectivity.h`, `euler.h`, `planarity.h`, `vf2.h`, `vf2pp.h` |
| [19](19-tsp-clique.md#ch-19) | `nearest_neighbor_tsp.h`, `greedy_tsp.h`, `insertion_tsp.h`, `christofides_tsp.h`, `opt2_tsp.h`, `grosso_locatelli_pullan_mc.h`, `tomita_seki_mc.h` |
| [20](20-lp.md#ch-20) | `lp_base.h`, `lp.h`, `lp_skeleton.h`, `glpk.h`, `clp.h`, `cbc.h`, `cplex.h`, `soplex.h` |
| [21](21-io.md#ch-21) | `lgf_reader.h`, `lgf_writer.h`, `dimacs.h`, `nauty_reader.h`, `graph_to_eps.h` |
| [A](A-writing-graph-map.md#app-a) | `bits/graph_extender.h`, `bits/alteration_notifier.h` |

## 1.3 Conventions {#sec-1-3}

Class names, members and code are set in `monospace`. A class is named without
the namespace `lemon`, and a member is written as `Class::member()` when the
class is not clear from the context. A graph has $n$ nodes and $m$ arcs (or
edges), while $C$ and $U$ denote the largest absolute value of a cost and of a
capacity when the data are integer. The complexities are those of the
implementation, and the chapters say so when they differ from the bounds of
the literature. Sections, figures, tables, algorithms, lemmas, theorems and
exercises are numbered by chapter (lemmas and theorems with separate
counters), and a proof ends with $\square$. A program is named by the file of
`manual/examples/` that holds it, and therefore by the test `manual_<name>`
that runs it (the hyphens of the name becoming underscores).
