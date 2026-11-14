# 11. Spanning trees and arborescences {#ch-11}

We are concerned in this chapter with two problems of choosing a cheapest set
of arcs that connects the nodes of a graph. The first one is the minimum
spanning tree of an undirected graph, which `kruskal()` of `lemon/kruskal.h`
solves by Kruskal's algorithm on a union-find structure
([§8.2](08-auxiliary.md#sec-8-2)). Its directed counterpart, the second one,
is the minimum cost arborescence of a digraph rooted at a given node;
`MinCostArborescence` of `lemon/min_cost_arborescence.h` solves it by the
algorithm of Chu, Liu and Edmonds, and also gives an optimal dual solution,
which proves the optimality of the arborescence. The two problems look alike;
however, the greedy choice that solves the first is not enough for the second,
whose algorithm has to contract cycles (cf. Figure 11.2).

## 11.1 Minimum spanning trees {#sec-11-1}

Let $G=(N,E)$ be an undirected graph with costs $c : E\to\mathbb{R}$ on the
edges. A spanning tree is a set $T\subseteq E$ of $n-1$ edges that contains no
cycle; if $G$ is connected, it therefore connects all the nodes. We consider
the problem
$$
\min\Big\{\, c(T)=\sum_{e\in T} c(e) \;:\; T \text{ is a spanning tree of } G \,\Big\}. \tag{11.1}
$$
If $G$ is not connected, the same algorithm gives a spanning forest of least
cost, i.e., a minimum spanning tree of each connected component. For a
spanning tree $T$ and an edge $f\notin T$, let $P_T(f)$ be the path of $T$
between the ends of $f$. For an edge $e\in T$, let $C_T(e)$ be the set of the
edges between the two components of $T\setminus\{e\}$. Since the optimal trees
are characterized by two equivalent conditions, one can check a tree in either
way.

<span id="thm-11-1"></span>**Theorem 11.1.** *A spanning tree $T$ is optimal
for (11.1) if and only if*
$$
c(f) \;\ge\; c(e) \qquad \text{for each } f\notin T \text{ and each } e\in P_T(f), \tag{11.2}
$$
*or, equivalently, if and only if*
$$
c(e) \;\le\; c(f) \qquad \text{for each } e\in T \text{ and each } f\in C_T(e). \tag{11.3}
$$

A proof is in [Ahuja, Magnanti and Orlin 1993,
§13.3](Z-bibliography.md#ref-amo93networkflows). In short, an edge that
violates (11.2) or (11.3) gives a cheaper tree by an exchange; conversely, a
tree that satisfies them can be turned into any optimal tree by exchanges that
do not increase its cost.

Kruskal's algorithm ([Kruskal
1956](Z-bibliography.md#ref-kruskal1956shortest)) builds a tree that satisfies
(11.2) greedily. It scans the edges in nondecreasing order of cost, and keeps
an edge if it joins two different components of the forest of the edges kept
so far, i.e., if it closes no cycle (Algorithm 11.1). Thus, an edge that is
discarded closes a cycle of edges kept before it, none of which costs more,
and (11.2) holds at the end.

<span id="alg-11-1"></span>**Algorithm 11.1** (Kruskal's algorithm,
`kruskal()`).

```
1  if the costs are given as a map: sort the edges by nondecreasing cost
2  make a union-find structure with one class per node; total := 0
3  for each edge e = uv in the order of the costs:
4     if join(u, v):                    [u and v were in different classes]
5        out(e) := true; total := total + c(e)
6     else: out(e) := false
7  return total
```

Sorting takes $O(m\log m)$ time, and the $m$ unions and finds take
$O(m\,\alpha(m,n))$ by Theorem 8.1, which is usually the dominating cost when
the edges are given already sorted. Note that the loop scans all the edges
even after the tree is complete, since the output receives a value for each of
them; the work after the last useful edge is, however, a union-find operation
per edge.

**Interface.** Function `kruskal(g, in, out)` accepts various types for its
arguments, and returns the cost of the tree (or forest) found, of the type of
the costs. Its graph `g` may be undirected, with the costs on its edges, or
directed, in which case the arcs are taken as edges and their direction is
disregarded. Its costs `in` may be given either (i) as a readable map on the
edges (or arcs), whose values are the costs, or (ii) as an STL container of
pairs `std::pair<GR::Edge, C>` (or `std::pair<GR::Arc, C>`), where `C` is the
type of the costs. In case (i), the function collects the pairs of each item
and its cost into a vector, and sorts it by cost with `std::sort`; in case
(ii), the pairs must be already sorted by nondecreasing cost, and no sort is
done.

Its output `out` may also be given in two ways. It may be (i) a writable map
on the edges with `bool` values, which receives `true` for the edges of the
tree and `false` for the others, each edge being written once. Alternatively,
it may be (ii) an output iterator, e.g., `std::back_inserter(tree)`, into
which the edges of the tree are copied in the order in which they are found
(the function wraps it in a `LoggerBoolMap`, cf. [chapter
6](06-maps.md#ch-6)). A second overload takes `out` by constant reference, so
that a temporary iterator may be passed.

This choice among the forms is made at compile time by the helper templates of
the namespace `_kruskal_bits`, which are not part of the interface. They are
`KruskalValueSelector`, `KruskalInputSelector`, `KruskalOutputSelector`,
`SequenceInputIndicator`, `MapInputIndicator`, `SequenceOutputIndicator`,
`MapOutputIndicator`, `RemoveConst` and `PairComp`; they recognize a sequence
by a `value_type` with a `first_type`, and a map by its `Value`.

Figure 11.1 shows the tree of the example below. Its edges, sorted by cost,
are kept in the order 1, 2, 3, 5, 6, while the edges of cost 4, 7, 8 and 9
close cycles.

![Figure 11.1. The minimum spanning tree found by Kruskal's algorithm, of total cost 17, with the cost of each edge.](../figures/11-kruskal.svg){#fig-11-1}

In the following program we compute the tree twice, with a map of costs in and
a map out, and then with a sorted vector of pairs in and an output iterator
out. We also check (11.2), by a search of the path of the tree between the
ends of each edge not in the tree, on the graph of the adaptor `FilterEdges`
([chapter 5](05-adaptors.md#ch-5)).

```cpp
--8<-- "examples/11-kruskal.cc"
```

## 11.2 Minimum cost arborescences {#sec-11-2}

Let $G=(N,A)$ be a digraph with costs $c : A\to\mathbb{R}$ and a root $r$, and
let $R\subseteq N$ be the set of the nodes reachable from $r$. An arborescence
rooted at $r$ is a set $B$ of arcs in which each node of $R\setminus\{r\}$ has
one entering arc, and from which each node of $R$ can be reached from $r$; in
other words, it is a spanning tree of $R$ whose arcs are directed away from
$r$. We look for one of least cost $c(B)=\sum_{a\in B}c(a)$. With several
sources $s_1,\dots,s_q$ in place of $r$, it is to find the cheapest union of
arborescences rooted at the sources that reaches all the nodes reachable from
them. It can be written as the linear program
$$
\min\Big\{\, \sum_{a\in A} c(a)\,x_a \;:\; \sum_{a\in\delta^-(X)} x_a \ge 1 \ \ \forall\, \emptyset\neq X\subseteq R\setminus\{r\},\ \ x\ge 0 \,\Big\}, \tag{11.4}
$$
where $\delta^-(X)$ is the set of the arcs entering $X$; its optimal vertices
are the incidence vectors of the minimum cost arborescences ([Edmonds
1967](Z-bibliography.md#ref-edmonds1967optimum)). Its dual has a variable
$y_X\ge 0$ for each set $X$. By complementary slackness, an arborescence $B$
is optimal if there is a $y\ge 0$ with
$$
\sum_{X \,:\, a\in\delta^-(X)} y_X \;\le\; c(a) \quad \text{for each arc } a, \text{ with equality for each } a\in B, \tag{11.5}
$$
and with $y_X>0$ only for sets $X$ entered by one arc of $B$; then
$c(B)=\sum_X y_X$.

Choosing the cheapest arc entering each node, which would mirror the greedy
algorithm of the undirected case, may give cycles: in Figure 11.2, the
cheapest arcs entering $a$ and $b$ form the cycle $a,b,a$. Thus, the algorithm
of [Chu and Liu 1965](Z-bibliography.md#ref-chu1965shortest) and [Edmonds
1967](Z-bibliography.md#ref-edmonds1967optimum) handles them by contraction
(Algorithm 11.2). Each node, and later each contracted set $X$, chooses the
arc of least reduced cost that enters it. That reduced cost is then subtracted
from the reduced costs of all the arcs entering $X$, and it becomes the dual
variable $y_X$. A cycle of chosen arcs, all of which have reduced cost zero at
that point, is contracted into a new set, which chooses in its turn. When the
chosen arcs reach the part of the digraph that is already connected to a
source, the contractions are expanded: the arc that enters a contracted set
from outside replaces the chosen arc of the node it enters, and the other
chosen arcs of the set stay.

![Figure 11.2. A minimum cost arborescence rooted at r: the cheapest arcs entering a and b, and those entering c and d, form two cycles, each contracted (the tinted sets) and then entered by one arc of the arborescence.](../figures/11-arborescence.svg){#fig-11-2}

<span id="alg-11-2"></span>**Algorithm 11.2** (Minimum cost arborescence by
contractions, `MinCostArborescence`).

```
addSource(r):    mark the nodes reachable from r; r is processed
processNextNode():                     v := the last node reached; skip it if processed
1  X := {v}; path := (X)
2  loop:
3     a = uw := an arc of least reduced cost c'(a) entering X from outside X
4     new dual variable y_X := c'(a); c'(b) := c'(b) - c'(a) for each arc b entering X
5     if u is processed: break
6     if u is in a set on the path: X := the union of the sets of the path from that set
                                    to the top (the cycle is contracted)
7     else: X := {u}; push X on the path
8  expand: from a, give each node of the path its entering arc among the chosen ones,
           taking them in the order in which they were chosen; mark the nodes processed
```

<span id="thm-11-2"></span>**Theorem 11.2.** *When `start()` returns, the arcs
`pred(v)` form a minimum cost arborescence of the nodes reachable from the
sources, and the dual variables $y_X$ satisfy (11.5) with it; hence,
`dualValue()` equals `arborescenceCost()`. The algorithm takes $O(n^2+m)$
time.*

Optimality is proved in [Edmonds
1967](Z-bibliography.md#ref-edmonds1967optimum) (see also [Schrijver
2003](Z-bibliography.md#ref-schrijver03combinatorial)), and the bound on the
time is the one stated by the documentation of the class. Faster
implementations, in $O(m+n\log n)$, exist ([Gabow et
al. 1986](Z-bibliography.md#ref-gabow1986efficient)), but the library does not
use them. Note that the dual solution makes the result somewhat easier to
check, as the example below does: (11.5) can be verified arc by arc, and the
sum of the $y_X$ must equal the cost of the arborescence.

**Interface.** `MinCostArborescence<GR, CM, TR>` takes the digraph type `GR`,
the type `CM` of the cost map (by default `GR::ArcMap<int>`), and the traits
class `TR`, by default `MinCostArborescenceDefaultTraits<GR, CM>`. Its traits
give `CostMap`, `Value` (the type of the costs), `ArborescenceMap` and
`PredMap`. The `ArborescenceMap` (by default an `ArcMap<bool>`) receives
`false` on each arc at `init()`, and then `true` once on each arc of the
arborescence; the `PredMap` is a `NodeMap<Arc>`. These traits also give
`HeapCrossRef` and `Heap`, i.e., a `NodeMap<int>` and a `BinHeap` on integer
priorities, which the expansion uses. Named parameters `SetArborescenceMap<T>`
and `SetPredMap<T>` set the types of the two maps, which must then be given by
`arborescenceMap(m)` and `predMap(m)`. Similarly, `SetHeap<H, CR>` sets the
type of the heap and of its cross reference map, which the algorithm
constructs on the digraph with $-1$ as the value of each node.

!!! note "abLEMON"
    `MinCostArborescence` takes the type of its heap from its traits class,
    with the named parameter `SetHeap`; LEMON 1.3.1 used a `BinHeap` that
    could not be changed.

Its constructor takes the digraph and the cost map, and the setters are
`arborescenceMap(m)` and `predMap(m)`; the maps not given are allocated by
`init()` and deleted by the destructor. Its execution control is made of
`init()`, `addSource(s)`, `processNextNode()`, `queueSize()`, `emptyQueue()`,
`start()` and `run(s)`, which is `init()`, `addSource(s)` and `start()`. A
call `addSource(s)` marks the nodes reachable from `s`, and may be repeated
for several sources. Despite the word "queue" of the documentation, the nodes
waiting to be processed are taken in the reverse of the order in which they
were reached.

Its queries on the arborescence are `arborescence(a)`, which is true if the
arc $a$ is the predecessor of its target, `arborescenceMap()`, `pred(v)`,
`predMap()`, and `arborescenceCost()`, computed in $O(m)$ from the
predecessors. Also, `reached(v)` is true if $v$ is reachable from a source,
and `processed(v)` if it has been processed. Its queries on the dual solution
are `dualNum()`, the number of the dual variables (some of which may have
value zero), and `dualValue(k)` and `dualSize(k)`, the value and the number of
nodes of the $k$-th one; `dualValue()` is the sum of all of them. An iterator
`DualIt(mca, k)` lists the nodes of the $k$-th set, and is compared with
`INVALID` to stop. Also, the internal records `CostArc`, `StackLevel` and
`DualVariable` hold the arcs of least reduced cost, the levels of the path and
the dual variables. Finally, the function
`minCostArborescence(g, cost, source, arborescence)` runs the algorithm from
one source, writes the arborescence into the given `bool` arc map, and returns
its cost.

In the following program we solve the instance of Figure 11.2 and read the
dual solution. Then we check (11.5) arc by arc and the equality of the primal
and dual values, and call the function-type interface.

```cpp
--8<-- "examples/11-arborescence.cc"
```

## 11.3 Exercises {#sec-11-3}

<span id="ex-11-1"></span>**Exercise 11.1.** Prove that the tree returned by
Kruskal's algorithm satisfies (11.3) as well as (11.2), and check both
conditions on random graphs by the method of the example
`examples/11-kruskal.cc`. (The test `test/kruskal_test.cc` runs `kruskal()`
with each form of input and output.)

<span id="ex-11-2"></span>**Exercise 11.2.** Compute a maximum spanning tree
with `kruskal()`, through a map that negates the costs (e.g., `negMap()` of
chapter 6), and explain why the greedy algorithm remains correct.

<span id="ex-11-3"></span>**Exercise 11.3.** Run `kruskal()` on a digraph and
on the graph of the adaptor `Undirector` of the same digraph, and verify that
the two trees have the same cost.

<span id="ex-11-4"></span>**Exercise 11.4.** Give a digraph on which choosing
the cheapest arc entering each node, and then breaking each cycle at its most
expensive arc, does not give a minimum cost arborescence, and check the answer
of `MinCostArborescence` on it. (The test `test/min_cost_arborescence_test.cc`
checks the dual solution as the example `examples/11-arborescence.cc` does.)

<span id="ex-11-5"></span>**Exercise 11.5.** Use two sources with
`addSource()` and verify that the result is the cheapest union of two
arborescences rooted at them; compare it with a single run from a new root
joined to the two sources by arcs of cost zero.
