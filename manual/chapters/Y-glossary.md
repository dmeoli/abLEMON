# Y. Glossary {#app-y}

The terms defined in the chapters, each with the section that defines it.

**2-optimal tour**: a tour that no 2-opt move, i.e., the replacement of two of
its edges by the two other edges that reconnect it, makes cheaper
([§19.3](19-tsp-clique.md#sec-19-3)).

**active item**: in an elevator, an item marked as active, e.g., a node with a
positive excess in a push-relabel algorithm; in the Bellman-Ford algorithm, a
node whose distance changed in the last round
([§8.3](08-auxiliary.md#sec-8-3), [§10.4](10-shortest-paths.md#sec-10-4)).

**admissible arc**: in a push-relabel or cost scaling algorithm, a residual
arc along which flow may be pushed: one whose head is on a lower level than
its tail (maximum flow), or one of negative reduced cost (cost scaling)
([§12.2](12-max-flow.md#sec-12-2), [§13.6](13-min-cost-flow.md#sec-13-6)).

**admissible heuristic**: a heuristic $h$ of A* with
$h(v) - h(t) \le \delta(v,t)$ for each node $v$, i.e., one that never
overestimates the distance of a node to the target when $h(t)=0$
([§10.3](10-shortest-paths.md#sec-10-3)).

**alteration notifier**: the object of a graph, one for each kind of item,
that informs the attached observers (the maps among them) of each item added
or erased, and of the graph being built or cleared
([§A.3](A-writing-graph-map.md#sec-a-3)).

**alternating path**: a path whose edges are alternately in a matching and out
of it; it is augmenting when neither of its ends is covered by the matching
([§17.1](17-matching.md#sec-17-1)).

**amortized cost**: a bound on the total cost of any sequence of operations
from the empty structure, divided among its operations, e.g., by means of a
potential function ([§7.1](07-heaps.md#sec-7-1)).

**arborescence**: a set of arcs in which each node reachable from the root,
the root excepted, has one entering arc and can be reached from the root
([§11.2](11-spanning.md#sec-11-2)).

**arc set**: a digraph (or, as an edge set, a graph) that has its own arcs on
the nodes of another graph, such as ListArcSet and SmartEdgeSet
([§4.7](04-graphs.md#sec-4-7)).

**archetype**: a concept class whose members are declared with empty bodies,
so that a template that compiles with it uses only what the concept guarantees
([§3.5](03-mental-model.md#sec-3-5)).

**assignment**: a matching of a bipartite graph that covers every red node;
MinCostAssignment finds one of least cost ([§17.3](17-matching.md#sec-17-3)).

**augmenting path**: a directed path from the source to the sink in the
residual digraph of a flow; sending along it its smallest residual capacity
increases the value of the flow ([§5.4](05-adaptors.md#sec-5-4)).

**awake set**: in the Hao-Orlin algorithm, the nodes among which the preflow
is pushed towards the current sink, as opposed to the sources and to the
dormant sets ([§15.2](15-min-cut.md#sec-15-2)).

**back arc**: in a depth-first search of a digraph, an arc whose target has
been reached and not yet left; a digraph is a DAG if and only if no search
meets one ([§18.3](18-properties.md#sec-18-3)).

**barrier (circulation)**: a set of nodes whose supply exceeds what the upper
bounds of the arcs leaving it, less the lower bounds of the arcs entering it,
can carry, which proves a circulation problem infeasible
([§12.4](12-max-flow.md#sec-12-4)).

**barrier (matching)**: a set of nodes whose removal leaves more odd
components than it has nodes, which bounds from below the number of nodes
uncovered by any matching; for fractional matchings, a set S with |S| − |N(S)|
as large as possible ([§17.1](17-matching.md#sec-17-1)).

**base node**: the node on which an incidence iterator was constructed; the
other end of its current arc or edge is the running node
([§3.3](03-mental-model.md#sec-3-3)).

**basis**: in the network simplex method, a spanning tree of the arcs together
with the bound at which each other arc lies; a basis of a vertex of the
polyhedron of the flows is a spanning tree containing its free arcs
([§13.4](13-min-cost-flow.md#sec-13-4), [§14.4](14-k-best-flows.md#sec-14-4)).

**bi-edge-connected component**: a class of the nodes of an undirected graph
joined by two edge-disjoint paths; the components are separated by the bridges
([§18.2](18-properties.md#sec-18-2)).

**bind arc**: the arc of a SplitNodes adaptor from the in-node to the out-node
of a node of the digraph, which carries the data of the node
([§5.5](05-adaptors.md#sec-5-5)).

**binomial heap**: a heap made of a list of heap-ordered binomial trees of
distinct degrees, like the binary digits of the number of items
([§7.3](07-heaps.md#sec-7-3)).

**bipartite graph**: a graph whose nodes split into two sets with no edge
inside either of them, i.e., a graph without cycles of odd length
([§18.1](18-properties.md#sec-18-1)).

**block**: a bi-node-connected component of an undirected graph, i.e., a class
of the edges that lie on a common cycle; the blocks are separated by the cut
nodes ([§18.2](18-properties.md#sec-18-2)).

**blossom**: an odd cycle closed by an alternating search tree, which Edmonds'
algorithms shrink into a single node and expand later
([§17.4](17-matching.md#sec-17-4)).

**breadth-first search**: the visit of the nodes reachable from the sources in
the order of their distance, i.e., of the least number of arcs of a path from
a source ([§9.2](09-search.md#sec-9-2)).

**bridge**: an edge whose removal increases the number of connected components
of a graph, also called a cut edge ([§18.2](18-properties.md#sec-18-2)).

**bucket heap**: a heap of non-negative integer priorities that keeps a list
of the items of each priority in a vector indexed by the priority
([§7.4](07-heaps.md#sec-7-4)).

**caption**: the optional name that follows the type in the header line of an
LGF section, by which a reader selects one of several sections of the same
type ([§21.1](21-io.md#sec-21-1)).

**cascading cut**: in a Fibonacci heap, the cut of a node that has lost a
second child since it became a child, repeated upwards
([§7.3](07-heaps.md#sec-7-3)).

**clique**: a set of pairwise adjacent nodes of an undirected graph
([§19.5](19-tsp-clique.md#sec-19-5)).

**closed set (residual digraph)**: a set of nodes that no residual arc of a
flow leaves; for a maximum flow, the minimum cuts are the closed sets that
contain the source and not the target ([§12.1](12-max-flow.md#sec-12-1),
[§14.1](14-k-best-flows.md#sec-14-1)).

**combinatorial embedding**: the cyclic order of the edges around each node of
a planar graph, which describes a plane drawing up to deformation
([§18.5](18-properties.md#sec-18-5)).

**concept**: the list of the types and functions that a class of a kind
(graph, map, heap) must provide, written as a class of lemon/concepts/ and
checked by checkConcept ([§3.5](03-mental-model.md#sec-3-5)).

**condensation**: the acyclic digraph with one node per strongly connected
component of a digraph and an arc between two components whenever an arc of
the digraph joins them ([§18.3](18-properties.md#sec-18-3)).

**consistent heuristic**: a heuristic $h$ of A* with $h(u)\le\ell(uv)+h(v)$ on
each arc, with which each node is scanned at most once
([§10.3](10-shortest-paths.md#sec-10-3)).

**core number**: the largest k such that a node belongs to a subgraph of
minimum degree k; the largest core number is the degeneracy of the graph
([§6.6](06-maps.md#sec-6-6)).

**critical arc**: an arc of an augmenting path whose residual capacity equals
the bottleneck, and which the augmentation removes from the residual digraph
([§5.4](05-adaptors.md#sec-5-4)).

**cross reference map**: the map from the items to integers that a heap shares
with its user, through which it finds the place of an item and records its
state (PRE_HEAP, IN_HEAP, POST_HEAP) ([§A.5](A-writing-graph-map.md#sec-a-5)).

**cross-reference map**: the map from the items to int through which a heap
finds an item in its storage, holding its position while the item is in the
heap and its state otherwise ([§7.1](07-heaps.md#sec-7-1)).

**cut**: a set of nodes containing the source and not the target; its capacity
is the sum of the capacities of the arcs that leave it
([§12.1](12-max-flow.md#sec-12-1)).

**cut node**: a node whose removal increases the number of connected
components of a graph, also called an articulation node
([§18.2](18-properties.md#sec-18-2)).

**d-ary heap**: a heap stored as a complete tree in which each node has at
most d children, laid out level by level in a vector
([§7.2](07-heaps.md#sec-7-2)).

**DAG**: a directed acyclic graph, i.e., a digraph without directed cycles
([§18.3](18-properties.md#sec-18-3)).

**degeneracy**: the largest least degree of a subgraph of an undirected graph,
i.e., the largest core number of its nodes ([§6.6](06-maps.md#sec-6-6)).

**degenerate vertex**: a vertex of the polyhedron of the flows whose free arcs
do not span the nodes, and which therefore has more than one basis
([§14.4](14-k-best-flows.md#sec-14-4)).

**depth-first search**: the visit that advances from the last node reached
along an arc not yet scanned and backtracks when there is none, a node being
processed (finished) when the search backtracks from it
([§9.3](09-search.md#sec-9-3)).

**dormant set**: in the Hao-Orlin algorithm, a set of nodes put aside, without
relabelling them, because the current sink can no longer be reached from them
in the residual digraph; the sets are woken up in the reverse order of their
creation ([§15.2](15-min-cut.md#sec-15-2)).

**dual ray**: a solution of the dual LP with its right-hand sides (the
objective coefficients of the primal) set to 0 and a positive objective value,
which proves the primal problem infeasible ([§20.4](20-lp.md#sec-20-4)).
(Replaces the entry of `Y-glossary.md`, which sets the primal bounds to 0
instead.)

**dual scale**: the constant (4 for integer weights, 1 otherwise) by which the
weighted matching classes multiply their dual values, so that the halvings of
the algorithm stay exact ([§17.5](17-matching.md#sec-17-5)).

**edge connectivity**: the least number of edges whose removal disconnects an
undirected graph, i.e., its minimum cut with unit capacities
([§15.1](15-min-cut.md#sec-15-1)).

**elevator**: the data structure of the labels of the push-relabel algorithms,
which keeps the items by level with the active ones first and gives the
highest active item in constant time ([§8.3](08-auxiliary.md#sec-8-3)).

**eligible arc**: in the network simplex method, a nonbasic arc whose reduced
cost is negative at its lower bound or positive at its upper bound, which may
enter the basis ([§13.4](13-min-cost-flow.md#sec-13-4)).

**epsilon-optimality**: the property of a flow and of potentials under which
no residual arc has a reduced cost below the opposite of a given epsilon
([§13.6](13-min-cost-flow.md#sec-13-6)).

**Euler tour**: a closed walk that traverses each edge of a graph (each arc of
a digraph, in its direction) once ([§18.4](18-properties.md#sec-18-4)).

**extender**: a class template of lemon/bits/ (DigraphExtender, GraphExtender,
BpGraphExtender) that completes a base class with the iterators, the maps, the
notifiers and the notifying modifications
([§A.2](A-writing-graph-map.md#sec-a-2)).

**extreme flow**: a vertex of the polyhedron of the flows of a minimum cost
flow problem, i.e., a feasible flow whose arcs strictly within their bounds
form a forest ([§14.4](14-k-best-flows.md#sec-14-4)).

**factor-critical graph**: a graph that has a perfect matching after the
removal of any one of its nodes ([§17.1](17-matching.md#sec-17-1)).

**Fibonacci heap**: a heap made of heap-ordered trees whose roots are linked
only at the removal of the minimum, with cascading cuts at the decreases
([§7.3](07-heaps.md#sec-7-3)).

**filter**: a bool map that decides which nodes, arcs or edges a filtering
adaptor shows ([§5.2](05-adaptors.md#sec-5-2)).

**forward arc**: the arc of an edge that goes from u(e) to v(e), for which
direction() is true; the other arc of the edge is the backward one
([§3.2](03-mental-model.md#sec-3-2)).

**fractional matching**: a solution of the linear relaxation of the matching
problem without the odd set inequalities, whose vertices take the values 0,
1/2 and 1 ([§17.1](17-matching.md#sec-17-1)).

**function-type interface**: a function (e.g., dijkstra()) that returns a
wizard object, on which the optional outputs of an algorithm are given by name
before a single run ([§3.6](03-mental-model.md#sec-3-6)).

**fundamental cut**: for an edge $e$ of a spanning forest $T$, the set of the
edges with one end in each of the two node sets into which $T\setminus\{e\}$
splits the component of $e$ ([§11.1](11-spanning.md#sec-11-1)).

**Gallai–Edmonds decomposition**: the partition of the nodes into those left
uncovered by some maximum matching (D), their other neighbours (A) and the
rest (C), which MaxMatching returns as the status of each node
([§17.1](17-matching.md#sec-17-1)).

**gap heuristic**: the lift to the top level of all the items above a level
that has become empty, which can no longer reach the sink
([§8.3](08-auxiliary.md#sec-8-3)).

**GEQ and LEQ forms**: the minimum cost flow problem with supply constraints
"greater or equal" (all the supplies leave their nodes, some demand may be
unmet) or "less or equal" (all the demands are met, some supply may stay)
([§13.1](13-min-cost-flow.md#sec-13-1)).

**Gomory-Hu tree**: a tree on the nodes of an undirected graph, with a weight
on each edge, such that the minimum cut between two nodes is the lightest edge
of the path between them in the tree, and the components left by removing that
edge are the two sides of the cut ([§15.1](15-min-cut.md#sec-15-1)).

**graph adaptor**: a class that presents a digraph or a graph as another one
(filtered, reversed, without orientation, residual, split) by translating each
query, without copying it ([§5.1](05-adaptors.md#sec-5-1)).

**graph6**: the format of the nauty package that encodes an undirected graph
in one line of printable characters, six bits of the upper triangle of the
adjacency matrix per character ([§21.6](21-io.md#sec-21-6)).

**handle**: a small value object (Node, Arc, Edge, RedNode, BlueNode) that
identifies an item of a graph, meaningful only together with its graph
([§3.2](03-mental-model.md#sec-3-2)).

**heap order**: the invariant of a heap-ordered tree that no item has a
smaller priority than its parent ([§7.2](07-heaps.md#sec-7-2)).

**identifier**: the integer that a graph gives to an item, unique among the
items of its kind and immutable while the item exists, used as the index of
the maps ([§3.2](03-mental-model.md#sec-3-2)).

**implicit graph**: a graph structure that stores only its sizes and computes
its items from their indices, such as FullGraph, GridGraph and HypercubeGraph
([§4.5](04-graphs.md#sec-4-5)).

**inherent orientation**: the orientation of an edge from u(e) to v(e), given
by the order of the end nodes at its creation (from red to blue in a bipartite
graph) ([§3.2](03-mental-model.md#sec-3-2)).

**INVALID**: the only object of the type Invalid, to which each handle and
iterator converts, and which marks the end of an iteration or the absence of
an item ([§3.2](03-mental-model.md#sec-3-2)).

**iterable map**: a graph map that also lists, in time proportional to their
number, the items that have a given value ([§6.6](06-maps.md#sec-6-6)).

**Kempe chain**: a connected component of the subgraph induced by the nodes of
two given colors, whose colors can be swapped without spoiling a coloring
([§18.6](18-properties.md#sec-18-6)).

**Kuratowski subdivision**: a subdivision of $K_5$ or of $K_{3,3}$ contained
in a graph, which certifies that the graph is not planar
([§18.5](18-properties.md#sec-18-5)).

**label**: in an LGF file, the value of the map named `label`, which
identifies a node or an arc in the rest of the file
([§21.1](21-io.md#sec-21-1)).

**laminar family**: a family of sets any two of which are disjoint or nested,
e.g., the sets contracted by the algorithm of the minimum cost arborescence
([§11.2](11-spanning.md#sec-11-2)).

**Lawler's partition**: the splitting of a set of solutions, once its best one
is known, into parts in which the elements of that solution are fixed in turn,
used to find the k best solutions in order
([§14.1](14-k-best-flows.md#sec-14-1), [§14.3](14-k-best-flows.md#sec-14-3)).

**LGF**: the LEMON Graph Format, a column-oriented text format made of
sections (`@nodes`, `@arcs`, `@edges`, `@attributes`, `@red_nodes`,
`@blue_nodes` and free extra sections) ([§21.1](21-io.md#sec-21-1)).

**low point**: in a depth-first search, the least index of discovery of a node
reachable from a given node by going down the tree and then along at most one
non-tree edge ([§18.2](18-properties.md#sec-18-2)).

**map adaptor**: a map whose values are computed at each read from one or two
other maps, a constant or a functor, which it keeps by reference
([§6.4](06-maps.md#sec-6-4)).

**matching**: a set of edges no two of which share an end; it is perfect when
it covers every node ([§17.1](17-matching.md#sec-17-1)).

**maximum adjacency ordering**: an ordering of the nodes of an undirected
graph in which each node maximizes the capacity of its edges to the nodes
before it; the orderings of the Nagamochi-Ibaraki algorithm
([§15.3](15-min-cut.md#sec-15-3)).

**maximum cardinality search**: the order of the nodes in which each node,
when chosen, has the largest total capacity of arcs to the nodes chosen before
it ([§9.6](09-search.md#sec-9-6)).

**mean cost (of a cycle)**: the cost of a directed cycle divided by its number
of arcs ([§16.1](16-min-mean-cycle.md#sec-16-1)).

**metric cost**: a cost on the edges of a complete graph that satisfies the
triangle inequality $c(u,w)\le c(u,v)+c(v,w)$
([§19.1](19-tsp-clique.md#sec-19-1)).

**minimum cut (global)**: a nonempty proper subset of the nodes whose leaving
arcs (or boundary edges, in an undirected graph) have the least total
capacity, with no source and no sink fixed ([§15.1](15-min-cut.md#sec-15-1)).

**named template parameter**: a nested template of an algorithm class (e.g.,
SetDistMap< M >) whose member Create is the class with one type of its traits
replaced ([§3.6](03-mental-model.md#sec-3-6)).

**negative cycle**: a directed cycle of negative total length, whose presence
makes the distances of the nodes reachable from it $-\infty$
([§10.4](10-shortest-paths.md#sec-10-4)).

**observer**: an object of a class derived from
AlterationNotifier::ObserverBase, attached to a notifier, whose functions add,
erase, build and clear are called at each change of the graph
([§A.3](A-writing-graph-map.md#sec-a-3)).

**odd set inequalities**: the constraints that a set S of odd cardinality
holds at most (|S| − 1)/2 edges of a matching, which with the degree
constraints describe the matching polytope ([§17.1](17-matching.md#sec-17-1)).

**operation traits**: the class of the operations (`zero()`, `plus()`,
`less()` and, for Bellman-Ford, `infinity()`) by which `Dijkstra` and
`BellmanFord` compute and compare the distances
([§10.2](10-shortest-paths.md#sec-10-2)).

**pairing heap**: a heap made of one heap-ordered tree, whose subtrees are
joined by the two-pass pairing at the removal of the root
([§7.3](07-heaps.md#sec-7-3)).

**path compression**: the rule of a find in a union-find structure that makes
each item met on the way to the root point to the root
([§8.2](08-auxiliary.md#sec-8-2)).

**path dumper**: an object that lists the arcs of a path, possibly from the
last to the first (when its `RevPathTag` is `True`), returned by the `path()`
queries of the search algorithms and meant to be assigned to a path class
([§8.1](08-auxiliary.md#sec-8-1)).

**perfect elimination order**: an order of the nodes of a graph in which the
neighbours of each node that follow it form a clique; a graph has one if and
only if it is chordal ([§9.6](09-search.md#sec-9-6)).

**planar graph**: a graph that can be drawn in the plane with no two edges
crossing ([§18.5](18-properties.md#sec-18-5)).

**policy graph**: in Howard's algorithm, the subgraph made of one chosen
leaving arc for each node, in which each weakly connected part contains one
cycle ([§16.4](16-min-mean-cycle.md#sec-16-4)).

**potentials**: the dual variables of a minimum cost flow problem, one per
node; the reduced cost of an arc is its cost plus the potential of its tail
minus that of its head ([§13.1](13-min-cost-flow.md#sec-13-1)).

**preflow**: a map within the bounds of the arcs whose excess, the flow into a
node less the flow out of it, is nonnegative at every node other than the
source ([§12.2](12-max-flow.md#sec-12-2)).

**primal ray**: a solution of the primal LP with its finite bounds set to 0
and an improving objective value, which proves the dual problem infeasible
([§20.4](20-lp.md#sec-20-4)).

**primal scale**: the constant 2 by which the fractional matching classes
multiply their primal values, so that the half-valued edges are reported as
integers ([§17.6](17-matching.md#sec-17-6)).

**priority queue**: the abstract data type of a set of items with priorities,
with the item of least priority, its removal and the change of a priority as
operations; the heaps implement it ([§7.1](07-heaps.md#sec-7-1)).

**problem type**: the outcome of a solved LP or MIP (`UNDEFINED`,
`INFEASIBLE`, `FEASIBLE`, `OPTIMAL` or `UNBOUNDED`), read after `solve()` by
`primalType()`, `dualType()` or `type()` ([§20.4](20-lp.md#sec-20-4)).

**radix heap**: a heap of integer priorities, monotone in the minimum, whose
items are kept in boxes of widths 1, 1, 2, 4, ... from the current minimum
([§7.4](07-heaps.md#sec-7-4)).

**range constraint**: a row of a linear program with both a finite lower and a
finite upper bound, written in the interface as `(s <= e) <= t`
([§20.1](20-lp.md#sec-20-1)).

**range identifier**: the identifier in $[0,n)$ that a RangeIdMap gives to
each of the $n$ items of a kind, kept contiguous under erasures
([§6.5](06-maps.md#sec-6-5)).

**realizer**: a partition of the inner edges of a triangulation into three
trees rooted at the nodes of the outer face, from which Schnyder's method
computes the coordinates of a straight-line drawing
([§18.5](18-properties.md#sec-18-5)).

**reduced length**: the length $\ell(uv)+\pi(u)-\pi(v)$ of an arc under a
potential $\pi$, which changes the length of all the paths between two nodes
by the same amount ([§10.1](10-shortest-paths.md#sec-10-1)).

**reference map**: a read-write map whose operator[] returns a reference to
the value, recognized by its tag ReferenceMapTag ([§6.1](06-maps.md#sec-6-1)).

**relaxation**: the update of the tentative distance of the target of an arc
to the distance of its source plus the length of the arc, when this is smaller
([§10.2](10-shortest-paths.md#sec-10-2)).

**residual digraph**: for a flow on a digraph, the digraph of the forward arcs
below their capacity and of the backward arcs of positive flow, each with its
residual capacity ([§5.4](05-adaptors.md#sec-5-4)).

**residual network**: for a flow, the network of the arcs along which flow may
still be sent, forward up to the upper bound and backward down to the lower
bound, the backward arcs with the opposite cost
([§12.1](12-max-flow.md#sec-12-1), [§13.1](13-min-cost-flow.md#sec-13-1)).

**shortcut**: of a closed walk that visits every node of a complete graph, the
tour that lists the nodes in the order of their first occurrence in the walk
([§19.1](19-tsp-clique.md#sec-19-1)).

**smallest-last order**: the order in which the nodes are removed from a graph
by always taking a node of least degree among the remaining ones
([§18.6](18-properties.md#sec-18-6)).

**smallest-last ordering**: the order of the nodes obtained by removing, one
at a time, a node of least degree in the graph that remains, read backwards
([§6.6](06-maps.md#sec-6-6)).

**snapshot**: a saved state of a list or smart graph, to which restore()
brings the graph back by erasing the items added after it
([§4.2](04-graphs.md#sec-4-2)).

**spanning forest**: a set of edges with no cycle that connects the nodes of
each connected component of a graph; a spanning tree when the graph is
connected ([§11.1](11-spanning.md#sec-11-1)).

**spanning tree**: a set of $n-1$ edges of a connected graph with no cycle,
which therefore connects all the nodes ([§11.1](11-spanning.md#sec-11-1)).

**splay tree**: the self-adjusting binary search tree of Sleator and Tarjan,
used by DynArcLookUp, whose operations take logarithmic amortized time
([§4.8](04-graphs.md#sec-4-8)).

**static graph**: a graph structure built in one call and not modifiable
afterwards, which stores the arcs out of each node contiguously
(StaticDigraph, StaticGraph, CompactDigraph) ([§4.4](04-graphs.md#sec-4-4)).

**strongly connected component**: a class of the nodes of a digraph joined by
directed paths in both directions ([§18.3](18-properties.md#sec-18-3)).

**strongly feasible tree**: in the network simplex method, a spanning tree of
a basis along which a positive amount of flow can be sent from each node to
the root within the bounds; the rule of the last blocking arc keeps it so and
prevents cycling ([§13.4](13-min-cost-flow.md#sec-13-4)).

**subgraph isomorphism**: an injective map of the nodes of a graph into those
of another that takes each edge to an edge; it is induced if it also takes
each non-edge to a non-edge ([§18.7](18-properties.md#sec-18-7)).

**successive shortest paths**: the algorithm of minimum cost flow that sends
flow along shortest paths of the residual digraph, keeping the reduced lengths
nonnegative by potentials, used by `Suurballe`
([§10.5](10-shortest-paths.md#sec-10-5)).

**tolerance**: the object of class Tolerance< T > that compares numbers
allowing for an absolute error up to its epsilon, exactly for the exact types
([§3.7](03-mental-model.md#sec-3-7)).

**topological order**: an order of the nodes of an acyclic digraph in which
the source of each arc precedes its target, e.g., the reverse of the finishing
order of a depth-first search ([§9.3](09-search.md#sec-9-3)).

**tour**: a Hamiltonian cycle of a complete graph, i.e., a cycle that visits
each node once; the solution of the travelling salesman problem
([§19.1](19-tsp-clique.md#sec-19-1)).

**traits class**: the class, a template parameter of an algorithm, that
defines the types the algorithm uses (heap, maps, operations) and creates
their objects ([§3.6](03-mental-model.md#sec-3-6)).

**unbounded cycle**: a directed cycle of negative cost whose arcs have
infinite upper bounds, which proves a feasible minimum cost flow problem
unbounded ([§14.2](14-k-best-flows.md#sec-14-2)).

**union-find structure**: a partition of a set of items into classes under the
union of two classes and the find of the representative of the class of an
item, stored as a forest with union by size and path compression
([§8.2](08-auxiliary.md#sec-8-2)).

**vertex cover**: a set of nodes containing at least one end of every edge; in
a bipartite graph its least cardinality equals that of a maximum matching
([§17.1](17-matching.md#sec-17-1)).

**violation**: of a set of nodes, its supply less the upper bounds of the arcs
leaving it plus the lower bounds of the arcs entering it; a positive violation
proves a minimum cost flow problem infeasible
([§14.1](14-k-best-flows.md#sec-14-1)).

**visitor**: an object whose members a search with a visitor interface
(`BfsVisit`, `DfsVisit`) calls at each event of the search
([§9.5](09-search.md#sec-9-5)).

**wizard**: the object returned by a function-type interface (e.g., `bfs()`,
`dijkstra()`), on which the maps and the outputs are given by named function
parameters before `run()` ([§9.4](09-search.md#sec-9-4)).
