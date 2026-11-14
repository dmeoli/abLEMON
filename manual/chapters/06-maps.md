# 6. Maps {#ch-6}

We are concerned in this chapter with the maps of the library, i.e., with the
objects that associate a value with a key. Since the graphs store no data of
their own, maps are the only way in which data are attached to the nodes, arcs
and edges of a graph (cf. [chapter 3](03-mental-model.md#ch-3)). A map is any
type that offers the types `Key` and `Value` together with an `operator[]` (to
read) or a `set()` (to write). Hence, besides the standard graph maps, which
store a value for each item, one can pass to an algorithm a map that computes
its values on demand, from other maps, from a function or from the graph.
Section 6.1 gives the concepts and section 6.2 the standard graph maps, as a
user sees them. Sections 6.3 to 6.6 describe the classes of `lemon/maps.h`
(the basic maps, the map adaptors, the maps tied to a graph and the iterable
maps, with an algorithm that the iterable maps make linear), section 6.7 the
functions that scan a map, and section 6.8 collects the exercises. For a map
on the items of one kind of a graph, $n_K$ denotes the number of the items of
that kind (nodes, arcs or edges), and $N_K$ the largest identifier of that
kind plus one.

## 6.1 The map concepts {#sec-6-1}

The header `lemon/concepts/maps.h` defines four concepts, as archetype classes
with a nested `Constraints` template that `checkConcept()` instantiates on a
candidate type (cf. [chapter 3](03-mental-model.md#ch-3)).
`concepts::ReadMap< K, T >` requires the types `Key` and `Value` and the
member `Value operator[]( const Key & ) const`. `concepts::WriteMap< K, T >`
requires the same types and `void set( const Key &, const Value & )`, and
`concepts::ReadWriteMap< K, T >` is both. Finally,
`concepts::ReferenceMap< K, T, R, CR >` is a read-write map whose `operator[]`
returns a `Reference` (`R`), and a `ConstReference` (`CR`) on a constant map,
through which values can be read and assigned. It declares the tag
`typedef True ReferenceMapTag`, by which the library recognizes it, and its
constraints are enabled only for a type that declares the tag. Note that an
algorithm reads a map only through `operator[]` and `set()`, and the result of
`operator[]` need not be a reference (in fact, the adaptors of section 6.4
return their values by copy).

Two helpers make writing a map short: `MapBase< K, V >` is an empty base class
that only defines `Key` and `Value`, and the classes of `lemon/maps.h` derive
from it. `MapTraits< M >`, in `lemon/bits/traits.h`, gives the
`ReferenceMapTag` of any map type and the types `ReturnValue` and
`ConstReturnValue` that the map returns. These are references for a reference
map and values otherwise, and the adaptors use them to forward the reference
of the map they wrap.

!!! note "abLEMON"
    The archetypes of `lemon/concepts/maps.h` return a static buffer of the
    size and alignment of the value, instead of dereferencing the address
    `sizeof(Value)` as LEMON 1.3.1 did (which is undefined behavior, and which
    the optimizer flagged as an access out of bounds), while the concepts are
    unchanged.

## 6.2 Graph maps {#sec-6-2}

Each graph structure of [chapter 4](04-graphs.md#ch-4) has the nested
templates `NodeMap< V >`, `ArcMap< V >` and, if it is undirected,
`EdgeMap< V >` (a bipartite graph also has `RedNodeMap< V >` and
`BlueNodeMap< V >`). They are reference maps, built by
`NodeMap( const Graph & g )`, which value-initializes the values, or by
`NodeMap( const Graph & g, const V & value )`. The graph keeps them up to
date, since each map registers itself as an observer of the alteration
notifier of the graph (cf. [appendix A](A-writing-graph-map.md#app-a)), and it
is told of each item added or erased. A key added after the construction of
the map receives the value `V()`, not the value given to the constructor, and
the value of an erased key is lost. The assignment operators of the standard
maps are private; one copies the values of a map into another by `mapCopy()`
(section 6.7).

The storage is chosen by `DefaultMap`, in `lemon/bits/default_map.h`: for
`bool`, the character types, the integer types, the floating-point types and
the pointers, it is a `VectorMap`, i.e., a `std::vector< V >` indexed by the
identifier of the item. For any other value type it is an `ArrayMap`, an array
allocated through `std::allocator`, whose capacity is the first power of two
above the largest identifier when the map is built, and which doubles until it
exceeds the identifier of an item added beyond it.

<span id="lem-6-1"></span>**Lemma 6.1.** *A read or a write of a standard
graph map costs $O(1)$; the notification of an added item costs $O(1)$
amortized over a sequence of additions, and the memory of the map is $O(N_K)$
values, whatever the number $n_K$ of the items present.*

*Proof.* Both storages index an array by the identifier of the item, and a
`VectorMap` grows by `resize()` of its vector to the new identifier plus one,
of amortized constant cost per element added. An `ArrayMap` that grows from
capacity $c$ to $2^jc$ moves its $O(n_K)\le O(c)$ values once, and since the
capacities are powers of two the total cost of the moves up to capacity $c$ is
$O(c)$, i.e., $O(1)$ per identifier added. The sizes are bounded by $N_K$
(rounded up to a power of two for the `ArrayMap`), and they do not shrink when
items are erased. $\square$

Each map also has three iterators, inherited from `MapExtender` (in
`lemon/bits/map_extender.h`). `MapIt` iterates on the items and dereferences
to a reference to their value, `ConstMapIt` dereferences to a constant
reference, and `ItemIt` only iterates on the items. The test
`test/maps_test.cc` uses `MapIt` and `ConstMapIt` on the maps of a
`SmartDigraph`.

!!! note "abLEMON"
    `ArrayMap` constructs and destroys its values through
    `std::allocator_traits`, since C++20 removed the members `construct()` and
    `destroy()` of `std::allocator`. When it grows, it moves the values into
    the larger buffer by `std::move_if_noexcept()`, rather than copying them.

## 6.3 Basic maps {#sec-6-3}

The basic maps of `lemon/maps.h` need no graph; `NullMap< K, V >` reads `V()`
at each key and absorbs whatever is written to it. It is the map to pass (in
place of a real one) when an algorithm requires a map whose content is not
wanted (e.g., a predecessor map that the caller does not read).
`ConstMap< K, V >` reads one value at each key, given to the constructor
(`ConstMap( const V & v )`) or default-constructed, and it also absorbs the
writes. Its `setAll(v)` changes the value, and a further constructor
`ConstMap( const ConstMap< K, V1 > &, const V & v )` builds the map of value
`v` from one of another value type. Its specialization
`ConstMap< K, Const< V, v > >` carries the value in its type, as the template
argument of the empty tag `Const< T, v >`, so that the compiler may fold the
read. The filtering adaptors of [chapter 5](05-adaptors.md#ch-5), e.g., use
`ConstMap< Node, Const< bool, true > >` in place of the filter they lack.
`TrueMap< K >` and `FalseMap< K >` are the read-only constant maps of `true`
and `false`, and `IdentityMap< T >` reads the key (its key and value types are
both `T`).

`RangeMap< V >` is a `std::vector< V >` with integer keys in
$[0,\mathrm{size})$. It is built from a size and a value
(`RangeMap( int size = 0, const V & value = V() )`), from a `std::vector`, or
from another `RangeMap` whose value type converts to `V`; `size()` returns the
size, and `resize(size, value)` changes the key set, giving `value` to the new
keys. It is a reference map, whose types `Reference` and `ConstReference` are
those of the vector, and, unlike the graph maps, it can be copied and
assigned; it is typically used as the cross-reference map of a heap whose
items are small integers (cf. [chapter 7](07-heaps.md#ch-7)).

`SparseMap< K, V, Comp >` is a `std::map< K, V, Comp >`, with
`Comp = std::less< K >` by default, together with a default value for the keys
that are not stored. It is built from the default value (by default `V()`),
from a `std::map` and a default value, or from another `SparseMap` with the
same key type. One remark is in order here: the non-constant `operator[]`,
which returns a reference, inserts an absent key with the default value, while
the constant one does not. `setAll(v)` changes the default value and erases
the stored keys, and the class can be copied but not assigned (its copy
assignment is private).

<span id="lem-6-2"></span>**Lemma 6.2.** *The reads and the writes of
`NullMap`, `ConstMap`, `TrueMap`, `FalseMap`, `IdentityMap` and `RangeMap`
cost $O(1)$; those of `SparseMap` cost $O(\log s)$ comparisons by `Comp`,
where $s$ is the number of stored keys, and `setAll()` of `SparseMap` costs
$O(s)$.*

*Proof.* The first five maps return a stored value, a constant or the key, and
`RangeMap` indexes a vector. `SparseMap` finds the key by `lower_bound()` or
`find()` of the balanced search tree of `std::map`, and inserts it, if absent,
with the position found as a hint; `setAll()` calls `clear()` of the tree.
$\square$

Each of these classes has a creator function with the name of the class in
lower case: `nullMap< K, V >()`, `constMap< K >( v )` (and
`constMap< K, V, v >()` for the inlined value), `identityMap< T >()`,
`rangeMap( size, value )` and `rangeMap( vector )`, `sparseMap< K >( value )`
(and `sparseMap( map, value )` from a `std::map`), `trueMap< K >()` and
`falseMap< K >()`.

`LoggerBoolMap< IT, KEY >` is a write-only `bool` map, which copies each key
set to `true` to the output iterator `IT` (named by the type `Iterator`), in
the order of the writes, and it ignores the keys set to `false`. Its `begin()`
and `end()` return the first iterator and the one after the last key written.
The map is meant for the algorithms that report a set by writing `true` once
per item. For instance,
`dfs( g ).processedMap( loggerBoolMap( std::back_inserter( v ) ) ).run( s )`
stores the nodes in the order in which the search processes them. The key type
`KEY` is deduced from the iterator (from its `value_type`, or from that of its
container for an insert iterator, by the traits class `IteratorTraits` of the
namespace `_maps_bits`), and the iterator must either be an inserter or point
to enough room. Being only writable, a `LoggerBoolMap` cannot serve where a
readable map is required, e.g., as the reached map of `Bfs`, `Dfs` or
`Dijkstra`.

## 6.4 Map adaptors and arithmetic {#sec-6-4}

A map adaptor is a map whose values are computed, at each read, from one or
two other maps. It stores references to the maps it combines (or copies of the
constants and functors it uses) and nothing else.

<span id="lem-6-3"></span>**Lemma 6.3.** *Building a map adaptor costs $O(1)$,
and a read (or, for the writable ones, a write) of it costs one read (or
write) of each map it adapts, plus one evaluation of its operator or functor.*

*Proof.* Each class of Table 6.1 stores references (or a copy of a functor or
of a constant), and its `operator[]` and `set()` are one expression in the
values of the adapted maps at the key, as the table gives them. $\square$

Two remarks apply to all the adaptors: first, they keep the maps by reference,
and the maps must therefore outlive them. For instance, a temporary map passed
to a creator function (e.g., `addMap( m, constMap< Node >( 1 ) )`) is
destroyed at the end of the full expression, and an adaptor that refers to it
cannot be stored. Second, most adaptors are read-only, since a write to a
computed value has, in general, no meaning; the exceptions are said
explicitly. Table 6.1 lists them, with `m`, `m1` and `m2` the adapted maps,
`x` a key, `v` a constant and `f` a functor.

<span id="tab-6-1"></span>**Table 6.1.** The map adaptors of `lemon/maps.h`:
the value at a key and, when the adaptor is writable, the effect of
`set(x, y)`.

| Class | Value at `x` | Writes | Key and value types |
|----------------------------|----------------------------|-----------------|--------------------------|
| `ComposeMap< M1, M2 >` | `m1[m2[x]]` | no | key of `M2`, value of `M1` |
| `CombineMap< M1, M2, F, V >` | `f(m1[x], m2[x])` | no | key of `M1`, value `V` |
| `FunctorToMap< F, K, V >` | `f(x)` | no | `K`, `V` |
| `MapToFunctor< M >` | `m[x]`, also as `operator()` | no | those of `M` |
| `ConvertMap< M, V >` | `m[x]` converted to `V` | no | key of `M`, `V` |
| `ForkMap< M1, M2 >` | `m1[x]` | to `m1` and `m2` | those of `M1` |
| `AddMap`, `SubMap`, `MulMap`, `DivMap< M1, M2 >` | `m1[x]` $\circ$ `m2[x]` | no | those of `M1` |
| `ShiftMap< M, C >` | `m[x] + v` | no | those of `M` |
| `ShiftWriteMap< M, C >` | `m[x] + v` | `m.set(x, y - v)` | those of `M` |
| `ScaleMap< M, C >` | `v * m[x]` | no | those of `M` |
| `ScaleWriteMap< M, C >` | `v * m[x]` | `m.set(x, y / v)` | those of `M` |
| `NegMap< M >` | `-m[x]` | no | those of `M` |
| `NegWriteMap< M >` | `-m[x]` | `m.set(x, -y)` | those of `M` |
| `AbsMap< M >` | `m[x]` if `m[x] >= 0`, else `-m[x]` | no | those of `M` |
| `AndMap`, `OrMap< M1, M2 >` | `m1[x] && m2[x]`, `m1[x] \|\| m2[x]` | no | key of `M1`, `bool` |
| `NotMap< M >` | `!m[x]` | no | key of `M`, `bool` |
| `NotWriteMap< M >` | `!m[x]` | `m.set(x, !y)` | key of `M`, `bool` |
| `EqualMap`, `LessMap< M1, M2 >` | `m1[x] == m2[x]`, `m1[x] < m2[x]` | no | key of `M1`, `bool` |

In the four arithmetic adaptors $\circ$ is `+`, `-`, `*` and `/`, and the key
and value types of `M2` must convert to those of `M1`. The constant type `C`
of the shift and scale adaptors defaults to the value type of `M`.
`ScaleWriteMap` can be written only when the constant is not zero and `/` is
defined between the value and `C`. `ComposeMap` returns its value with the
`ConstReturnValue` of `M1` (by reference, when `M1` is a reference map), while
the others return by value. Each class has a creator function with the name of
the class in lower case, e.g., `composeMap( m1, m2 )`, `addMap( m1, m2 )`,
`shiftMap( m, v )` and `notWriteMap( m )`. The creators of the writable
adaptors take the adapted maps by non-constant reference, and
`convertMap< V >( m )` takes the value type as its explicit template argument.

`FunctorToMap` takes its key and value types from the members `argument_type`
and `result_type` of the functor when they are not given. Since lambdas (and
most modern functors) lack these members, `K` and `V` must then be given
explicitly, as in `functorToMap< int, double >( f )`; for a plain function
pointer `V (*)(K)`, an overload of `functorToMap()` deduces them. Conversely,
`MapToFunctor` defines `argument_type` and `result_type`, for the code that
still uses them. Likewise, `combineMap( m1, m2, f )` deduces the value type
from the functor, and an overload takes a function pointer `V (*)(K1, K2)`.

One passes the adaptors to the algorithms like any other map, but when the
algorithm is a class, its map type must be named, which the creator functions
do not do; in C++11 and later, `decltype` names it. The program
`manual/examples/06-map-adaptors.cc` (the test `manual_06_map_adaptors`) runs
`Dijkstra` on the travel time of each arc as the quotient of a length map and
a speed map, through a `DivMap` that is never stored, and on that time plus a
delay through a `CombineMap` of a lambda. It checks the two distances and the
results of five utilities of section 6.7.

!!! note "abLEMON"
    The value type of `CombineMap` defaults to the type that the functor
    returns on the values of the two maps, deduced by `decltype` (the helper
    `CombineValue` of the namespace `_maps_bits`). LEMON 1.3.1 read the member
    `result_type` of the functor instead, which C++20 removed from the
    function objects of `<functional>` and which lambdas never had; thus,
    `combineMap( m1, m2, f )` now accepts a lambda.

## 6.5 Maps of the items of a graph {#sec-6-5}

The maps of this section are tied to a graph, whose type `GR` is their first
template parameter, and most of them take the item type `K` (`GR::Node`,
`GR::Arc` or `GR::Edge`) as the second. They are of two kinds: views compute
their value from the graph (and possibly from a map) at each read, while
look-up maps store a value per item together with an inverse structure that
finds the items by their value. All of them name the graph by the type
`Graph`, and the classes with an item type also name it by `Item`.

*Views.* `IdMap< GR, K >` reads the identifier `id()` of an item, which is
unique among the items of its kind, and it does not change while the item
exists (even if other items are erased). Its `operator()(id)`, and its nested
`InverseMap` (returned by `inverse()`, or built from the graph or from the
`IdMap`), give the item with a given identifier, through `fromId()` of the
graph. `SourceMap< GR >` and `TargetMap< GR >` read the source and the target
of an arc, while `ForwardMap< GR >` and `BackwardMap< GR >` read the arcs
`direct(e, true)` and `direct(e, false)` of an edge of an undirected graph.
`PotentialDifferenceMap< GR, POT >` reads, at an arc $(i,j)$, the difference
$\pi_j-\pi_i$ of a node map of potentials $\pi$; for a zero cost, it is the
reduced cost of the arc up to the sign convention (cf. [chapter
13](13-min-cost-flow.md#ch-13)). Each view reads in $O(1)$ calls of the graph
and of the potential map, and each has a creator function, i.e.,
`idMap< K >( g )`, `sourceMap( g )`, `targetMap( g )`, `forwardMap( g )`,
`backwardMap( g )` and `potentialDifferenceMap( g, pot )`.

*Degree maps.* `InDegMap< GR >` and `OutDegMap< GR >` read the in-degree and
the out-degree of a node of a digraph. They keep the degrees in a private node
map, `AutoNodeMap`, which gives the degree zero to a new node, and they
observe the arcs of the digraph.

<span id="lem-6-4"></span>**Lemma 6.4.** *An `InDegMap` (or `OutDegMap`) is
built in $O(n+m)$ time, it reads a degree in $O(1)$, and it costs $O(1)$ per
arc added or erased through the notifier of the digraph; its values are the
in-degrees (out-degrees) as long as the digraph is changed only by additions
and erasures of nodes and arcs, and by `clear()` and `build()`.*

*Proof.* The constructor attaches the map to the notifier of the arcs and
counts the incoming arcs of each node by `countInArcs()`, i.e., it visits each
arc once. Thereafter, the functions `add()` and `erase()` of the observer
increase or decrease the degree of the target of each arc added or erased, and
a node added gets degree zero from `AutoNodeMap`; the erasure of a node is
preceded by the erasure of its arcs, which the notifier reports. `build()`
recounts and `clear()` sets all degrees to zero, while a change of an arc that
is not reported as an erasure and an addition breaks the invariant. $\square$

As the header warns, the functions that change an arc without adding or
erasing it (e.g., `changeSource()`, `changeTarget()` and `reverseArc()` of
`ListDigraph`) leave the degrees wrong.

*Look-up maps.* `RangeIdMap< GR, K >` gives each item of its kind an
identifier in $[0,n_K)$, called its range identifier, and it keeps the inverse
in a `std::vector`. Hence, `operator()(id)` (and `InverseMap::operator[]`, of
the constant map returned by `inverse()`) return the item with a given range
identifier, and `size()` (also a member of the `InverseMap`) returns $n_K$ as
an `unsigned int`. The identifiers follow the order of the items at
construction, and a new item takes the next one. When an item is erased, the
item with the largest identifier takes its identifier; the range thus stays
contiguous, but the identifiers may change (and they are usually not those of
`IdMap`). Furthermore, `swap(p, q)` exchanges the identifiers of two items, by
which one can impose any order; the creator function is
`rangeIdMap< K >( g )`.

<span id="lem-6-5"></span>**Lemma 6.5.** *At any time the range identifiers of
the items of a `RangeIdMap` are a bijection onto $[0,n_K)$, whose inverse is
the vector of the map. A read in either direction, an addition, an erasure and
`swap()` cost $O(1)$, and the construction $O(n_K)$.*

*Proof.* The invariant holds after the construction, which numbers the items
in the order of the notifier and pushes them on the vector. An addition gives
the new item the identifier $n_K$ and pushes it, so that the range grows by
one. An erasure of the item of identifier $r$ moves the last item of the
vector, of identifier $n_K-1$, to position $r$, gives it the identifier $r$
and pops the vector, so that the range shrinks by one and both directions stay
consistent. Finally, `swap()` exchanges two identifiers and the two entries of
the vector. Each of these steps is a constant number of map and vector
operations. $\square$

`CrossRefMap< GR, K, V >` stores a value for each item, and it keeps a
`std::multimap` from the values to the items. Its `operator()(val)`, or the
`InverseMap`, returns one item with the value (or `INVALID`), `count(val)`
returns the number of items with a value, and `beginValue()` and `endValue()`
delimit a forward iterator, `ValueIt` (also named `ValueIterator`), on the
values, with multiplicity and in increasing order. We remark that the inverse
structure holds only the values written by `set()`: on a newly built map,
which reads `V()` at each item, `count( V() )` is zero. The map is meant for
values that are all different, or nearly so; when many items share a value,
`IterableValueMap` (section 6.6) is usually the better choice. Neither
`CrossRefMap` nor `IterableValueMap` is a reference map, i.e., their values
are written only by `set()`.

<span id="lem-6-6"></span>**Lemma 6.6.** *Let $c$ be the number of items whose
value equals the old value of the key. In a `CrossRefMap`, `set(key, val)`
costs $O(\log n_K + c)$ time, `operator()(val)` costs $O(\log n_K)$,
`count(val)` costs $O(\log n_K + c')$ with $c'$ the result, and the erasure of
an item from the graph costs $O(c\log n_K)$, with $c$ the number of items of
its value.*

*Proof.* The multimap holds at most $n_K$ pairs, and `set()` computes the
range of the old value by one `equal_range()`, scans its at most $c$ pairs for
the key, erases the pair found, and inserts the new one, each in
$O(\log n_K)$; the other searches are one `find()` and one `count()`. The
function `erase()` called by the notifier scans the range of the value of the
item as `set()` does, but it recomputes the end of the range by
`equal_range()` at each step of the scan. $\square$

!!! note "abLEMON"
    `CrossRefMap::set()` computes the range of the old value of the key once,
    where LEMON 1.3.1 recomputed it at each step of the search of the key
    among the items of that value, for $O(c\log n_K)$; the iterators `ValueIt`
    of `CrossRefMap` and `IterableValueMap` define their member types, instead
    of deriving from `std::iterator`, which C++17 deprecated.

## 6.6 Iterable maps {#sec-6-6}

An iterable map stores a value for each item of a graph and, besides the reads
and writes of a map, it lists the items that have a given value, in time
proportional to their number. `IterableBoolMap` and `IterableIntMap` are
reference maps, whose `operator[]` returns a proxy object (the nested class
`Reference`) that converts to the value and writes through `set()`;
`IterableValueMap` is a read-write map, and the three classes are kept up to
date when items are added to or erased from the graph.

`IterableBoolMap< GR, K >` keeps the items in one vector, together with the
position of each item, and the items mapped to `true` are in the positions
before a separator, and those mapped to `false` after it (Figure 6.1).
`set(key, value)` moves the item across the separator by swapping it with the
item next to the separator on the other side, `setAll(value)` moves only the
separator, and `trueNum()` and `falseNum()` count the two sides. The iterators
`TrueIt`, `FalseIt` and `ItemIt(map, value)` list the items of one side, from
the separator outwards, and so do the ranges `trueKeys()`, `falseKeys()` and
`items(value)` of the range-based `for` loops. The constructor
`IterableBoolMap( const GR & g, bool def = false )` maps all the items to
`def`, and an item added later is mapped to `false`; the proxy also offers
`&=`, `|=` and `^=`.

![Figure 6.1. IterableBoolMap: the items in one vector, the true ones before the separator; set(e, true) swaps e with the first false item and moves the separator.](../figures/06-iterable-bool.svg){#fig-6-1}

<span id="lem-6-7"></span>**Lemma 6.7.** *In an `IterableBoolMap` with vector
$a_0,\dots,a_{n_K-1}$ and separator $\sigma$, an item is mapped to `true` if
and only if its position is below $\sigma$, and the position stored for each
item is its index in the vector. `set()`, `setAll()`, `trueNum()`,
`falseNum()`, the reads, the additions and the erasures of items cost $O(1)$,
and an iterator lists the items of one value in $O(1)$ per item.*

*Proof.* The constructor pushes the items in the order of the notifier and
puts $\sigma$ at $n_K$ or at $0$. To set an item at position $p\ge\sigma$ to
`true`, `set()` exchanges it with $a_\sigma$, updates both positions and
increases $\sigma$; to set an item at $p<\sigma$ to `false`, it decreases
$\sigma$ and exchanges the item with $a_\sigma$. In both cases only the two
items exchanged change position, and only the moved item changes side. A new
item is pushed at the end, i.e., on the `false` side; an erased item at
$p<\sigma$ is replaced by $a_{\sigma-1}$, which is replaced by the last item,
and $\sigma$ is decreased, while one at $p\ge\sigma$ is replaced by the last
item. `trueNum()` is $\sigma$, `falseNum()` is $n_K-\sigma$, and an iterator
steps from a position to the previous one until it crosses $\sigma$ (or $0$).
$\square$

`IterableIntMap< GR, K >` stores an integer per item, and it keeps a doubly
linked list of the items for each non-negative value, with the heads in a
vector indexed by the value. Thus, `set(key, value)` unlinks the item from the
list of its old value and links it at the head of the list of the new one.
`size()` returns the largest value with a non-empty list plus one, and
`ItemIt(map, value)` and `items(value)` list the items with a value, while an
item with a negative value is in no list and cannot be listed. The constructor
without a value sets all the values to $-1$, the one with a value sets them to
it, and an item added later reads $-1$. The proxy returned by `operator[]`
offers the integer operators (`++`, `--`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`,
`|=`, `^=`, `<<=`, `>>=`), each of which goes through `set()`.

`IterableValueMap< GR, K, V >` does the same for any value type with an
`operator<`, and it keeps a linked list per value, with the heads in a
`std::map< V, K >` that holds only the values in use. Its `ItemIt(map, value)`
and `items(value)` list the items with a value, and `beginValue()` and
`endValue()` delimit a forward iterator, `ValueIt`, on the distinct values, in
increasing order. The constructor maps all the items to a value (by default
`V()`), and an item added later reads `V()`. The records of the lists are the
structures `IterableIntMapNode` and `IterableValueMapNode` of the namespace
`_maps_bits`, which are not meant to be used directly.

<span id="lem-6-8"></span>**Lemma 6.8.** *Let $w$ be the largest value of an
`IterableIntMap`, and $d$ the number of distinct values of an
`IterableValueMap`. A `set()` of an `IterableIntMap` costs $O(1)$ plus the
change of length of the vector of heads, which is a growth to the new value
plus one, or a shrink by the number of empty lists at its end; the memory is
$O(N_K+w)$. A `set()` and the construction of an `ItemIt` of an
`IterableValueMap` cost $O(\log d)$ comparisons. In both maps an `ItemIt`
lists the items of a value in $O(1)$ per item.*

*Proof.* `set()` of an `IterableIntMap` unlinks the item (one update of its
two neighbours or of the head), pops the empty heads at the end of the vector,
sets the value and links the item at the head of its new list, after resizing
the vector if the value exceeds it. The steps other than the pops and the
resize are $O(1)$. In an `IterableValueMap` the head of a value is found by
`find()` on `std::map`, and a value whose list becomes empty is erased from
it, in $O(\log d)$; an `ItemIt` follows the links `next` of the records.
$\square$

!!! note "abLEMON"
    The default constructor of `IterableIntMapNode`, by which a graph map
    creates the record of a new item, and the constructor of
    `IterableValueMapNode` initialize the links to `INVALID`, which LEMON
    1.3.1 left indeterminate.

The iterable integer map is the natural data structure for the buckets of
degrees of the smallest-last ordering of [Matula and Beck
1983](Z-bibliography.md#ref-matula1983smallest). Let $G=(V,E)$ be a simple
undirected graph (no loops, no parallel edges), and, for $H$ a subgraph, let
$\delta(H)$ be its least degree. The $k$-core of $G$ is the union of the
subgraphs $H$ with $\delta(H)\ge k$ (itself such a subgraph, since a node of
the union has at least $k$ neighbours in the subgraph that contains it). The
core number $c(v)$ of a node is the largest $k$ such that $v$ is in the
$k$-core, and the degeneracy of $G$ is
$\delta^*(G)=\max_{H\subseteq G}\delta(H)$. Algorithm 6.1 removes, one at a
time, a node of least degree in the graph that remains, and records the
running maximum of the degrees at removal.

<span id="alg-6-1"></span>**Algorithm 6.1** (Smallest-last ordering and core
numbers by an iterable integer map).
```
input:  simple graph G = (V, E)
deg := IterableIntMap on V, deg[v] := degree of v      (deg.set(v, .))
k := 0;  d := 0
repeat |V| times
  while ItemIt(deg, d) == INVALID: d := d + 1
  u := ItemIt(deg, d)                  (a node of least current degree)
  k := max(k, d);  core[u] := k;  append u to the order
  deg.set(u, -1)                       (u leaves all the lists)
  for each edge (u, w) with deg[w] > 0: deg.set(w, deg[w] - 1)
  d := max(d - 1, 0)
output: the order (reversed, the smallest-last order), core, k
```

<span id="thm-6-1"></span>**Theorem 6.1.** *Algorithm 6.1 removes at each
iteration a node of least degree in the graph that remains, and on output
$\mathrm{core}[v]=c(v)$ for each node and $k=\delta^*(G)$; also, each node has
at most $k$ neighbours that are removed after it.*

*Proof.* Let $u_1,\dots,u_n$ be the nodes in the order of removal, $G_t$ the
subgraph induced by $u_t,\dots,u_n$, and $d_t$ the value of $d$ when $u_t$ is
chosen. We first show that $d\le\delta(G_t)$ at the start of iteration $t$,
which holds for $t=1$, since $d=0$. If it holds at $t$, the buckets below
$\delta(G_t)$ are empty and that of $\delta(G_t)$ is not, so that the loop
stops at $d_t=\delta(G_t)$ and $u_t$ has least degree in $G_t$. The neighbours
of $u_t$ that remain (exactly those with `deg` positive, since a remaining
neighbour has degree at least 1 and a removed node has $-1$) lose one unit of
degree each, as $G$ is simple, so that $\delta(G_{t+1})\ge d_t-1$, and the new
$d$ is at most $\delta(G_{t+1})$. Hence,
$\mathrm{core}[u_t]=k_t=\max_{s\le t}\delta(G_s)$. Let $r\le t$ attain the
maximum; $G_r$ contains $u_t$ and has least degree $k_t$, so that
$c(u_t)\ge k_t$. Conversely, let $H$ be the $c(u_t)$-core and $s$ the first
index with $u_s\in H$; then $s\le t$, $H\subseteq G_s$, and
$\delta(G_s)=\deg_{G_s}(u_s)\ge\deg_H(u_s)\ge c(u_t)$, so that
$k_t\ge c(u_t)$. Since every subgraph $H$ has $\delta(H)\le c(v)$ for its
nodes $v$, $\delta^*(G)=\max_v c(v)=\max_t k_t=k$; finally, the neighbours of
$u_t$ removed after it are its neighbours in $G_t$, i.e., $\delta(G_t)\le k$
of them. $\square$

<span id="thm-6-2"></span>**Theorem 6.2.** *Algorithm 6.1 runs in $O(|V|+|E|)$
time.*

*Proof.* The initial degrees cost $O(|V|+|E|)$, and there are $|V|$ initial
`set()`, $|V|$ removals and at most $|E|$ decrements (an edge decrements one
end, when its other end is removed). The vector of heads grows only during the
initialization, to the largest degree $\Delta+1$, and the shrinks after it
remove at most $\Delta+1$ heads in all; by Lemma 6.8, all the `set()` cost
$O(|V|+|E|+\Delta)$. The value $d$ decreases by at most one per iteration and
never exceeds $\Delta$, so that the tests of the inner loop are at most
$|V|+\Delta$ plus one per iteration; since $\Delta\le|E|$, the total is
$O(|V|+|E|)$. $\square$

On a graph with parallel edges, a removal may lower a degree by more than one,
and the last step of Algorithm 6.1 would have to lower $d$ accordingly. The
program `manual/examples/06-degeneracy.cc` (the test `manual_06_degeneracy`)
implements Algorithm 6.1 on a graph of seven nodes, checks the degeneracy, the
core numbers and the first node removed, and marks the nodes of the largest
core in an `IterableBoolMap`, whose `trueNum()` gives their number and whose
`TrueIt` lists them.

## 6.7 Map utilities {#sec-6-7}

The functions at the end of `lemon/maps.h` scan the items of one kind of a
graph, the kind given by the key type of the map, and use the values of a map.

-   `mapCopy(g, from, to)` writes `from[x]` into `to` for each item `x` of
    the kind of `to`. An edge map can thus be copied to an arc map of the same
    graph, since an arc converts to its edge, but not vice versa.
    `mapFill(g, map, val)` writes `val` at each item.
-   `mapCompare(g, m1, m2)` returns `true` if the two maps agree, by
    `operator==`, at each item of the kind of `m2`.
-   `mapMin(g, map)` and `mapMax(g, map)` return an item of least and of
    largest value, or `INVALID` on an empty item set. When there are ties
    (which may well happen on integer data), they return the first one met in
    the order of the item iterator. The variants with a third argument take a
    comparison functor in place of `std::less< V >`. `mapMinValue()` and
    `mapMaxValue()` (with or without the functor) return the value, and they
    require a non-empty item set, since they read the map at the item that
    `mapMin()` or `mapMax()` returns.
-   `mapFind(g, map, val)` and `mapFindIf(g, map, pred)` return the first
    item whose value equals `val` (or satisfies `pred`), and `INVALID` if
    there is none, while `mapCount(g, map, val)` and
    `mapCountIf(g, map, pred)` return the number of such items.

<span id="lem-6-9"></span>**Lemma 6.9.** *Each utility makes one pass over the
items of its kind, with $O(n_K)$ reads or writes of the maps and $O(n_K)$
evaluations of `==`, of the comparison or of the predicate (the searches stop
at the first item found).*

*Proof.* Each function is one loop of the item iterator of the kind, with one
read (or write) of each map and one evaluation per item; `mapMin()` keeps the
current minimum and replaces it only on a strict comparison, which gives the
first item of least value. $\square$

The program `manual/examples/06-map-adaptors.cc` uses `mapMin()`,
`mapCountIf()`, `mapCopy()`, `mapCompare()` and `mapMaxValue()`, and the test
`test/maps_test.cc` checks all the utilities.

## 6.8 Exercises {#sec-6-8}

<span id="ex-6-1"></span>**Exercise 6.1.** Write a map class, derived from
`MapBase`, whose value at an arc is the Euclidean length of the arc, computed
from a node map of coordinates. Check that it conforms to `concepts::ReadMap`
by `checkConcept()`, and then obtain the same map without writing a class,
from `SourceMap`, `TargetMap`, `ComposeMap` and `CombineMap`.

<span id="ex-6-2"></span>**Exercise 6.2.** In
`manual/examples/06-map-adaptors.cc`, replace the `DivMap` by a `CombineMap`
with a lambda, and the delay by a `ShiftMap` of the `DivMap`; check that the
distances do not change.

<span id="ex-6-3"></span>**Exercise 6.3.** Explain why the declaration
`AddMap< M, ConstMap< K, int > > m = addMap( length, constMap< K >( 1 ) );`
compiles, while a later read of `m` has undefined behavior, and correct it.

<span id="ex-6-4"></span>**Exercise 6.4.** Modify
`manual/examples/06-degeneracy.cc` so that it colors the graph greedily in the
smallest-last order, with at most $k+1$ colors (by the last statement of
Theorem 6.1), and check the coloring. Then extend Algorithm 6.1 to graphs with
parallel edges, and prove that it still runs in $O(|V|+|E|)$ time.

<span id="ex-6-5"></span>**Exercise 6.5.** Use a `RangeIdMap` and its `swap()`
to number the nodes of an acyclic digraph in a topological order, and check
that each arc goes from a smaller to a larger range identifier (the test
`test/maps_test.cc` checks `swap()` after an erasure).
