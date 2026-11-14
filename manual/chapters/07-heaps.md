# 7. Heaps {#ch-7}

We are concerned in this chapter with the heaps of the library, i.e., with the
implementations of the abstract data type priority queue, which the algorithms
of Part III use to choose the next node to scan. A heap stores a set of items,
each with a priority. It gives the item of least priority, removes it, and
changes the priority of an item in the heap. The cost of these operations
differs (sometimes rather markedly) from one structure to another; hence, the
best choice depends on the mix of operations of the algorithm and on the type
of the priorities. Section 7.1 gives the interface common to all the heaps
(the concept `concepts::Heap`). Sections 7.2 to 7.4 describe the three
families of implementations, i.e., the implicit $d$-ary heaps, the linked
heaps and the heaps of integer priorities, each with its algorithms, its
invariants and its costs. Section 7.5 compares them and shows how an algorithm
is given a heap, and section 7.6 collects the exercises. Throughout the
chapter, $n$ is the number of items in the heap, "$p<q$" between priorities
means `comp(p, q)` for the comparison functor of the heap, and an amortized
cost is a bound on the total cost of any sequence of operations from the empty
heap, divided among its operations.

## 7.1 The heap concept {#sec-7-1}

The concept `concepts::Heap< PR, IM, CMP >` of `lemon/concepts/heap.h`
describes the interface. `PR` is the type of the priorities (named `Prio`),
`IM` a reference map from the items to `int`, called the cross-reference map
(named `ItemIntMap`), and `CMP` a functor that compares two priorities, by
default `std::less< PR >`. The type of the items, `Item`, is `IM::Key`; thus,
the items are whatever the map takes as keys (nodes, arcs, or integers with a
`RangeMap`). The heap keeps a reference to the map, which must therefore
outlive it, and it uses the map to find an item in its storage. The states of
an item are the values of the enum `State`: `PRE_HEAP` ($-1$), if the item has
never been in the heap, `IN_HEAP` ($0$), if it is in the heap, and `POST_HEAP`
($-2$), if it has been in the heap and has left it. The map must hold
`PRE_HEAP` for each item before the first insertion, which is the precondition
of the constructors `explicit Heap( IM & map )` and
`Heap( IM & map, const CMP & comp )`. The concept also requires the heap to be
copy-constructible.

The operations are the following; Table 7.1 gives their costs.

-   `size()` returns the number of items, and `empty()` returns whether it is
    zero.
-   `push(i, p)` inserts the item `i`, which must not be in the heap, with
    priority `p`.
-   `top()` and `prio()` return the item of least priority and its priority
    (the heap must not be empty). With `std::greater` as `CMP`, they return
    the item of largest priority.
-   `pop()` removes the item of least priority (the heap must not be empty),
    whose state becomes `POST_HEAP`, and `erase(i)` removes the item `i`,
    which must be in the heap.
-   `operator[](i)` returns the priority of the item `i`, which must be in
    the heap.
-   `decrease(i, p)` and `increase(i, p)` change the priority of an item in
    the heap to `p`, which must not be larger (respectively, smaller) than the
    current one under `CMP`. The heaps do not check it.
-   `set(i, p)` changes the priority of `i` if it is in the heap, by
    `decrease()` or `increase()` as the case may be, and it inserts `i`
    otherwise (also when it is `POST_HEAP`).
-   `state(i)` returns the state of an item. `state(i, st)`, with `st` equal
    to `PRE_HEAP` or `POST_HEAP`, erases the item if it is in the heap and
    then writes `st` into the map (`IN_HEAP` is ignored).
-   `clear()` empties the heap without touching the map. To reuse the heap on
    the same items, the caller must also reset the map to `PRE_HEAP`.

What the map holds for an item depends on the family. In the implicit heaps of
section 7.2 and in the heaps of section 7.4 it holds the position of the item
in the storage while the item is in the heap, and `POST_HEAP` after the item
has left it. In the linked heaps of section 7.3 it holds the index of the
record of the item from its first insertion until `clear()` or `state(i, st)`,
and `state(i)` reads, in the record, whether the item is in the heap. In both
cases `state(i)` costs $O(1)$, and it lets an algorithm such as `Dijkstra`
distinguish a scanned node from an unreached one without a further map.
Furthermore, the heaps write the map through `operator[]` (e.g.,
`state(i, st)` in all of them, `decrease()` and `erase()` of `BinomialHeap`,
and `push()`, `pop()` and `erase()` of `RadixHeap` and `BucketHeap`), and
therefore need a reference map, as their documentation says; the maps used in
practice (a graph map of `int` or a `RangeMap< int >`) are reference maps.

Besides the concept, `BinHeap`, `QuadHeap`, `DHeap`, `BucketHeap` and
`SimpleBucketHeap` have `push(pair)`, which takes a `std::pair< Item, Prio >`
(the type `Pair`, which `FibHeap` also names), and the comparison heaps name
their functor by `Compare`. Also, `BinHeap`, `QuadHeap` and `DHeap` have
`replace(i, j)`, which gives the place and the priority of the item `i` (in
the heap) to the item `j` (not in the heap), in $O(1)$; after it, the map
holds for `i` the state that it held for `j`. The test `test/heap_test.cc`
checks each heap against the concept, by `checkConcept()`, and then by
sorting, by increasing priorities, by priorities with many ties, by erasures
under `std::less` and `std::greater`, and inside `Dijkstra`.

## 7.2 Implicit d-ary heaps {#sec-7-2}

A $d$-ary heap ([Johnson 1975](Z-bibliography.md#ref-johnson75heap)) is a
complete $d$-ary tree, stored level by level in a vector, whose items satisfy
the heap order: the priority of an item is not smaller than that of its
parent. With positions from $0$, the parent of position $i>0$ is
$\lfloor (i-1)/d \rfloor$, and its children are $di+1,\dots,di+d$ (those below
$n$). The binary heap of [Williams
1964](Z-bibliography.md#ref-williams1964heapsort) is the case $d=2$ (Figure
7.1). The cross-reference map holds the position of each item, and each move
of an item updates it.

![Figure 7.1. A binary heap as a tree and as the vector that stores it; a decrease of the item in position 4 moves it along the path to the root, here through the positions 1 and 0.](../figures/07-bin-heap.svg){#fig-7-1}

All the operations reduce to the two procedures of Algorithm 7.1, which move
an item along a path of the tree until the heap order holds again.

<span id="alg-7-1"></span>**Algorithm 7.1** (Sifting in a $d$-ary heap).
```
bubbleUp(hole, (i, p)):           (the item i with priority p goes up)
  while hole > 0 and p < prio(parent(hole)):
    move the item of parent(hole) to hole;  hole := parent(hole)
  put (i, p) at hole
bubbleDown(hole, (i, p), n):      (the item i with priority p goes down)
  while hole has children among the positions 0 .. n-1:
    c := the child of least priority
    if not prio(c) < p: break
    move the item of c to hole;  hole := c
  put (i, p) at hole
push(i, p):    append a position n; bubbleUp(n, (i, p))
pop():         mark top POST_HEAP; bubbleDown(0, last item, n - 1)
decrease(i,p): bubbleUp(position of i, (i, p))
increase(i,p): bubbleDown(position of i, (i, p), n)
set(i, p):     push if i is not in the heap, else decrease if p < prio(i),
               else increase
erase(i):      mark i POST_HEAP; put the last item in the hole of i by
               bubbleUp and, if it did not move, by bubbleDown
```
Each `move` and each `put` writes the new position into the cross-reference
map. `BinHeap` implements `erase()` as written; `QuadHeap` and `DHeap` compare
the last item with the parent of the hole first, and call `bubbleDown()` if
the parent is smaller and `bubbleUp()` otherwise, which has the same effect.

<span id="lem-7-1"></span>**Lemma 7.1.** *A $d$-ary heap of $n\ge1$ items has
$L=\lceil\log_d(n(d-1)+1)\rceil$ levels, i.e., a path from the root to a leaf
has at most $L$ positions.*

*Proof.* The levels $0,\dots,L-2$ are full, and level $L-1$ holds at least one
item; since level $h$ holds $d^h$ positions,
$\sum_{h<L-1}d^h<n\le\sum_{h<L} d^h$, i.e., $d^{L-1}<n(d-1)+1\le d^L$, which
is the statement. $\square$

<span id="thm-7-1"></span>**Theorem 7.1.** *Each operation of Algorithm 7.1
keeps the heap order and keeps in the cross-reference map the position of each
item in the heap; hence, `top()` returns an item of least priority. `push()`
and `decrease()` cost $O(\log_d n)$ comparisons, `pop()`, `increase()` and
`erase()` cost $O(d\log_d n)$, and `top()`, `prio()`, `operator[]`, `state()`
and `replace()` cost $O(1)$.*

*Proof.* Call a tree with a hole at position $h$ ordered if the heap order
holds between all the pairs of positions other than $h$ and, moreover, between
the parent of $h$ and the children of $h$. `bubbleUp()` is called on an
ordered hole whose children are not smaller than $p$ (for `push()`, the hole
has no child; for `decrease()`, the children were not smaller than the old
priority, which is not smaller than $p$); each step moves the parent into the
hole, which keeps the tree ordered with the hole one level up and keeps the
children of the new hole not smaller than $p$; it stops when $p$ is not
smaller than the parent, and then putting $p$ restores the heap order.
Symmetrically, `bubbleDown()` is called on an ordered hole whose parent is not
larger than $p$, moves the least child up while it is smaller than $p$, and
stops when no child is smaller. For `erase()`, the last item is put into the
hole of $i$: `bubbleUp()` succeeds when it is smaller than the parent of the
hole, and otherwise the parent is not larger than it and `bubbleDown()`
applies. Every move writes the new position into the map. By the heap order,
the root has least priority. As for the costs, `bubbleUp()` makes one
comparison per level, and `bubbleDown()` makes $d$ per level (at most $d-1$ to
find the least child and one with $p$); by Lemma 7.1 there are $O(\log_d n)$
levels. `top()` and `prio()` read position $0$, `operator[]` and `state()`
read the map and the vector, and `replace()` writes two values of the map and
one item of the vector. $\square$

The degree $d$ thus trades the cost of `push()` and `decrease()` for that of
`pop()`. In an algorithm with many decreases per removal, as `Dijkstra` on a
dense digraph, the choice of $d$ balances the two, as follows.

<span id="thm-7-2"></span>**Theorem 7.2.** *(Johnson.) `Dijkstra` on a digraph
of $n$ nodes and $m$ arcs with a $d$-ary heap makes
$O(nd\log_d n + m\log_d n)$ comparisons in the heap; with
$d=\max(2,\lceil m/n\rceil)$, this is $O(m\log_{2+m/n}n)$.*

*Proof.* `Dijkstra` pushes and pops each node at most once and decreases a
priority at most once per arc, which by Theorem 7.1 gives the first bound.
With $d=\max(2,\lceil m/n\rceil)$ one has $nd=O(n+m)$ and
$\log_d n=O(\log n/\log(2+m/n))$, and $n\le m$ may be assumed for the nodes
that are reached from the source. $\square$

Of course, a larger degree also means fewer levels and fewer cache misses,
which in practice often (but not always) favours $d=4$ over $d=2$, even when
the counts of comparisons do not.

The library has three classes of this family, all with the template parameters
`PR`, `IM` and `CMP` (by default `std::less< PR >`) and with both constructors
of the concept.

-   `BinHeap< PR, IM, CMP >`, in `lemon/bin_heap.h`, is the binary heap. It
    is the default heap of `Dijkstra`, `BidirDijkstra`, `AStar`, `Suurballe`
    and `MinCostArborescence`, and the heap (with `std::greater`) of
    `NagamochiIbaraki` and of `MaxCardinalitySearch` on general capacities;
    the matching algorithms also use it.
-   `QuadHeap< PR, IM, CMP >`, in `lemon/quad_heap.h`, is the 4-ary heap (the
    header calls it "fourary"), whose four children are compared by unrolled
    code.
-   `DHeap< PR, IM, D, CMP >`, in `lemon/dheap.h`, is the $d$-ary heap, with
    the degree as the template parameter `D`, by default $16$. The header
    suggests a power of two, which lets the compiler turn the divisions and
    multiplications of the positions into shifts.

!!! note "abLEMON"
    `CapacityScaling` (cf. [chapter 13](13-min-cost-flow.md#ch-13)) keeps the
    nodes of its shortest path searches in a `QuadHeap`, where LEMON 1.3.1
    used a `BinHeap`.

## 7.3 Linked heaps {#sec-7-3}

The heaps of this section store the items in heap-ordered trees, linked by
indices. Two trees are joined in $O(1)$ by making the root of larger priority
a child of the other (a link). Each of the three classes keeps one record per
item in a vector, indexed by the cross-reference map. The record stays after
the item leaves the heap (a flag records whether it is in), and it is reused
when the item comes back; hence, the memory is proportional to the number $N$
of distinct items pushed since the last `clear()`, and $n\le N$. The three
classes have the template parameters `PR`, `IM` and `CMP` and both
constructors of the concept, and in all three `increase()` is `erase()`
followed by `push()`.

*Fibonacci heaps.* `FibHeap< PR, IM, CMP >`, in `lemon/fib_heap.h`, is the
heap of [Fredman and Tarjan 1987](Z-bibliography.md#ref-fredman1987fibonacci).
The roots of its trees form a circular list, with a pointer to a root of least
priority, and the children of a node form a circular list as well; a node may
be marked. `push()` adds a root, and `decrease()` cuts the item from its
parent when the heap order breaks; the parent, if it was marked, is cut in
turn (the cascading cut), and so on upwards, while an unmarked parent that is
not a root is marked. `pop()` removes the minimum, moves its children to the
list of the roots, and then consolidates the roots, linking two roots of equal
degree until all the degrees differ. Furthermore, `erase(i)` cuts the item
(with the cascading cut), makes it the minimum (in the words of a comment of
the code, "as if its prio would be -infinity"), and pops it.

<span id="alg-7-2"></span>**Algorithm 7.2** (Decrease and pop in a Fibonacci
heap).
```
decrease(i, p):
  prio(i) := p;  q := parent(i)
  if q exists and p < prio(q):
    cut(i, q)                       (i becomes a root, unmarked)
    while q is not a root:          (cascading cut, cascade())
      if q is unmarked: mark q; stop
      r := parent(q);  cut(q, r);  q := r
  if p < prio(min): min := i
pop():
  remove min from the roots and add its children to them
  A[0 .. maxdeg - 1] := none,  maxdeg = floor(2.08 ln N) + 1   (balance())
  for each root r:                   (consolidation)
    while A[degree(r)] is a root s:
      link the one of larger priority under the other, unmarked,
        call the result r            (on a tie, s stays the root)
      A[degree(r) - 1] := none
    A[degree(r)] := r
  min := the root of least priority
erase(i):
  if i has a parent q: cut(i, q) and the cascading cut from q
  min := i;  pop()
```

<span id="lem-7-2"></span>**Lemma 7.2.** *Let $\phi=(1+\sqrt5)/2$. In a
Fibonacci heap, a node of degree $k$ has at least $F_{k+2}\ge\phi^k$ nodes in
its subtree, where $F_j$ is the $j$-th Fibonacci number; hence, every degree
is at most $\log_\phi n$, and the array of `balance()`, of size
$\lfloor 2.08\ln N\rfloor+1$, is large enough.*

*Proof.* Let $y_1,\dots,y_k$ be the children of a node $x$ in the order in
which they were linked to it. When $y_h$ was linked, $x$ had at least the
children $y_1,\dots,y_{h-1}$, and links join roots of equal degree; hence,
$y_h$ had degree at least $h-1$, and since then it has lost at most one child,
because a node that is not a root is marked at its first loss and cut at its
second (marks are cleared by links and cuts). Thus $y_h$ has degree at least
$h-2$. Let $s_k$ be the least size of a subtree whose root has degree $k$;
then $s_0=1$, $s_1=2$, and $s_k\ge 2+\sum_{h=2}^k s_{h-2}$, which by induction
gives $s_k\ge F_{k+2}$, since $F_{k+2}=1+\sum_{j=0}^k F_j$; the bound
$F_{k+2}\ge\phi^k$ is the classical one. Hence $k\le\log_\phi n=\ln n/\ln\phi$
with $1/\ln\phi\approx2.078$, and the degrees met during the consolidation, at
most $\log_\phi n\le 2.078\ln N$, are indices of the array. $\square$

<span id="thm-7-3"></span>**Theorem 7.3.** *In a `FibHeap`, `top()`, `prio()`,
`operator[]` and `state()` cost $O(1)$, `push()` and `decrease()` cost $O(1)$
amortized time, and `pop()`, `erase()` and `increase()` cost $O(\log N)$
amortized time; every operation keeps the heap order, and `pop()` and
`erase()` remove exactly the item they should, whatever the type of the
priorities and the comparison.*

*Proof.* (Sketch.) With the potential $\Phi=t+2\mu$, where $t$ is the number
of roots and $\mu$ that of the marked nodes, `push()` raises $\Phi$ by one; a
`decrease()` that makes $c$ cuts costs $O(c)$, adds $c$ roots and unmarks at
least $c-1$ nodes, so that its amortized cost is $O(1)$; a `pop()` costs
$O(D+t)$, where $D=O(\log N)$ bounds the degrees and the array (Lemma 7.2),
and leaves at most $D+1$ roots, so that its amortized cost is $O(D)$. An
`erase()` is a `decrease()` followed by a `pop()`, and `increase()` adds a
`push()`. The details are in [Fredman and Tarjan 1987,
§2](Z-bibliography.md#ref-fredman1987fibonacci) and in [Cormen et al. 2001,
§20.2 to §20.4](Z-bibliography.md#ref-clrs01algorithms). As for the
correctness, a cut makes a subtree a root and links put the larger root under
the smaller, so that every tree stays heap-ordered; `pop()` ends by scanning
all the roots for the least priority. `erase()` never compares the priority of
the item: it makes the item a root, and the `pop()` that follows removes that
root and finds the new minimum by the scan. $\square$

Accordingly, the header warns that a heap with many increases and erasures is
better a binary one.

*Pairing heaps.* `PairingHeap< PR, IM, CMP >`, in `lemon/pairing_heap.h`, is
the self-adjusting heap of [Fredman, Sedgewick, Sleator and Tarjan
1986](Z-bibliography.md#ref-fredman1986pairing), i.e., a single heap-ordered
tree in which each node keeps its children in a list (stored in the records as
a binary tree of first children and right siblings). `push()` links the new
item with the root, and `decrease()` cuts the subtree of the item, when the
heap order with its parent breaks, and links it with the root. `pop()` removes
the root and joins its subtrees in two passes: it first links them in pairs
from left to right, and then it links the results from right to left into one
tree (the two-pass pairing). `erase(i)` cuts the subtree of the item and makes
the item the root over the rest, whatever the priorities, and then pops it.

<span id="alg-7-3"></span>**Algorithm 7.3** (Pairing heap).
```
link(a, b):   make b the first child of a                  (fuse(a, b))
push(i, p):   if the heap is empty: root := i
              else if p < prio(root): link(i, root); root := i
              else link(root, i)
decrease(i, p):
  prio(i) := p
  if i is not the root and p < prio(parent(i)):
    cut the subtree of i                                    (cut())
    if prio(root) < p: link(root, i) else link(i, root); root := i
pop():
  T[0 .. k-1] := the subtrees of the children of the root, left to right
  for h = 0, 2, 4, ... while h + 1 < k:          (first pass)
    link the larger of T[h], T[h+1] under the other, into T[h]
  h := the last even index;  for h, h - 2, ..., 2:   (second pass)
    link the larger of T[h], T[h-2] under the other, into T[h-2]
  root := T[0], or none if k = 0
erase(i):
  if i is not the root: cut the subtree of i; link(i, root); root := i
  pop()
```

<span id="thm-7-4"></span>**Theorem 7.4.** *Every operation of a `PairingHeap`
keeps a single heap-ordered tree whose root is an item of least priority, and
`erase(i)` removes exactly $i$, whatever the type of the priorities and the
comparison. `push()`, `decrease()`, `top()` and `prio()` cost $O(1)$ actual
time, and `pop()` costs $O(1+k)$, with $k$ the number of children of the root.
In amortized terms, every operation costs $O(\log n)$ ([Fredman et
al. 1986](Z-bibliography.md#ref-fredman1986pairing)), and `push()` and
`decrease()` cost $O(2^{2\sqrt{\log\log n}})$ while `pop()` costs $O(\log n)$
([Pettie 2005](Z-bibliography.md#ref-pettie2005towards)).*

*Proof.* A link of two heap-ordered trees, with the root of larger priority
under the other, is heap-ordered; a cut subtree is heap-ordered, and so is the
rest. Hence, `push()`, `decrease()` and `pop()` keep the order, and the root
of an ordered tree has least priority. In `erase(i)`, the tree made by linking
the rest under $i$ may violate the order only at its root $i$; `pop()` removes
that root and joins its subtrees, which are heap-ordered, by links that
compare only their roots, so that the priority of $i$ is never compared and
the result is an ordered tree of the other items. The actual costs follow from
the algorithm: one link, one cut, or one pass over the $k$ subtrees. The
amortized bounds are the analyses cited, which are proved there for the
two-pass pairing heap that cuts at each decrease (the class cuts only when the
heap order breaks), and whose proofs take several pages. $\square$

The amortized cost of `decrease()` is thus $o(\log n)$, and it cannot be
$O(1)$ for this kind of structure ([Fredman
1999](Z-bibliography.md#ref-fredman1999efficiency)). Yet, the pairing heap is
often reported to be as fast as a binary heap (or faster) on the decreases of
the shortest path algorithms.

*Binomial heaps.* `BinomialHeap< PR, IM, CMP >`, in `lemon/binomial_heap.h`,
is the heap of [Vuillemin 1978](Z-bibliography.md#ref-vuillemin1978data). A
binomial tree of degree $0$ is a single node, and one of degree $k$ is the
link of two binomial trees of degree $k-1$; the heap is a list of heap-ordered
binomial trees of distinct degrees, in increasing order of degree, like the
binary digits of $n$. Its item of least priority is kept, so that `top()` and
`prio()` cost $O(1)$.

<span id="alg-7-4"></span>**Algorithm 7.4** (Binomial heap).
```
merge(list L):                                              (merge())
  interleave L with the roots, in increasing order of degree
  scan the roots: when two consecutive roots x, y have the same degree
    and the root after y has a different one, link the larger under
    the smaller (as in a binary addition)
push(i, p):    merge the tree of the single node i;  min := findMin()
pop():         remove min from the roots;
               merge the list of its children, reversed;  min := findMin()
decrease(i, p):
  prio(i) := p
  while i has a parent q and p < prio(q):
    exchange the items (and priorities) of the records of i and q,
    updating the cross-reference map;  i := q
  if p < prio(min): min := i
erase(i):
  move each item of the path from the root of the tree of i to i one
  record down, and put i in the root, updating the map;  min := that root
  pop()
```
Here `findMin()` scans the roots for one of least priority, comparing the
priorities by `CMP`.

<span id="lem-7-3"></span>**Lemma 7.3.** *A binomial tree of degree $k$ has
$2^k$ nodes and height $k$, and its root has degree $k$; hence, a binomial
heap of $n$ items has at most $\lfloor\log_2 n\rfloor+1$ trees, and every path
from a root to a leaf has at most $\lfloor\log_2 n\rfloor+1$ nodes.*

*Proof.* By induction on $k$, from the definition: the link of two trees of
$2^{k-1}$ nodes, height $k-1$ and root degree $k-1$ has $2^k$ nodes, height
$k$ and root degree $k$. The trees of a heap have distinct degrees, so that
$n=\sum_k b_k2^k$ with $b_k\in\{0,1\}$ is the binary representation of $n$,
which has at most $\lfloor\log_2 n\rfloor+1$ digits, and no tree has degree
above $\log_2 n$. $\square$

<span id="thm-7-5"></span>**Theorem 7.5.** *Every operation of a
`BinomialHeap` keeps a list of heap-ordered binomial trees of distinct degrees
and keeps `min` on a root of least priority. `push()`, `pop()`, `decrease()`,
`erase()` and `increase()` cost $O(\log n)$, and `top()`, `prio()`,
`operator[]` and `state()` cost $O(1)$. `erase(i)` removes exactly $i$,
whatever the type of the priorities and the comparison.*

*Proof.* A merge of two lists of binomial trees of distinct degrees links only
trees of equal degree, which gives a binomial tree of the next degree, and it
ends with distinct degrees, as the carry of a binary addition; each link puts
the larger root under the smaller, so the trees stay heap-ordered. `pop()`
merges the children of the minimum, which are binomial trees of degrees
$0,\dots,k-1$. `decrease()` moves the decreased item up by exchanges with its
parent while it is smaller, which restores the heap order as in `bubbleUp()`
(Theorem 7.1). In `erase()`, the items on the path from the root $r$ to $i$
move one record down; each of them is then above records that held its own
descendants, whose priorities are not smaller, so that the order can fail only
at $r$, which holds $i$; `pop()` removes $r$ without comparing its priority,
and `findMin()` recomputes the minimum. The costs follow from Lemma 7.3: a
merge and `findMin()` scan $O(\log n)$ roots, and `decrease()` and `erase()`
climb $O(\log n)$ records. $\square$

!!! note "abLEMON"
    `BinomialHeap` compares the priorities of the roots as priorities, where
    LEMON 1.3.1 compared them as `int` and truncated the fractional ones,
    taking for the minimum a root that was not; and `push()` looks for the
    minimum among the roots after the merge, since the merge may make the
    former minimum the child of a root of equal priority, after which `pop()`
    of LEMON 1.3.1 looped for ever (trac #646). The test `test/heap_test.cc`
    gives every comparison heap floating-point priorities with many ties, and
    the case of the ticket.

!!! note "abLEMON"
    `PairingHeap` and `BinomialHeap` erase an item by making it the root (as
    in Algorithms 7.3 and 7.4) and popping it. LEMON 1.3.1 decreased its
    priority to `prio() - 1` and popped it, which needs a numeric priority
    type and removes the minimum in place of the item whenever `prio() - 1`
    does not compare smaller than `prio()` under `CMP`, i.e., with
    `std::greater`, or with priorities so large that $x - 1 = x$ (e.g., above
    $2^{53}$ in `double`); the test `heap_test` checks both cases for all the
    heaps.

## 7.4 Heaps of integer priorities {#sec-7-4}

When the priorities are integers and the sequence of the minima removed does
not decrease (as in `Dijkstra` with non-negative integer lengths), an array
indexed by the priority may do better than a comparison heap. The library has,
indeed, three such heaps. Their priority type is `int`, they have no
comparison functor, and their only constructor takes the map (plus the initial
minimum and capacity, for `RadixHeap`).

*Radix heaps.* `RadixHeap< IM >`, in `lemon/radix_heap.h`, is the one-level
radix heap of [Ahuja, Mehlhorn, Orlin and Tarjan
1990](Z-bibliography.md#ref-ahuja1990faster). The items are kept in boxes
$0,\dots,K-1$, each a doubly linked list, and box $k$ covers the priorities in
$[b_k,b_{k+1})$, where $b_0\le b_1\le\dots\le b_K$ (Figure 7.2). Box $k$ has a
nominal width $w_k$, i.e., $1,1,2,4,8,\dots$, and its range is never wider
than $w_k$. Each box is a private record `RadixBox` (the start $b_k$ of its
range, its width and the head of its list), and each item a record with its
links, its box and its priority.

![Figure 7.2. The boxes of a radix heap from the minimum m, of widths 1, 1, 2, 4, 8; the first non-empty box (here box 2) is the one that is redistributed when box 0 is empty.](../figures/07-radix-boxes.svg){#fig-7-2}

<span id="alg-7-5"></span>**Algorithm 7.5** (Radix heap).
```
push(i, p):
  while p >= b_K: add a box K of width 2 w_{K-1} after the last  (extend())
  put i in the box of p, searching downwards from box K-1;
  if p < b_0, throw PriorityUnderflowError                  (findDown())
decrease(i, p):  prio(i) := p; if p < b_(box of i): move i to the box of
                 p, searching downwards, or throw as in push()
increase(i, p):  prio(i) := p; if p >= b_(box of i + 1): move i to the
                 box of p, searching upwards and extending   (findUp())
moveDown():                          (called by top(), prio() and pop())
  b := the first non-empty box                              (findFirst())
  if b > 0:
    m' := the least priority in box b;  e := b_(b+1)
    for k = 0 .. b: b_k := min(m' + w_0 + ... + w_(k-1), e)
    move each item of box b down to the box of its priority
pop():           moveDown(); remove the first item of box 0
erase(i):        remove i from its box
```
The record of an item removed is replaced by the last record of the vector,
whose position is then updated in the cross-reference map.

<span id="lem-7-4"></span>**Lemma 7.4.** *The operations of a `RadixHeap` keep
$b_0\le b_1\le\dots\le b_K$, keep each range $[b_k,b_{k+1})$ no wider than
$w_k$, and keep each item in the box whose range contains its priority. After
`moveDown()`, box $0$ is not empty and all its items have the least priority
of the heap; every item of the box $b$ redistributed moves to a box of smaller
index.*

*Proof.* The constructor creates the boxes $[m,m+1)$, $[m+1,m+2)$ and then
boxes of doubling width, each starting where the previous one ends, and
`extend()` does the same; `push()`, `decrease()` and `increase()` place an
item by searching for the box whose range contains its priority. In
`moveDown()`, the least priority $m'$ of the first non-empty box $b$ lies in
$[b_b,e)$, whose width is at most $w_b=2^{b-1}=w_0+\dots+w_{b-1}$ for $b\ge1$.
Hence, the new starts $b_k=\min(m'+w_0+\dots+w_{k-1},e)$ for $k\le b$ are
non-decreasing, they begin at $b_0=m'$, the range of each box $k<b$ is no
wider than $w_k$, the boxes $0,\dots,b-1$ cover $[m',e)$, and box $b$ gets the
empty range $[e,e)$; the boxes above $b$ are unchanged. Every item of box $b$
has its priority in $[m',e)$, so that it moves to a box of index below $b$,
and the item of priority $m'$ moves to box $0$, whose range is $[m',m'+1)$.
Since the boxes below $b$ were empty and those above hold larger priorities,
$m'$ is the least priority of the heap. $\square$

Hence, the priorities must not go below $b_0$. This is `minimum` at first and,
once `top()`, `prio()` or `pop()` has found box $0$ empty, the least priority
in the heap at that moment. A `push()`, `set()` or `decrease()` below it
throws the exception `RadixHeap< IM >::PriorityUnderflowError`, even if no
item has been removed yet. The constructor
`RadixHeap( IM & map, int minimum = 0, int capacity = 0 )` creates the boxes
for the priorities from `minimum` to `minimum + capacity - 1`, the heap adds
boxes when a larger priority arrives, and `clear(minimum, capacity)` empties
it and creates the boxes again.

<span id="thm-7-6"></span>**Theorem 7.6.** *Let $K$ be the number of boxes,
i.e., $O(\log U)$ with $U$ the largest priority pushed minus the initial
minimum (or the capacity, if larger). In a `RadixHeap`, `push()` and
`increase()` cost $O(K)$, `erase()` costs $O(1)$, and, in amortized terms,
`decrease()` costs $O(1)$ and `top()`, `prio()` and `pop()` cost $O(K)$.*

*Proof.* Let $\Phi$ be the sum, over the items in the heap, of the index of
their box. A search downwards (or upwards) over $j$ boxes costs $O(1+j)$ and
lowers (raises) $\Phi$ by $j$. `push()` costs $O(K)$ for the extensions and
the search, and raises $\Phi$ by at most $K$; `increase()` likewise.
`decrease()` costs $O(1+j)$ and lowers $\Phi$ by $j$, i.e., $O(1)$ amortized.
`moveDown()` costs $O(b)$ to find box $b$, plus $O(s)$ to scan its $s$ items
for $m'$ and to move them, and each of these moves lowers $\Phi$ by at least
one (Lemma 7.4); hence, its amortized cost is $O(K)$. `erase()` and the
removal of `pop()` unlink one item and move one record. $\square$

In `Dijkstra` with integer lengths in $[0,C]$, the priorities in the heap lie
in $[d,d+C]$, where $d$ is the last distance removed, so that the priorities
never go below $b_0$; for this case, [Ahuja et
al. 1990](Z-bibliography.md#ref-ahuja1990faster) (cf. [Ahuja, Magnanti and
Orlin 1993, §4.8](Z-bibliography.md#ref-amo93networkflows)) give the bound
$O(m+n\log(nC))$ on the running time, $m$ and $n$ being the numbers of arcs
and nodes, which follows from Theorem 7.6 with $U\le nC$.

!!! note "abLEMON"
    The boxes of a `RadixHeap` stay contiguous: in LEMON 1.3.1, `moveDown()`
    gave the boxes before the first non-empty one ranges that could reach past
    the start of the next box, which still held its items, so that after an
    `increase()` an item could sit in a box before one with a smaller priority
    and be popped first (trac #681); `test/heap_test.cc` has the case of the
    ticket and a long monotone sequence of operations checked against a sorted
    set.

*Bucket heaps.* `BucketHeap< IM, MIN >`, in `lemon/bucket_heap.h`, keeps a
doubly linked list of the items of each priority, with the heads in a vector
indexed by the priority (the buckets of [Dial
1969](Z-bibliography.md#ref-dial1969algorithm)), together with a pointer $\mu$
to a bucket at or below that of the minimum. The priorities must be
non-negative, since they index the vector, which grows up to the largest
priority plus one. The items are private records `BucketItem` (the item, its
priority and its two links), kept contiguous in a vector: an erased record is
replaced by the last one, whose position is then updated in the
cross-reference map.

<span id="alg-7-6"></span>**Algorithm 7.6** (Bucket heap, `MIN = true`).
```
push(i, p):      link i at the head of bucket p (growing the vector);
                 if p < mu: mu := p
decrease(i, p):  unlink i; if p < mu: mu := p; link i in bucket p
increase(i, p):  unlink i; link i in bucket p
top(), prio(), pop():
                 while bucket mu is empty: mu := mu + 1
                 return (or unlink) the head of bucket mu
erase(i):        unlink i
```

<span id="thm-7-7"></span>**Theorem 7.7.** *A `BucketHeap` keeps every
non-empty bucket at or above $\mu$, so that `top()` returns an item of least
priority. `push()`, `erase()`, `decrease()` and `increase()` cost $O(1)$ apart
from the growth of the vector, and `top()`, `prio()` and `pop()` cost
$O(1+j)$, where $j$ is the number of buckets that $\mu$ passes. On a sequence
of operations with $r$ removals in which no priority pushed or decreased is
below the last minimum removed, the advances of $\mu$ add up to at most $C$,
the largest priority, and all the removals cost $O(r+C)$.*

*Proof.* The invariant holds when the heap is built ($\mu=0$); `push()` and
`decrease()` lower $\mu$ to the new priority when it is below, `increase()`
and `erase()` empty no bucket below $\mu$, and the loop of `top()` advances
$\mu$ over empty buckets only. The costs of the list operations are $O(1)$.
After a removal $\mu$ equals the priority removed, and under the hypothesis no
later operation lowers it; hence, $\mu$ never decreases and never exceeds $C$.
$\square$

With `MIN = false` the heap gives the item of largest priority, and the
pointer moves downwards (Theorem 7.7 holds with the order reversed); e.g.,
`MaxCardinalitySearch` uses `BucketHeap< IM, false >` when its capacity map is
the constant map `ConstMap< Arc, Const< int, 1 > >`. The header states that
the class "is not intended to use as a Dijkstra heap". It works there (the
test runs `Dijkstra` with it), but its time and memory grow with the largest
distance, rather than with the number of nodes.

`SimpleBucketHeap< IM, MIN >` is a reduced bucket heap, with singly linked
lists and a free list of records. It supports only `push()`, `top()`,
`prio()`, `pop()`, `size()`, `empty()`, `clear()` and `state(i)`, with the
costs of Theorem 7.7; it has no `erase()`, `decrease()`, `increase()`, `set()`
or `state(i, st)`, and it does not conform to the concept. Its `operator[](i)`
scans all the buckets, i.e., it costs $O(n+C)$, and it returns $-1$ for an
item that is not in the heap.

## 7.5 Choosing a heap {#sec-7-5}

Table 7.1 summarizes the costs proved in sections 7.2 to 7.4; $n$ is the
number of items in the heap, $N\ge n$ the number of records of a linked heap,
$d$ the degree of a `DHeap`, $K$ the number of boxes of a `RadixHeap` (Theorem
7.6), and $C$ the largest priority of a bucket heap.

<span id="tab-7-1"></span>**Table 7.1.** The costs of the heaps (am. =
amortized; "−" = not available).

| Heap | `push` | `top` | `pop` | `decrease` | `increase`, `erase` |
|--------------------|---------------|---------------|---------------|---------------|--------------------|
| `BinHeap` | $O(\log n)$ | $O(1)$ | $O(\log n)$ | $O(\log n)$ | $O(\log n)$ |
| `QuadHeap` | $O(\log_4 n)$ | $O(1)$ | $O(4\log_4 n)$ | $O(\log_4 n)$ | $O(4\log_4 n)$ |
| `DHeap` | $O(\log_d n)$ | $O(1)$ | $O(d\log_d n)$ | $O(\log_d n)$ | $O(d\log_d n)$ |
| `FibHeap` | $O(1)$ | $O(1)$ | $O(\log N)$ am. | $O(1)$ am. | $O(\log N)$ am. |
| `PairingHeap` | $O(1)$ | $O(1)$ | $O(\log n)$ am. | $o(\log n)$ am. | $O(\log n)$ am. |
| `BinomialHeap` | $O(\log n)$ | $O(1)$ | $O(\log n)$ | $O(\log n)$ | $O(\log n)$ |
| `RadixHeap` | $O(K)$ | $O(K)$ am. | $O(K)$ am. | $O(1)$ am. | $O(K)$, $O(1)$ |
| `BucketHeap` | $O(1)$ | $O(C)$ | $O(C)$ | $O(1)$ | $O(1)$ |
| `SimpleBucketHeap` | $O(1)$ | $O(C)$ | $O(C)$ | − | − |

The costs of `push()` and `decrease()` of the pairing heap are its actual
costs, and the amortized ones are those of Theorem 7.4. For the bucket heaps,
the costs of `top()` and `pop()` are worst case; on a monotone sequence of
operations, they add up to $O(r+C)$ in all (Theorem 7.7).

These bounds suggest a choice, which measurements on the instances of interest
should confirm. A binary or 4-ary heap is the safe default, since it is
simple, it has small constants and good locality, and it supports each
operation in logarithmic time with any priority type and comparison. A
Fibonacci or pairing heap may pay off when the decreases far outnumber the
removals and the heap is large; the counts of comparisons then favour it, but
the constants often do not. A radix heap or a bucket heap fits integer
priorities with a monotone sequence of minima and a "small" span of
priorities.

The algorithms of the library take the type of their heap from their traits
class, and they accept another one through a named template parameter
(cf. [chapter 3](03-mental-model.md#ch-3) and [chapter
10](10-shortest-paths.md#ch-10)). `SetStandardHeap< H, CR >` gives the heap
type `H` and the cross-reference type `CR` (by default
`Digraph::NodeMap< int >`), and the algorithm then creates both objects, as
`CR( digraph )` and `H( cr )`. `SetHeap< H, CR >` gives only the types, and
the objects are passed by the function `heap(h, cr)` before `run()` or
`init()`. For instance, the following declaration runs `Dijkstra` on a radix
heap.
```
Dijkstra< ListDigraph, ListDigraph::ArcMap< int > >
  ::SetStandardHeap< RadixHeap< ListDigraph::NodeMap< int > > >
  ::Create dijkstra( g, length );
```
The program `manual/examples/07-dijkstra-heaps.cc` (the test
`manual_07_dijkstra_heaps`) solves one shortest path problem on a grid digraph
with each heap of the library in this way, and checks the distances against
those of the default binary heap.

!!! note "abLEMON"
    `BidirDijkstra` and `AStar` (which LEMON 1.3.1 did not have) and
    `MinCostArborescence` take the type of their heap from their traits, and
    they accept another one through the named parameter `SetHeap`.

The program `manual/examples/07-heap-sort.cc` (the test `manual_07_heap_sort`)
uses the heaps directly: each of them sorts one sequence of priorities after a
decrease, an increase and a `set()`, with the states checked after each
removal; then a `BinHeap` with `std::greater` and a `BucketHeap` with
`MIN = false` give the largest priority first, a `SimpleBucketHeap` sorts by
push and pop only, and a `RadixHeap` throws on a priority below the last one
removed.

## 7.6 Exercises {#sec-7-6}

<span id="ex-7-1"></span>**Exercise 7.1.** From Lemma 7.1, count the
comparisons of `pop()` in a `DHeap` of $n$ items as a function of $d$, and
find the degree that minimizes the bound of Theorem 7.2 when
$m=n^{1+\epsilon}$ for a fixed $\epsilon>0$.

<span id="ex-7-2"></span>**Exercise 7.2.** Extend
`manual/examples/07-heap-sort.cc` with `DHeap` of degrees 2, 8 and 32, and
with `std::greater` as the comparison for each comparison heap; then call
`erase()` on an item that is not the top and check that the others still come
out in order (Theorems 7.3 to 7.5). The function `heapEraseTest()` of
`test/heap_test.cc` does it for the comparison heaps.

<span id="ex-7-3"></span>**Exercise 7.3.** On grid digraphs of growing size,
time `manual/examples/07-dijkstra-heaps.cc` with each heap and with lengths in
$[1,9]$ and in $[1,10^6]$, and explain the ranking of `BucketHeap` and
`RadixHeap` in the two cases by Theorems 7.6 and 7.7. The test
`test/heap_test.cc` checks the distances of each heap (function
`dijkstraHeapTest()`).

<span id="ex-7-4"></span>**Exercise 7.4.** Show by an example that a
`RadixHeap` throws `PriorityUnderflowError` on a `push()` below the current
minimum after a call of `top()` with no removal (Lemma 7.4), and explain why
`Dijkstra` never does it.

<span id="ex-7-5"></span>**Exercise 7.5.** Write the cross-reference map of a
heap whose items are the integers $0,\dots,k-1$ as a `RangeMap< int >`, and
reuse one heap for several sorts, resetting the map between them with
`state(i, PRE_HEAP)` instead of a new map. The function `heapSortTest()` of
`test/heap_test.cc` sorts with a `RangeMap`.
