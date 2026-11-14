# 9. Graph search {#ch-9}

We are concerned in this chapter with the visit of the nodes of a digraph
$G=(N,A)$, with $n=|N|$ and $m=|A|$, that can be reached along the arcs from a
given set $S\subseteq N$ of sources. The breadth-first search of `lemon/bfs.h`
visits them in the order of their distance from $S$, i.e., of the least number
of arcs of a path from a source. Instead, the depth-first search of
`lemon/dfs.h` goes as deep as it can along each path before it backtracks.
Both take $O(n+m)$ time, and both are usually the building blocks of many
algorithms of the following chapters (e.g., connectivity, matchings and
flows). Finally, the maximum cardinality search of
`lemon/max_cardinality_search.h` is a third order of visit, driven by a
priority queue; it recognizes chordal graphs, and it underlies the minimum cut
algorithm of Nagamochi and Ibaraki of chapter 15. All three classes follow the
pattern of the algorithm classes of [chapter 3](03-mental-model.md#ch-3)
(traits, named template parameters, `run()`, `init()` and `start()`, queries),
which we recall in [§9.1](#sec-9-1) for the first two.

## 9.1 The common interface {#sec-9-1}

`Bfs` and `Dfs` have the same shape. Their template parameters are the digraph
type `GR` (`ListDigraph` by default) and a traits class `TR`, by default
`BfsDefaultTraits<GR>` and `DfsDefaultTraits<GR>`. These traits give the types
of four maps on the nodes, namely `PredMap`, the arc by which each node was
reached (by default a `NodeMap<Arc>`), `DistMap`, its distance or depth (a
`NodeMap<int>`), `ReachedMap` (a `NodeMap<bool>`) and `ProcessedMap`. By
default, the last one is a `NullMap`, i.e., a map that discards what is
written into it, since most programs do not need it; the named parameter
`SetStandardProcessedMap` makes it a `NodeMap<bool>`.

Named parameters `SetPredMap<T>`, `SetDistMap<T>`, `SetReachedMap<T>` and
`SetProcessedMap<T>` set the type of a map to `T`. With them, the algorithm
does not allocate the map: one of type `T` must be given by the setter of the
same name (`predMap()`, `distMap()`, `reachedMap()` or `processedMap()`)
before `init()`, or an assertion fails. With the default types, a map that is
not given is allocated at `init()` and deleted by the destructor. A map that
is given, on the other hand, typically stays the property of the caller, and
keeps the results after the algorithm object is gone.

In the simplest use, one calls `run(s)`, which visits the nodes reachable from
`s`, and then reads the results by the queries. For more control, `init()`
allocates the maps and marks all the nodes unreached, `addSource(s)` adds a
source (several may be added), and one of the functions `start()` performs the
search. One can also run the search in steps of one node (`processNextNode()`
of `Bfs`) or of one arc (`processNextArc()` of `Dfs`), while `emptyQueue()`,
`queueSize()` and `nextNode()` or `nextArc()` give what is left to do. A
search may be stopped when a target node is reached (`start(t)` and
`run(s, t)`) or when a condition given by a map is met. Finally, `run()`
visits all the nodes, starting a new search from each node that the previous
ones have not reached.

Its queries are `reached(v)`, `dist(v)`, `predArc(v)` and `predNode(v)`
(`INVALID` for a source and for a node not reached), `distMap()` and
`predMap()`. Moreover, `path(t)` returns a path dumper
(cf. [§8.1](08-auxiliary.md#sec-8-1)) of the path from a source to `t` along
the arcs of the predecessor map. We remark that `dist(v)` is undefined for a
node not reached, since `init()` does not write the distances.

Each class also has a function-type interface, `bfs()` and `dfs()`
([§9.4](#sec-9-4)), and a version with a visitor, `BfsVisit` and `DfsVisit`,
which calls the members of an object of the user at each event of the search
in place of filling maps ([§9.5](#sec-9-5)).

## 9.2 Breadth-first search {#sec-9-2}

Let $d(v)$ be the distance of $v$ from $S$, i.e., the least number of arcs of
a path from a node of $S$ to $v$, with $d(v)=\infty$ if there is no such path.
A breadth-first search computes $d(v)$ for all the nodes, together with a
shortest path tree (a forest, if there are several sources). It keeps a queue
of the nodes that are reached but not yet processed, and processes them in the
order in which they entered it; hence, all the nodes at distance $k$ are
processed before any node at distance $k+1$. Algorithm 9.1 gives the steps as
`Bfs` implements them.

<span id="alg-9-1"></span>**Algorithm 9.1** (Breadth-first search, `Bfs`).

```
init():
1  for each node v: pred(v) := INVALID; reached(v) := false; processed(v) := false
2  Q := empty queue
addSource(s):
3  if not reached(s): reached(s) := true; pred(s) := INVALID; d(s) := 0; push s on Q
start():
4  while Q is not empty:
5     u := pop the first node of Q; processed(u) := true
6     for each arc a = uv out of u:
7        if not reached(v): reached(v) := true; pred(v) := a; d(v) := d(u) + 1
8                           push v on Q
```

<span id="thm-9-1"></span>**Theorem 9.1.** *When `start()` returns, the nodes
reached are those with $d(v)<\infty$, and `dist(v)` equals $d(v)$ for each of
them. The arcs `predArc(v)` form a forest in which the path from a source to
$v$ has $d(v)$ arcs, and the search takes $O(n+m)$ time.*

We prove it by induction on $d$, using that the distances of the nodes in the
queue never decrease and differ by at most one (cf. [Cormen et al. 2001,
§22.2](Z-bibliography.md#ref-clrs01algorithms)). Each node enters the queue at
most once and each arc is scanned once, from its source, and therefore the
bound holds; the queue is a vector of $n$ places, allocated by `init()`. Note
that the class does not read $d(u)$ to compute $d(u)+1$: it counts the levels
by remembering where the nodes of the next level begin in the queue, and the
distance of a node is written once, when the node is reached.

Otherwise, the interface is the one of [§9.1](#sec-9-1), and a call of
`processNextNode()` processes the first node of the queue and returns it (the
queue must not be empty). Its form `processNextNode(target, reach)` also sets
`reach` to `true` if `target` is reached in the step, and
`processNextNode(nm, rnode)` sets `rnode`, if it was `INVALID`, to the first
node reached in the step whose value in the `bool` node map `nm` is true.
Accordingly, `start(t)` processes nodes until `t` is reached, i.e., until it
enters the queue (its distance and path being then final), and `start(nm)`
processes them until a node with `nm` true is reached, which it returns (or
`INVALID`). Thus, `run(s, t)` is `init()`, `addSource(s)` and `start(t)`, and
returns `reached(t)`. We remark that the documentation of `run(s, t)` states
that the search stops when `t` is processed, while the code stops it as soon
as `t` is reached; this saves the processing of the rest of its level (usually
a somewhat small saving), and gives the same results for `t`. Finally,
`nextNode()` returns the first node of the queue (or `INVALID`), `queueSize()`
the number of nodes in it, and `emptyQueue()` whether there are none.

Figure 9.1 shows, on the left, the tree and the distances of a breadth-first
search. On the right, the tree of a depth-first search of the same digraph is
the subject of the next section.

![Figure 9.1. A breadth-first search (left) and a depth-first search (right) of the same digraph from s, the out-arcs of each node scanned from top to bottom: the arcs of the two trees, the distance of each node in the first and its depth in the tree in the second.](../figures/09-bfs-dfs.svg){#fig-9-1}

## 9.3 Depth-first search {#sec-9-3}

The depth-first search advances from the last node reached along an arc not
yet scanned, and backtracks to the node it came from when the node has no such
arc left. `Dfs` implements it without recursion, by a stack that holds, for
each node of the current path from the source, the next out-arc to scan from
it (Algorithm 9.2). A node is processed, in the sense of the `ProcessedMap`,
when the search backtracks from it, i.e., when all the nodes reachable from it
through unreached nodes have been visited. In `Dfs`, the value `dist(v)` is
the depth of $v$ in the tree of the search, i.e., the number of arcs of the
path of the tree from the source; it is the height of the stack when $v$ is
reached, and in general it is not a distance in the digraph.

<span id="alg-9-2"></span>**Algorithm 9.2** (Depth-first search with a stack
of arcs, `Dfs`).

```
addSource(s):                                  the stack must be empty
1  if not reached(s): reached(s) := true; pred(s) := INVALID
2     if s has an out-arc: push its first out-arc; depth(s) := 0
3     else: processed(s) := true; depth(s) := 0
processNextArc():
4  a = uv := the arc on the top of the stack
5  if not reached(v): reached(v) := true; pred(v) := a
6                     push the first out-arc of v (INVALID if none); depth(v) := height - 1
7  else: replace the top of the stack by the next out-arc of u
8  while the top of the stack is INVALID:
9     processed(node of the top) := true; pop
10    if the stack is not empty: replace its top by the next out-arc of its source
11 return a
```

Each arc is pushed once and each node is reached once, which gives $O(n+m)$
time; the stack is a vector of $n$ places allocated by `init()`. This
finishing order of the nodes, i.e., the order in which they are processed, has
the properties for which the depth-first search is used ([Tarjan
1972](Z-bibliography.md#ref-tarjan1972depth)). In particular, an arc $uv$
examined while $v$ is still on the stack closes a directed cycle; conversely,
if the digraph is acyclic, each arc $uv$ has $v$ finished before $u$.

<span id="lem-9-1"></span>**Lemma 9.1.** *If $G$ is acyclic, the reverse of
the order in which `run()` processes the nodes is a topological order, i.e.,
the source of each arc precedes its target.*

A proof is in [Cormen et al. 2001,
§22.4](Z-bibliography.md#ref-clrs01algorithms). The example at the end of
[§9.5](#sec-9-5) computes such an order, by recording the nodes as they are
processed.

Otherwise, the interface follows [§9.1](#sec-9-1), with three differences.
First, the unit of work is an arc: `processNextArc()` scans the arc on the top
of the stack and returns it, and `nextArc()` returns the next arc to be
scanned, as an `OutArcIt` (`INVALID` if the stack is empty), while
`queueSize()` is the height of the stack. Second, `addSource()` requires an
empty stack (checked by `LEMON_DEBUG`), since a source added to a search in
progress would make the previous sources lose one of their arcs, and their
depths would be wrong; this is why `run()` adds each new source only after the
previous search has finished. Third, the stopping condition of `start(am)` is
an arc map, not a node map: the search stops before scanning an arc $a$ with
`am[a]` true and returns it, or returns `INVALID` if the search ends first. As
in `Bfs`, `start(t)` and `run(s, t)` stop as soon as `t` is reached, and
`run(s, t)` returns `reached(t)`.

## 9.4 Function-type interfaces {#sec-9-4}

The functions `bfs(g)` and `dfs(g)` return a `BfsWizard` and a `DfsWizard` on
the digraph `g`. Their members set the maps of the algorithm and the outputs,
each returning a new wizard, and their `run()` runs a `Bfs` or a `Dfs` with
them. Named function parameters `predMap(m)`, `distMap(m)`, `reachedMap(m)`
and `processedMap(m)` pass the maps that the algorithm fills. Their types are
deduced from the arguments, so that any writable map may be passed, e.g., a
`LoggerBoolMap` (cf. [chapter 6](06-maps.md#ch-6)) that records the nodes in
the order of processing. For a search between two nodes, `path(p)` gives a
path into which the path to the target is copied, and `dist(d)` a variable
that receives its distance or depth.

Three runs are available, `run(s)`, `run(s, t)`, which returns whether `t` was
reached and fills the path and the distance, and `run()`, which visits all the
nodes. Maps not given are allocated by the algorithm for the duration of the
run, and the path given to `path()` may be of any path type that a path dumper
can be assigned to (by default a `Path`). Also, the classes `BfsWizardBase`
and `DfsWizardBase` hold the pointers to the parameters, and are not used
directly. Two typical calls are the following.

```
bool found = bfs(g).path(p).dist(d).run(s, t);
dfs(g).processedMap(loggerBoolMap(std::back_inserter(order))).run();
```

## 9.5 Visitors {#sec-9-5}

`BfsVisit<GR, VS, TR>` and `DfsVisit<GR, VS, TR>` perform the same searches as
`Bfs` and `Dfs`. However, in place of filling a predecessor and a distance
map, they call a member of a visitor object of type `VS` at each event of the
search; the visitor is given to the constructor together with the digraph.
They keep only a `ReachedMap` (a `NodeMap<bool>` by default, set by
`reachedMap()` or by the named parameter `SetReachedMap<T>`). Their execution
control is that of `Bfs` and `Dfs` (`init()`, `addSource()`,
`processNextNode()` or `processNextArc()`, `nextNode()` or `nextArc()`,
`emptyQueue()`, `queueSize()`, the three `start()` and the three `run()`), and
their only query is `reached(v)`. In the library, the visitors
`BfsVisitor<GR>` and `DfsVisitor<GR>` define the events as members with an
empty body. A visitor of the user may derive from them, and redefine only the
members it needs.

`BfsVisit` calls `start(s)` for each source and `reach(v)` when a node is
reached for the first time (a source included). It calls `process(u)` when a
node is taken from the queue, `discover(a)` when an arc reaches a node not yet
reached (followed by `reach()` of its target), and `examine(a)` when an arc is
scanned whose target was already reached. `DfsVisit` calls `start(s)` and
`stop(s)` when the search from the source `s` begins and ends, and `reach(v)`,
`discover(a)` and `examine(a)` as `BfsVisit` does. It also calls `leave(v)`
when the search backtracks from a node (the moment at which `Dfs` marks it
processed), and `backtrack(a)` when the search returns along the arc `a` of
the tree to its source. Thus, `reach()` and `leave()` delimit the time during
which a node is on the stack, and an `examine(a)` whose target is on the stack
reveals a directed cycle, which the program below exploits. Note that
`DfsVisit::nextArc()` returns an `Arc`, while `Dfs::nextArc()` returns an
`OutArcIt`.

In the following program we compute the distances of a breadth-first search, a
shortest path by `bfs()`, and a topological order by the finishing order of
`dfs()` (Lemma 9.1). Then we find a directed cycle by a visitor of `DfsVisit`.

```cpp
--8<-- "examples/09-search.cc"
```

## 9.6 Maximum cardinality search {#sec-9-6}

The maximum cardinality search orders the nodes of a graph so that each node,
when it is chosen, has the largest number of neighbours among those already
chosen. In abLEMON, `MaxCardinalitySearch<GR, CAP, TR>` extends it to
capacities on the arcs. The cardinality of an unprocessed node $u$ is
$$
\kappa(u) \;=\; \sum_{uv\in A,\ v \text{ processed}} c(uv), \tag{9.1}
$$
and at each step the algorithm processes an unprocessed node of largest
cardinality (Algorithm 9.3), recording its cardinality at that moment. On a
digraph the sum (9.1) is over the arcs from $u$ to the processed nodes, since
the algorithm scans the in-arcs of the node it processes. On an undirected
graph, whose arcs are the two directions of the edges, it is over the edges
that join $u$ to processed nodes; with unit capacities, $\kappa(u)$ is then
the number of processed neighbours of $u$. When the heap becomes empty, i.e.,
when no unprocessed node is joined to a processed one, `run()` takes a new
source among the nodes not reached.

<span id="alg-9-3"></span>**Algorithm 9.3** (Maximum cardinality search,
`MaxCardinalitySearch`).

```
1  H := empty heap of maximum priority; push the source s with priority 0
2  while H is not empty:
3     u := the node of largest priority in H; cardinality(u) := its priority; pop u
4     processed(u) := true
5     for each arc wu entering u:
6        if w was never in H: push w with priority c(wu)
7        else if w is in H: increase the priority of w by c(wu)
```

With unit capacities, the reverse of the order of a maximum cardinality search
of a graph is a perfect elimination order (one in which the neighbours of each
node that follow it form a clique) if and only if the graph is chordal, i.e.,
if every cycle of four or more nodes has a chord ([Tarjan and Yannakakis
1984](Z-bibliography.md#ref-tarjan1984simple)). Such a test can be done in
linear time, and the example at the end of this section implements a simple
version of it. With general capacities, the order is a maximum adjacency
order, which the algorithm of Nagamochi and Ibaraki ([chapter
15](15-min-cut.md#ch-15)) needs.

Its template parameters are the graph type `GR`, the type `CAP` of the map of
the capacities, by default `ConstMap<GR::Arc, Const<int, 1>>` (unit
capacities), and the traits class `TR`, by default
`MaxCardinalitySearchDefaultTraits<GR, CAP>`. Its traits give the
`CardinalityMap` (a `NodeMap` of the value type of the capacities), the
`ProcessedMap` (a `NullMap` by default), the `HeapCrossRef` and the `Heap`.
Note that the heap is a `BucketHeap` of maximum priority for the unit
capacities, and a `BinHeap` with `std::greater`, i.e., again of maximum
priority, otherwise; the internal templates `HeapSelector` and `Selector` of
the traits make this choice. Hence, the search takes $O(n+m)$ time with unit
capacities, and $O(m\log n)$ with the binary heap. Named parameters
`SetCapacityMap<T>`, `SetCardinalityMap<T>`, `SetProcessedMap<T>` and
`SetHeap<H, CR>` change these types and require the map, or the heap, to be
given by the corresponding setter; `SetStandardHeap<H, CR>` changes the type
of the heap, which the algorithm then allocates with its cross reference map.

Its constructor takes the graph and the capacity map, or only the graph, in
which case the map of unit capacities is created. Its setters are
`capacityMap(m)`, `cardinalityMap(m)`, `processedMap(m)` and `heap(hp, cr)`,
and each has a reader without arguments (the comment of the reader
`processedMap()` calls it the cardinality map, but it returns the processed
map). Its execution control is made of `init()`, `addSource(s, capacity)`,
which pushes `s` with the given priority (0 by default) if it was never in the
heap, `processNextNode()`, `nextNode()`, `emptyQueue()` and `emptySize()`,
which, despite its name, is the number of nodes in the heap. Searches are run
by `start()`, `run(s)`, `run()`, and `start(dest)` and `start(nm)`; the last
two stop when `dest`, or a node with `nm` true, is on the top of the heap, and
write its cardinality (and `true` into the `ProcessedMap`) without removing it
from the heap.

Its queries are `cardinality(v)`, i.e., the cardinality of $v$ when it was
processed (undefined for a node never processed), `currentCardinality(v)`, its
priority in the heap (for a node reached but not processed), and
`cardinalityMap()`. Also, `reached(v)` is true if $v$ has ever been in the
heap, and `processed(v)` if it has left it.

In the following program we test two graphs for chordality: we run the search
step by step, recording the order of the nodes, and we check that the
neighbours of each node chosen before it form a clique, and that their number
is the cardinality given by the algorithm.

```cpp
--8<-- "examples/09-max-cardinality.cc"
```

## 9.7 Exercises {#sec-9-7}

<span id="ex-9-1"></span>**Exercise 9.1.** Run `Bfs` from several sources
added with `addSource()` and show that `dist(v)` is the distance from the
nearest source; compare with a single search from a new node joined by an arc
to each source. (The test `test/bfs_test.cc` exercises the interface of
`Bfs`.)

<span id="ex-9-2"></span>**Exercise 9.2.** Use `Bfs` on an undirected graph to
test whether it is bipartite, by checking that no edge joins two nodes at the
same distance from the source of their component; compare the answer with
`bipartite()` of chapter 18.

<span id="ex-9-3"></span>**Exercise 9.3.** Write a visitor of `DfsVisit` that
numbers the nodes in preorder and in postorder, and use the two numbers to
classify the arcs into tree, back, forward and cross arcs; check that a
digraph is acyclic if and only if there is no back arc. (The example
`examples/09-search.cc` detects the back arcs; the test `test/dfs_test.cc`
exercises `Dfs` and `DfsVisit`.)

<span id="ex-9-4"></span>**Exercise 9.4.** Stop a `Dfs` by `start(am)` at the
first arc whose target is on the current path, with an arc map computed on the
fly from the `ReachedMap` and the `ProcessedMap`; explain why a node map would
not suffice for this, which is why `start()` of `Dfs` takes an arc map.

<span id="ex-9-5"></span>**Exercise 9.5.** Run `MaxCardinalitySearch` with the
capacities of the edges of a graph, and verify that the cardinality of the
last node processed, $t$, equals the capacity of a minimum cut between $t$ and
the node $s$ processed just before it (the lemma behind the algorithm of
Nagamochi and Ibaraki), by comparing it with `Preflow` of chapter 12. (The
test `test/max_cardinality_search_test.cc` checks the cardinalities on a small
digraph.)
