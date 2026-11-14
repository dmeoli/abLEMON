# 12. Maximum flows and circulations {#ch-12}

We are concerned in this chapter with the classes of `lemon/preflow.h`,
`lemon/edmonds_karp.h` and `lemon/circulation.h`. Of these, the first two
compute a flow of maximum value between two nodes of a digraph, together with
a cut that proves it maximum, and share the interface of the concept in
`lemon/concepts/max_flow.h`. The third one finds a flow within lower and upper
bounds that meets given supplies, or a set of nodes that proves that no such
flow exists. Section 12.1 states the problem and the duality on which the
results of the chapter rest, sections 12.2 and 12.3 describe the two maximum
flow algorithms, section 12.4 the circulation algorithm, and section 12.5 the
tolerances and the concept that they share. We treat the minimum cost version
of the circulation problem in [chapter 13](13-min-cost-flow.md#ch-13), and the
minimum cuts of a graph with no source and no target in [chapter
15](15-min-cut.md#ch-15).

## 12.1 The problem and its dual {#sec-12-1}

Let $G=(V,A)$ be a digraph with $n$ nodes and $m$ arcs, let
$cap: A\rightarrow\mathbf{R}_{\geq 0}$ be the capacities of the arcs, and let
$s\neq t$ be two nodes, the source and the target. In the maximum flow problem
one looks for a flow $f: A\rightarrow\mathbf{R}$ that solves

$$
\begin{aligned}
\max\ & \sum_{sv\in A} f(sv) - \sum_{vs\in A} f(vs) \\
& \sum_{uv\in A} f(uv) = \sum_{vu\in A} f(vu) \qquad \forall u\in V\setminus\{s,t\} \\
& 0 \leq f(uv) \leq cap(uv) \qquad \forall uv\in A
\end{aligned}
\tag{12.1}
$$

where the objective, i.e., the net flow out of $s$, is the *value* of the
flow, and the equalities impose the conservation of the flow at the nodes
other than $s$ and $t$. A *cut* is a set $S\subset V$ with $s\in S$ and
$t\notin S$, whose capacity

$$
cap(S) = \sum_{uv\in A,\ u\in S,\ v\notin S} cap(uv)
\tag{12.2}
$$

bounds from above the value of any feasible flow, since that value equals the
flow out of $S$ less the flow into it. Given a flow $f$, the *residual
digraph* has an arc $uv$ wherever $f(uv)<cap(uv)$ and an arc $vu$ wherever
$f(uv)>0$, with the residual capacities

$$
r_f(uv) = cap(uv) - f(uv) , \qquad r_f(vu) = f(uv) .
\tag{12.3}
$$

A path from $s$ to $t$ in it is an *augmenting path*, along which the flow may
be increased by the smallest residual capacity of its arcs. All the results of
this chapter rest on the following classical theorem.

<span id="thm-12-1"></span>**Theorem 12.1.** *(Max-flow min-cut.) A feasible
flow $f$ is maximum if and only if the residual digraph of $f$ has no path
from $s$ to $t$. Then the set $S$ of the nodes that $s$ reaches in it is a cut
with $cap(S)$ equal to the value of $f$, and therefore a minimum cut.* For a
proof see [Ahuja, Magnanti and Orlin 1993, chapter
6](Z-bibliography.md#ref-amo93networkflows).

Thus, the answer of the two maximum flow classes can be checked by the user.
Both give, besides the flow and its value, a minimum cut through `minCut()`
and `minCutMap()`, and the capacity (12.2) of that cut, computed on the data,
must equal the value of the flow. Of course, the minimum cut need not be
unique, and one may have to choose among several. In fact, the set reached
from $s$ in the residual digraph is the smallest source side of a minimum cut,
and the complement of the set of the nodes that reach $t$ is the largest one.
The two classes need not give the same one (cf. sections 12.2.2 and 12.3).

![Figure 12.1. The network of the examples of sections 12.2 and 12.3: on each arc its flow in a maximum flow and its capacity; the source side of the minimum cut, {s, a, b, d}, of capacity 12, is on a tint, and the two arcs that leave it are saturated.](../figures/12-min-cut.svg){#fig-12-1}

## 12.2 The push-relabel algorithm: Preflow {#sec-12-2}

### 12.2.1 Preflows and labels {#sec-12-2-1}

We begin with the class `Preflow`, which implements the push-relabel algorithm
of Goldberg and Tarjan ([Goldberg and Tarjan
1988](Z-bibliography.md#ref-goldberg88newapproach); see also [Cormen et
al. 2001](Z-bibliography.md#ref-clrs01algorithms) and [Ahuja, Magnanti and
Orlin 1993, chapter 7](Z-bibliography.md#ref-amo93networkflows)). This
algorithm does not keep a feasible flow during its course. It keeps instead a
*preflow*, i.e., a map $f$ within the bounds of (12.1) whose *excess*
$e_f(u)=\sum_{vu\in A} f(vu)-\sum_{uv\in A} f(uv)$ is nonnegative at the nodes
other than $s$, together with integer *labels* $d(u)$ (the levels of an
`Elevator`, see [chapter 8](08-auxiliary.md#ch-8)) such that

$$
d(u) \leq d(v) + 1 \qquad \text{for each arc } uv \text{ of the residual digraph} .
\tag{12.4}
$$

When $d(t)=0$, condition (12.4) implies that $d(u)$ never exceeds the number
of arcs of a shortest residual path from $u$ to $t$. A node of positive excess
is *active*, and an arc $uv$ of the residual digraph with $d(u)=d(v)+1$ is
*admissible*. A *push* moves along an admissible arc the smaller of $e_f(u)$
and $r_f(uv)$; a *relabel* of an active node with no admissible arc raises
$d(u)$ to one more than the least label of the heads of its residual arcs,
which preserves (12.4). When no node is active, the preflow is a flow, and
(12.4) proves it maximum by Theorem 12.1, since no residual path can then lead
from $s$ (whose label is $n$) to $t$.

The implementation runs in two phases, which are rather different. In the
first one, the excess is pushed towards $t$ only, and the nodes from which $t$
is no longer reachable are lifted to the top level of the elevator (the level
$n$, which marks the nodes that cannot reach $t$). At its end the excess of
$t$ is the maximum flow value, and the nodes at the top level form the source
side of a minimum cut; however, some excess may still be stranded at the nodes
that cannot reach $t$. Hence, a second phase sends that excess back to $s$,
with labels computed from $s$ in the same way, and it leaves a maximum flow.
In both phases the active node of highest label is processed first. Also, a
level that becomes empty lifts the nodes above it to the top, which is the
*gap* heuristic: by (12.4), a node above an empty level cannot reach $t$
through a residual path. In the words of the header, the result is "a mixture
of the highest label and the bound decrease heuristics". In the code, the
second rule is the processing of the active nodes from the current level
downwards, without looking for the highest one again, for $20n$ steps after
each sweep of $n$ steps of the first kind.

<span id="alg-12-1"></span>**Algorithm 12.1** (Preflow, first phase).

```
1  f(sv) := cap(sv) for the arcs sv whose head reaches t; f := 0 elsewhere
2  d(u) := BFS distance from u to t in the residual digraph (n if none)
3  while some node u != t with d(u) < n has e(u) > 0:
4      u := an active node of highest label
5      for each residual arc uv with d(v) < d(u), while e(u) > 0:
6          push min{ e(u), r(uv) } along uv
7      if e(u) > 0:
8          d(u) := min{ n, 1 + min{ d(v) : uv residual } }
9          if the old level of u is now empty:
10             lift the nodes above it to level n
11 return e(t) as the value, { u : d(u) = n } as the source side of the cut
```

Steps 1 and 2 are `init()`, which builds the labels by a breadth first search
backwards from $t$ over the arcs of positive capacity. Note that it saturates
only the arcs out of $s$ whose head can reach $t$. Steps 3 to 10 are
`startFirstPhase()`; step 5 pushes along any residual arc to a lower level,
not only to the level just below, which still preserves (12.4).

As stated in the header, the worst case running time is $O(n^2\sqrt{m})$. In
practice the algorithm is usually the fastest maximum flow method of the
library, and the documentation of the module recommends it "in most cases". We
remark that the header warns against infinite or very large capacities (e.g.,
the largest value of the type), since the initial pushes of step 1 and the
sums of the excesses at a node must fit in the value type.

### 12.2.2 The interface {#sec-12-2-2}

`Preflow<GR, CAP, TR>` takes the type `GR` of the digraph, the type `CAP` of
the capacity map (by default `GR::ArcMap< int >`), and a traits class `TR`, by
default `PreflowDefaultTraits< GR , CAP >`. These traits define the value type
`Value` (that of `CAP`), the type `FlowMap` of the flow map
(`GR::ArcMap< Value >`), the type `Elevator` of the labels
(`Elevator< GR , GR::Node >`, whose alternative is `LinkedElevator`), the
functions that create the last two, and the `Tolerance< Value >` used in the
comparisons. One rarely sets the traits directly, and we do not recommend it.
Instead, the named template parameters `SetFlowMap< T >`, `SetElevator< T >`
and `SetStandardElevator< T >` replace one type each, through their member
`Create`. With `SetFlowMap` and `SetElevator` the object must be given by
`flowMap()` or `elevator()` before `init()`, since the traits they make cannot
create it; `SetStandardElevator`, instead, makes a type that the class still
allocates, from the digraph and the largest level.

The constructor `Preflow(digraph, capacity, source, target)` stores the data
by reference, and the setters `capacityMap()`, `flowMap()`, `source()`,
`target()`, `elevator()` and `tolerance()` change them, each returning the
object; a flow map or an elevator that is not given is allocated by `init()`
and released by the destructor. The execution follows the pattern of [chapter
3](03-mental-model.md#ch-3):

-   `run()` is `init()`, `startFirstPhase()` and `startSecondPhase()`, and
`runMinCut()` is `init()` and `startFirstPhase()`, which suffice for the value
and for the minimum cut;
-   `init()` starts from the zero flow, and `init(flowMap)` from a given map,
which must be at least a preflow: after copying the map, it returns `false` if
the excess of a node other than $s$ is negative beyond the tolerance, and
otherwise it saturates the residual arcs out of $s$ (and empties the arcs into
it) towards the nodes that reach $t$;
-   `flowValue()` returns the excess of $t$, i.e., the maximum value already
after the first phase, while `flow(arc)` and `flowMap()` give a maximum flow
only after the second phase;
-   `minCut(node)` returns `true` if a node is on the source side of the
minimum cut found, and `minCutMap(map)` writes that side into a writable
`bool` node map in $O(n)$ time.

After the first phase, `minCut()` gives the nodes at the top level, i.e.,
those that do not reach $t$ in the residual digraph, which form the largest
source side of a minimum cut. After the second phase it gives the nodes whose
label is below the top, and the header remarks that the two answers may differ
slightly with inexact computation.

!!! note "abLEMON"
    With a floating-point `Value`, `init(flowMap)` judges the excess of a node
    relative to the flow through the node, rather than against the absolute
    epsilon of the tolerance. Hence, the maximum flow of another `Preflow` on
    large capacities (e.g., of the order of $10^9$) is taken as a preflow,
    while LEMON 1.3.1 refused it because the rounding errors of the sums
    exceed that epsilon (trac #625). A program may then start from the maximum
    flow of a previous run after a change of the capacities, provided that the
    old flow is still a preflow for the new ones (e.g., after some capacities
    have grown).

We give below a program that solves the network of Figure 12.1, and that
checks the value against the capacity of the cut. Then it gives the maximum
flow to a second `Preflow`, after the capacity of the arc $(d,t)$ has grown
from 6 to 10; the second run starts from a flow of value 12 and reaches 15.

```cpp
--8<-- "examples/12-max-flow.cc"
```

## 12.3 Augmenting paths: EdmondsKarp {#sec-12-3}

Next, the class `EdmondsKarp` implements the algorithm of [Edmonds and Karp
1972](Z-bibliography.md#ref-edmondskarp72theoretical), i.e., the augmenting
path method of Ford and Fulkerson in which each augmenting path is a shortest
one (in the number of arcs) of the residual digraph. Since the distance from
$s$ to a node in the residual digraph never decreases, while some arc of the
path is saturated at each step, at most $nm/2$ augmentations are made, each of
which costs $O(m)$ for a breadth first search. Thus, the running time is
$O(nm^2)$, which the header gives together with the advice to use `Preflow`
when only the maximum flow is wanted.

<span id="alg-12-2"></span>**Algorithm 12.2** (Edmonds and Karp).

```
1  f := 0 (or a given feasible flow)
2  repeat:
3      find by BFS a path P from s to t with the fewest arcs,
       along the residual arcs uv with r(uv) > 0
4      if there is none: stop, f is maximum
5      delta := min{ r(uv) : uv in P }
6      increase f along P by delta (decrease it on the backward arcs)
```

Steps 3 to 6 are one call of `augment()`, which returns `false` when no path
is found. Then `start()` repeats it, and `run()` is `init()` followed by
`start()`. In our view, the advantage of the class over `Preflow` lies in this
control: between two calls of `augment()` the flow is a feasible flow, whose
value `flowValue()` returns. One can then stop early, look at the intermediate
flows, or interleave the augmentations with some other work, as a teaching or
a debugging program may want.

Its template parameters are those of `Preflow`, with the traits
`EdmondsKarpDefaultTraits< GR , CAP >` (the value type, the type of the flow
map and its creator, and the tolerance) and the single named parameter
`SetFlowMap< T >`. The constructor
`EdmondsKarp(digraph, capacity, source, target)` asserts that $s\neq t$; the
setters `capacityMap()`, `flowMap()`, `source()`, `target()` and `tolerance()`
return the object, and the queries `flowValue()`, `flow(arc)`, `flowMap()`,
`minCut(node)` and `minCutMap(map)` are as in `Preflow`. Here the cut is the
one of the last search, i.e., the set of the nodes reached from $s$ in the
residual digraph when `augment()` failed, which by Theorem 12.1 is the
smallest source side of a minimum cut. In the example of Figure 12.1 the
minimum cut is unique, and the three augmenting paths found by the program of
section 12.2.2 lead to the same set $\{s,a,b,d\}$ as `Preflow`. Finally,
`init(flowMap)` starts from a given flow, and `checkedInit(flowMap)` is kept
as another name of the same function.

!!! note "abLEMON"
    `init(flowMap)` returns whether the map is a feasible flow, with the
    tolerance of the algorithm (conservation at the nodes other than $s$ and
    $t$, and $0\leq f\leq cap$ on the arcs), as `Preflow::init()` does; the
    flow is set in any case. In LEMON 1.3.1 it returned nothing, and only
    `checkedInit()` made the same checks.

## 12.4 Feasible circulations: Circulation {#sec-12-4}

### 12.4.1 The problem and its certificate {#sec-12-4-1}

Let $lower: A\rightarrow\mathbf{R}$ and
$upper: A\rightarrow\mathbf{R}\cup\{\infty\}$, with $lower\leq upper$, be
bounds on the flow of the arcs, and let $sup: V\rightarrow\mathbf{R}$ be
signed supplies (a node with $sup(u)>0$ supplies $sup(u)$ units, one with
$sup(u)<0$ demands $-sup(u)$). In the *network circulation problem* of the
class `Circulation` one looks for any $f$ such that

$$
\sum_{uv\in A} f(uv) - \sum_{vu\in A} f(vu) \geq sup(u) \quad \forall u\in V ,
\qquad lower(uv) \leq f(uv) \leq upper(uv) \quad \forall uv\in A .
\tag{12.5}
$$

The system can be feasible only if $\sum_{u\in V} sup(u)\leq 0$, since its
left-hand sides sum to zero; when the supplies sum to zero, the inequalities
hold as equalities, and (12.5) defines the feasible flows with supplies and
demands. One obtains the opposite inequalities (all the demands met, some
supply left) by reversing the arcs and negating the supplies, as the header
suggests, e.g., with the adaptor `ReverseDigraph` of [chapter
5](05-adaptors.md#ch-5) and the map `NegMap` of [chapter 6](06-maps.md#ch-6).
As a certificate of infeasibility the class gives a *barrier*, i.e., a set
$B\subseteq V$ whose supply exceeds what can leave it:

$$
\sum_{uv\in A,\ u\in B,\ v\notin B} upper(uv) - \sum_{uv\in A,\ u\notin B,\ v\in B} lower(uv) < \sum_{v\in B} sup(v) .
\tag{12.6}
$$

<span id="thm-12-2"></span>**Theorem 12.2.** *(Hoffman.) The system (12.5) has
a solution if and only if no set $B\subseteq V$ satisfies (12.6).* This is the
circulation theorem of [Hoffman 1960](Z-bibliography.md#ref-hoffman1960some),
which in the form with supplies is the feasibility theorem of [Gale
1957](Z-bibliography.md#ref-gale1957theorem). Both follow from Theorem 12.1,
applied to the network that joins a source to the supply nodes and the demand
nodes to a target.

### 12.4.2 The algorithm and its interface {#sec-12-4-2}

`Circulation` is a push-relabel algorithm written directly for (12.5). In it,
the excess of a node is $sup(u)$ plus the flow into it less the flow out of
it; hence, a node of positive excess is one whose inequality is violated. Its
flow starts at the lower bounds (`init()`) or at a greedy choice
(`greedyInit()`), and all the nodes start at level 0. The greedy choice sets
each arc, in the order of the arcs, to the deficit of its head clipped to the
bounds of the arc. Then `start()` pushes from the active node of highest level
along the residual arcs to lower levels, and it relabels as `Preflow` does. It
ends with `false` when a node cannot be lifted (no residual arc leaves it) or
when a lift leaves a level empty; the nodes at or above that level form a
barrier, since no residual arc leaves them and their excess is positive. A
comment in `init()` records that a global relabeling was tried and gave
somewhat worse running times on random digraphs; we know of no bound on the
running time stated in the header, and we give none.

<span id="alg-12-3"></span>**Algorithm 12.3** (Circulation).

```
1  set f by the greedy rule; e(u) := sup(u) + f(in(u)) - f(out(u))
2  d(u) := 0 for each node u
3  while some node has e(u) > 0:
4      u := an active node of highest level
5      push from u along residual arcs uv with d(v) < d(u)
6      if e(u) > 0:
7          if no residual arc leaves u: L := n; return false
8          d(u) := 1 + min{ d(v) : uv residual }
9          if the old level of u is now empty: L := old level; return false
10 return true            (the barrier is { u : d(u) >= L } after false)
```

Its template parameters are `GR`, the lower bound map `LM` (by default
`GR::ArcMap< int >`), the upper bound map `UM` (by default `LM`), the supply
map `SM` (by default `GR::NodeMap< UM::Value >`), whose value type is the
`Value` of the algorithm, and the traits
`CirculationDefaultTraits< GR , LM , UM , SM >`. Its named parameters
`SetFlowMap< T >`, `SetElevator< T >` and `SetStandardElevator< T >` are those
of `Preflow`, and its constructor `Circulation(graph, lower, upper, supply)`
takes the three maps, which `lowerMap()`, `upperMap()` and `supplyMap()`
change, while `flowMap()`, `elevator()` and `tolerance()` are as in `Preflow`.
`run()` is `greedyInit()` followed by `start()`, and it returns whether a
feasible circulation was found. In that case `flow(arc)` and `flowMap()` give
it; otherwise `barrier(node)` and `barrierMap(map)` give the barrier (which is
empty after a feasible run). Two checkers verify an answer on the data:
`checkFlow()` tests the bounds and the inequalities of (12.5), the latter up
to the tolerance, and `checkBarrier()` tests (12.6) on the barrier found, an
infinite upper bound out of the set making the test fail.

Our second program solves a network of four nodes, and then it lowers the
capacity of the arc $(b,d)$ from 4 to 1. The node $d$, which demands 3 units,
can then receive at most 2, `run()` returns `false`, and the program checks
(12.6) on the barrier.

```cpp
--8<-- "examples/12-circulation.cc"
```

!!! note "abLEMON"
    On a digraph without nodes `Circulation` read the first of no items of its
    elevator in LEMON 1.3.1; now the elevator holds one item more than the
    digraph has, and the empty digraph has the empty circulation.

## 12.5 Tolerances and the common interface {#sec-12-5}

All three classes compare the values of the flows through a
`Tolerance< Value >` (see [chapter 3](03-mental-model.md#ch-3)), which is set
and read by `tolerance()`. A residual capacity counts as positive only beyond
the epsilon of the tolerance, and so does an excess; also, a push saturates an
arc unless its residual capacity exceeds the excess by more than the epsilon.
For an integer `Value` the comparisons are exact. For a floating-point one the
default epsilon of `Tolerance` ($10^{-10}$ for `double`) is absolute, which
usually suits capacities of moderate size, and a program with data of a rather
different scale should set another one. Indeed, the only comparison that is
relative to the data is the one of `Preflow::init(flowMap)` (cf. section
12.2.2).

`Preflow` and `EdmondsKarp` meet the concept `concepts::MaxFlow< GR , CAP >`
of `lemon/concepts/max_flow.h`, against which a class is checked by
`checkConcept` (see [chapter 3](03-mental-model.md#ch-3)). The concept
requires the member `Create` and the named parameter `SetFlowMap`, the
constructor from the digraph, the capacities, the source and the target, and
the setters `capacityMap()`, `flowMap()`, `source()`, `target()` and
`tolerance()`. It also requires the two `init()` functions and `run()`, and
the queries `flowValue()`, `flow()`, `flowMap()`, `minCut()` and
`minCutMap()`; a function template written against these members accepts
either class.

!!! note "abLEMON"
    The concepts of the maximum flow, minimum cost flow and minimum mean cycle
    classes are in `lemon/concepts` (`MaxFlow`, `MinCostFlow` and
    `MinMeanCycle`), where the classes of the library are held to them; in
    LEMON 1.3.1 they were defined inside the tests.

## 12.6 Exercises {#sec-12-6}

<span id="ex-12-1"></span>**Exercise 12.1.** Modify the program of section
12.2.2 so that it stops `EdmondsKarp` after its first augmenting path and
prints the residual capacities (12.3) of the arcs. Then check by hand that the
value of the flow and the number of paths agree with Algorithm 12.2. (The
program `manual/examples/12-max-flow.cc` counts the paths.)

<span id="ex-12-2"></span>**Exercise 12.2.** Show that the set of the nodes
from which $t$ is not reachable in the residual digraph of a maximum flow is a
minimum cut, and that it contains the cut of Theorem 12.1. On the network of
Figure 12.1, add an arc that makes the two sets differ, and compare
`minCutMap()` of `Preflow` after `runMinCut()` with that of `EdmondsKarp`.

<span id="ex-12-3"></span>**Exercise 12.3.** Give `Preflow::init(flowMap)` a
map that is not a preflow (e.g., one unit more out of a node than into it),
and check that it returns `false`. Then repeat with capacities of the order of
$10^9$ in `double`, where the absolute epsilon of the tolerance would refuse a
true preflow. (Solved by `checkInitPreflowLarge()` in
`test/max_flow_test.cc`.)

<span id="ex-12-4"></span>**Exercise 12.4.** Write a function template that
takes a class meeting `concepts::MaxFlow`, runs it, and returns the capacity
(12.2) of its cut minus its value; instantiate it with both `Preflow` and
`EdmondsKarp`. (The concept is checked in `test/max_flow_test.cc`.)

<span id="ex-12-5"></span>**Exercise 12.5.** Solve a problem with the opposite
inequalities of (12.5), i.e., all the demands met and some supply left, by
`Circulation` on a `ReverseDigraph` with a `NegMap` of the supplies, and
verify the answer with `checkFlow()`. (The class is tested in
`test/circulation_test.cc`.)

<span id="ex-12-6"></span>**Exercise 12.6.** Prove Theorem 12.2 from Theorem
12.1, by joining a source to the supply nodes and the demand nodes to a
target, and by replacing each lower bound with a shift of the supplies. Then
check (12.6) on the barrier of the second network of
`manual/examples/12-circulation.cc`.
