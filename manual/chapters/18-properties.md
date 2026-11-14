# 18. Connectivity, planarity, isomorphism {#ch-18}

This chapter collects the algorithms that decide a structural property of a
graph, or compute the object that certifies it, instead of optimizing a
weight. They are the components of a graph with the nodes, edges and arcs that
separate them, the tests of acyclicity and the topological order
(`lemon/connectivity.h`), the Euler tours (`lemon/euler.h`), the planarity
test with its embedding, its straight-line drawing and the colorings of a
planar graph (`lemon/planarity.h`), and the search for an isomorphic copy of a
graph inside another (`lemon/vf2.h`, `lemon/vf2pp.h`). All of them read only
the structure of the graph (and, for the isomorphism, two node labelings), and
most of them run in time linear in its size (which, on the graphs met in
practice, makes them rather cheap compared with the optimization algorithms of
the previous chapters). We remark two exceptions: the five coloring, which is
quadratic in the worst case, and the isomorphism search, which solves an
NP-complete problem and may take exponential time. We write $G=(V,E)$ for an
undirected graph and $D=(V,A)$ for a digraph, with $n=|V|$ and $m=|E|$ or
$m=|A|$. We use the iterators, the maps and the graph structures of [chapter
3](03-mental-model.md#ch-3) and [chapter 4](04-graphs.md#ch-4), while the
depth-first and breadth-first searches, on which most functions are built, are
those of [chapter 9](09-search.md#ch-9).

## 18.1 Connected components, acyclicity and simple graphs {#sec-18-1}

Two nodes of an undirected graph are *connected* if a path joins them. This is
an equivalence relation, whose classes are the *connected components*, and a
graph is connected if it has one component (by convention, the graph without
nodes is connected and has zero components, as the documentation of each
function states). Three functions compute the components, i.e.,
`bool connected(g)`, `int countConnectedComponents(g)` and
`int connectedComponents(g, compMap)`. Of these, `connected()` runs one
depth-first search from the first node and checks that it has reached all the
nodes, while `countConnectedComponents()` starts a breadth-first search from
each node not yet reached and counts the searches. In the same way,
`connectedComponents()` writes in the writable node map `compMap` the index of
the component of each node, from $0$ to the number of components minus one,
and returns that number; the nodes of one component receive their index
consecutively, in the order in which the search reaches them. Each function
costs $O(n+m)$, i.e., the cost of one search over the graph.

A graph is *acyclic* (a forest) if it contains no cycle, and it is a *tree*
if, furthermore, it is connected. In particular, `acyclic(g)` runs a
depth-first search from each unreached node, and it returns `false` at the
first arc that leads to a reached node without being the opposite of the arc
by which the search entered the current node. Hence, two parallel edges form a
cycle of length two for it, and a loop a cycle of length one. Similarly,
`tree(g)` makes the same test from the first node only and then checks that
all the nodes have been reached; thus, the empty graph is a tree.

A graph is *bipartite* if its nodes can be split into two sets with no edge
inside either of them, i.e., if it has no cycle of odd length. One decides it
with `bipartite(g)`, while `bool bipartitePartitions(g, partMap)` also writes
the two sides into the writable `bool` node map `partMap` (`true` on the side
of the first node of each component). Both run a breadth-first search that
puts each discovered node on the side opposite to that of its parent, and they
stop with `false` as soon as an edge joins two nodes of the same side. In that
case `partMap` has been written only on the nodes reached so far, and its
values are of no use. Both functions cost $O(n+m)$.

Finally, three functions test whether a graph is simple, and they apply to
digraphs and to undirected graphs alike. Here `loopFree(g)` returns `true` if
no arc has the same source and target, `parallelFree(g)` if no two arcs that
leave a node enter the same node, and `simpleGraph(g)` if both conditions
hold. The latter two mark the targets of the arcs leaving a node with the
index of that node in a node map; hence, they cost $O(n+m)$ and need no
sorting. Several algorithms of this chapter (cf. [§18.5](#sec-18-5) and
[§18.6](#sec-18-6)) assume a simple graph (a graph read from a file, e.g., may
well have parallel edges), and `simpleGraph()` is the check to run before
them.

## 18.2 Biconnectivity {#sec-18-2}

A connected graph may be disconnected by the removal of one node or of one
edge, and the parts that survive such removals measure how "reliable" its
connections are (e.g., those of a communication network). A *cut node* (or
articulation node) is a node whose removal increases the number of connected
components, and a *cut edge* (or bridge) is an edge with the same property.
Two edges are in the same *bi-node-connected component*, or *block*, if they
lie on a common cycle; this is an equivalence relation on the edges, whose
classes are separated by the cut nodes. Two nodes are in the same
*bi-edge-connected component* if two edge-disjoint paths join them; this is an
equivalence relation on the nodes, whose classes are separated by the bridges.
For instance, the graph of [Figure 18.1](#fig-18-1) has four blocks (the
triangles A and B, which share the cut node 2, the bridge C and the triangle
D), three cut nodes and two bi-edge-connected components.

![Figure 18.1. A connected graph with four blocks A, B, C and D. The cut nodes 2, 4 and 5 are in the primary colour, the bridge 4-5 in the colour of the added items, and the two bi-edge-connected components, which the bridge separates, are the shaded regions.](../figures/18-components.svg){#fig-18-1}

The header provides, for the nodes and for the edges, the same kinds of
function as for the connected components. For the blocks they are
`bool biNodeConnected(g)`, `int countBiNodeConnectedComponents(g)`,
`int biNodeConnectedComponents(g, compMap)` and
`int biNodeConnectedCutNodes(g, cutMap)`. For the bi-edge-connected components
they are `bool biEdgeConnected(g)`, `int countBiEdgeConnectedComponents(g)`,
`int biEdgeConnectedComponents(g, compMap)` and
`int biEdgeConnectedCutEdges(g, cutMap)`. Note that the map of the blocks is
an edge map, since a block is a set of edges and a cut node belongs to several
blocks, whereas the map of the bi-edge-connected components is a node map;
both are numbered from $0$ as in [§18.1](#sec-18-1). Those that find the cut
nodes and the cut edges set `cutMap` to `true` on them (once each) and return
their number, but they leave the other items untouched, and the caller must
therefore initialize the map to `false` when it reads the other items too.
The map is only written, as its concept check requires: in LEMON 1.3.1
`biNodeConnectedCutNodes()` read it in order not to count a node twice, so
that a node already `true` on entry was not counted (the test
`connectivity_test` passes a map that is `true` everywhere).

At the boundary we follow the conventions of the header. A graph with zero or
one node is bi-node-connected, two isolated nodes are not, and two nodes
joined by an edge are. In general, `biNodeConnected()` returns `false` if the
graph has an isolated node together with some other node, and otherwise
whether it has at most one block. On the other hand, `biEdgeConnected()`
returns whether there is at most one bi-edge-connected component; hence, the
empty graph is bi-edge-connected. A loop forms a separate block, and
`biNodeConnectedCutNodes()` reports the node that carries it as a cut node.

All these functions rely on one depth-first search, with the low points of
[Hopcroft and Tarjan 1973](Z-bibliography.md#ref-hopcroft1973algorithm447) and
[Tarjan 1972](Z-bibliography.md#ref-tarjan1972depth). We denote by
$\mathrm{num}(v)$ the index of $v$ in the order of discovery, and by
$\mathrm{low}(v)$ the least index of a node that one can reach from $v$ by
going down the tree of the search and then following at most one non-tree
edge.

<span id="lem-18-1"></span>**Lemma 18.1.** *In a depth-first search of a
connected graph, the root is a cut node if and only if it has at least two
children in the tree, and any other node $u$ is a cut node if and only if it
has a child $w$ with $\mathrm{low}(w)\ge\mathrm{num}(u)$. The tree edge $uw$,
with $u$ the parent of $w$, is a bridge if and only if
$\mathrm{low}(w)>\mathrm{num}(u)$, where the low point is computed without
using the edge $uw$.*

Its proof rests on the fact that each non-tree edge of a depth-first search of
an undirected graph joins a node to one of its ancestors; thus, the subtree of
$w$ is attached to the rest of the graph only through $u$ and through the
non-tree edges that leave it (see [Tarjan
1972](Z-bibliography.md#ref-tarjan1972depth)). In the library, the visitor of
the cut nodes applies the lemma as we show next.

<span id="alg-18-1"></span>**Algorithm 18.1** (Cut nodes by the low points).

```
1  for each node r not yet reached: root := r; rootCut := false; DFS(r)
2  DFS(v): num(v) := low(v) := next index
3     for each arc vw:
4        if w is not reached: pred(w) := v; DFS(w)
5           low(v) := min(low(v), low(w))
6           if num(v) <= low(w):
7              if v != root or rootCut: mark v as a cut node
8              else rootCut := true
9        else if w != pred(v): low(v) := min(low(v), num(w))
```

Step 7 marks the root at the second of its children that satisfy the condition
of step 6, the flag `rootCut` recording the first one; since all the children
of the root satisfy that condition, this is the rule of Lemma 18.1. Similarly,
the count of the blocks grows by one at each tree edge that satisfies the
condition of step 6, which closes a block, and the function that numbers the
blocks keeps on a stack the edges discovered since the block was opened. Step
9 skips all the edges to the parent, which does not change the blocks. In
contrast, the functions on the bi-edge-connected components store the arc by
which each node was entered and skip only its opposite, since a second edge
parallel to a tree edge does give a path back and prevents that edge from
being a bridge. There, a node $v$ with $\mathrm{low}(v)=\mathrm{num}(v)$
closes a component, and the arc that entered it is a bridge. Each function
costs $O(n+m)$.

## 18.3 Strong components and topological order {#sec-18-3}

Two nodes of a digraph are *strongly connected* if directed paths join them in
both directions. This is an equivalence relation, whose classes are the
*strongly connected components*, and the digraph that has one node per
component, and an arc from one component to another whenever some arc of $D$
goes so, is acyclic (we call it the *condensation* of $D$). The library offers
`bool stronglyConnected(d)`, `int countStronglyConnectedComponents(d)`,
`int stronglyConnectedComponents(d, compMap)` and
`int stronglyConnectedCutArcs(d, cutMap)`, with the conventions of
[§18.1](#sec-18-1), i.e., the empty digraph is strongly connected and has zero
components. Of these, `stronglyConnected()` runs a depth-first search from the
first node on $D$ and another on its reverse (the adaptor `ReverseDigraph` of
[chapter 5](05-adaptors.md#ch-5)), and it checks that both searches reach all
the nodes. Instead, the other three functions use the algorithm usually
attributed to Kosaraju and published by [Sharir
1981](Z-bibliography.md#ref-sharir1981strong).

<span id="alg-18-2"></span>**Algorithm 18.2** (Strongly connected components
by two depth-first searches).

```
1  run a DFS of D from each node not yet reached, and list the nodes
   in the order in which the search leaves them
2  k := 0
3  for each node v in the reverse of that list:
4     if v is not reached by the second search:
5        run a DFS of the reverse of D from v; give index k to the nodes
         it reaches; k := k + 1
6  return k
```

The node that the first search leaves last lies in a component that no arc
enters from another component, and in the reverse digraph its search cannot
leave that component; by induction, each search of step 5 collects one
component (see, e.g., [Cormen et al. 2001,
§22.5](Z-bibliography.md#ref-clrs01algorithms)). Thus, the components are
found in a topological order of the condensation, and the header guarantees
accordingly that no arc goes from a component of higher index to one of lower
index. In turn, the cut arcs of `stronglyConnectedCutArcs()` are the arcs
whose ends lie in different components, i.e., the arcs of the condensation.
This function sets them to `true` in `cutMap`, leaves the other arcs untouched
(the map must start at `false`), and returns their number. Each of the four
functions costs $O(n+m)$.

A digraph without directed cycles is a *DAG*, and a *topological order* of a
DAG is a numbering of its nodes in which each arc goes from a lower to a
higher number. Three functions deal with them, i.e., `bool dag(d)`,
`void topologicalSort(d, order)` and `bool checkedTopologicalSort(d, order)`.
The first one runs a depth-first search from each unreached node and returns
`false` at the first arc whose target has been reached but not yet left (a
back arc, which closes a cycle; a loop is one). Next, `topologicalSort()`
writes in `order` the values from $n-1$ down to $0$, in the order in which a
depth-first search leaves the nodes; hence, `order[u] < order[v]` holds for
each arc $uv$ of a DAG. It does not check that the digraph is a DAG, which is
its precondition. By contrast, `checkedTopologicalSort()` does check it, and
it needs a map that can also be read: it first sets all the values to $-1$,
and it returns `false` at the first arc that leads to a node reached and still
at $-1$, leaving the map partly written. All three cost $O(n+m)$. One usually
runs them, or the strongly connected components, before an algorithm that
assumes a DAG or a strongly connected digraph; for instance, the minimum mean
cycle algorithms of [chapter 16](16-min-mean-cycle.md#ch-16) call
`stronglyConnectedComponents()` and then work on one component at a time.

In the example we build the graph of [Figure 18.1](#fig-18-1) and check the
numbers of its components and of its cut items. We then build a digraph with
three strongly connected components, whose numbering we check to be a
topological order of the condensation, and we sort a "small" DAG.

```cpp
--8<-- "examples/18-components.cc"
```

## 18.4 Euler tours {#sec-18-4}

An *Euler tour* of a graph is a closed walk that traverses each edge once (for
a digraph, each arc in its direction). We recall its characterization, which
is the oldest theorem of graph theory.

<span id="thm-18-1"></span>**Theorem 18.1.** *A graph whose edges lie in one
connected component has an Euler tour if and only if each node has even
degree. A digraph whose arcs lie in one connected component of the underlying
undirected graph has an Euler tour if and only if each node has as many
entering arcs as leaving ones.*

Indeed, the proof of sufficiency, due to [Hierholzer and Wiener
1873](Z-bibliography.md#ref-hierholzer1873moglichkeit), is also the algorithm.
From any node, a walk that never reuses an edge can stop only at its starting
node, since each other node it enters has an unused edge to leave by; if the
closed walk so found misses some edge, one of its nodes has an unused edge,
and a closed walk from that node can be spliced into the first one.

In abLEMON the tour is given by an iterator, `DiEulerIt< GR >` for a digraph
and `EulerIt< GR >` for an undirected graph. It is constructed from the graph
and, optionally, from the starting node (by default, the first node that has a
leaving arc), and it is advanced with `++` until it compares equal to
`INVALID`. `DiEulerIt` converts to `GR::Arc`, while `EulerIt` converts to
`GR::Arc` and to `GR::Edge`, and it gives arcs in order to show in which
direction each edge is traversed. As for the other iterators of the library,
the postfix `++` returns the arc instead of the iterator.

<span id="alg-18-3"></span>**Algorithm 18.3** (Euler tour, as `DiEulerIt`
produces it).

```
1  for each node v: next(v) := first arc leaving v
2  L := empty list; s := start
3  while next(s) != INVALID: append next(s) to L; advance next(s);
                             s := target of the arc appended
4  (operator*) the current arc is the first of L
5  (operator++) s := target of the first arc of L; remove it from L;
6     while next(s) != INVALID: insert next(s) at the front of the rest
      of L, after the arcs inserted before; advance next(s);
      s := target of that arc
```

The constructor walks greedily from the start (step 3), and each increment
splices, right after the arc just given, the closed walk that starts where
that arc ends and uses the arcs not yet taken (step 6). Each arc enters the
list once and the pointer `next(v)` of a node only advances; hence, the whole
tour costs $O(n+m)$, and the list holds the part of the tour not yet given.
`EulerIt` works in the same way with an edge map that marks the edges already
used, since each edge appears as an arc leaving each of its two ends. If the
graph has no Euler tour, the iterators still give a walk (possibly a rather
short one), which then is not closed or does not contain all the edges. One
should therefore check the property first, with `bool eulerian(g)`, which
works on digraphs and on undirected graphs (the two versions are selected
through the undirected tag of the graph type). On an undirected graph it
returns `true` if each node has even degree (`countIncEdges()`) and the graph
is connected. On a digraph it returns `true` if each node has as many entering
as leaving arcs and the underlying undirected graph (the adaptor `Undirector`)
is connected. We remark that `eulerian()` requires that all the nodes be
connected, whereas Theorem 18.1 requires it of the edges only; thus, a graph
with an Euler tour and an isolated node is not Eulerian for the function, as
its documentation warns and as the example below checks.

```cpp
--8<-- "examples/18-euler.cc"
```

## 18.5 Planarity, embedding and drawing {#sec-18-5}

We recall that a graph is *planar* if it can be drawn in the plane with no two
edges crossing. Up to deformation, such a drawing is described by its
*combinatorial embedding*, i.e., by the cyclic order of the edges around each
node (counterclockwise in the drawing). In turn, the *faces* of the drawing
are the orbits of the map that takes an arc $uv$ to the arc that follows $vu$
around $v$, and a connected plane graph has $f=m-n+2$ faces (Euler's formula).
Since each face of a simple plane graph with $n\ge3$ nodes is bounded by at
least three edges, one has $m\le 3n-6$. Hence, a planar simple graph has
$O(n)$ edges and a node of degree at most five, a fact used in
[§18.6](#sec-18-6). Furthermore, the obstacles to planarity are known.

<span id="thm-18-2"></span>**Theorem 18.2.** *A graph is planar if and only if
it contains no subdivision of $K_5$ (the complete graph on five nodes) and no
subdivision of $K_{3,3}$ (the complete bipartite graph with three nodes on
each side).*

This is the theorem of [Kuratowski
1930](Z-bibliography.md#ref-kuratowski1930probleme). A subdivision of either
graph inside a nonplanar graph, called a *Kuratowski subdivision*, certifies
that the graph is not planar, as an embedding certifies that it is.

We now describe the implementation of the algorithm of [Boyer and Myrvold
2004](Z-bibliography.md#ref-boyer2004cutting), which computes in linear time
either an embedding or a Kuratowski subdivision, and which is usually regarded
as one of the simplest linear planarity tests. The function
`bool checkPlanarity(g)` runs a simplified version of it that builds neither
certificate, and the class `PlanarEmbedding< Graph >` runs the full one. Both
expect an undirected simple graph (a precondition of the constructor), which
`simpleGraph()` of [§18.1](#sec-18-1) checks. Once the class is constructed
from the graph, `run(bool kuratowski = true)` returns whether the graph is
planar. If it is, `next(a)` gives the arc that follows the arc `a` in the
cyclic order of the arcs leaving its source, and `embeddingMap()` returns the
whole order as a `const` reference to an arc map of arcs (the type
`EmbeddingMap`, i.e., `Graph::ArcMap< Arc >`). If it is not, and `run()` was
called with `true`, `kuratowski(e)` returns whether the edge `e` belongs to
the subdivision found, whose nodes of degree three or four are those of a
$K_{3,3}$ or of a $K_5$. According to the header, the running time is $O(n)$,
which is the bound for a planar simple graph, whose edges are $O(n)$; on a
nonplanar graph the initial depth-first search already costs $O(n+m)$, after
which the method stops at the first back edge that it cannot embed.

<span id="alg-18-4"></span>**Algorithm 18.4** (Planarity by edge addition,
outline of `PlanarEmbedding::run()`).

```
1  run a DFS; record the order of discovery, the tree edges, the low points
   and, for each node, the least ancestor joined to it by a back edge
2  for each node v in reverse order of discovery:
3     embed each tree edge from v to a child as a separate biconnected
      component, rooted at a virtual copy of v
4     for each back edge from a descendant w to v: walk up from w and mark
      the components that the edge will cross (walkUp)
5     for each marked root: walk down along the external faces, embedding
      the back edges to v and merging the components on the way, flipping
      a component when its orientation requires it (walkDown)
6     if a back edge to v is left unembedded:
         if kuratowski: isolate a Kuratowski subdivision
         return false
7  merge the remaining components and store the cyclic orders; return true
```

We only sketch the invariant of the method, which [Boyer and Myrvold
2004](Z-bibliography.md#ref-boyer2004cutting) prove: after processing $v$, the
tree, together with the back edges to the nodes processed so far, is embedded
with each node that still has a back edge to an unprocessed ancestor on the
external face of its component. Step 6 fails only when no embedding of that
subgraph has this property, and the obstruction found at that point yields the
Kuratowski subdivision. Also, the flips of step 5 are recorded lazily, with
one flag per component, which keeps the method linear.

The class `PlanarDrawing< Graph >` computes a straight-line drawing on an
integer grid by the method of [Schnyder
1990](Z-bibliography.md#ref-schnyder1990embedding). Its coordinates have the
type `dim2::Point< int >` (the type `Point`, stored in the node map type
`PointMap`), all the nodes lie in the square $[0,n-2]\times[0,n-2]$, and the
time is $O(n)$ according to the header. Its interface is the constructor from
a simple graph and the two functions `run()` and `run(embedding)`. Of these,
the former computes an embedding with `PlanarEmbedding` and returns `false` if
the graph is not planar; the latter draws according to a given combinatorial
embedding (e.g., `embeddingMap()` of a `PlanarEmbedding`) and has no return
value. Then the coordinates are read with `operator[](node)` or, as a node
map, with `coords()`. Schnyder's method draws a maximal planar graph (a
triangulation, with $m=3n-6$) from the three trees into which its inner edges
decompose. On any other graph, `run(embedding)` copies the graph into an edge
set and adds edges, consistently with the embedding, to make it connected,
then bi-node-connected and then maximal planar; it draws the triangulation and
keeps the coordinates of the nodes. A graph with fewer than three nodes is
drawn on a vertical line, with $x=0$ and $y=0,1$. The grid is somewhat small
(of side $n-2$), which is convenient for a picture, although some angles
between edges may be quite sharp. [Figure 18.2](#fig-18-2) shows the drawing
of the cube that the example below computes.

![Figure 18.2. The straight-line drawing of the cube computed by `PlanarDrawing` in the example 18-planar.cc: the eight nodes on the grid $[0,6]\times[0,6]$, at the coordinates that the example prints, and no two edges crossing.](../figures/18-cube-drawing.svg){#fig-18-2}

!!! note "abLEMON"
    In LEMON 1.3.1, `PlanarDrawing::run()` without arguments passed the
    `PlanarEmbedding` object, instead of its embedding map, to
    `run(embedding)`, and therefore did not compile once instantiated. abLEMON
    has the correction made in the repository of LEMON after that release
    (trac #610), as well as the drawing of the graphs with fewer than three
    nodes (trac #611), on which the algorithm failed.

## 18.6 Coloring planar graphs {#sec-18-6}

We call *coloring* of a graph an assignment of a color to each node, with
different colors at the ends of each edge. As is well known, every planar
graph can be colored with four colors, by the theorem of [Appel and Haken
1977](Z-bibliography.md#ref-appel1977every), whose proof yields a quadratic
algorithm that the header judges of no practical use. Deciding whether a
planar graph can be colored with three colors is NP-complete, and two colors,
when they suffice, are found by `bipartitePartitions()` of [§18.1](#sec-18-1).
The class `PlanarColoring< Graph >` computes colorings with six and with five
colors, both from the order in which the nodes are removed from the graph when
one always takes a node of least degree among the remaining ones (a
*smallest-last* order).

<span id="lem-18-2"></span>**Lemma 18.2.** *In the smallest-last order of a
planar simple graph, each node has at most five neighbors that are removed
after it.*

Indeed, when a node is removed it has the least degree in the remaining graph,
which is planar and simple, and therefore it has at most five neighbors left,
by the bound $m\le 3n-6$ of [§18.5](#sec-18-5) (with fewer than three nodes
the claim is trivial). Hence, we can give the colors greedily in the reverse
of that order.

<span id="alg-18-5"></span>**Algorithm 18.5** (Coloring of a planar graph with
six or five colors).

```
1  repeatedly remove a node of least degree in the remaining graph
   (a bucket heap keyed by the degree), and record the order
2  for each node v in the reverse of that order:
3     give v the least color in {0,..,k-1} not used by its colored
      neighbors (k = 6 or k = 5)
4     if there is none (k = 5): take the first four colored neighbors
      u1, u2, u3, u4 of v in the cyclic order of the embedding;
5        if the Kempe chain of u1 (the component of the colors of u1, u3
         that contains u1) does not contain u3: swap its two colors,
         and give v the old color of u1
6        else swap the colors in the Kempe chain of u2 (colors of u2 and
         u4), and give v the old color of u2
```

By Lemma 18.2, step 3 cannot fail with six colors on a planar graph. With five
colors it may fail, but only when the (at most five) colored neighbors use all
five colors, and then the two chains of steps 5 and 6 cannot both connect
their ends, since they would cross in the embedding; this is the argument of
[Kempe 1879](Z-bibliography.md#ref-kempe1879geographical), which is correct
for five colors. As for its interface, `PlanarColoring` has the constructor
from a simple graph and three functions. `runSixColoring()` returns `false` if
step 3 fails, and then the graph is not planar (although `true` does not prove
that it is); `runFiveColoring(embedding)` needs a combinatorial embedding and
has no return value; `runFiveColoring()` computes the embedding with
`PlanarEmbedding` and returns `false` on a nonplanar graph. One reads the
results with `colorIndex(node)`, between $0$ and $4$ or $5$, and with
`color(node)`, a `Color` of a palette of six (red, green, blue, yellow,
magenta and cyan, in the order of the indices). As maps, `colorIndexMap()`
returns a const reference to the node map of the indices (the type
`IndexMap`; in LEMON 1.3.1 it returned a copy, made in $O(n)$), and
`colorMap()` the map of the colors (the type `ColorMap`, a `ComposeMap` of the
palette and of the indices). In all, the six coloring costs $O(n+m)$, whereas
the five coloring adds a breadth-first search of a Kempe chain at each failure
of step 3, and its cost is therefore $O(n^2)$ in the worst case, as the header
states.

The example computes the embedding of the cube and counts its faces against
Euler's formula, draws the cube on the grid and checks that no two edges
cross, and colors it with six and with five colors. Finally, it finds the
Kuratowski subdivision of $K_{3,3}$, which coincides with $K_{3,3}$.

```cpp
--8<-- "examples/18-planar.cc"
```

## 18.7 Subgraph isomorphism: VF2 and VF2++ {#sec-18-7}

Given two undirected graphs $G_1=(V_1,E_1)$ and $G_2=(V_2,E_2)$, a *mapping*
(or embedding) is an injective function $f:V_1\to V_2$. In the *Subgraph
Isomorphism Problem* one looks for a mapping with $f(u)f(v)\in E_2$ whenever
$uv\in E_1$, and in the *Induced Subgraph Isomorphism Problem* one requires,
furthermore, that $f(u)f(v)\notin E_2$ whenever $uv\notin E_1$. In the *Graph
Isomorphism Problem* the induced mapping must also be bijective. When the
nodes carry labels $l_1:V_1\to L$ and $l_2:V_2\to L$, one also requires that
$l_1(u)=l_2(f(u))$ for each $u\in V_1$. An enumeration `MappingType` (in the
namespace `lemon`, from `lemon/bits/vf2_internals.h`) selects the problem,
with the values `SUBGRAPH` (the default), `INDUCED` and `ISOMORPH`. According
to the header, `ISOMORPH` is equivalent to `INDUCED` when the two graphs have
the same number of nodes, and to `SUBGRAPH` when they also have the same
number of edges, but it may be faster than both, since its cutting rules are
stronger.

Of course, the subgraph isomorphism problem is NP-complete, since it contains
the clique problem. Hence, the two algorithms of this section are backtracking
searches that may take exponential time, and their speed in practice depends
mostly (and often decisively) on the order in which they consider the nodes of
$G_1$ and on the rules with which they cut the search tree. We now give the
search common to both.

<span id="alg-18-6"></span>**Algorithm 18.6** (VF2 and VF2++ search).

```
1  order the nodes of G1 as v_0, .., v_{n1-1} (VF2: breadth first from
   each unreached node; VF2++: see the text)
2  depth := 0; all the nodes of G1 unmapped
3  while depth >= 0:
4     if depth = n1: report the mapping (find() returns true); depth -= 1
5     u := v_depth; take the next candidate w for u in G2 (a neighbor of
      the image of an already mapped neighbor of u if u has one, else any
      node of G2 compatible with the type), after the one tried last
6     if there is none: unmap u; depth -= 1; continue
7     if feasible(u, w): map u to w; depth += 1
```

The test `feasible(u, w)` checks the label (or the equivalence of the nodes),
and it checks that the edges between $u$ and the mapped nodes correspond, as
the type requires, to the edges between $w$ and their images. It then looks
ahead at the neighbors of $u$ and of $w$ that are adjacent to the mapped part
(and, for `INDUCED` and `ISOMORPH`, at those that are not): their numbers,
precomputed for $u$ along the order, must not be larger for $u$ than for $w$,
and must be equal for `ISOMORPH`. Each call of `find()` resumes the search
where the previous one stopped, so that repeated calls give each mapping once.
Note that the search is a loop with an explicit depth, and the stack of the
program does not grow with the size of the graphs.

The class `Vf2< G1, G2, M, NEQ >` implements the algorithm of [Cordella et
al. 2004](Z-bibliography.md#ref-cordella2004sub). Its template parameters are
the type of the graph to be embedded (`G1`, by default `ListGraph`) and that
of the graph it is embedded into (`G2`, by default `ListGraph`), the type of
the read-write node map of $G_1$ that stores the mapping (`M`, by default
`G1::NodeMap< G2::Node >`), and a binary functor on a node of $G_1$ and one of
$G_2$ that decides whether the first may be mapped to the second (`NEQ`, by
default a functor that always returns `true`). Its constructor
`Vf2(g1, g2, m, neq = NEQ())` computes the order of step 1 and sets all the
values of `m` to `INVALID`. Then `mappingType()` returns the type and
`mappingType(t)` sets it, and `find()` searches for the next mapping, writes
it into `m` and returns `true`, or returns `false` when none is left.

The class `Vf2pp< G1, G2, M, M1, M2 >` implements the algorithm of [Jüttner
and Madarasi 2018](Z-bibliography.md#ref-VF2PP), which differs from VF2 in two
respects. First, its labels are integer node maps (`M1` and `M2`, by default
`G1::NodeMap< int >` and `G2::NodeMap< int >`), whose values must be
$0,1,\ldots,K-1$ for $K$ distinct labels, since the algorithm keeps vectors
indexed by the label; there is no functor of node equivalence. Second, the
order of step 1 is chosen to "fail early". Each breadth-first tree starts from
a node whose label is the "rarest" among the nodes of $G_2$ (among those, one
of largest degree), and within each level of the tree the nodes are taken by
most neighbors already ordered, then by largest degree, then by rarest label;
the look-ahead of `feasible()` counts the neighbors label by label. According
to the experiments of [Jüttner and Madarasi
2018](Z-bibliography.md#ref-VF2PP), these two changes typically reduce the
search considerably, especially on labeled graphs; we have not measured the
two classes against each other, and on "easy" instances the difference may
well be small. Its constructor is `Vf2pp(g1, g2, m, labels1, labels2)`, and
`mappingType()` and `find()` are as for `Vf2`.

Both algorithms also have a function-type interface, `vf2(g1, g2)` and
`vf2pp(g1, g2)`, which return objects of the classes `Vf2Wizard` and
`Vf2ppWizard`. Their named parameters are `mapping(m)`, the map that receives
the result (by default, a map is allocated and deleted inside),
`nodeLabels(l1, l2)`, two node maps whose values `vf2()` compares with `==`
and which `vf2pp()` takes as integer labels as above (without them, `vf2()`
may map any node to any node, and `vf2pp()` gives all the nodes the label
$0$), `nodeEq(f)`, the functor (for `vf2()` only), and `mappingType(t)`,
`induced()` and `iso()`. Such an expression ends with `run()`, which returns
whether a mapping exists and leaves it in the map given by `mapping()`, with
`count()`, which returns the number of mappings, or with `getPtrToVf2Object()`
(respectively `getPtrToVf2ppObject()`). This last function returns a pointer
to a new algorithm object, on which one calls `find()` repeatedly and which
the caller deletes; the object then deletes the maps that the wizard
allocated, if any. We remark that both classes work on undirected graphs only
(they iterate on the incident edges), and that the count of the mappings onto
a graph with symmetries includes its automorphisms (e.g., the cycle on five
nodes has 10 automorphisms). Of course, on the same input the two algorithms
give the same set of mappings, possibly in another order, and `Vf2` remains
the one to use when a functor of node equivalence is more "natural" than
integer labels.

!!! note "abLEMON"
    `Vf2` and `Vf2pp` were added to the repository of LEMON after the release
    of 1.3.1 (trac #597) and were never in a release of LEMON, so that abLEMON
    is the first release that contains them. Furthermore, abLEMON reads the
    connection counter of the image of a neighbor only when the neighbor is
    mapped, where the code indexed a node map with `INVALID` (trac #672);
    `getPtrToVf2Object()` and `getPtrToVf2ppObject()` compile, the wizards
    being friends of the classes whose flags of deallocation they set (trac
    #682); and `lemon/vf2pp.h` includes `lemon/list_graph.h`, whose
    `ListGraph` its default arguments name, so that each of the two headers is
    self-contained.

The example counts the automorphisms of the cycle on five nodes and the
embeddings of a path of three nodes into the cycle on four nodes and into
$K_4$, as subgraphs and as induced subgraphs; into $K_4$ there are 24 of the
former and none of the latter, since any two nodes of $K_4$ are adjacent. It
then repeats the counts with labels, and it enumerates the mappings one by one
with `find()`, checking each of them.

```cpp
--8<-- "examples/18-isomorphism.cc"
```

## 18.8 Exercises {#sec-18-8}

<span id="ex-18-1"></span>**Exercise 18.1.** Add the edge 1-6 to the graph of
[Figure 18.1](#fig-18-1), and predict by Lemma 18.1 the blocks, the cut nodes
and the bridges of the new graph. Check the prediction with the functions of
[§18.2](#sec-18-2), starting from the example 18-components.cc (the test
`test/connectivity_test.cc` checks the same functions on other graphs).

<span id="ex-18-2"></span>**Exercise 18.2.** Build the condensation of a
digraph from the map of `stronglyConnectedComponents()`, with one node per
component and one arc per cut arc of `stronglyConnectedCutArcs()`. Verify with
`dag()` and `checkedTopologicalSort()` that it is acyclic and that the indices
of the components are a topological order of it (the example 18-components.cc
checks the second property on the original digraph).

<span id="ex-18-3"></span>**Exercise 18.3.** A connected graph with two nodes
of odd degree, $s$ and $t$, has an Euler path from $s$ to $t$. Obtain it with
`EulerIt` by adding a new edge $ts$, starting the tour at $s$ and dropping the
new edge from the walk, and check the result as the example 18-euler.cc checks
a tour (the test `test/euler_test.cc` checks the iterators on several graphs).

<span id="ex-18-4"></span>**Exercise 18.4.** Show that the faces of an
embedding of a planar graph with $c$ connected components, counted as in the
example 18-planar.cc (one orbit per face of each component), satisfy
$f+n-c=m+1$, and verify it on a graph with two components; the test
`test/planarity_test.cc` checks this identity in `checkEmbedding()`.

<span id="ex-18-5"></span>**Exercise 18.5.** Run `PlanarEmbedding` on $K_5$
and on the Petersen graph, and check that the edges marked by `kuratowski()`
form a subdivision of $K_5$ (five nodes of degree four) or of $K_{3,3}$ (six
nodes of degree three), the other nodes having degree zero or two in it; the
test `test/planarity_test.cc` makes this check in `checkKuratowski()`.

<span id="ex-18-6"></span>**Exercise 18.6.** Count with
`vf2(g, g).iso().count()` and with `vf2pp(g, g).iso().count()` the
automorphisms of the cube and of the Petersen graph (48 and 120), and compare
the number of calls of `find()` with and without labels that distinguish one
node. The example 18-isomorphism.cc does the same on the cycle on five nodes,
and the test `test/vf2_test.cc` checks the mappings found by both classes.

<span id="ex-18-7"></span>**Exercise 18.7.** Show that, for a pattern $G_1$
that is a complete graph, the counts of the `SUBGRAPH` and of the `INDUCED`
mappings into any $G_2$ coincide, and that, for a pattern without edges, the
`INDUCED` mappings correspond to the ordered independent sets of $G_2$ of that
size. Check both claims on small graphs, starting from the example
18-isomorphism.cc.
