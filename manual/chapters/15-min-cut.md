# 15. Minimum cuts {#ch-15}

We are concerned in this chapter with the cuts of minimum capacity of a graph
taken as a whole, i.e., with the nonempty proper subsets of the nodes whose
boundary has the least capacity, when no source and no sink are fixed in
advance. Between two given nodes, a minimum cut is the dual of a maximum flow,
and we refer to [chapter 12](12-max-flow.md#ch-12) for it. abLEMON has three
classes for these problems, all in the group [Minimum Cut
Algorithms](https://dmeoli.github.io/abLEMON/group__min__cut.html): `HaoOrlin`
(`lemon/hao_orlin.h`) for a digraph, `NagamochiIbaraki`
(`lemon/nagamochi_ibaraki.h`) for an undirected graph, and `GomoryHu`
(`lemon/gomory_hu.h`), which builds a tree from which the minimum cut between
any two nodes of an undirected graph can be read. Section 15.1 states the
problems and the two facts on which the algorithms rest, sections 15.2 to 15.4
describe the algorithms and their classes, and section 15.5 compares them.

## 15.1 The problems {#sec-15-1}

Let $D=(V,A)$ be a digraph with $n=|V|\geq 2$ nodes, $m=|A|$ arcs and
nonnegative capacities $c:A\rightarrow\mathbb{R}_+$. For $X\subseteq V$ we
write $\delta^+(X)$ for the arcs that leave $X$ and $c(\delta^+(X))$ for the
sum of their capacities. A *cut* is a set $X$ with
$\emptyset\neq X\subsetneq V$, and the *minimum cut* of $D$ is the value

$$\lambda(D)\;=\;\min\{\,c(\delta^+(X))\;:\;\emptyset\neq X\subsetneq V\,\}\tag{15.1}$$

together with a set that attains it. We call such a set a minimum cut as well.
For two distinct nodes $s$ and $t$, the minimum $s$-$t$ cut $\lambda(s,t)$ is
the same minimum restricted to the sets with $s\in X$ and $t\notin X$; by the
max-flow min-cut theorem it equals the value of a maximum flow from $s$ to $t$
(cf. [Ahuja, Magnanti and Orlin 1993, chapter
6](Z-bibliography.md#ref-amo93networkflows)). For an undirected graph
$G=(V,E)$ with capacities on the edges, $\delta(X)$ is the set of the edges
with one end in $X$ and the other outside, and $\lambda(G)$ is defined as in
(15.1). In this case the cut function is symmetric, i.e.,
$c(\delta(X))=c(\delta(V\setminus X))$. With unit capacities $\lambda(G)$ is
the edge connectivity of $G$, i.e., the least number of edges whose removal
disconnects it; the documentation of the library mentions this use first, as a
test of the "reliability" of a network.

Our first fact shows that $n-1$ minimum $s$-$t$ cuts in each direction are
enough to find $\lambda(D)$, and it is the starting point of the algorithm of
Hao and Orlin.

<span id="thm-15-1"></span>**Theorem 15.1.** *Let $r\in V$ be any node. Then*

$$\lambda(D)\;=\;\min\Big\{\,\min_{t\neq r}\lambda(r,t)\,,\;\min_{t\neq r}\lambda(t,r)\,\Big\}.\tag{15.2}$$

*Proof.* A cut $X$ that contains $r$ separates $r$ from any node $t\notin X$,
and hence $c(\delta^+(X))\geq\lambda(r,t)$; a cut that does not contain $r$
separates any node $t\in X$ from $r$. Conversely, an $s$-$t$ cut is a cut.
$\square$

For an undirected graph one has $\lambda(r,t)=\lambda(t,r)$, and the second
half of (15.2) can be dropped; we use this in section 15.2.2. The second fact,
due to [Gomory and Hu (1961)](Z-bibliography.md#ref-gomory1961multi), is that
the $n(n-1)/2$ values $\lambda(s,t)$ of an undirected graph take at most $n-1$
distinct values, and that one tree stores them together with a minimum cut for
each pair.

<span id="thm-15-2"></span>**Theorem 15.2.** *Let $G=(V,E)$ be an undirected
graph with nonnegative capacities. There is a tree $T=(V,F)$, with a weight
$w(f)$ on each edge $f\in F$, such that for any two distinct nodes $s$ and $t$
the value $\lambda(s,t)$ is the least weight $w(f)$ of an edge $f$ on the path
of $T$ from $s$ to $t$, and the two components of $T-f$ are the two sides of a
minimum $s$-$t$ cut of $G$.*

A proof is in [Gomory and Hu (1961)](Z-bibliography.md#ref-gomory1961multi),
and one that follows the construction of section 15.4 is in [Gusfield
(1990)](Z-bibliography.md#ref-gusfield1990very). Such a tree is called a
*Gomory-Hu tree*, and it is rarely unique. We remark that its edges need not
be edges of $G$, and that even when they are (as in Figure 15.2) their weights
are usually not the capacities of $G$ (which only bound them from below,
cf. Theorem 15.2); also, $\lambda(G)$ is the weight of the lightest edge of
$T$.

## 15.2 The Hao-Orlin algorithm {#sec-15-2}

### 15.2.1 The method {#sec-15-2-1}

Theorem 15.1 requires $2(n-1)$ maximum flows, each of which costs
$O(n^2\sqrt{m})$ with the highest-label preflow push-relabel method of
[Goldberg and Tarjan (1988)](Z-bibliography.md#ref-goldberg88newapproach) (see
[chapter 12](12-max-flow.md#ch-12)). [Hao and Orlin
(1994)](Z-bibliography.md#ref-hao1994faster) observed that the minimum cuts
from $r$ to all the other nodes can be found in the time of one such
computation. To this aim the sinks are taken one after the other, and a node
that has been the sink becomes a source; the preflow and most distance labels
of one computation then remain valid at the start of the next one. Indeed, the
labels that would become wrong are those of the nodes from which the current
sink can no longer be reached in the residual digraph. Hao and Orlin put these
nodes aside in *dormant* sets, instead of relabelling them, and it brings the
sets back (the last one first) when the current part of the digraph has been
exhausted. At any moment the nodes are therefore split into the sources $S$, a
stack of dormant sets, and the *awake* nodes $W$, among which lies the current
sink $t$. Since the algorithm pushes flow only between awake nodes, the
following invariant holds, and it gives the value of a cut when no awake node
other than $t$ has an excess.

<span id="lem-15-1"></span>**Lemma 15.1.** *During the algorithm, no arc of
the residual digraph goes from $V\setminus W$ to $W$. Hence, when $t$ is the
only awake node with a positive excess, the excess of $t$ equals
$c(\delta^+(V\setminus W))$, and $V\setminus W$ is a minimum cut among those
that contain $S$ but not $t$.*

*Proof.* When a node becomes a source, the arcs out of it are saturated. A set
becomes dormant only when its nodes have no residual arc into $W$, either
because of a gap in the labels or because the single node that becomes dormant
has none; also, no flow is ever pushed from an awake node to a node that is
not awake, and hence no residual arc into $W$ is created afterwards. Thus, the
total excess of $W$ is the flow that enters it, i.e.,
$c(\delta^+(V\setminus W))$. That no cut which separates $S$ from $t$ has a
smaller capacity follows from the argument of the push-relabel method on the
labels of the awake nodes (see [Hao and Orlin
1994](Z-bibliography.md#ref-hao1994faster)). $\square$

We give below the phase that finds the cuts with $r$ inside. For the cuts with
$r$ outside one runs the same algorithm on the reverse digraph (the pushes go
backwards along the arcs), and the cut is then the awake set $W$, which
contains the sink but not $r$.

<span id="alg-15-1"></span>**Algorithm 15.1** (Hao-Orlin, the cuts that
contain $r$).

```
 1  S <- {r}; lambda <- +inf; saturate every arc (r, v) with c(r, v) > 0
 2  label the other nodes by backward breadth-first searches: the first
    search makes the awake set W, every later one a dormant set; t <- a
    node of W of lowest label
 3  loop
 4    while a node v of W - {t} has e(v) > 0, take the one of highest label:
 5      push from v along the residual arcs (v, w), w in W, d(w) = d(v) - 1
 6      if e(v) > 0 still:
 7        if v is alone in its label: the nodes of W of label >= d(v)
          become a new dormant set (a gap)
 8        else if v has no residual arc into W: {v} becomes a dormant set
 9        else d(v) <- 1 + min{ d(w) : (v, w) residual, w in W }
10    if e(t) < lambda then lambda <- e(t); X <- V - W
11    S <- S + {t}; W <- W - {t}; saturate the residual arcs out of t
12    if W is empty: stop if no dormant set is left, else W <- the dormant
      set made last
13    t <- a node of W of lowest label
```

Step 4 is the highest-label rule of the generic method. Steps 7 to 9 replace
the relabelling of the generic method (in `findMinCutOut()` of
`lemon/hao_orlin.h`, the buckets are lists of nodes of equal label, grouped by
dormant set). Step 10 compares the new cut with the best one by an exact `<`,
and step 11 turns the old sink into a source. According to the header, the
running time is $O(n^2\sqrt{m})$ for the two phases with the highest-label
rule, and of $O(nm)$ with unit capacities. Furthermore, [Hao and Orlin
(1994)](Z-bibliography.md#ref-hao1994faster) give the bound $O(nm\log(n^2/m))$
for a version with dynamic trees, which is not the one implemented.

### 15.2.2 The class HaoOrlin {#sec-15-2-2}

The class is declared as

```cpp
template< typename GR , typename CAP = typename GR::template ArcMap< int > ,
          typename TOL = Tolerance< typename CAP::Value > >
class HaoOrlin;
```

where `GR` is the digraph type, `CAP` a readable arc map of capacities of any
numeric type, and `TOL` the tolerance of the comparisons; there are no named
template parameters, and no traits class. Its constructor
`HaoOrlin(graph, capacity, tolerance = Tolerance())` stores references to the
digraph and to the capacities, and `tolerance(tol)` and `tolerance()` set and
read the tolerance afterwards. An execution starts with `init()`, which takes
the first node of the digraph as $r$, or with `init(source)`, which takes the
given one. Then `calculateOut()` runs the phase of Algorithm 15.1 (the cuts
with $r$ inside) and `calculateIn()` the other one (the cuts with $r$
outside), in either order, and one may also call only one of them. Also,
`run()` and `run(s)` call `init()` (respectively `init(s)`), `calculateOut()`
and `calculateIn()`.

Note that `init()` sets the stored value to the largest value of the capacity
type, and each phase replaces the stored cut only if it finds one of strictly
smaller value. Hence, after `calculateOut()` the stored cut is a minimum one
among the sets that contain $r$, and after both phases it is a minimum cut of
the digraph. We read the results by `minCutValue()`, which returns the value,
and by `minCutMap(cutMap)`, which also returns it. The latter writes into a
writable `bool` node map the characteristic vector of the set $X$ of (15.1),
i.e., of the side whose leaving arcs have the least capacity. Of course, the
digraph is assumed to have at least two nodes, since a cut is a nonempty
proper subset of them.

In this class the tolerance decides which arcs exist for the algorithm and
which nodes are active. An arc whose capacity (or residual capacity) is not
positive for the tolerance is ignored by the initial searches and by the
pushes, and an excess is used up when the residual capacity is not less than
it; a node keeps being processed while its excess is nonzero for the
tolerance. We recall that `Tolerance< CAP::Value >` is exact by default on
integer types, while on floating-point ones it compares with the absolute
epsilon of the library (i.e., $a < b$ is $a + \varepsilon < b$) (see [chapter
3](03-mental-model.md#ch-3)).

!!! note "abLEMON"
    The constructor and `tolerance()` are as in LEMON 1.3.1. In abLEMON,
    however, the test that a node still has an excess goes through the
    tolerance (`nonZero()`) instead of a comparison with an exact zero; hence,
    on floating-point capacities a residue of rounding no longer keeps a node
    active.

On an undirected graph one may run `HaoOrlin` on the digraph `Undirector<GR>`
of [chapter 5](05-adaptors.md#ch-5), with the capacities of the original arcs.
By Theorem 15.1 and the symmetry of the cut function, `init()` followed by
`calculateOut()` is then enough (the test `test/hao_orlin_test.cc` runs both
phases on an `Undirector`). In the example below the two phases are run one at
a time on the digraph of Figure 15.1. Its minimum cut with node 0 inside is
$\{0,1,2\}$, of value 5, whereas the minimum cut of the digraph is
$X=\{3,4,5\}$, of value 4, which only the second phase finds; each value is
checked against the enumeration of the $2^6-2$ cuts.

![Figure 15.1. A digraph with the capacity of each arc on its plate; its minimum cut is $X=\{3,4,5\}$, whose two leaving arcs carry 4 units in all.](../figures/15-digraph-cut.svg){#fig-15-1}

```cpp
--8<-- "examples/15-hao-orlin.cc"
```

## 15.3 The Nagamochi-Ibaraki algorithm {#sec-15-3}

### 15.3.1 Maximum adjacency orderings {#sec-15-3-1}

On an undirected graph, [Nagamochi and Ibaraki
(1992)](Z-bibliography.md#ref-nagamochi1992computing) find $\lambda(G)$
without any flow, by orderings of the nodes that certify a lower bound on
$\lambda(u,v)$ for many pairs at once. An ordering $v_1,\ldots,v_n$ of the
nodes is a *maximum adjacency ordering* if $v_1$ is arbitrary and each $v_j$,
$j\geq 2$, maximizes the adjacency $c(v,\{v_1,\ldots,v_{j-1}\})$ among the
nodes $v$ not yet ordered (the adjacency being the capacity of the edges
between $v$ and the nodes already ordered). For an edge $e=v_iv_j$ with $i<j$
we write $q(e)=c(v_j,\{v_1,\ldots,v_i\})$; this is the adjacency of $v_j$ just
after $v_i$ has been ordered, i.e., the key of $v_j$ in the priority queue
when $e$ is scanned.

<span id="lem-15-2"></span>**Lemma 15.2.** *In a maximum adjacency ordering,
every edge $e=v_iv_j$ with $i<j$ satisfies $\lambda(v_i,v_j)\geq q(e)$. In
particular, the last edge scanned into $v_n$ has $q(e)=c(\delta(v_n))$.*

A proof is in [Nagamochi and Ibaraki
(1992)](Z-bibliography.md#ref-nagamochi1992computing), and it is rather short.
It follows that an edge with $q(e)$ not smaller than the value $\mu$ of a cut
already found can be contracted: a cut that separates its ends has capacity at
least $\lambda(v_i,v_j)\geq q(e)\geq\mu$, and it is not better than the one at
hand. Each phase of the algorithm computes one ordering and takes as $\mu$ the
least capacity of a prefix $\{v_1,\ldots,v_k\}$, $1\leq k<n$, of the ordering
(these are cuts, and their capacities are computed while the ordering is
built). Then it contracts all the edges with $q(e)\geq\mu$. Since
$\mu\leq c(\delta(\{v_1,\ldots,v_{n-1}\}))=c(\delta(v_n))$, by the second part
of Lemma 15.2 at least one edge is contracted, and at most $n-1$ phases are
made.

<span id="alg-15-2"></span>**Algorithm 15.2** (Nagamochi-Ibaraki).

```
 1  merge the parallel edges; lambda <- min_v c(delta(v)); X <- that {v}
 2  if lambda = 0: stop
 3  repeat
 4    build a maximum adjacency ordering v_1, ..., v_n' of the current
      nodes from the first one, with q(e) for every edge e
 5    if it does not reach every node: X <- the nodes reached;
      lambda <- 0; stop
 6    mu <- min{ c(delta({v_1, ..., v_k})) : 1 <= k < n' }
 7    if mu < lambda: lambda <- mu; X <- the original nodes of v_1..v_k
 8    contract every edge e with q(e) >= mu, merging parallel edges
 9  until a single node is left
```

In the class, step 1 is `init()`. Steps 4 to 8 are one call of
`processNextPhase()`. In `lemon/nagamochi_ibaraki.h` the keys of the ordering
are kept in a heap that maximizes them, and the nodes merged into a node are
kept in a linked list, which step 7 follows to write the original nodes. Step
5 detects a disconnected graph, whose cut of value 0 is the connected
component of the first node. In abLEMON the comparisons of steps 1 and 7 go
through the tolerance, while those of steps 6 and 8 are exact. A phase costs
$O(m\log n)$ with the binary heap, which gives $O(nm\log n)$ in all; the
header adds that a Fibonacci heap would give $O(nm+n^2\log n)$, and a bucket
heap $O(nm)$ with unit capacities.

### 15.3.2 The class NagamochiIbaraki {#sec-15-3-2}

The class is declared as

```cpp
template< typename GR , typename CM = typename GR::template EdgeMap< int > ,
          typename TR = NagamochiIbarakiDefaultTraits< GR , CM > >
class NagamochiIbaraki;
```

where `GR` is an undirected graph type, `CM` a readable edge map of
capacities, and `TR` the traits class. Its default traits
`NagamochiIbarakiDefaultTraits` define `Value`, `Graph`, `CapacityMap`, the
cross reference `HeapCrossRef` (i.e., `GR::NodeMap< int >`) and the heap
`BinHeap< Value , HeapCrossRef , std::greater< Value > >`, a binary heap that
maximizes the keys. We have two constructors: a first one,
`NagamochiIbaraki(graph, capacity)`, takes the graph and the capacities; the
second one, `NagamochiIbaraki(graph)`, is meant for the named template
parameter `SetUnitCapacity`, whose `Create` type allocates a constant map of
value 1 on the edges; e.g.,
`NagamochiIbaraki< G >::SetUnitCapacity::Create ni(g)` computes the edge
connectivity of `g`.

Furthermore, the class declares the named parameters `SetHeap< H , CR >` and
`SetStandardHeap< H , CR >` (the heap must maximize the keys), and the
function `heap(hp, cr)`, which hands an external heap and its cross reference
to the algorithm. However, in the present code the two named parameters do not
compile: the function that allocates the structures passes the graph to the
`createHeapCrossRef()` of their traits, which takes an `int`. Hence, the
binary heap of the default traits is, for now, the only usable one. The test
`test/nagamochi_ibaraki_test.cc` does not instantiate them.

In the execution, `init()` builds the internal copy of the graph (loops are
skipped and parallel edges are merged) and makes step 1. Then
`processNextPhase()` makes one phase and returns `true` when the algorithm has
finished, `start()` calls it until then, and `run()` is `init()` followed by
`start()`; `tolerance(tol)` and `tolerance()` set and read the
`Tolerance< Value >` of the comparisons. As for the results, `minCutValue()`
returns the value of the best cut found so far, which after `init()` is
already the capacity of a cut (the lightest star of a node) and at the end is
$\lambda(G)$. Similarly, `minCutMap(cutMap)` writes the characteristic vector
of one side of that cut into a writable `bool` node map, and returns its
value. Also, the header warns that the value type must hold the cut values of
the graph, since their sums may otherwise overflow, and notes that the
capacities are supposed to be integer. With floating-point capacities the
result rests on the tolerance and on the exact comparisons of steps 6 and 8,
and the tests of the library cover only integer capacities. In section 15.4.2
an example runs the algorithm phase by phase on the graph of Figure 15.2 (with
three calls of `processNextPhase()` and the value 3).

!!! note "abLEMON"
    `tolerance()` is new in abLEMON. LEMON 1.3.1 compared the cut values
    exactly, whereas abLEMON compares through the tolerance the stars of step
    1, the value 0 that ends step 2, and the update of step 7.

## 15.4 Gomory-Hu trees {#sec-15-4}

### 15.4.1 The construction {#sec-15-4-1}

Gomory and Hu's original construction ([Gomory and Hu
1961](Z-bibliography.md#ref-gomory1961multi)) computes $n-1$ maximum flows on
graphs obtained by contracting parts of $G$. [Gusfield
(1990)](Z-bibliography.md#ref-gusfield1990very) showed that a tree with the
properties of Theorem 15.2 can be obtained by $n-1$ maximum flows on the
original graph, if the cuts are kept in a rooted tree that is rearranged after
each flow. This second construction is the one of `lemon/gomory_hu.h`; there
each node $v$ other than the root $r$ has a parent $p(v)$ and a weight $w(v)$,
the weight of the edge $v\,p(v)$ of the tree.

<span id="alg-15-3"></span>**Algorithm 15.3** (Gomory-Hu tree, the
construction of Gusfield).

```
 1  r <- the first node; p(v) <- r for every v != r
 2  for each node v != r, in the order of the node iterator:
 3    compute a minimum cut (X, V - X) between v and p(v), v in X, and its
      value f, by Preflow on G
 4    w(v) <- f
 5    for each u != v with u in X and p(u) = p(v): p(u) <- v
 6    if p(p(v)) is defined and lies in X:
 7      u <- p(v); p(v) <- p(u); p(u) <- v; w(v) <- w(u); w(u) <- f
 8  number the nodes so that every node comes after its parent
```

Step 3 is a call of `Preflow::runMinCut()` with the tolerance of the class.
Step 7 makes $v$ the parent of its former parent when the cut puts its
grandparent on its side, and step 8 computes the numbering by which the
queries climb the two paths of the tree towards their common ancestor. In all,
$n-1$ maximum flows are computed by the preflow algorithm of [chapter
12](12-max-flow.md#ch-12), and the header states an overall bound of
$O(n^3\sqrt{m})$.

### 15.4.2 The class GomoryHu {#sec-15-4-2}

The class is declared as
`template< typename GR , typename CAP = typename GR::template EdgeMap< int > > class GomoryHu`,
where `GR` is an undirected graph type and `CAP` a readable edge map of
capacities (the constructor checks the `ReadMap` concept on it). Its public
types are `Graph`, `Capacity` and `Value`, and it has no traits class. Its
constructor `GomoryHu(graph, capacity)` stores the references,
`tolerance(tol)` and `tolerance()` set and read the `Tolerance< Value >` given
to the maximum flows, and `run()` executes Algorithm 15.3 (there is no
separate `init()`). One reads the tree by `predNode(v)`, the parent of $v$
(`INVALID` for the root), and by `predValue(v)`, the weight of the edge
between $v$ and its parent (undefined for the root, where it is in fact the
largest value of `Value`). Note that the documentation of `rootDist(v)` calls
it the distance of $v$ from the root, but the function returns the number
given to $v$ by step 8: $0$ for the root, and for any other node a number
larger than that of its parent, distinct for distinct nodes.

A minimum cut between two nodes is read by `minCutValue(s, t)`, which climbs
the two paths of the tree to their common ancestor and returns the least
weight met, in $O(n)$ time. In the same way `minCutMap(s, t, cutMap)` finds
the lightest edge of the path, writes `true` on the side of $s$ and `false` on
the other side into a readable and writable `bool` node map, and returns the
value; for $s=t$ both functions return the largest value of `Value`. We can
also list a cut by two iterators. `MinCutNodeIt(gomory, s, t, side = true)`
lists the nodes of the side of $s$ (or those of the other side if `side` is
`false`), and `MinCutEdgeIt(gomory, s, t, side = true)` lists the edges of the
cut, each once, as the arc that leaves the side of $s$ (or enters it if `side`
is `false`); the latter converts to both `Arc` and `Edge`. Each constructor
calls `minCutMap()` on a node map that the iterator holds, at a cost of
$O(n)$, and the postfix increments return a `Node` (respectively an `Arc`)
instead of an iterator. Since a minimum cut need not be unique,
`MinCutNodeIt(gomory, s, t, true)` and `MinCutNodeIt(gomory, t, s, false)` may
list different nodes, whereas `MinCutNodeIt(gomory, s, t, true)` and
`MinCutNodeIt(gomory, s, t, false)` together list each node once.

!!! note "abLEMON"
    `tolerance()` is new in abLEMON. LEMON 1.3.1 gave its maximum flows the
    default tolerance of the value type, while abLEMON gives them the one set
    on the class.

![Figure 15.2. Left, an undirected graph $G$ and its minimum cut $X=\{0,1,2\}$, of value 3, with the two cut edges; right, the Gomory-Hu tree computed by `GomoryHu`, whose lightest edge (between 2 and 4) gives the same cut.](../figures/15-gomory-hu.svg){#fig-15-2}

Our example below computes the minimum cut of the graph of Figure 15.2 by
`NagamochiIbaraki`, one phase at a time, and then its Gomory-Hu tree. It
checks the value of the cut against the enumeration of all the cuts; for each
of the 15 pairs of nodes, it also checks the value read from the tree against
a maximum flow, the sides given by `minCutMap()`, and the sum of the
capacities listed by `MinCutEdgeIt`. On this graph the tree is the path
$0,1,2,4,3,5$ with the weights $5,7,3,6,6$; hence, e.g., $\lambda(0,5)=3$ and
$\lambda(1,2)=7$.

```cpp
--8<-- "examples/15-undirected-cuts.cc"
```

## 15.5 Choosing an algorithm {#sec-15-5}

For a digraph `HaoOrlin` is the only class of the chapter, and it is arguably
also the natural choice when one needs the best cut on a given side of a node
(the stored cut after `calculateOut()`). For the minimum cut of an undirected
graph, `NagamochiIbaraki` uses no flow, and its phases may be far fewer than
$n-1$, since all the edges with $q(e)\geq\mu$ are contracted at once. On the
other hand, `HaoOrlin` with the first phase only, on the `Undirector`, has the
bound of a single maximum flow. Which of the two is faster typically depends
on the instance, and neither the headers nor the works cited here give a rule
that settles it in general. `GomoryHu` costs $n-1$ maximum flows; it pays off
when the cuts of "many" pairs of nodes are needed, since each one is then read
from the tree in $O(n)$ time, whereas for the cut of one pair (or of a "few"
pairs) a maximum flow of [chapter 12](12-max-flow.md#ch-12) is cheaper.

## 15.6 Exercises {#sec-15-6}

<span id="ex-15-1"></span>**Exercise 15.1.** Prove Theorem 15.1, and check it
on the digraph of Figure 15.1 by computing $\lambda(0,t)$ and $\lambda(t,0)$
with `Preflow` for every $t\neq 0$ and comparing the least of them with
`HaoOrlin::minCutValue()` (the example `manual/examples/15-hao-orlin.cc` gives
the value 4).

<span id="ex-15-2"></span>**Exercise 15.2.** Show that, on the digraph
`Undirector<GR>` of an undirected graph, the value stored after
`calculateOut()` is already $\lambda(G)$, and verify it on the graph of
`test/hao_orlin_test.cc`, whose test runs both phases.

<span id="ex-15-3"></span>**Exercise 15.3.** Compute the edge connectivity of
the graph of Figure 15.2 with
`NagamochiIbaraki< SmartGraph >::SetUnitCapacity::Create`, and check the cut
it gives by counting its edges (the last block of
`test/nagamochi_ibaraki_test.cc` does the same on another graph).

<span id="ex-15-4"></span>**Exercise 15.4.** Print the value `minCutValue()`
of `NagamochiIbaraki` on the graph of Figure 15.2 after `init()` and after
each call of `processNextPhase()`. Explain why the sequence is nonincreasing,
and why its first value is the lightest star of a node (5, at node 0).

<span id="ex-15-5"></span>**Exercise 15.5.** Using only `predNode()` and
`predValue()` of `GomoryHu`, count the distinct values among the
$\lambda(s,t)$ of a graph, and prove that there are at most $n-1$ of them;
check the values against `minCutValue(s, t)` for each pair, as
`test/gomory_hu_test.cc` does against `Preflow`.

<span id="ex-15-6"></span>**Exercise 15.6.** Show that the sum of the
capacities listed by `MinCutEdgeIt(gomory, s, t)` equals `minCutValue(s, t)`,
and that the nodes listed by `MinCutNodeIt` with `side` true and false
partition $V$ (both facts are checked by `test/gomory_hu_test.cc`, the first
also by `manual/examples/15-undirected-cuts.cc`).
