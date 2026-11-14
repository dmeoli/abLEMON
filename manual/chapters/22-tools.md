# 22. The tools {#ch-22}

This chapter describes the programs distributed with the library: the three
command-line tools of `tools/`, i.e., `dimacs-solver`, `dimacs-to-lgf` and
`lgf-gen`, the script `lemon-0.x-to-1.x.sh`, and the three demos of `demo/`.
These tools are small applications of the classes of the previous chapters,
and their options are read from their sources; we also give, for each of them,
the classes it uses, so that one can reproduce its work in a program (as the
example of [§22.6](22-tools.md#sec-22-6) does for two of them).

## 22.1 Building and calling the tools {#sec-22-1}

By default CMake builds the tools, when the option `LEMON_BUILD_TOOLS` is on,
and installs them into the directory of the executables (`bin` under the
prefix, by `GNUInstallDirs`); the script `lemon-0.x-to-1.x.sh` is installed
with them, except on Windows. Similarly, the demos are built when
`LEMON_BUILD_DEMOS` is on, also by default, but they are not installed. Both
can also be built with GNU make, through the makefiles of `tools/` and
`demo/`, without a configuration step ([chapter 2](02-installation.md#ch-2)).

All three tools parse their command line with `ArgParser` ([chapter
8](08-auxiliary.md#ch-8)), and hence they share its conventions. An option is
a word preceded by a single dash, e.g., `-q` or `-infcap 100`, the options of
a group marked as exclusive may not be given together, and `-h`, `-help` and
`--help` print the list of the options with their descriptions and exit. The
words that are not options are the file arguments; in `dimacs-solver` and
`dimacs-to-lgf` they are an optional input file and an optional output file,
which default to the standard input and output, and more than two of them is
an error. Since the input file is read once and the problem line comes first,
both tools also read from a pipe.

## 22.2 dimacs-solver {#sec-22-2}

Our first tool, `dimacs-solver`, reads a problem in one of the DIMACS formats
of [§21.5](21-io.md#sec-21-5) and solves it with the algorithm of the library
for that type, reporting the times of the reading, of the construction of the
algorithm object, and of the run, and it writes the solution to the output
file, or to the standard output when no output file is given. The reports go
to the standard error, so that the standard output holds the solution alone.

| Option | Meaning | Default |
|-----------|----------------------------|----------------------------|
| `-q` | print no report | off |
| `-int` | `int` for capacities, costs and supplies | on |
| `-long` | `long long` (only where the compiler has it) | off |
| `-double` | `double` | off |
| `-ldouble` | `long double` | off |
| `-infcap v` | the value of an infinite capacity ([§21.5](21-io.md#sec-21-5)) | `0`, i.e., the infinity or the maximum of the type |

<span id="tab-22-1"></span>**Table 22.1.** The options of `dimacs-solver`; the
four options of the type form an exclusive group.

At the start, the tool reads the problem line by `dimacsType()`, and, unless
`-q` is given, writes the type of the problem and the numbers of nodes and
arcs; a file that cannot be opened, or of unknown type, ends the program with
a message and the exit status 1. Then it builds a `SmartDigraph` (or a
`SmartGraph` for the type `mat`) and proceeds by type, as follows. For a `sp`
file, it reads the digraph and the lengths by `readDimacsSp()` and computes
the shortest paths from the source by `Dijkstra` ([chapter
10](10-shortest-paths.md#ch-10)), and writes a line `d v d(v)` for each node
`v` reached. For a `max` file, it reads the capacities by `readDimacsMax()`,
computes a maximum flow by `Preflow` ([chapter 12](12-max-flow.md#ch-12)), and
writes its value as `s value` and a line `f u v x` for each arc $(u,v)$ with a
nonzero flow $x$. For a `mat` file, it reads the graph by `readDimacsMat()`,
computes a maximum matching by `MaxMatching` ([chapter
17](17-matching.md#ch-17)), and writes its cardinality as `s k` and a line
`m u v` for each matched edge. For a `min` file, it reads the problem by
`readDimacsMin()` and solves it by `NetworkSimplex` ([chapter
13](13-min-cost-flow.md#ch-13)), as in Algorithm 22.1, printing whether a
feasible flow was found and, unless the problem is infeasible, its cost, which
is computed in a wider type (`long long` when the values are `int`, the type
of the values otherwise); it writes the cost as `s cost` and a line `f u v x`
for each arc with a nonzero flow, or `s INFEASIBLE` or `s UNBOUNDED`. The
nodes are numbered from 1, as in the input file.

<span id="alg-22-1"></span>**Algorithm 22.1** (The minimum cost flow problem
in `dimacs-solver`).

```
1  read lower, capacity, cost, supply (readDimacsMin, infinite = -infcap)
2  sigma := sum of the supplies
3  eps := the data tolerance of NetworkSimplex for the largest magnitude
          of the supplies, of the lower bounds and of the finite capacities
4  if |sigma| <= eps: sigma := 0
5  if sigma > 0: supply type LEQ, else GEQ (the default of NetworkSimplex)
6  run NetworkSimplex; report "found" or "not found" and the total cost,
   and write the cost and the flows (or the cause) to the output
```

Note that the choice of line 5 follows from the meaning of the two supply
types of [chapter 13](13-min-cost-flow.md#ch-13). With a total supply larger
than the total demand the problem can only be feasible if some supply is left
unused, which the type `LEQ` allows; with a smaller one, it needs the type
`GEQ`, under which some demand stays unsatisfied, and with a zero sum the two
types are equivalent to the equations. Lines 3 and 4 treat as zero a sum that
only the rounding of fractional supplies keeps away from zero, with the
tolerance that `NetworkSimplex` itself uses to balance the supplies ([chapter
13](13-min-cost-flow.md#ch-13)).

!!! note "abLEMON"
    In the version after 1.9.0, `dimacs-solver` takes as zero a sum of the
    supplies within the tolerance of `NetworkSimplex`; before, a sum of
    $10^{-12}$ left by fractional supplies of a `double` instance made it
    choose and announce the type `LEQ`. Also, in LEMON 1.3.1 it wrote no
    solution (the output file, opened when given, stayed empty), wrote the
    first lines of its report to the standard output, and ended with an
    uncaught exception on a file of unknown type.

## 22.3 dimacs-to-lgf {#sec-22-3}

Next, `dimacs-to-lgf` converts a DIMACS file into an LGF file
([§21.1](21-io.md#sec-21-1)), and it has no option besides the input and the
output file. It reads the problem by `dimacsType()` and by the reader of its
type, all values being of type `double`, into a `SmartDigraph`, and it writes
the digraph by `DigraphWriter` with the maps and attributes of Table 22.2; the
nodes are labeled by their ids, i.e., by the numbers of the DIMACS file minus
one.

| Type | Node maps | Arc maps | Attributes |
|-----|---------|---------------------------|----------------------------|
| `min` | `supply` | `lower`, `capacity`, `cost` | `problem` = `min` |
| `max` | | `capacity` | `source`, `target`, `problem` = `max` |
| `sp` | | `capacity` (the lengths) | `source`, `problem` = `sp` |
| `mat` | | | `problem` = `mat` |

<span id="tab-22-2"></span>**Table 22.2.** The contents of the LGF file
written by `dimacs-to-lgf`, by type of the DIMACS file.

Two remarks are in order. First, the infinite capacities of a `min` or `max`
file are written as the infinity of `double`, i.e., as the token `inf`;
whether a program can read this token back depends on the `operator>>` of its
standard library, and it may need a converter ([§21.2](21-io.md#sec-21-2)).
Second, the values are written with the default conversion, i.e., with 6
significant digits ([§21.3](21-io.md#sec-21-3)), which is exact for the
integer data of the usual DIMACS instances below $10^6$ but not for larger or
fractional values. As in `dimacs-solver`, a file of unknown type makes
`dimacsType()` throw `FormatError`, which ends the tool.

## 22.4 lgf-gen {#sec-22-4}

Finally, `lgf-gen` generates random plane graphs, i.e., graphs whose nodes are
points of the plane, and writes them as LGF files with the coordinates of the
nodes, optionally with an EPS drawing. It is a convenient source of test
instances for the geometric and graph algorithms of the library.

| Option | Meaning | Default |
|----------------------------|----------------------------|----------------|
| `-n N` | number of nodes | 100 |
| `-disc`, `-square`, `-gauss` | the nodes uniformly in the unit disc, uniformly in the square $[-1,1]^2$, or by a two-dimensional Gaussian distribution | `-disc` |
| `-cities k` | the nodes in $k$ clusters, with random centres and sizes | 1 |
| `-area a` | the scale of the clusters (their full relative area) | 1 |
| `-2con`, `-tree`, `-tsp`, `-tsp2`, `-dela` | the edges: a 2-connected planar graph, a minimum spanning tree, a TSP tour, a tree-based TSP tour, the Delaunay triangulation | no edges |
| `-g d` | the girth parameter of `-2con` | 10 |
| `-dir` | write a digraph, each edge as two opposite arcs | off |
| `-eps` | also write `prefix.eps` | off |
| `-nonodes` | draw only the edges in the EPS file | off |
| `-rand`, `-seed s` | seed the generator with the time, or with `s` | the default seed |

<span id="tab-22-3"></span>**Table 22.3.** The options of `lgf-gen`; the
distributions, the algorithms and the two ways of seeding form three exclusive
groups. The only file argument is the prefix of the output files,
`lgf-gen-out` by default.

Thus, the nodes are placed first. With $k$ clusters, each cluster $s$ receives
a random weight $w_s$, uniform in $[0,1)$, and a share of the $N$ nodes
proportional to it; its centre is a random point of the unit disc (the origin
when $k=1$), and its points are drawn by the chosen distribution and scaled by
$a\sqrt{w_s/\sum_r w_r}$. Then the edges are built by the chosen algorithm,
the lengths being the Euclidean distances, as follows.

<span id="alg-22-2"></span>**Algorithm 22.2** (The edges of `lgf-gen`).

```
-dela : the Delaunay triangulation of the points (delaunay() of the tool)
-tree : the Delaunay triangulation; keep the spanning tree that
        kruskal() finds on the squared lengths
-tsp  : the cycle of the nodes in the order of creation, improved by
        2-opt moves on the lengths until none applies
-tsp2 : the tree of -tree; join its nodes of odd degree in pairs, by
        increasing distance; shortcut the Euler tour of the result to
        a tour; improve it by 2-opt
-2con : for each pair of nodes by increasing length: add the edge if it
        crosses no edge already added and the two nodes do not have two
        edge-disjoint paths of total length at most d (Suurballe, unit
        lengths); then remove, in reverse order, each edge whose ends
        keep two such paths without it
```

The algorithm `-2con` thus gives a plane graph, since no two edges cross, and
$d$ is the value of `-g`; `Suurballe` is the algorithm of [chapter
10](10-shortest-paths.md#ch-10). At the end, the tool prints the number of
nodes and edges and the total length of the edges, and, for `-tsp` and
`-tsp2`, the number of 2-opt moves. It writes `prefix.lgf`, by `GraphWriter`,
or by `DigraphWriter` with `-dir`; the only node maps are `coordinates_x` and
`coordinates_y`, the coordinates multiplied by 600. With `-eps`, it also
writes `prefix.eps` by `graphToEps()` ([§21.7](21-io.md#sec-21-7)), scaled to
an A4 page.

## 22.5 lemon-0.x-to-1.x.sh and the demos {#sec-22-5}

We now turn to `lemon-0.x-to-1.x.sh`, which helps to port a program written
for the old series 0.x of LEMON: it rewrites in place each file given as an
argument by a long sequence of `sed` substitutions, and with no argument, or
with `-h`, `-help` or `--help`, it prints its usage. The substitutions rename
the graph concepts of the old series: a graph becomes a digraph, an undirected
graph a graph, an edge an arc and an undirected edge an edge, and the `A`- and
`B`-nodes of the bipartite graphs become red and blue nodes. They also rename
the headers (e.g., `digraph_adaptor.h` becomes `adaptors.h` and `topology.h`
becomes `connectivity.h`), the named template parameters `Def*` into `Set*`,
the adaptors into their current names (e.g., `SubDigraphAdaptor` becomes
`SubDigraph` and `ResDigraphAdaptor` becomes `ResidualDigraph`), several maps
(e.g., `InvertableMap` becomes `CrossRefMap`), and the LP classes (`LpGlpk`
becomes `GlpkLp`, and so on). Since the substitutions are textual, the result
must be read and compiled; in particular, the words *graph* and *edge* in the
comments are renamed too.

Each of the three demos of `demo/` is a short program that shows the use of
one tool of the library. First, `arg_parser_demo` declares options of all the
kinds of `ArgParser` (a mandatory integer `-n`, doubles with and without a
reference, a synonym, a string, switches, an exclusive and mandatory group of
three options and the file arguments), parses the command line with
`throwOnProblems()`, and prints what it found. Then `graph_to_eps_demo` builds
a small digraph and writes seven EPS files, `graph_to_eps_demo_out_1_pure.eps`
to `graph_to_eps_demo_out_7_colors.eps`, which show the plain drawing, the
sizes, shapes, colors and widths given by maps, the arrowheads, the parallel
arcs, the fit to an A4 page and, on a second graph, the colors of a `Palette`
with the texts of the nodes in distant colors. Finally, `lgf_demo` reads the
digraph, the capacities and the source and target of the file `digraph.lgf` of
the directory of the demos, which it must be started from, and writes them
back on the standard output.

## 22.6 An example {#sec-22-6}

Our program does, on a small minimum cost flow problem, what two of the tools
do. It declares the options of `dimacs-solver` and parses a given command line
with them, reads a DIMACS `min` file from a string with an arc of infinite
capacity, chooses the supply type by the sign of the sum of the supplies as in
Algorithm 22.1, and solves the problem by `NetworkSimplex`. Then it writes the
problem as `dimacs-to-lgf` writes it, reads the LGF text back, and checks the
costs and the attribute `problem`.

```cpp
--8<-- "examples/22-tools.cc"
```

## 22.7 Exercises {#sec-22-7}

<span id="ex-22-1"></span>**Exercise 22.1.** Generate a Delaunay triangulation
with `lgf-gen -dela -n 1000`, read it by `GraphReader` with the coordinates,
and check that its number of edges is at most $3N-6$, as for any simple plane
graph (and that `PlanarityChecking` of [chapter 18](18-properties.md#ch-18)
accepts it).

<span id="ex-22-2"></span>**Exercise 22.2.** Generate a TSP tour with
`lgf-gen -tsp` and with `lgf-gen -tsp2` on the same seed, and compare their
lengths with the tours of the classes of [chapter 19](19-tsp-clique.md#ch-19)
on the same points (read by `GraphReader`, with the costs given by the
coordinates).

<span id="ex-22-3"></span>**Exercise 22.3.** Write a `min` file whose supplies
sum to a positive value, and check that `dimacs-solver` announces the type
`LEQ` and finds the same cost as a program that solves the problem by
`NetworkSimplex` with this supply type (`manual/examples/22-tools.cc`).

<span id="ex-22-4"></span>**Exercise 22.4.** Convert a `max` file with an arc
of negative capacity by `dimacs-to-lgf`, and read the result back by
`DigraphReader` with a converter that maps the token `inf` to a large finite
capacity; check that `Preflow` gives the same value as `dimacs-solver` on the
original file.

<span id="ex-22-5"></span>**Exercise 22.5.** Run `arg_parser_demo` without the
mandatory option `-n`, with two options of the group `-gra`, `-grb`, `-grc`,
and with none of them, and explain each message from the declarations of the
demo.
