# 16. Minimum mean cycles {#ch-16}

We are concerned in this chapter with the directed cycles of least mean cost
of a digraph, i.e., with the cycles whose cost divided by their number of arcs
is minimum. abLEMON has three algorithms for this problem, in the group
[Minimum Mean Cycle
Algorithms](https://dmeoli.github.io/abLEMON/group__min__mean__cycle.html):
Karp's algorithm (`KarpMmc`, in `lemon/karp_mmc.h`), its variant with early
termination by Hartmann and Orlin (`HartmannOrlinMmc`, in
`lemon/hartmann_orlin_mmc.h`), and Howard's policy iteration (`HowardMmc`, in
`lemon/howard_mmc.h`). All three have the same interface, stated as the
concept `concepts::MinMeanCycle` of `lemon/concepts/min_mean_cycle.h`. Inside
the library, the minimum mean cycle is used by the cycle-canceling algorithm
of [chapter 13](13-min-cost-flow.md#ch-13) and by the certificates of
unboundedness of [chapter 14](14-k-best-flows.md#ch-14). Section 16.1 states
the problem and its optimality conditions, sections 16.2 to 16.4 describe the
three algorithms, and section 16.5 describes the interface they share.

## 16.1 The problem {#sec-16-1}

Let $D=(V,A)$ be a digraph with $n$ nodes, $m$ arcs and a cost
$c(a)\in\mathbb{R}$ on each arc $a$. For a directed cycle $C$, with $|C|$ arcs
and cost $c(C)=\sum_{a\in C}c(a)$, the *mean cost* of $C$ is
$\mu(C)=c(C)/|C|$, and the minimum cycle mean of $D$ is

$$\mu^*(D)\;=\;\min\{\,\mu(C)\;:\;C\ \text{a directed cycle of}\ D\,\}.\tag{16.1}$$

A digraph without directed cycles has no minimum cycle mean, and the
algorithms report this case. Since each directed cycle lies in one strongly
connected component, $\mu^*(D)$ is the least of the minimum cycle means of the
components; the three classes therefore find the components first (by
`stronglyConnectedComponents()` of [chapter 18](18-properties.md#ch-18)),
solve the problem on each component that has a cycle, and keep the best cycle.
This problem is closely related to conservative costs, i.e., to costs under
which no directed cycle has a negative cost (cf. [Ahuja, Magnanti and Orlin
1993, chapter 5](Z-bibliography.md#ref-amo93networkflows)).

<span id="thm-16-1"></span>**Theorem 16.1.** *Let $\mu\in\mathbb{R}$. Every
directed cycle of $D$ has mean cost at least $\mu$ if and only if there are
potentials $\pi:V\rightarrow\mathbb{R}$ such that*

$$c(u,v)-\mu+\pi(u)-\pi(v)\;\geq\;0\qquad\text{for every arc}\ (u,v)\in A.\tag{16.2}$$

*Proof.* If (16.2) holds, summing it along a cycle $C$ cancels the potentials
and gives $c(C)-\mu|C|\geq 0$. Conversely, if no cycle has mean less than
$\mu$, the costs $c-\mu$ are conservative, and the distances from a virtual
source joined to all the nodes by arcs of cost 0 are finite and satisfy
(16.2). $\square$

Hence, $\mu^*(D)$ is the largest $\mu$ for which the costs $c-\mu$ are
conservative, i.e., the negative of the smallest amount that, added uniformly
to the costs, makes them conservative (as the documentation of the group puts
it). Furthermore, a cycle $C$ together with potentials that satisfy (16.2) for
$\mu=\mu(C)$ certifies that $C$ is a minimum mean cycle. Hartmann and Orlin
check this certificate to stop early, Howard's algorithm maintains potentials
of this kind (the distances of section 16.4), and the example of section 16.5
checks the certificate by a Bellman-Ford computation.

## 16.2 Karp's algorithm {#sec-16-2}

Let $D$ be strongly connected and let $s$ be any node. For $k=0,\ldots,n$ and
$v\in V$, let $D_k(v)$ be the least cost of a directed walk from $s$ to $v$
with exactly $k$ arcs ($+\infty$ if there is none), so that $D_0(s)=0$,
$D_0(v)=+\infty$ for $v\neq s$, and

$$D_k(v)\;=\;\min\{\,D_{k-1}(u)+c(u,v)\;:\;(u,v)\in A\,\}\qquad k=1,\ldots,n.\tag{16.3}$$

We use the following characterization, due to [Karp
(1978)](Z-bibliography.md#ref-karp78characterization).

<span id="thm-16-2"></span>**Theorem 16.2.** *If $D$ is strongly connected,
then*

$$\mu^*(D)\;=\;\min_{v\in V:\,D_n(v)<\infty}\;\max_{0\leq k<n:\,D_k(v)<\infty}\;\frac{D_n(v)-D_k(v)}{n-k}.\tag{16.4}$$

A short proof is in [Karp
(1978)](Z-bibliography.md#ref-karp78characterization); it also shows that a
walk of $n$ arcs that attains $D_n(v)$ at a minimizing node $v$ contains a
cycle of mean $\mu^*(D)$. Karp's algorithm computes the table (16.3) round by
round and then evaluates (16.4). In `KarpMmc` this is done on each strongly
connected component, with $s$ its first node and $n$ its number of nodes.

<span id="alg-16-1"></span>**Algorithm 16.1** (Karp, on a strongly connected
component of n nodes).

```
1  s <- the first node; D_0(s) <- 0; D_0(v) <- +inf for v != s
2  for k = 1, ..., n:
3    for each arc (u, v) of the component with D_{k-1}(u) < +inf:
4      if D_{k-1}(u) + c(u, v) < D_k(v):
5        D_k(v) <- D_{k-1}(u) + c(u, v); pred_k(v) <- (u, v)
6  for each node v with D_n(v) < +inf:
7    M(v) <- max{ (D_n(v) - D_k(v)) / (n - k) : 0 <= k < n, D_k(v) < +inf }
8  mu <- min_v M(v), attained at v*
9  (findCycle) walk back from v* along pred_n, pred_{n-1}, ... until a
   node repeats; the arcs between its two visits form the cycle
```

In the code the rounds of steps 2 to 5 first scan only the nodes already
reached, until all the nodes of the component have been reached, and then all
of them (`processNextBuildRound()` and `processNextFullRound()`); the
comparison of step 4 goes through the tolerance. In steps 7 and 8 the ratios
are compared without division, by cross-multiplication in the type
`LargeCost`. Each round costs $O(m)$, and therefore the algorithm runs in
$O(nm)$ time; it stores $n+1$ values for each node, which gives $O(n^2+m)$
space, as the header states. We remark that `findCycleMean()` stops at step 8,
and that until `findCycle()` has been called, `cycleCost()` and `cycleSize()`
return the numerator and the denominator of the ratio in (16.4) at $v^*$.
Their quotient is $\mu^*(D)$, but they need not be the cost and the length of
a cycle; after `findCycle()`, they are those of the cycle found in step 9.

## 16.3 The Hartmann-Orlin algorithm {#sec-16-3}

[Hartmann and Orlin (1993)](Z-bibliography.md#ref-hartmann93finding) observed
that Karp's table often contains a minimum mean cycle rather long before round
$n$, and that Theorem 16.1 allows one to recognize it. At some rounds $k$ the
algorithm follows backwards the walks that end at round $k$, takes the cycle
$C$ of least mean among those that these walks close, and computes the
potentials

$$\pi(v)\;=\;\min_{0\leq j\leq k}\;\big(\,|C|\,D_j(v)-j\,c(C)\,\big),\tag{16.5}$$

i.e., $|C|$ times the least cost under $c-\mu(C)$ of a walk from $s$ to $v$
with at most $k$ arcs. If these potentials satisfy (16.2) for $\mu=\mu(C)$
(scaled by $|C|$), then $C$ is optimal by Theorem 16.1, and the algorithm
stops.

<span id="alg-16-2"></span>**Algorithm 16.2** (Hartmann-Orlin, on a strongly
connected component of n nodes).

```
1  as steps 1 to 5 of Algorithm 16.1, but after the rounds k = 4, 6, 9,
   13, ... (each the previous one times 3/2) and after the round k = n:
2    for each node v with D_k(v) < +inf, walk back from v along
     pred_k, pred_{k-1}, ...; each node met twice closes a cycle
3    C <- the cycle of least mean among those closed in step 2
4    if a cycle was found and k < n:
5      compute pi by (16.5)
6      if |C| c(u, v) - c(C) >= pi(v) - pi(u) for every arc (u, v):
7        stop: C is a minimum mean cycle of the component
8    if k = n: stop with the cycle C
```

This schedule of the checks is the one of `processRounds()` in
`lemon/hartmann_orlin_mmc.h` (the first check after round 4, then at rounds 6,
9, 13, and so on, rounded down), and the test of step 6 goes through the
tolerance; step 6 scans the arcs inside the component, the only ones whose
ends have potentials.

!!! note "abLEMON"
    In LEMON 1.3.1 step 6 scanned the arcs of the whole digraph, whose ends
    outside the component have no potentials computed, so that the test could
    fail more often and delay the stop.

In the worst case the checks add to the work of Karp's algorithm, and the
bounds of the header are the same, $O(nm)$ time and $O(n^2+m)$ space.
According to the header, the early termination makes the algorithm
significantly faster on some instances, but slower on others.

## 16.4 Howard's policy iteration {#sec-16-4}

Howard's algorithm comes from the policy iteration of Markov decision
processes ([Howard 1960](Z-bibliography.md#ref-howard1960dynamic)); its use
for the minimum mean cycle is analysed in [Cochet-Terrasson et
al. (1998)](Z-bibliography.md#ref-cochetterrasson1998numerical) and in [Dasdan
and Gupta (1998)](Z-bibliography.md#ref-dasdan98minmeancycle). A *policy*
chooses one leaving arc for each node of a strongly connected component, and
the arcs chosen form the *policy graph*, in which every node has out-degree 1.
Each weakly connected part of the policy graph therefore contains one cycle,
and the algorithm evaluates the policy by the cycle of least mean among them.
Then it improves the policy by the distances under $c-\lambda$, where
$\lambda$ is that mean, towards the cycle.

<span id="alg-16-3"></span>**Algorithm 16.3** (Howard, on a strongly connected
component).

```
1  for each node u: policy(u) <- an arc (u, v) of least cost
2  loop
3    find the cycles of the policy graph; C <- the one of least mean
4    d(v0) <- 0 for a node v0 of C; backwards from v0 along the policy
     arcs (u, v): d(u) <- d(v) + |C| c(u, v) - c(C)
5    for the nodes not reached yet, backwards along any arc (u, v) from
     the nodes reached: policy(u) <- (u, v); d(u) <- d(v) + |C| c(u, v) - c(C)
6    improved <- false
7    for each arc (u, v): if d(v) + |C| c(u, v) - c(C) < d(u):
8      d(u) <- d(v) + |C| c(u, v) - c(C); policy(u) <- (u, v); improved <- true
9    if not improved: return C
```

Steps 1, 3 and 4 to 9 are `buildPolicyGraph()`, `findPolicyCycle()` and
`computeNodeDistances()` of `lemon/howard_mmc.h`, and each loop is one
iteration. Note that the distances are scaled by $|C|$, so that integer costs
give integer distances, and the test of step 7 goes through the tolerance.
When step 7 improves no distance, the distances satisfy (16.2) for
$\mu=\mu(C)$ (scaled by $|C|$), and $C$ is optimal by Theorem 16.1; if it
improves some, the mean of the best cycle of the policy does not increase. As
the header remarks, the best known bound on the number of iterations is
exponential; however, the documentation of the group reports that in practice
Howard's algorithm is by far the most efficient of the three, in agreement
with the experiments of [Dasdan
(2004)](Z-bibliography.md#ref-dasdan04experimental).

One can also limit the iterations, and then the algorithm returns a cycle that
is usually good, but not necessarily optimal. Indeed, `findCycleMean(limit)`
takes the largest number of iterations (by default the largest `int`, i.e., no
limit), counted over all the components together. It returns a value of the
enumeration `TerminationCause`: `NO_CYCLE` (0) if the digraph has no directed
cycle, `OPTIMAL` (1) if the minimum mean was found, and `ITERATION_LIMIT` if
the limit was reached first. In the last case the cycle kept is the best one
met so far, whose mean is an upper bound on $\mu^*(D)$, and the components
after the one where the limit was reached are not processed at all. Since
`NO_CYCLE` is 0, the result converts to `false` only when there is no cycle,
and `run()` keeps the meaning of a `bool`.

Howard's algorithm ends on each component with a policy whose cycle is a
minimum mean cycle of that component. `HowardMmc` keeps these cycles, one for
each component that has a cycle, and gives them by `componentCycleNum()`, the
number of such components, by `componentCycleCost(i)` and
`componentCycleSize(i)`, the cost and the number of arcs of the $i$-th cycle,
and by `componentCycle(i, path)`, which appends the $i$-th cycle to a path
that has `addBack()`. These cycles are pairwise node-disjoint, since the
components are. They are of minimum mean in their components only if
`findCycleMean()` was not stopped by its limit; otherwise they are those of
the components processed until then (a component where the limit stopped the
search before its first policy cycle keeps none). The min-cost flow algorithms
of [chapter 13](13-min-cost-flow.md#ch-13) take from one run of `HowardMmc`
the disjoint negative cycles that their function `unboundedCycles(k)` returns
(see [chapter 14](14-k-best-flows.md#ch-14)).

!!! note "abLEMON"
    `componentCycleNum()`, `componentCycleCost()`, `componentCycleSize()` and
    `componentCycle()` are new in abLEMON (version 1.9.0), and
    `unboundedCycles(k)` of the four minimum cost flow algorithms reads them.
    On the other hand, the iteration limit of `findCycleMean()` was already in
    LEMON 1.3.

![Figure 16.1. A digraph with two strongly connected components (the tinted regions) and the cost of each arc on its plate: the cycle 0, 1, 2 of mean 2 is the best of the first component, and the cycle 4, 5, 6 of mean 4/3 the minimum mean cycle of the digraph.](../figures/16-mmc.svg){#fig-16-1}

## 16.5 The common interface {#sec-16-5}

Each of the three classes has the form

```cpp
template< typename GR , typename CM = typename GR::template ArcMap< int > ,
          typename TR = KarpMmcDefaultTraits< GR , CM > >
class KarpMmc;
```

(with `HartmannOrlinMmcDefaultTraits` and `HowardMmcDefaultTraits` for the
other two), where `GR` is the digraph type, `CM` a readable arc map of costs
and `TR` the traits class. Their default traits define `Digraph`, `CostMap`,
`Cost` (the value type of the costs), `LargeCost`, the type of the internal
computations, `Tolerance`, which is `lemon::Tolerance< LargeCost >`, and
`Path`, the type of the cycle found, which is `lemon::Path< Digraph >`. By
default, `LargeCost` is `long long` when `Cost` is an integer type (`long` if
the compiler has no `long long`) and `double` otherwise; `Cost` must be
convertible to it. Also, `Path` must meet the concept of [chapter
8](08-auxiliary.md#ch-8) and have `addFront()` for `KarpMmc` and
`HartmannOrlinMmc`, `addBack()` for `HowardMmc`, which are the only functions
the algorithms call on it. Two named template parameters change the traits:
`SetLargeCost< T >`, which sets `LargeCost` to `T` and `Tolerance` to
`lemon::Tolerance< T >`, and `SetPath< T >`, which sets `Path`; e.g.,
`HowardMmc< G , CM >::SetPath< ListPath< G > >::Create` stores the cycle in a
`ListPath`.

Their constructor takes the digraph and the costs, `cycle(path)` makes the
algorithm store the cycle in an external path (otherwise it allocates one,
which its destructor deletes), and `tolerance(tol)` and `tolerance()` set and
read the tolerance; the first two return the object, so that calls may be
chained, as in `mmc.cycle(p).run()`. To execute the algorithm one calls
`run()`, which is `findCycleMean() && findCycle()` and returns `true` if the
digraph has a directed cycle. It may be called again after the digraph or the
costs have changed. In more detail, `findCycleMean()` computes the minimum
mean (it returns a `bool` in `KarpMmc` and `HartmannOrlinMmc`, a
`TerminationCause` in `HowardMmc`), and `findCycle()` builds the cycle from
its data, appending the arcs to the path. Note that `findCycleMean()` clears
the path, and `findCycle()` does not; hence, `findCycle()` should be called
once after each `findCycleMean()`. We read the results by `cycleCost()` (a
`Cost`), `cycleSize()` (an `int`), `cycleMean()`, which is
`double(cycleCost()) / cycleSize()`, and `cycle()`, a constant reference to
the path.

In all three, the tolerance is used where the algorithms compare costs of
walks or distances, namely in step 4 of Algorithm 16.1, in steps 2 to 6 of
Algorithm 16.2 and in step 7 of Algorithm 16.3. On the contrary, the
comparisons of the means of two cycles are exact cross-multiplications in
`LargeCost`, which must therefore hold the products of a cost of a cycle by a
number of arcs; with integer costs, this is the reason why `LargeCost` is
`long long` by default. Finally, the concept
`concepts::MinMeanCycle< GR , Cost >` of `lemon/concepts/min_mean_cycle.h`
checks this interface (with `SetPath< ListPath< GR > >` and
`SetLargeCost< Cost >`), and `test/min_mean_cycle_test.cc` checks the three
classes against it, with `int` and `float` costs.

!!! note "abLEMON"
    The concept `concepts::MinMeanCycle` is in
    `lemon/concepts/min_mean_cycle.h` since abLEMON 1.9.0, where the classes
    added to the library are held to it; in LEMON 1.3.1 the same checks were
    written in the test.

Our example below solves the problem of Figure 16.1 by the three classes,
which agree on the mean $4/3$. Each cycle found is checked to be a closed walk
with the cost and the length reported, and the mean is certified by Theorem
16.1: the costs $|C|\,c(a)-c(C)$ are given to `BellmanFord` of [chapter
10](10-shortest-paths.md#ch-10), which, started with all the distances equal
to 0, finds no negative cycle. Then `HowardMmc` gives the two cycles of the
components, of means 2 and $4/3$, and with a limit of one iteration a cycle
whose mean is not less than the optimal one.

```cpp
--8<-- "examples/16-min-mean-cycle.cc"
```

In our view, the choice among the three is usually simple: `HowardMmc` is the
fastest in practice and the only one with the cycles of the components and the
limit on the iterations, while `KarpMmc` and `HartmannOrlinMmc` have the
polynomial bound $O(nm)$, which may matter when the running time must be
bounded a priori.

## 16.6 Exercises {#sec-16-6}

<span id="ex-16-1"></span>**Exercise 16.1.** Prove Theorem 16.1, and verify it
on the digraph of Figure 16.1 by computing with `BellmanFord` the potentials
of (16.2) for $\mu=4/3$, scaled by 3 (the example
`manual/examples/16-min-mean-cycle.cc` checks only that no negative cycle
exists).

<span id="ex-16-2"></span>**Exercise 16.2.** Compute by hand the table
$D_k(v)$ of (16.3) for the component $\{4,5,6\}$ of Figure 16.1, with $s=4$,
and evaluate (16.4); then compare `cycleCost()` and `cycleSize()` of `KarpMmc`
after `findCycleMean()` and after `findCycle()`.

<span id="ex-16-3"></span>**Exercise 16.3.** Run the three classes on the four
cost functions of `test/min_mean_cycle_test.cc`, and check that they find the
cycles that the test expects (the test does it with `checkMmcAlg()`).

<span id="ex-16-4"></span>**Exercise 16.4.** For the cost function `len1` of
`test/min_mean_cycle_test.cc`, find the least limit for which
`HowardMmc::findCycleMean(limit)` returns `OPTIMAL` (the test checks that 2
iterations are not enough and 4 are).

<span id="ex-16-5"></span>**Exercise 16.5.** Use `componentCycleNum()` and
`componentCycle()` of `HowardMmc` to list, for a digraph with several strongly
connected components, the node-disjoint cycles of minimum mean of the
components, and keep those of negative mean, as `unboundedCycles(k)` does (the
example of section 16.5 lists the two cycles of Figure 16.1).

<span id="ex-16-6"></span>**Exercise 16.6.** Show that adding a constant
$\alpha$ to all the costs adds $\alpha$ to $\mu^*(D)$ and leaves the minimum
mean cycles unchanged, and check it with any of the three classes on the
digraph of Figure 16.1.
