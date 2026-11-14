# 17. Matchings {#ch-17}

We are concerned in this chapter with finding, in an undirected graph
$G=(V,E)$ with $n$ nodes and $m$ edges, a set of edges no two of which share
an end, i.e., a *matching*, which is as large as possible, or of largest total
weight, or of least total cost among those that cover a prescribed set of
nodes. abLEMON solves these problems with three headers. The first,
`bipartite_matching.h`, holds the algorithms for bipartite graphs
(`HopcroftKarp` and `MinCostAssignment`); the second, `matching.h`, holds the
blossom algorithms of Edmonds for general graphs (`MaxMatching`,
`MaxWeightedMatching` and `MaxWeightedPerfectMatching`); the third,
`fractional_matching.h`, holds the algorithms for the relaxation in which an
edge may be taken by halves (`MaxFractionalMatching`,
`MaxWeightedFractionalMatching` and `MaxWeightedPerfectFractionalMatching`),
which `matching.h` also uses to start its weighted algorithms. Each class
gives, besides the matching, a dual solution that proves it optimal, and we
describe the two together, since one usually checks a result against its dual
(a "proof" that the user can verify in linear time, or nearly so). The
structure of the chapter is as follows. Section 17.1 states the problems and
the duality results on which the algorithms rest; sections 17.2 and 17.3 treat
the bipartite case, sections 17.4 and 17.5 the general one, section 17.6 the
fractional relaxations, and section 17.7 gives the exercises.

## 17.1 Problems, duality and the choice of an algorithm {#sec-17-1}

Let $\delta(u)$ denote the set of the edges incident to the node $u$, and
$\gamma(X)$ the set of the edges with both ends in $X\subseteq V$. A matching
$M$ *covers* the ends of its edges, and it is *perfect* when it covers all the
nodes; the cardinality of a maximum matching is denoted by $\nu(G)$. With a
weight $w_e$ on each edge, the maximum weight matching problem is the integer
program
$$
\max\Big\{\sum_{e\in E} w_e x_e \;:\; \sum_{e\in\delta(u)} x_e\le 1
\ \ \forall u\in V,\ \ x\in\{0,1\}^E\Big\},
\tag{17.1}
$$
of which the maximum cardinality matching problem is the case $w\equiv 1$ and
the maximum weight perfect matching problem the case with equations in place
of the inequalities (a minimum cost perfect matching is then found by negating
the costs). An *alternating path* with respect to $M$ is a path whose edges
alternate between $M$ and $E\setminus M$; it is *augmenting* when neither of
its ends is covered by $M$. Exchanging the edges of $M$ and of the path along
an augmenting path gives a matching with one more edge, and the algorithms of
the chapter that compute an integer matching all rest on the converse.

<span id="thm-17-1"></span>**Theorem 17.1** (Berge). *A matching $M$ of $G$ is
maximum if and only if no path is augmenting with respect to $M$.*

For a proof see [Berge 1957](Z-bibliography.md#ref-berge1957theorems) or
[Schrijver 2003, ch. 24](Z-bibliography.md#ref-schrijver03combinatorial). In a
bipartite graph, whose nodes are split into the *red* nodes $R$ and the *blue*
nodes $B$ with each edge joining a red node to a blue one (the `BpGraph`
structures of [chapter 4](04-graphs.md#ch-4)), the dual object of a matching
is a *vertex cover*, i.e., a set of nodes that contains at least one end of
each edge. Since the edges of a matching need distinct nodes of any cover,
$|M|\le|C|$ for a matching $M$ and a cover $C$; in the bipartite case the two
optima coincide.

<span id="thm-17-2"></span>**Theorem 17.2** (König). *In a bipartite graph the
maximum cardinality of a matching equals the minimum cardinality of a vertex
cover.*

See [König 1931](Z-bibliography.md#ref-konig1931grafok) and [Schrijver 2003,
ch. 16](Z-bibliography.md#ref-schrijver03combinatorial); `HopcroftKarp`
returns such a cover with the matching (cf. section 17.2). In a general graph
a minimum vertex cover may be twice as large as a maximum matching (as in a
triangle), and the certificate is rather a set of nodes $A$, called a
*barrier*, whose removal leaves many components with an odd number of nodes.
In fact, if $o(G-A)$ denotes the number of the odd components of $G-A$, each
of them has a node that is either matched to $A$ or not covered at all; hence,
at least $o(G-A)-|A|$ nodes are left uncovered by any matching.

<span id="thm-17-3"></span>**Theorem 17.3** (Tutte–Berge formula and
Gallai–Edmonds decomposition). *For every graph,*
$$
\nu(G) \;=\; \min_{A\subseteq V}\ \tfrac12\big(n+|A|-o(G-A)\big).
\tag{17.2}
$$
*Let $D$ be the set of the nodes left uncovered by at least one maximum
matching, $A$ the set of the nodes not in $D$ with a neighbour in $D$, and
$C=V\setminus(D\cup A)$. Then $A$ attains the minimum in (17.2), and each
component of the subgraph induced by $D$ is factor-critical (i.e., it has a
perfect matching after the removal of any one of its nodes). Also, the
subgraph induced by $C$ has a perfect matching, and each maximum matching
matches the nodes of $A$ to nodes of $D$ in distinct components.*

See [Tutte 1947](Z-bibliography.md#ref-tutte1947factorization), [Gallai
1964](Z-bibliography.md#ref-gallai1964maximale) and [Schrijver 2003, ch.
24](Z-bibliography.md#ref-schrijver03combinatorial); `MaxMatching` returns the
three sets as the status of each node (cf. section 17.4). For the weighted
problems the inequalities of (17.1) do not describe the convex hull of the
matchings, since the point with $x_e=\frac12$ on the edges of a triangle
satisfies them and is not a combination of matchings. Edmonds showed that the
*odd set inequalities* are the missing ones; the weighted classes of
`matching.h` work on his description.

<span id="thm-17-4"></span>**Theorem 17.4** (Edmonds). *Let $\mathcal O$ be
the family of the subsets of $V$ of odd cardinality at least 3. The convex
hull of the incidence vectors of the matchings of $G$ is the set of the
$x\ge 0$ with $\sum_{e\in\delta(u)}x_e\le 1$ for each $u\in V$ and*
$$
\sum_{e\in\gamma(S)} x_e \;\le\; \frac{|S|-1}{2}
\qquad \forall\, S\in\mathcal O ,
\tag{17.3}
$$
*and that of the perfect matchings is obtained by turning the node
inequalities into equations.*

See [Edmonds 1965b](Z-bibliography.md#ref-edmonds1965maximum) and [Schrijver
2003, ch. 25 and 26](Z-bibliography.md#ref-schrijver03combinatorial). The
linear programming dual of this description has a variable $y_u$ for each node
and a variable $z_S\ge 0$ for each odd set:
$$
\min\Big\{\sum_{u\in V} y_u+\sum_{S\in\mathcal O}\frac{|S|-1}{2}\,z_S
\;:\; y_u+y_v+\sum_{S\in\mathcal O:\,uv\in\gamma(S)} z_S\ \ge\ w_{uv}
\ \ \forall\, uv\in E\Big\},
\tag{17.4}
$$
with $y\ge 0$ for (17.1) and $y$ free for the perfect version. A matching and
a dual solution of equal value prove each other optimal. This happens when (i)
each edge of the matching has zero *slack*, i.e., $y_u+y_v+\sum z_S-w_{uv}=0$,
(ii) each node with $y_u>0$ is covered (in the version with inequalities), and
(iii) each set with $z_S>0$ holds $\frac{|S|-1}2$ edges of the matching; these
are the complementary slackness conditions of the description of Theorem 17.4
and of (17.4). Dropping (17.3) gives the *fractional matching* problem, whose
vertices are half-integral: each of them is made of a matching, with value 1
on its edges, and of node-disjoint odd cycles, with value $\frac12$ on theirs
(see [Schrijver 2003, ch.
30](Z-bibliography.md#ref-schrijver03combinatorial)). Its dual is (17.4)
without the $z$ variables. Its optimum bounds that of (17.1) from above. A
triangle with unit weights shows that the bound may be strict ($\frac32$
against 1).

As for the choice among the classes, a bipartite graph is best handled by
`bipartite_matching.h`, i.e., by `HopcroftKarp` for the cardinality, in
$O(m\sqrt n)$ time, and by `MinCostAssignment` for a matching of least cost
that covers all the red nodes, in $O(n_R\,m\log n)$ time with $n_R=|R|$. A
general graph needs the blossoms (which make the algorithms somewhat more
involved), i.e., `MaxMatching` for the cardinality and `MaxWeightedMatching`
or `MaxWeightedPerfectMatching` for the weights, the latter two in
$O(nm\log n)$ time. In our view, the fractional classes are mostly of interest
when the relaxation is itself the object (e.g., as a bound), and as the
starting point of the weighted blossom algorithms, which call them by default.
Of course, one can also write an assignment problem as a minimum cost flow
problem ([chapter 13](13-min-cost-flow.md#ch-13)), and a bipartite cardinality
matching as a maximum flow problem ([chapter 12](12-max-flow.md#ch-12)). This
requires building the network, which is rather wasteful when the graph is
"large". The classes of this chapter work on the graph itself, and they give
the dual objects described above directly.

## 17.2 Maximum cardinality matching in bipartite graphs {#sec-17-2}

`HopcroftKarp< BPGR >` finds a maximum matching of a bipartite graph of type
`BPGR`, together with the minimum vertex cover of Theorem 17.2, by the
algorithm of [Hopcroft and Karp
1973](Z-bibliography.md#ref-hopcroft1973algorithm). The algorithm grows the
matching in phases. In each phase a breadth-first search from the uncovered
red nodes labels each red node with the number of matched edges on a shortest
alternating path that reaches it. Then a depth-first search from each
uncovered red node follows these labels, one layer at a time, until it meets
an uncovered blue node, and the matching is augmented along the path found.
The header states the bound $O(m\sqrt n)$ of the original algorithm (a bound
usually far from attained in practice), which rests on the fact that the
length of the shortest augmenting paths grows at each phase, so that
$O(\sqrt n)$ phases suffice.

<span id="alg-17-1"></span>**Algorithm 17.1** (Hopcroft–Karp).

```
 1  M <- greedy matching (greedyInit) or empty matching (init)
 2  loop
 3     for each red r do d(r) <- 0 if r is uncovered, d(r) <- inf otherwise
 4     breadth-first search from the uncovered red nodes, from a red x, for
        each edge xb, if b is covered by the edge yb of M and d(y) = inf,
        then d(y) <- d(x) + 1; L <- d(x) at the first x with an uncovered
        b, and the search stops before the red nodes with d > L
 5     if no uncovered blue node was seen then exit the loop
 6     for each uncovered red node s do
 7        depth-first search from s, from a red x, scan the edges xb from
           the one it stopped at; if b is uncovered and d(x) = L, augment
           M along the path found and stop; if b is matched to y with
           d(y) = d(x)+1 <= L, go down to y
 8        a red node all of whose edges fail gets d <- inf
 9  R* <- nodes reached by alternating paths from the uncovered red nodes
10  cover <- (red nodes not in R*) + (blue nodes in R*)
```

Steps 3 to 5 are `bfs()`, steps 6 to 8 `dfs()`, and steps 9 and 10
`findCover()`, which is called once at the end of `start()`. Note that the
depth-first search is written without recursion, and each red node keeps the
edge it has reached in a map; hence, each edge is scanned once per phase. The
breadth-first search stops at the layer $L$ of the first uncovered blue node,
and the depth-first searches augment only along paths of that length; hence,
the paths of a phase are shortest augmenting paths, as the bound requires. The
certificate does not depend on them, since step 10 builds it from the final
matching alone. Indeed, the set of step 10 is a cover because an edge from a
red node of $R^*$ to a blue node outside $R^*$ would extend an alternating
path. Furthermore, it has $|M|$ nodes. Its nodes are all covered by $M$ (an
uncovered blue node in $R^*$ would end an augmenting path), and no edge of $M$
has both ends in it.

This class follows the pattern of the algorithm classes of [chapter
3](03-mental-model.md#ch-3), although it has no named template parameters,
since it uses no auxiliary structure that one could replace. Its constructor
`HopcroftKarp( const BpGraph & graph )` builds the maps of the matching, of
the layers and of the cover. Execution starts with `init()`, which empties the
matching, or with `greedyInit()`, which also matches each red node, in the
order of `RedNodeIt`, to the first uncovered blue node among its neighbours.
Then `start()` augments the matching until it is maximum and computes the
cover, and `run()` is simply `greedyInit()` followed by `start()`. Among the
queries, `matchingSize()`, `matching( const Edge & )` (whether the edge is in
the matching), `matching( const Node & )` (the matching edge at the node, or
`INVALID`), `matchingMap()` (a const reference to the node map of type
`MatchingMap`, i.e., `BpGraph::NodeMap< Edge >`) and `mate( const Node & )`
give the primal solution. Finally, `cover( const Node & )` returns whether the
node is in the minimum vertex cover, which is meaningful only after `start()`
or `run()`.

!!! note "abLEMON"
    Both `HopcroftKarp` and `MinCostAssignment` are new in abLEMON (version
    1.5.0, LEMON trac ticket #168). LEMON 1.3.1 has bipartite graphs but no
    matching algorithm written for them; its general matching classes give no
    vertex cover and no assignment potentials.

## 17.3 Minimum cost assignment {#sec-17-3}

We now give a cost $c_e$ to each edge of a bipartite graph, which may have any
number of blue nodes and need not be complete. In this setting
`MinCostAssignment< BPGR, CM >` solves
$$
\min\Big\{\sum_{e\in E} c_e x_e \;:\; \sum_{e\in\delta(r)} x_e = 1
\ \ \forall r\in R,\ \ \sum_{e\in\delta(b)} x_e\le 1\ \ \forall b\in B,
\ \ x\in\{0,1\}^E\Big\},
\tag{17.5}
$$
i.e., it finds a matching of least cost among those that cover all the red
nodes, if one exists. Its dual solution is a *potential* $\pi$ on the nodes
with
$$
c_e \;\ge\; \pi(b)-\pi(r) \qquad \forall\, e=rb\in E ,
\tag{17.6}
$$
with equality on the edges of the matching, and with the blue nodes left
uncovered all at the largest potential $K=\max_{b\in B}\pi(b)$ of the blue
nodes. Setting $\alpha_r=K-\pi(r)$ and $\beta_b=\pi(b)-K\le 0$ gives a
feasible solution of the linear programming dual of (17.5) whose value equals
the cost of the matching. Thus, the optimal cost is
$\sum_{r\in R}\big(\pi(b_r)-\pi(r)\big)$, where $b_r$ is the mate of $r$.

The method is that of the successive shortest augmenting paths, with the
potentials of [Edmonds and Karp
1972](Z-bibliography.md#ref-edmondskarp72theoretical) and [Tomizawa
1971](Z-bibliography.md#ref-tomizawa1971techniques). Here the red nodes are
matched one at a time, each along a shortest path of the residual graph, in
which an edge not in the matching is traversed from its red end at cost $c_e$
and an edge of the matching from its blue end at cost $-c_e$. Each path is
found by Dijkstra's algorithm ([Dijkstra
1959](Z-bibliography.md#ref-dijkstra1959note)) on the costs reduced by $\pi$,
which (17.6) keeps nonnegative.

<span id="alg-17-2"></span>**Algorithm 17.2** (Successive shortest paths for
the assignment).

```
 1  M <- empty; pi(b) <- 0 for each blue b
 2  for each red r do if r has no edge then return false, else
       pi(r) <- - (least cost of an edge of r)
 3  for each red node s, in the order of RedNodeIt do
 4     Dijkstra from s on the reduced costs, red x -> blue b along xb not
        in M at c(xb) + pi(x) - pi(b); matched blue b -> its mate y at
        d(b) - (c(yb) + pi(y) - pi(b)); stop when an uncovered blue node t
        is taken from the heap
 5     if no uncovered blue node is reached then return false
 6     for each node v do pi(v) += d(v) if v was taken from the heap with
        d(v) < d(t), and pi(v) += d(t) otherwise
 7     augment M along the path from s to t
 8  return true
```

Step 2 makes the reduced costs nonnegative before the first search, and step 6
keeps them so, with zero reduced cost on the new path. Since the nodes not
finished by the search all receive the same increase $d(t)$, the uncovered
blue nodes, which are never finished, keep the largest blue potential. As for
the heap of step 4, it is a `std::priority_queue` of pairs (distance, node id)
with lazy deletion: a node reached again at a shorter distance is pushed
again, and the stale entries are skipped when popped. A search therefore takes
$O(m\log n)$ time, and the method $O(n_R\,m\log n)$, as the header states.
Note that the comparisons are exact (the class has no tolerance), which is
usually adequate for integer costs but may be somewhat fragile with
floating-point ones, and that the cost type must be a signed arithmetic type.

The interface is reduced to what the method needs. Its constructor is
`MinCostAssignment( const BpGraph & graph , const CostMap & cost )`, with `CM`
defaulting to `BpGraph::EdgeMap< int >`. Its only execution function is
`run()`, which returns `true` if a matching covering all the red nodes exists
and `false` otherwise (in which case the matching is left as the method had
grown it, and the potentials have no meaning). As in `HopcroftKarp`,
`totalCost()`, `matching( const Edge & )`, `matching( const Node & )` and
`matchingMap()` give the primal solution, while `potential( const Node & )`
gives the dual solution (17.6).

The program below solves a "small" instance with both classes of the header.
It checks the cover of Theorem 17.2, the conditions (17.6) and the value of
the dual solution. Then it compares the optimal cost with an enumeration of
all the assignments.

```cpp
--8<-- "examples/17-bipartite.cc"
```

## 17.4 Maximum cardinality matching in general graphs {#sec-17-4}

We now turn to general graphs, in which an alternating search tree may close
an odd cycle, and the search then has to treat the cycle as a single node.
This is the *blossom* of [Edmonds
1965a](Z-bibliography.md#ref-edmonds1965paths), and `MaxMatching< GR >`
implements his alternating forest algorithm on an undirected graph of type
`GR`. Each node has a status, `EVEN`, `ODD`, `MATCHED` or `UNMATCHED`. A
search tree is grown from an uncovered root, which is `EVEN`; a covered node
reached from an `EVEN` node becomes `ODD`, and its mate `EVEN`. An edge
between two `EVEN` nodes of the same tree in distinct blossoms closes a
blossom, whose nodes all become `EVEN`, and an edge from an `EVEN` node to an
uncovered node gives an augmenting path. Internally, the blossoms are kept by
a `UnionFindEnum` and the trees by an `ExtendFindEnum` (see [chapter
8](08-auxiliary.md#ch-8)).

<span id="alg-17-3"></span>**Algorithm 17.3** (Edmonds' alternating forest,
sparse variant).

```
 1  M <- greedy or empty matching; covered nodes MATCHED, others UNMATCHED
 2  for each node n, while more than L nodes are UNMATCHED,
       (L = 0, or L = 1 without the decomposition) do
 3     if n is UNMATCHED then make n the EVEN root of a new tree; Q <- (n)
 4     while Q is not empty do take u from Q; for each arc uv do
 5        if v is EVEN and in another blossom then shrink the blossom closed
           by uv; its ODD nodes become EVEN and enter Q
 6        if v is MATCHED then extend, v becomes ODD, its mate EVEN (into Q)
 7        if v is UNMATCHED then augment along uv and the tree path of u; the
           nodes of the tree become MATCHED; go to the next root
 8  (a tree that did not augment keeps its labels)
```

Step 5 is `shrinkOnEdge()`, step 6 `extendOnArc()` and step 7
`augmentOnArc()`. A tree that ends without augmenting is a Hungarian tree,
i.e., no later search can enter it with an augmenting path, and its labels
stay. Hence, when the loop ends with no node `UNMATCHED`, the `EVEN` nodes
form the set $D$ of Theorem 17.3, the `ODD` nodes the barrier $A$ and the
`MATCHED` nodes the set $C$, which proves the matching maximum by (17.2).
`startDense()` runs the same search with the heuristic of postponing the
shrinks. It first extends the tree from the nodes in the queue. Only when no
extension or augmentation is left does it shrink the blossoms, extending again
after each shrink; the header reports this to be faster on "dense" graphs
(presumably a matter of constants, since the worst case is the same). The
header states no bound on the running time, and we do not attempt one here.
The bounds of the implementations of the blossom algorithm are surveyed in
[Galil 1986](Z-bibliography.md#ref-galil1986efficient).

Besides the constructor `MaxMatching( const Graph & graph )`, the class has
three functions that set the initial matching. These are `init()` (the empty
one), `greedyInit()` (each node, in the order of `NodeIt`, matched to its
first uncovered neighbour, loops excluded) and
`matchingInit( const MatchingMap & matching )`, which takes a `bool` edge map
and returns `false` if two edges with value `true` share a node. Two
functions, `startSparse( bool decomposition = true )` and
`startDense( bool decomposition = true )`, run the two variants of Algorithm
17.3, and `run( bool decomposition = true )` calls `greedyInit()` and
`startSparse()` if $m<2n$, and `init()` and `startDense()` otherwise. Of the
queries, `matchingSize()` counts the covered nodes (in $O(n)$ time), and
`matching( const Edge & )`, `matching( const Node & )` (the matching arc at
the node, directed away from it, or `INVALID`), `matchingMap()` (a
`Graph::NodeMap< Arc >`) and `mate( const Node & )` complete the primal
solution. Lastly, `status( const Node & )` and `statusMap()` give the values
of the enum `Status`, i.e., `EVEN` $=1$ (also called `D`), `MATCHED` $=0$
(also `C`), `ODD` $=-1$ (also `A`) and `UNMATCHED` $=-2$, and
`barrier( const Node & )` is `true` on the `ODD` nodes.

With `decomposition` set to `false`, the loop of step 2 stops as soon as at
most one node is left uncovered. At that point the matching is maximum, since
an augmenting path needs two uncovered ends; however, the last uncovered node
has not been searched from, and the statuses do not form the decomposition.
According to the header, this may save much time, in particular on a dense
graph with an odd number of nodes, where the last search, from a node that
remains uncovered, usually explores the whole graph and finds no augmenting
path.

![Figure 17.1. The graph of the example below, with the maximum matching found by MaxMatching (the edges in the colour of the added objects) and its Gallai–Edmonds decomposition: the EVEN nodes (D) induce the factor-critical components {a, b, c}, {d} and {e}, the barrier A is {x}, and the MATCHED nodes (C) induce a square with a perfect matching. By (17.2), at least 3 − 1 = 2 nodes are uncovered by any matching.](../figures/17-gallai-edmonds.svg){#fig-17-1}

The program below computes the matching of Figure 17.1. It counts the
components of the subgraph induced by the `EVEN` nodes with a `FilterNodes`
adaptor ([chapter 5](05-adaptors.md#ch-5)) and `countConnectedComponents()`
([chapter 18](18-properties.md#ch-18)). Then it checks the equality of (17.2)
for the barrier found, and it runs the algorithm once more without the
decomposition.

```cpp
--8<-- "examples/17-edmonds.cc"
```

!!! note "abLEMON"
    The parameter `decomposition` of `startSparse()`, `startDense()` and
    `run()` is not in LEMON 1.3.1: it was added to the repository of LEMON in
    2018, after its last release, and abLEMON is the first release to
    distribute it. With the default `true`, a program written for LEMON 1.3.1
    behaves as before.

## 17.5 Maximum weight matchings in general graphs {#sec-17-5}

`MaxWeightedMatching< GR, WM >` solves (17.1), and
`MaxWeightedPerfectMatching< GR, WM >` its perfect version, by the primal-dual
blossom algorithm of [Edmonds 1965a](Z-bibliography.md#ref-edmonds1965paths).
Throughout, the algorithm keeps a matching and a dual solution of (17.4) that
satisfy the complementary slackness conditions (i) and (iii) of section 17.1,
and changes the duals until condition (ii) also holds (in the perfect version,
until all the nodes are covered). Its weight map `WM` defaults to
`GR::EdgeMap< int >`; we remark that loops are ignored. Both classes use
binary heaps (`BinHeap`) for the kinds of dual change, four in
`MaxWeightedMatching` and three in the perfect version, and a `HeapUnionFind`
for the nested blossoms, which gives the bound $O(nm\log n)$ stated by the
header, i.e., that of [Galil, Micali and Gabow
1986](Z-bibliography.md#ref-galil1986algorithm).

<span id="alg-17-4"></span>**Algorithm 17.4** (Primal-dual blossom algorithm).

```
 1  initial duals y(u) <- max(0, max of w(e)/2 over the edges e at u),
       z <- 0 (init); or the optimal duals and matching of the fractional
       relaxation (fractionalInit, the default of run())
 2  each uncovered node is the EVEN root of a tree; each node a blossom
 3  while some node is uncovered do
 4     d1 <- least y(u) over the EVEN nodes (not in the perfect version)
       d2 <- least slack of an edge from an EVEN blossom to a node out of
             the forest
       d3 <- half the least slack of an edge between two EVEN blossoms
       d4 <- least change that brings to zero the z(B) of a nontrivial
             ODD blossom
 5     if the least of them is infinite then return false (perfect version)
 6     move the duals by the least d, y down on EVEN nodes and up on ODD
        ones, z(B) up on EVEN blossoms and down on ODD ones
 7     d1 -> the node stays uncovered (its y is now 0) and leaves the forest
       d2 -> extend the tree, or augment if the node reached is uncovered
       d3 -> same tree, shrink a new blossom; distinct trees, augment
       d4 -> expand the blossom
 8  extract the matching and the duals, the nested blossoms flattened
```

In the code, the quantities of step 4 are kept in the heaps `_delta1` to
`_delta4`, and the dual changes of step 6 are applied lazily through a running
sum; thus, a step costs a few heap operations instead of a pass over the
graph. If the weights are of an integer type, each dual value is multiplied by
the constant `dualScale`, which is 4 (and 1 for a floating-point type), so
that the halvings of steps 1 and 4 stay exact. At the end, the value of the
duals equals `dualScale` times the weight of the matching. By default the
perfect version starts from the optimal perfect fractional matching
(cf. section 17.6); when this does not exist (e.g., when two pendant edges end
at the same node), `fractionalInit()` already detects the failure, which
`start()` then returns. The header calls this initialization the "jumpstart"
heuristic, and it may change which of several optimal matchings is found, but
not the optimal value.

The two classes have the same members, except for the return type of the
execution functions. Their constructor takes
`( const Graph & graph , const WeightMap & weight )`. Either `init()` or
`fractionalInit()` sets the initial solution, the latter by the jumpstart from
the fractional relaxation (solved by `MaxWeightedFractionalMatching` or
`MaxWeightedPerfectFractionalMatching`, with loops excluded); then `start()`
runs the algorithm, and `run()` is `fractionalInit()` followed by `start()`.
In `MaxWeightedPerfectMatching`, both `start()` and `run()` return `false`
when the graph has no perfect matching. As in `MaxMatching`,
`matchingWeight()`, `matchingSize()`, `matching( const Edge & )`,
`matching( const Node & )`, `matchingMap()` and `mate( const Node & )` give
the primal solution. For the dual one, `dualValue()` returns the value of the
scaled dual solution, i.e., $\sum_u y_u + \sum_B \lfloor |B|/2\rfloor z_B$,
and `nodeValue( const Node & )` the scaled $y_u$. Furthermore, `blossomNum()`
returns the number of the blossoms with a dual variable in the final solution,
`blossomSize( int k )` and `blossomValue( int k )` (the scaled $z_B$) describe
the $k$-th of them, and the nested class `BlossomIt`, built as
`BlossomIt( algorithm , k )`, lists its nodes. A blossom comes before the
blossoms nested in it, whose nodes form contiguous parts of its own list.

The program below computes a maximum weight matching of a triangle with two
pendant edges. It verifies the conditions of section 17.1 on each edge (the
slack computed from `nodeValue()` and from the blossoms that contain both
ends, which `BlossomIt` lists) and the equality of the primal and dual values,
and it observes that the graph has no perfect matching; it ends with the
fractional classes of the next section.

```cpp
--8<-- "examples/17-weighted.cc"
```

!!! note "abLEMON"
    In LEMON 1.3.1 the extraction of the nested blossoms at the end of
    `start()` (step 8 of Algorithm 17.4) was recursive, with one level per
    level of nesting, and it could overflow the stack on large graphs (LEMON
    trac ticket #638); in abLEMON it uses an explicit stack, a change made in
    the repository of LEMON in 2021 and never released by it. Furthermore, the
    node records of the two classes initialize all their fields, some of which
    LEMON 1.3.1 copied while still indeterminate. The results and the
    interface do not change.

## 17.6 Fractional matchings {#sec-17-6}

We finally describe the three classes of `fractional_matching.h`, which solve
the relaxation of section 17.1 without the odd set inequalities (17.3), whose
optimal vertices are made of matching edges with value 1 and of odd cycles
with value $\frac12$. They represent a fractional matching by a node map of
arcs, in which each covered node points along an arc to a neighbour; the two
ends of an edge with value 1 point to each other, and the nodes of a
half-valued odd cycle point to their successors on the cycle. Since the values
are multiples of $\frac12$, the primal solution is reported multiplied by the
constant `primalScale`, which is 2. Thus, `matching( const Edge & )` returns
an `int` in $\{0,1,2\}$ (the number of ends of the edge that point along it),
and `matchingSize()` returns the number of the covered nodes, i.e., twice the
value of the fractional matching. All three constructors take a flag
`allow_loops`, `true` by default, which decides whether a loop may be taken
(as a cycle of length one, with value $\frac12$).

`MaxFractionalMatching< GR, TR >` maximizes the cardinality. The relaxation is
equivalent to a bipartite matching problem on the double cover of $G$, in
which each node has a copy on each side and each edge $uv$ joins the first
copy of $u$ to the second copy of $v$ and vice versa. This problem is solved
by a push-relabel method in the spirit of [Goldberg and Tarjan
1988](Z-bibliography.md#ref-goldberg88newapproach), with the levels kept by an
`Elevator` ([chapter 8](08-auxiliary.md#ch-8)).

<span id="alg-17-5"></span>**Algorithm 17.5** (Push-relabel for the maximum
fractional matching).

```
 1  each node u points to its first neighbour (loops skipped unless
       allowed); indeg(v) <- number of the nodes pointing to v;
       all nodes on level 0; the nodes with indeg 0 are active
 2  while some node is active do n <- an active node on the highest level l
 3     if a neighbour u of n points to a node v on a level below l then
          u points to n instead; indeg(v) -= 1, indeg(n) += 1;
          n becomes inactive, v active if indeg(v) = 0
 4     else lift n to one level above the lowest such v (or to the top);
          if level l is left empty, lift the nodes above it to the top
 5  a node pointing to a node v with indeg(v) > 1 drops its pointer until
       indeg(v) = 1
 6  if postprocess then turn the alternating paths and the even cycles of
       pointers into matching edges, leaving odd cycles of halves
```

The perfect variant `startPerfect()` stops with `false` as soon as a node
would be lifted to the top or a level is left empty, either of which proves
that no perfect fractional matching exists. The dual object, which we also
call a *barrier*, is a set $S$ of nodes with $|S|-|N(S)|$ as large as
possible, where $N(S)$ is the set of the neighbours of the nodes of $S$. Each
fractional matching leaves at least $|S|-|N(S)|$ nodes uncovered. Conversely,
a maximum one leaves exactly that many for some $S$, i.e.,
$$
n-\texttt{matchingSize()} \;=\; \max_{S\subseteq V}\big(|S|-|N(S)|\big),
\tag{17.7}
$$
which is König's theorem on the double cover. `barrier( const Node & )`
returns the nodes on the top level of the elevator after `start()`, and those
on or above the level found empty after a failed `startPerfect()`; the example
of section 17.5 checks (17.7) on the former. The header states no bound on the
running time, although push-relabel methods are often fast in practice.

We have modeled the description on the push-relabel classes of [chapter
12](12-max-flow.md#ch-12). The traits class
`MaxFractionalMatchingDefaultTraits< GR >` defines `MatchingMap`
(`GR::NodeMap< GR::Arc >`) and `Elevator` (`LinkedElevator< GR, GR::Node >`)
with their factory functions, and the named template parameters
`SetMatchingMap< T >`, `SetElevator< T >` (an elevator that the caller passes
with `elevator()`) and `SetStandardElevator< T >` (an elevator that the
algorithm allocates) replace them. Its constructor is
`( const Graph & graph , bool allow_loops = true )`. Two functions,
`matchingMap( MatchingMap & )` and `elevator( Elevator & )`, set external
structures, and `elevator()` returns the one in use. Execution is controlled
by `init()`, `start( bool postprocess = true )` and
`startPerfect( bool postprocess = true )` (which returns whether a perfect
fractional matching exists), or by their shortcuts
`run( bool postprocess = true )` and `runPerfect( bool postprocess = true )`.
Its queries are `matchingSize()`, `matchingMap()`, `matching( const Edge & )`,
`matching( const Node & )` and `barrier( const Node & )`. With `postprocess`
set to `false`, the result may also contain even cycles and paths of
half-valued edges, with the same value.

`MaxWeightedFractionalMatching< GR, WM >` and
`MaxWeightedPerfectFractionalMatching< GR, WM >` solve the weighted
relaxation, without and with the perfection constraint, by the primal-dual
method of Algorithm 17.4, in which a closed odd cycle is not shrunk into a
blossom but kept as a half-valued cycle of the solution. There are no $z$
variables, and hence no fourth kind of dual change; the header states the
bound $O(nm\log n)$ for both. Their constructor is
`( const Graph & graph , const WeightMap & weight , bool allow_loops = true )`,
and their execution functions are `init()`, `start()` and `run()`, which in
the perfect class return `false` when no perfect fractional matching exists.
Their queries are `matchingWeight()` (the weight scaled by `primalScale`),
`matchingSize()`, `matching( const Edge & )`, `matching( const Node & )`,
`matchingMap()`, `dualValue()` and `nodeValue( const Node & )`, with the duals
scaled by `dualScale` as in section 17.5. At the optimum, `primalScale` times
`dualValue()` equals `dualScale` times `matchingWeight()`. Again, the example
of section 17.5 checks this on a triangle, whose perfect fractional matching
has weight $\frac{15}2$, while the triangle has no perfect matching at all.

## 17.7 Exercises {#sec-17-7}

<span id="ex-17-1"></span>**Exercise 17.1.** Prove that the set returned by
`cover()` after `HopcroftKarp::run()` is a vertex cover with `matchingSize()`
nodes, using only the fact that the final matching admits no augmenting path;
then check it on random bipartite graphs against `MaxMatching` run on the same
graph. (The test `test/bipartite_matching_test.cc` does the latter; the
example `manual/examples/17-bipartite.cc` checks the cover on one instance.)

<span id="ex-17-2"></span>**Exercise 17.2.** Show that the potentials returned
by `MinCostAssignment` give a feasible solution of the linear programming dual
of (17.5) with the value of the matching, and use them to write a check that
needs no other solver. Then compare `totalCost()` with the optimum of the same
problem written as a minimum cost flow problem for `NetworkSimplex` ([chapter
13](13-min-cost-flow.md#ch-13)). (The test `test/bipartite_matching_test.cc`
makes this comparison and checks the conditions (17.6).)

<span id="ex-17-3"></span>**Exercise 17.3.** Modify the graph of Figure 17.1
so that its barrier has two nodes, predict the statuses of the nodes from
Theorem 17.3, and verify the prediction and the equality of (17.2) with the
program `manual/examples/17-edmonds.cc`.

<span id="ex-17-4"></span>**Exercise 17.4.** On a complete graph with an odd
number of nodes, measure the running time of `MaxMatching::run()` with and
without the decomposition, and explain the difference from step 2 of Algorithm
17.3. (The test `test/matching_test.cc` checks that the two give matchings of
the same size.)

<span id="ex-17-5"></span>**Exercise 17.5.** Find a minimum cost perfect
matching of a general graph with `MaxWeightedPerfectMatching`, by negating the
costs, and verify its optimality from `nodeValue()`, `blossomValue()` and
`BlossomIt` as the function `reducedWeight()` of
`manual/examples/17-weighted.cc` does, with the sign of $y$ free. (The test
`test/matching_test.cc` checks the dual solutions of both weighted classes in
this way.)

<span id="ex-17-6"></span>**Exercise 17.6.** On a graph made of node-disjoint
triangles with unit weights, compare the values of
`MaxWeightedFractionalMatching` and `MaxWeightedMatching`, and find the odd
set inequalities (17.3) that close the gap; then compare the running time of
`MaxWeightedMatching` started by `init()` and by `fractionalInit()`. (The test
`test/fractional_matching_test.cc` checks the primal and dual solutions of the
fractional classes.)
