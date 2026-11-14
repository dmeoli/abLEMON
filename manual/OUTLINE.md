# abLEMON User Manual — Outline

*Internal plan of the manual, excluded from the site and from the PDF. Each
chapter lists the headers of `lemon/` it describes; every header is in
exactly one chapter, and every public class of a header is described in
the chapter of that header. The conventions are in `MAINTENANCE.md`.*

## Part I — Foundations

1. **Introduction** (`01-introduction.md`). What abLEMON is, its relation
   to LEMON 1.3.1 (what changed, from the README and NEWS), the audience,
   how to read the manual, the map of the headers by chapter; `core.h`,
   `config.h`, `lemon.cppm` named here.
2. **Installation and first program** (`02-installation.md`). The
   packages (PPA, conda-forge, vcpkg, Conan, Homebrew), the build from
   source and its CMake options, `find_package(LEMON)` and pkg-config, the
   C++20 module `lemon`, the optional LP and MIP backends.
3. **Graphs, items and maps** (`03-mental-model.md`). The first program;
   the handles (Node, Arc, Edge, RedNode, BlueNode) and `INVALID`; the
   iterators; the maps as the only way to attach data; the concepts and
   the checking of them; the pattern of the algorithm classes (named
   template parameters, `run()`, `init()`/`start()`, queries); `Tolerance`
   and the numeric traits. Headers: `core.h` (the items and the basic
   utilities, with the details in chapter 4), `concept_check.h`,
   `tolerance.h`, `math.h`, `bits/traits.h` as needed.

## Part II — Data structures

4. **Graph structures** (`04-graphs.md`). `list_graph.h` (ListDigraph,
   ListGraph, ListBpGraph, Snapshot), `smart_graph.h`, `static_graph.h`,
   `compact_graph.h`, `full_graph.h`, `grid_graph.h`, `hypercube_graph.h`,
   `edge_set.h` (ListArcSet, ListEdgeSet, SmartArcSet, SmartEdgeSet), and
   the utilities of `core.h` (counting, `findArc`, ConArcIt, ArcLookUp,
   AllArcLookUp, DynArcLookUp, `digraphCopy`/`graphCopy`/`bpGraphCopy`);
   the concepts of `concepts/digraph.h`, `concepts/graph.h`,
   `concepts/bpgraph.h`, `concepts/graph_components.h`.
5. **Graph adaptors** (`05-adaptors.md`). `adaptors.h`: ReverseDigraph,
   SubDigraph, SubGraph, FilterNodes, FilterArcs, FilterEdges,
   Undirector, Orienter, ResidualDigraph, SplitNodes, and their map
   classes (CombinedArcMap, ResidualCapacity, ...).
6. **Maps** (`06-maps.md`). `maps.h` (constant, identity, range and
   sparse maps, the map adaptors and arithmetic, the iterable maps, the
   look-up maps CrossRefMap, InverseMap, RangeIdMap, the degree maps),
   `concepts/maps.h`, and the graph maps of `bits/` as far as a user sees
   them.
7. **Heaps** (`07-heaps.md`). `bin_heap.h`, `quad_heap.h`, `dheap.h`,
   `fib_heap.h`, `pairing_heap.h`, `binomial_heap.h`, `radix_heap.h`,
   `bucket_heap.h` (BucketHeap, SimpleBucketHeap), `concepts/heap.h`.
8. **Paths and auxiliary structures** (`08-auxiliary.md`). `path.h`
   (Path, SimplePath, ListPath, StaticPath, the path utilities),
   `concepts/path.h`, `unionfind.h`, `elevator.h`, `dim2.h`,
   `radix_sort.h`, `random.h`, `counter.h`, `time_measure.h`, `color.h`,
   `error.h`, `assert.h`, `arg_parser.h`.

## Part III — Algorithms

9. **Graph search** (`09-search.md`). `bfs.h`, `dfs.h` (and their
   visitors and wizards), `max_cardinality_search.h`.
10. **Shortest paths** (`10-shortest-paths.md`). `dijkstra.h`,
    `bidir_dijkstra.h`, `astar.h`, `bellman_ford.h`, `suurballe.h`.
11. **Spanning trees and arborescences** (`11-spanning.md`). `kruskal.h`,
    `min_cost_arborescence.h`.
12. **Maximum flows and circulations** (`12-max-flow.md`). `preflow.h`,
    `edmonds_karp.h`, `circulation.h`, `concepts/max_flow.h`.
13. **Minimum cost flows** (`13-min-cost-flow.md`). `network_simplex.h`
    (and its pivot rules), `cost_scaling.h`, `capacity_scaling.h`,
    `cycle_canceling.h`, `concepts/min_cost_flow.h`, the supply types and
    the problem types of `doc/min_cost_flow.dox`, the tolerances of
    `bits/mcf_tolerance.h` and the feasible flow of
    `bits/mcf_feasible_flow.h`.
14. **Certificates and the k best flows** (`14-k-best-flows.md`). The
    certificates of infeasibility and of unboundedness of the four
    algorithms (`bits/mcf_certificate.h`, `infeasibleCut()`,
    `infeasibleCuts(k)`, `unboundedCycles(k)`), the alternative optimal
    potentials, `k_best_flows.h`, `k_best_extreme_flows.h`.
15. **Minimum cuts** (`15-min-cut.md`). `hao_orlin.h`,
    `nagamochi_ibaraki.h`, `gomory_hu.h`.
16. **Minimum mean cycles** (`16-min-mean-cycle.md`). `karp_mmc.h`,
    `hartmann_orlin_mmc.h`, `howard_mmc.h`, `concepts/min_mean_cycle.h`.
17. **Matchings** (`17-matching.md`). `matching.h` (MaxMatching,
    MaxWeightedMatching, MaxWeightedPerfectMatching),
    `bipartite_matching.h` (HopcroftKarp, MinCostAssignment),
    `fractional_matching.h`.
18. **Connectivity, planarity, isomorphism** (`18-properties.md`).
    `connectivity.h`, `euler.h`, `planarity.h`, `vf2.h`, `vf2pp.h`.
19. **Travelling salesman and maximum clique** (`19-tsp-clique.md`).
    `nearest_neighbor_tsp.h`, `greedy_tsp.h`, `insertion_tsp.h`,
    `christofides_tsp.h`, `opt2_tsp.h`, `grosso_locatelli_pullan_mc.h`,
    `tomita_seki_mc.h`.
20. **The LP and MIP interface** (`20-lp.md`). `lp_base.h`, `lp.h`,
    `lp_skeleton.h`, `glpk.h`, `clp.h`, `cbc.h`, `cplex.h`, `soplex.h`.

## Part IV — Input, output and tools

21. **Input and output** (`21-io.md`). `lgf_reader.h`, `lgf_writer.h`
    (with the format of `doc/lgf.dox`), `dimacs.h`, `nauty_reader.h`,
    `graph_to_eps.h`.
22. **The tools** (`22-tools.md`). `tools/`: dimacs-solver,
    dimacs-to-lgf, lgf-gen, lemon-0.x-to-1.x.sh; the demos of `demo/`.

## Part V — Appendices

- **A. Writing a graph, a map or a heap** (`A-writing-graph-map.md`). The
  concepts as contracts, the extenders of `bits/`, the alteration
  notifier, checking a new class with `checkConcept`.
- **Y. Glossary** (`Y-glossary.md`).
- **Z. Bibliography** (`Z-bibliography.md`).
