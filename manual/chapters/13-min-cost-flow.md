# 13. Minimum cost flows {#ch-13}

We are concerned in this chapter with the four minimum cost flow algorithms of
the library, `NetworkSimplex` (`lemon/network_simplex.h`), `CostScaling`
(`lemon/cost_scaling.h`), `CapacityScaling` (`lemon/capacity_scaling.h`) and
`CycleCanceling` (`lemon/cycle_canceling.h`), and with the concept
`concepts::MinCostFlow` of `lemon/concepts/min_cost_flow.h` that they meet. We
also describe two internal headers that a user meets through their effects,
i.e., the default tolerances of `lemon/bits/mcf_tolerance.h`, and the starting
flow and the bound on the infinite capacities of
`lemon/bits/mcf_feasible_flow.h`. Section 13.1 states the problem in the forms
that the library takes, together with its optimality conditions; section 13.2
describes the interface that the four classes share, and section 13.3 the way
in which they handle floating-point data. Sections 13.4 to 13.8 describe the
algorithms one by one, and section 13.9 gives some advice on the choice among
them. In [chapter 14](14-k-best-flows.md#ch-14) we treat the certificates that
the four classes give when a problem has no optimal flow, and the classes that
enumerate several flows.

## 13.1 The problem, its forms and its dual {#sec-13-1}

Let $G=(V,A)$ be a digraph with $n$ nodes and $m$ arcs, and let
$lower: A\rightarrow\mathbf{R}$ and
$upper: A\rightarrow\mathbf{R}\cup\{+\infty\}$, with $lower\leq upper$, be
bounds on the flow of the arcs. Let also $cost: A\rightarrow\mathbf{R}$ be the
cost of a unit of flow on each arc, and $sup: V\rightarrow\mathbf{R}$ the
signed supplies of the nodes (a supply if positive, a demand if negative).
Following [Ahuja, Magnanti and Orlin
1993](Z-bibliography.md#ref-amo93networkflows) and the page
`doc/min_cost_flow.dox` of the documentation, the library solves the problem
in the form with "greater or equal" supply constraints (GEQ):

$$
\begin{aligned}
\min\ & \sum_{uv\in A} cost(uv)\, f(uv) \\
& \sum_{uv\in A} f(uv) - \sum_{vu\in A} f(vu) \geq sup(u) \qquad \forall u\in V \\
& lower(uv) \leq f(uv) \leq upper(uv) \qquad \forall uv\in A .
\end{aligned}
\tag{13.1}
$$

Since the left-hand sides sum to zero, (13.1) can be feasible only if
$\sum_{u\in V} sup(u)\leq 0$. Thus, the supplies must all leave their nodes,
while some demand may be left unmet. When the supplies sum to zero, the
constraints hold as equalities, and (13.1) is the usual equality form of the
problem; a user who wants that form must therefore make the supplies sum to
zero. In addition, `NetworkSimplex` takes the opposite form with "less or
equal" constraints (LEQ),

$$
\sum_{uv\in A} f(uv) - \sum_{vu\in A} f(vu) \leq sup(u) \qquad \forall u\in V ,
\tag{13.2}
$$

which needs $\sum_{u\in V} sup(u)\geq 0$, and in which the demands must be met
while some supply may stay at its node. The other three classes take only the
GEQ form; one can bring the LEQ form to it by reversing the arcs and negating
the supplies, e.g., with `ReverseDigraph` ([chapter 5](05-adaptors.md#ch-5))
and `NegMap` ([chapter 6](06-maps.md#ch-6)).

In the dual problem the variables are the *potentials*
$\pi: V\rightarrow\mathbf{R}$ of the nodes, and the *reduced cost* of an arc
under them is

$$
cost^\pi(uv) = cost(uv) + \pi(u) - \pi(v) .
\tag{13.3}
$$

Hence, the potentials are the opposite of the prices of the textbook
convention, in which $cost^\pi(uv)=cost(uv)-\pi(u)+\pi(v)$; we keep the sign
of the library throughout. For the GEQ form, the optimality conditions are the
following complementary slackness conditions of linear programming.

<span id="thm-13-1"></span>**Theorem 13.1.** *A feasible flow $f$ of (13.1) is
optimal if and only if there are potentials $\pi$ such that, for each arc $uv$
and each node $u$,*

$$
\begin{aligned}
& cost^\pi(uv) > 0 \Rightarrow f(uv) = lower(uv) , \qquad
cost^\pi(uv) < 0 \Rightarrow f(uv) = upper(uv) , \\
& \pi(u) \leq 0 , \qquad
\textstyle\sum_{uv\in A} f(uv) - \sum_{vu\in A} f(vu) \neq sup(u) \Rightarrow \pi(u) = 0 .
\end{aligned}
\tag{13.4}
$$

*For the LEQ form the same holds with $\pi(u)\geq 0$ in place of
$\pi(u)\leq 0$.* This is the duality theorem of linear programming applied to
(13.1), whose constraint matrix is totally unimodular; see [Ahuja, Magnanti
and Orlin 1993, chapter 9](Z-bibliography.md#ref-amo93networkflows). We shall
often use a consequence of it, which needs the residual network of a flow $f$.
This network has an arc $uv$ of residual capacity $upper(uv)-f(uv)$ and cost
$cost(uv)$ wherever $f(uv)<upper(uv)$, and an arc $vu$ of residual capacity
$f(uv)-lower(uv)$ and cost $-cost(uv)$ wherever $f(uv)>lower(uv)$.

<span id="thm-13-2"></span>**Theorem 13.2.** *(Negative cycle optimality.) A
feasible flow of the equality form is optimal if and only if its residual
network has no directed cycle of negative cost. Then (13.4) holds for the
potentials $d$, where $d(u)\leq 0$ is the cost of a shortest residual path to
$u$ from an extra node joined to all the others by arcs of cost zero.* This is
due to [Klein 1967](Z-bibliography.md#ref-klein67primal), and it underlies the
cycle canceling methods of section 13.8 and the final potentials of cost
scaling.

Three outcomes are possible, and `run()` returns them as the constants of the
nested enum `ProblemType` of each class. With `OPTIMAL` a flow and potentials
meeting (13.4) were found; with `INFEASIBLE` no flow satisfies the
constraints; and with `UNBOUNDED` the problem is feasible, while a directed
cycle of negative cost, whose arcs have infinite upper bounds, lets the cost
decrease without limit. An infinite upper bound is the constant `INF` of the
class, i.e., the infinity of `Value` if the type has one and its largest value
otherwise. Integer data have integer optimal flows, since the matrix of (13.1)
is totally unimodular, and on integer data the four algorithms return one of
them.

## 13.2 The common interface {#sec-13-2}

All four classes take as template parameters the type `GR` of the digraph, the
number type `V` of the flows, of the bounds and of the supplies (by default
`int`), and the number type `C` of the costs and of the potentials (by default
`V`); both must be signed, as the constructors assert. `CostScaling` and
`CapacityScaling` take a fourth parameter, a traits class, which we describe
with them. The constructor takes the digraph, which must not change afterwards
unless `reset()` is called. Then the parameters of the problem are given by
functions that copy the data at once (hence, the maps need not outlive the
call), each returning the object:

-   `lowerMap(map)`, the lower bounds, zero on the arcs if it is not called;
-   `upperMap(map)`, the upper bounds, `INF` on the arcs if it is not called;
-   `costMap(map)`, the costs, 1 on the arcs if it is not called;
-   `supplyMap(map)`, the supplies, zero if neither it nor `stSupply()` is
called, and `stSupply(s, t, k)`, which gives $k$ to $s$, $-k$ to $t$ and zero
to the other nodes;
-   `supplyType(type)`, with `GEQ` (the default) or `LEQ`, in `NetworkSimplex`
only.

Then `run()` solves the problem and returns its `ProblemType`, with arguments
that differ from class to class (Table 13.1). After a run, the queries are
`totalCost()`, i.e., the cost of the flow in $O(m)$ time, `flow(arc)` and
`flowMap(map)`, which give the flow, and `potential(node)` and
`potentialMap(map)`, which give the potentials. With a template argument,
e.g., `totalCost< double >()`, the sum is made in another type, which helps
when it may overflow `Cost`. We describe the certificates `infeasibleCut()`,
`infeasibleCuts(k)`, `disjointInfeasibleCuts()`, `unboundedCycle()` and
`unboundedCycles(k)`, which are meaningful after a run that returned
`INFEASIBLE` or `UNBOUNDED`, in [chapter 14](14-k-best-flows.md#ch-14), and
the tolerances in section 13.3.

One may call `run()` again: the parameters are kept, and only those that
changed have to be given anew. `resetParams()` sets the parameters back to
their defaults (and the supply type to `GEQ`), while `reset()` also rebuilds
the internal copy of the digraph, as is needed when the digraph itself was
modified after the construction or after the last `reset()`. In any case a
second run solves the problem from scratch, whatever changed.

<span id="tab-13-1"></span>**Table 13.1.** The four classes: the arguments of
`run()` with their defaults, the worst case running time stated in each header
($U$ is the largest supply or capacity, $K$ the largest cost), the supply
types, and the template parameters beyond `GR`, `V` and `C`.

| Class | `run()` | Worst case | Forms | Traits |
|-----------------|----------------------------|-------------------------|--------|-----------|
| `NetworkSimplex` | `pivot_rule = BLOCK_SEARCH` | not stated | GEQ, LEQ | none |
| `CostScaling` | `method = PARTIAL_AUGMENT`, `factor = 16` | $O(n^2m\log(nK))$ | GEQ | `LargeCost` |
| `CapacityScaling` | `factor = 4` | $O(m\log U\,(n+m)\log n)$ | GEQ | `Heap` |
| `CycleCanceling` | `method = CANCEL_AND_TIGHTEN` | $O(n^2m^2\log n)$ | GEQ | none |

All four meet the concept `concepts::MinCostFlow< GR , V , C >` of
`lemon/concepts/min_cost_flow.h`. It requires the constructor from the
digraph, `reset()`, `resetParams()`, the five setters of the parameters
(without `supplyType()`), `run()` with its defaults, the queries above, the
tolerance functions of section 13.3, and the single certificates
`infeasibleCut()` and `unboundedCycle()`. A function template written against
these members accepts any of the four, as does the function `optimal()` of the
program below. This program solves a transshipment problem of seven nodes with
the four classes, and it checks that their costs agree and that the flow and
the potentials of each meet (13.4). Then it changes the cost of one arc and
runs `NetworkSimplex` again. Finally it gives two more units of supply to one
node; the problem is then infeasible in the GEQ form, but it has an optimal
flow in the LEQ form, with nonnegative potentials.

```cpp
--8<-- "examples/13-min-cost-flow.cc"
```

!!! note "abLEMON"
    The four classes answer `OPTIMAL`, with the empty flow, on a digraph
    without nodes, which LEMON 1.3.1 reported as infeasible. They answer
    `UNBOUNDED` on the same problems, i.e., the feasible ones with a cycle of
    negative cost made of arcs of infinite upper bound: in LEMON 1.3.1
    `CostScaling` and `CycleCanceling` gave that answer on any arc of negative
    cost and infinite upper bound, `CapacityScaling` gave a wrong answer on
    feasible problems that its finite replacement of the infinite bounds made
    infeasible, and `NetworkSimplex` did not check feasibility first. The
    function `runWarm()`, which versions 1.4.0 to 1.5.0 added to re-optimize
    from the last basis, flow or potentials, was removed in 1.6.0: a
    re-optimization belongs to whoever knows what has changed, and a second
    `run()` solves the changed problem.

## 13.3 Tolerances and floating-point data {#sec-13-3}

With integer types the data must be integer, and the comparisons are exact.
With floating-point types the data need not be integer; however, a comparison
of a residual capacity or of a reduced cost with an exact zero would make the
algorithms pivot or push round-off for ever, or find a feasible problem
infeasible. Instead, the four classes compare against two tolerances, one on
the flows and one on the reduced costs, defined once in
`lemon/bits/mcf_tolerance.h`. Unless the caller sets them, they are zero for
an exact type and otherwise

$$
\epsilon_f = 10^6\,\varepsilon\,\max\{1,\ |sup(u)|,\ |lower(uv)|,\ |upper(uv)| < \infty\} ,
\qquad
\epsilon_c = 10^6\,\varepsilon\,\max\{1,\ |cost(uv)|\} ,
\tag{13.5}
$$

where $\varepsilon$ is the machine epsilon of the type (about
$2.2\cdot 10^{-16}$ for `double`), and the maxima run over the nodes and the
arcs, with the infinite upper bounds left out. In other words, the tolerances
are $10^6$ units in the last place of the largest datum, or of 1 when the data
are smaller. An excess, a deficit or a residual capacity within $\epsilon_f$
is taken as zero, and a reduced cost above $-\epsilon_c$ as nonnegative. Also,
a sum of the supplies within $\epsilon_f$ of zero is taken as zero, since
supplies such as $0.1$, $0.2$ and $-0.3$, which sum to zero in decimal, in
binary sum to a few units in the last place of either sign.

A call of `tolerances(flow, cost)` sets the two tolerances, and a negative
value, the default, leaves them to (13.5). In turn, `flowTolerance()` and
`costTolerance()` return the tolerances in use, as a `Tolerance< Value >` and
a `Tolerance< Cost >` (types named `FlowTolerance` and `CostTolerance` in each
class) whose epsilon is zero for an exact type. We remark that
`NetworkSimplex` computes them from its data at any time, while the other
three return those that the last `run()` used, and zero before the first one.
A tolerance of zero reproduces the behavior of LEMON 1.3.1, in which rounding
errors may keep the algorithm going for ever; hence, one should set it only on
data known to be exact in binary.

Our next program solves with the four classes a problem whose supplies are
$0.1$, $0.2$ and $-0.3$, and whose capacities and costs are fractional. It
checks the optimal cost, $0.14$, and the tolerances (13.5), which here are
$10^6\varepsilon$ on the flows (the data being below 1) and
$1.1\cdot 10^6\varepsilon$ on the costs. Finally it sets other tolerances,
which `flowTolerance()` then returns.

```cpp
--8<-- "examples/13-floating-point.cc"
```

!!! note "abLEMON"
    The four algorithms solve problems with floating-point costs, capacities
    and supplies. In LEMON 1.3.1 `NetworkSimplex` compared reduced costs and
    residual flows with an exact zero, and typically pivoted for ever or
    reported infeasibility on such data, while the other three required
    integer data and answered `INFEASIBLE` on supplies that sum to zero in
    decimal but not in binary. The default tolerances have one definition for
    the four (version 1.9.0), those of `NetworkSimplex` on the flows counting
    the finite bounds too, and `tolerances()`, `flowTolerance()` and
    `costTolerance()` are new.

## 13.4 The network simplex method {#sec-13-4}

### 13.4.1 Bases, pivots and potentials {#sec-13-4-1}

`NetworkSimplex` implements the primal network simplex method ([Dantzig
1963](Z-bibliography.md#ref-dantzig63linearprog); [Ahuja, Magnanti and Orlin
1993, chapter 11](Z-bibliography.md#ref-amo93networkflows); [Kelly and O'Neill
1991](Z-bibliography.md#ref-kellyoneill91netsimplex)), i.e., the simplex
method of linear programming specialized to (13.1), in which a basis is a
spanning tree. After moving the lower bounds into the supplies, the class adds
an artificial root node and, for each node, an artificial arc between it and
the root, oriented by the sign of its supply. The initial basis is the tree of
these arcs, which carry the supplies. Depending on the sign of the sum of the
supplies, a second set of artificial arcs lets the nodes keep the unmet demand
(GEQ) or the unused supply (LEQ) at no cost. The artificial arcs whose flow
would leave a constraint violated have a large cost, larger than the cost of
any path: for an exact `Cost` it is half of the largest value of the type plus
one, and otherwise $(1+\max_{uv}|cost(uv)|)\,n$. Thus, the method is the
"big-M" variant of the simplex method, and the problem is infeasible exactly
when some flow (beyond $\epsilon_f$) is left at the end on the artificial arcs
that may carry none.

Each nonbasic arc is at its lower or at its upper bound, and the potentials
are those that give a zero reduced cost to the arcs of the tree. A nonbasic
arc is *eligible* if its reduced cost (13.3) is negative at its lower bound,
or positive at its upper bound, since sending flow around the cycle that it
closes with the tree then decreases the cost. A *pivot* chooses an eligible
arc by the pivot rule, and finds its cycle through the *join* node, i.e., the
first common ancestor of its ends in the tree. Then it sends around the cycle
the largest amount that the bounds allow, and it replaces by the entering arc
the first arc of the cycle that reaches a bound (Figure 13.1). When no arc is
eligible, the flow and the potentials satisfy (13.4). An amount of zero (a
*degenerate* pivot) changes the basis but not the flow, while an unbounded
amount (a cycle of arcs with infinite upper bounds and negative cost) proves
the problem unbounded, provided it is feasible.

![Figure 13.1. A pivot of the network simplex method: the tree of the basis hung from the root r (in the primary colour), the entering arc (4,5) of negative reduced cost, which closes a cycle with the tree path from 5 through the join node r back to 4, and the leaving arc (1,r), the first arc of the cycle to reach a bound.](../figures/13-pivot.svg){#fig-13-1}

<span id="alg-13-1"></span>**Algorithm 13.1** (Network simplex).

```
1  move the lower bounds into the supplies; if the sum of the supplies has
   the wrong sign for the supply type: return INFEASIBLE
2  T := tree of the artificial arcs from and to the root; set the flows,
   the potentials and the thread of T
3  make the heuristic initial pivots (see below)
4  while the pivot rule finds an eligible arc e:
5      w := join node of the ends of e in T
6      delta := largest amount around the cycle of e and T;
       f := first arc of the cycle to reach a bound
7      if delta is infinite: go to 12
8      send delta around the cycle; if f != e: T := T - f + e
9      update the potentials of the smaller part that f splits off
10 if some artificial arc carries more than eps_f: return INFEASIBLE
11 put the lower bounds back, shift the potentials; return OPTIMAL
12 solve the problem again with zero costs: if it is infeasible, return
   INFEASIBLE; otherwise solve it again with its costs: return UNBOUNDED
```

Step 9 is `updatePotential()`. When the subtree that the pivot moves holds
more than half of the nodes, it changes instead the potentials of the rest of
the tree, in the opposite direction; this leaves the reduced costs as they
are, and moves the potential of the root away from zero (within a limit) until
a final shift brings it back. When the supplies sum to zero, step 11 shifts
the potentials so that their largest value is at most zero under GEQ, and
their smallest at least zero under LEQ, as (13.4) requires. Finally, step 12
is `feasibleUnbounded()`, by which the answer `UNBOUNDED` implies a feasible
problem.

In step 3, the initial pivots are a heuristic of the original code. With a
single supply node and a single demand node, they are made on the arcs found
by a search backwards from the demand node over the arcs whose capacity is at
least the total supply; with more nodes, on the cheapest arc into each demand
node (or, under GEQ with less supply than demand, the cheapest arc out of each
supply node). Each of these arcs enters the basis if it is eligible. Also, the
constructor takes a flag `arc_mixing`, `true` by default, which stores the
arcs in an interleaved order (with a stride of $\max\{m/n,3\}$) rather than in
the order of the digraph. According to the header, this usually performs like
the original order, but it makes the method more robust and, in special cases,
significantly faster.

The header states no bound on the number of pivots, and the rules of the class
are not known to be polynomial. Each pivot costs the search of the rule, plus
time proportional to the length of the cycle and to the size of the smaller
part of the tree. In practice the method is usually the fastest one of the
library on graphs of up to some thousands of nodes and on dense graphs
(cf. section 13.9).

!!! note "abLEMON"
    The potentials are updated on the smaller side of the tree at each pivot
    (version 1.4.0). In floating point, the leaving arc goes to the bound it
    is nearer to, its flow being set to that bound, rather than to the upper
    bound for a flow of, e.g., $10^{-16}$ (version 1.9.0), which may change
    the sequence of pivots. A problem is answered `UNBOUNDED` only after a
    check of its feasibility (version 1.4.1).

### 13.4.2 Pivot rules {#sec-13-4-2}

The argument of `run()` is the rule that chooses the entering arc, i.e., a
constant of the nested enum `PivotRule`. The arcs are scanned in a wraparound
fashion, and an arc is eligible when its reduced cost, signed by its state, is
below $-\epsilon_c$. Let $m'$ be the number of arcs searched, i.e., those of
the digraph and, under GEQ or LEQ with unbalanced supplies, also the second
artificial arcs; the five rules are then as follows.

-   `FIRST_ELIGIBLE` takes the next eligible arc after the last one taken.
-   `BEST_ELIGIBLE` takes the most violating arc among all of them.
-   `BLOCK_SEARCH`, the default, examines blocks of
$\max\{\lfloor\sqrt{m'}\rfloor, 10\}$ arcs and takes the best eligible arc of
the first block that has one.
-   `CANDIDATE_LIST` builds, in a major iteration, a list of at most
$\max\{\lfloor 0.25\sqrt{m'}\rfloor, 10\}$ eligible arcs, and then takes the
best arc of the list in at most $\max\{\lfloor 0.1\,\ell\rfloor, 3\}$ minor
iterations, $\ell$ being the largest length of the list.
-   `ALTERING_LIST` keeps the $\max\{\lfloor 0.01\,b\rfloor, 3\}$ best
candidates of the previous iteration and extends them with a block of
$b=\max\{\lfloor\sqrt{m'}\rfloor, 10\}$ arcs at each iteration.

Each rule is a private class of `NetworkSimplex` (`FirstEligiblePivotRule`,
`BestEligiblePivotRule`, `BlockSearchPivotRule`, `CandidateListPivotRule` and
`AlteringListPivotRule`), which `run()` instantiates for the chosen rule, and
whose constants are those given above. According to the header, `BLOCK_SEARCH`
and `ALTERING_LIST` were the most efficient rules on the instances on which
they were tested, and `BLOCK_SEARCH`, being simpler and somewhat more robust,
was made the default.

## 13.5 The starting flow and the infinite bounds {#sec-13-5}

In contrast, the other three classes work on a residual network, in which each
arc of the digraph appears forward and backward. An extra root node is joined
to the nodes by a pair of arcs, which under GEQ take the unmet demand, with no
cost and a capacity of one more than the deficit of the supplies. Two
functions of `lemon/bits/mcf_feasible_flow.h` prepare that network.

The first one, `feasibleFlow()`, finds the flow that `CostScaling` and
`CycleCanceling` start from, i.e., a flow within the bounds that meets the
supplies. It runs the push-relabel algorithm of `Circulation` ([chapter
12](12-max-flow.md#ch-12)), with the active node of highest level first, all
the nodes at level zero at the start, a push towards any lower level, and the
first empty level as the proof that no such flow exists ([Goldberg and Tarjan
1988](Z-bibliography.md#ref-goldberg88newapproach); [Cherkassky and Goldberg
1997](Z-bibliography.md#ref-cherkassky97implementing)). Its starting point is
the greedy flow of `Circulation`, in which each arc takes the deficit of its
head within its bounds. When it fails, the nodes above the empty level,
together with the node being lifted, form a set that no residual arc leaves
and whose excess is positive, which proves the problem infeasible; [chapter
14](14-k-best-flows.md#ch-14) uses that set as a certificate. In floating
point, an excess or a residual capacity within $\epsilon_f$ counts as zero,
and at the end the residual capacities within it are moved to the reverse
arcs, lest the algorithms send round-off around.

The second function, `infiniteBound()`, gives the bound that replaces an
infinite upper bound in `CostScaling`, `CapacityScaling` and `CycleCanceling`.
Suppose that no cycle of negative cost is made of arcs of infinite upper
bound. Then some optimal flow sends on each arc at most

$$
B = 1 + \sum_{u\in V} |sup(u)| + \sum_{uv:\ upper(uv)<\infty} |upper(uv)| + \sum_{uv\in A} |lower(uv)|
\tag{13.6}
$$

beyond its lower bound. Indeed, a flow decomposes into paths, which carry at
most the supplies, and cycles, each of which contains an arc of finite bound
(the others having a nonnegative cost, and hence being of no use). An arc that
reaches $B$ then keeps a nonnegative reduced cost in an optimal solution;
hence, the potentials are optimal for the infinite bounds too. If necessary,
the bound is reduced until the sums of the flows over the replaced arcs fit in
`Value`. Whether such a cycle exists is decided beforehand by
`negativeInfiniteCycle()`, a Bellman-Ford algorithm restricted to the arcs of
infinite upper bound, which runs only if one of them has a negative cost; if
such a cycle exists, a feasible problem is answered `UNBOUNDED`.

## 13.6 Cost scaling {#sec-13-6}

`CostScaling` implements the cost scaling algorithm of Goldberg and Tarjan
([Goldberg and Tarjan 1990](Z-bibliography.md#ref-goldberg90approximation);
[Goldberg 1997](Z-bibliography.md#ref-goldberg97efficient); [Bünnagel, Korte
and Vygen 1998](Z-bibliography.md#ref-bunnagel98efficient)), a primal-dual
method that one may regard as the generalization to costs of the push-relabel
algorithm of `Preflow`. A flow $f$ and potentials $\pi$ are
*$\varepsilon$-optimal* when

$$
cost^\pi(uv) \geq -\varepsilon \qquad \text{for each arc } uv \text{ of the residual network of } f ,
\tag{13.7}
$$

and $\varepsilon=0$ gives back (13.4). With integer costs multiplied by $n+1$,
an $\varepsilon$-optimal flow with $\varepsilon<1$ is optimal for the original
costs. The algorithm starts from $\varepsilon$ equal to the largest scaled
cost divided by the scaling factor $\alpha$, i.e., by the argument `factor` of
`run()` (16 by default, and at least 2), and it divides $\varepsilon$ by
$\alpha$ after each phase. In a phase it *refines* the flow. First it
saturates the residual arcs of negative reduced cost, which makes the flow
$\varepsilon$-optimal but creates excesses and deficits; then it removes them
by pushes along the *admissible* arcs (those of negative reduced cost) and by
relabels, each of which lowers a potential by the least amount that makes an
arc admissible, plus $\varepsilon$.

<span id="alg-13-2"></span>**Algorithm 13.2** (Cost scaling).

```
1  find a feasible flow f (section 13.5); if none: return INFEASIBLE
2  if a cycle of infinite capacity has negative cost: return UNBOUNDED
3  scale the costs by (n+1) alpha; eps := largest scaled cost / alpha;
   pi := 0
4  while eps >= eps_min:
5      price refinement (heuristic); if it proves f eps-optimal: skip to 9
6      saturate the residual arcs of negative reduced cost
7      while some node u has positive excess (FIFO order):
           push or augment along admissible arcs, or relabel u;
           every few relabels, make a global update of the potentials
8      (the flow is now eps-optimal)
9      eps := eps / alpha, or eps_min if eps / alpha would fall below it
10 divide the potentials by (n+1) alpha; if some residual arc has a
   reduced cost below -eps_c, correct them by Bellman-Ford
11 shift the potentials to at most zero; return OPTIMAL
```

The nested enum `Method` selects the operation of step 7. With `PUSH` the flow
moves on one admissible arc at a time, with a look-ahead that avoids pushing
into a node that would only send the flow back. With `AUGMENT` it moves along
admissible paths from a node with excess to a node with deficit. With
`PARTIAL_AUGMENT`, the default and, according to the header, the most
efficient and robust one, it moves along admissible paths of at most 4 arcs.

We remark that two heuristics of [Goldberg
1997](Z-bibliography.md#ref-goldberg97efficient) are applied. The price
refinement of step 5, tried at each phase from the second one on, computes
potentials that may prove the current flow $\varepsilon$-optimal before any
push. The global update of step 7 recomputes the potentials by a bucket-based
search from the deficits, after a number of relabels proportional to $n+s^2$,
$s$ being the number of supply nodes.

For the internal computations the class uses the type `LargeCost` of the
traits class `CostScalingDefaultTraits< GR , V , C >`, which is `long long`
for an integer `Cost` (or `long` without `LEMON_HAVE_LONG_LONG`) and `double`
otherwise; the named parameter `SetLargeCost< T >` changes it, provided that
`Cost` is convertible to `T`. The last value $\varepsilon_{min}$ of the
scaling is 1 for an integer `LargeCost`, which proves the flow optimal, and
otherwise $\epsilon_c\,(n+1)\,\alpha$, i.e., the tolerance on the reduced
costs in the scaled units. As stated in the header, the worst case running
time is $O(n^2m\log(nK))$, where $K$ is the largest arc cost. In practice the
method is typically the most efficient one of the library on large sparse
graphs (cf. section 13.9).

!!! note "abLEMON"
    `CostScaling` finds its starting flow on its residual network (version
    1.4.0), where LEMON 1.3.1 ran `Circulation` on the digraph, and it takes
    floating-point costs, capacities and supplies (version 1.6.0). The price
    refinement of LEMON 1.3.1 could write past the end of its buckets, which
    is fixed.

## 13.7 Capacity scaling {#sec-13-7}

`CapacityScaling` implements the capacity scaling version of the successive
shortest path algorithm ([Edmonds and Karp
1972](Z-bibliography.md#ref-edmondskarp72theoretical); [Ahuja, Magnanti and
Orlin 1993, chapter 10](Z-bibliography.md#ref-amo93networkflows)). It is a
dual method, which keeps potentials under which the residual arcs of capacity
at least $\Delta$ have nonnegative reduced costs, and which sends flow from
the nodes with excess to the nodes with deficit along shortest paths. In a
phase with a given $\Delta$, the arcs of negative reduced cost and residual
capacity at least $\Delta$ are saturated first. Then, while some node has an
excess and some node a deficit of at least $\Delta$, the algorithm of Dijkstra
on the reduced costs, over the residual arcs of capacity at least $\Delta$,
finds a shortest path from a node with excess to a node with deficit. The flow
is augmented along it by at least $\Delta$, and the potentials of the nodes
that the search has settled are updated by their distances. Afterwards
$\Delta$ is divided by the scaling factor (the argument `factor` of `run()`, 4
by default), down to the smallest unit.

<span id="alg-13-3"></span>**Algorithm 13.3** (Capacity scaling).

```
1  saturate the arcs of negative cost (an infinite bound replaced by B,
   section 13.5); Delta := largest power of 2 (times the unit) not above
   the smallest of the largest excess, deficit and residual capacity
2  loop:
3      saturate the residual arcs with r >= Delta and reduced cost < -eps_c
4      while a node s has e(s) >= Delta and some node has e <= -Delta:
5          Dijkstra from s on the arcs with r >= Delta, until a node t
           with e(t) <= -Delta is settled
6          if none is reached: (at the last Delta) return INFEASIBLE
7          augment along the path by min{ e(s), -e(t), its capacities }
8          update the potentials of the settled nodes
9      if Delta = unit: return OPTIMAL (or UNBOUNDED, section 13.5)
10     Delta := max{ unit, Delta / factor }
```

Here the unit is 1 for an exact `Value` and $\epsilon_f$ otherwise; hence, in
floating point the last phase sends any amount above the tolerance. With a
factor of at most 1 the scaling is disabled, and the method becomes the plain
successive shortest path algorithm, which is usually the fastest choice when
the supplies and the capacities are small. In the searches the class uses the
heap type `Heap` of the traits class
`CapacityScalingDefaultTraits< GR , V , C >`, by default
`QuadHeap< Cost , RangeMap< int > >`, whose fewer levels make the searches
cheaper than on a binary heap ([Johnson
1975](Z-bibliography.md#ref-johnson75heap)). The named parameter
`SetHeap< T >` replaces it by any heap that meets the concept of [chapter
7](07-heaps.md#ch-7) with the priority type `Cost` and the cross reference map
`RangeMap< int >`. As stated in the header, the worst case running time is
$O(m\log U\,(n+m)\log n)$, where $U$ is the largest supply or capacity.

!!! note "abLEMON"
    The default heap is a 4-ary heap (version 1.4.0), the class compiles on
    value types narrower than `int`, and it takes floating-point data (version
    1.6.0).

## 13.8 Cycle canceling {#sec-13-8}

`CycleCanceling` implements three cycle canceling methods ([Klein
1967](Z-bibliography.md#ref-klein67primal); [Goldberg and Tarjan
1989](Z-bibliography.md#ref-goldberg89cyclecanceling)), which start from the
feasible flow of section 13.5 and cancel cycles of negative cost of the
residual network until none is left, i.e., until the flow is optimal by
Theorem 13.2. The nested enum `Method` selects one of them.

-   `SIMPLE_CYCLE_CANCELING` finds negative cycles by the algorithm of Bellman
and Ford on the residual network, with a limit on its rounds that starts at 2
and grows by a factor of 1.5 when no cycle is found, and cancels node-disjoint
cycles of the predecessor graph.
-   `MINIMUM_MEAN_CYCLE_CANCELING` cancels a cycle of minimum mean cost at
   each
step, found by Howard's algorithm with a limit of $\max\{n,5\}$ iterations
and, if the limit is reached, by the strongly polynomial algorithm of Hartmann
and Orlin ([chapter 16](16-min-mean-cycle.md#ch-16)); the header gives the
bound $O(n^2m^3\log n)$.
-   `CANCEL_AND_TIGHTEN`, the default, which the header calls by far the most
efficient and robust, is the improved version of the previous method of
[Goldberg and Tarjan 1989](Z-bibliography.md#ref-goldberg89cyclecanceling),
with the bound $O(n^2m^2\log n)$.

In Cancel-and-Tighten an arc is admissible when its reduced cost is negative.
A phase cancels, by a depth first search, the cycles of admissible arcs, until
the admissible network is acyclic; then it *tightens* the potentials along a
topological order of that network, and it recomputes $\varepsilon$ as the
largest violation of (13.4) by a residual arc. Every
$\max\{\lfloor\sqrt{n+1}\rfloor,5\}$ phases, instead, $\varepsilon$ is set to
the opposite of the minimum cycle mean of the residual network, and the
potentials are recomputed from it by the algorithm of Bellman and Ford.

<span id="alg-13-4"></span>**Algorithm 13.4** (Cancel-and-Tighten).

```
1  find a feasible flow f (section 13.5); if none: return INFEASIBLE
2  if a cycle of infinite capacity has negative cost: return UNBOUNDED
3  pi := 0; eps := largest -cost(uv) over the residual arcs
4  while eps * (n+1) >= 1 (exact costs), or eps > eps_c (floating point):
5      cancel the cycles of admissible arcs (cost^pi < 0) by DFS
6      if this is not the k-th phase:
7          tighten pi along a topological order of the admissible arcs;
           eps := largest -cost^pi(uv) over the residual arcs
8      else:
9          eps := - (minimum cycle mean of the residual network);
           pi := potentials for eps, by Bellman-Ford
10 compute the final potentials by Bellman-Ford; return OPTIMAL
```

During the phases the potentials are `double`, fractional even with an exact
`Cost`, and the test of admissibility uses a tolerance of $10^{-6}$ then, as
in LEMON 1.3.1, and $\epsilon_c$ with a floating-point `Cost`. After the last
phase of the two methods based on mean cycles, the potentials are computed by
the algorithm of Bellman and Ford on the residual network (in
`SIMPLE_CYCLE_CANCELING` its distances already give them). Its operation
traits count an improvement only beyond $\epsilon_c$, since in floating point
the round-off of the sums can make a cycle of cost zero (e.g., an arc and its
reverse) look negative for ever. The class has no traits parameter, and the
header warns that, in practice, the method is typically orders of magnitude
slower than the scaling algorithms and `NetworkSimplex`.

!!! note "abLEMON"
    `CycleCanceling` finds its starting flow on its residual network (version
    1.4.0), takes floating-point data (version 1.6.0), and compares with zero
    only through the tolerances, its inner minimum mean cycle searches
    included (version 1.9.0).

## 13.9 Choosing an algorithm {#sec-13-9}

In the documentation of the module, which follows the experimental study of
[Király and Kovács 2012](Z-bibliography.md#ref-KiralyKovacs12MCF),
`NetworkSimplex` and `CostScaling` are recommended as the most efficient
classes in general. `NetworkSimplex` is usually the fastest on relatively
small graphs (up to several thousands of nodes) and on dense graphs, while
`CostScaling` is typically more efficient on large graphs (e.g., hundreds of
thousands of nodes or more), especially sparse ones. Other classes may be
faster in special cases; e.g., when the total supply or the capacities are
rather small, `CapacityScaling` without scaling is usually the fastest. In our
view, `CycleCanceling` is mostly of interest for its strongly polynomial bound
and for teaching. We remark that these are statements about typical instances,
and that the ranking on a given family of problems is best found by running
the four classes on it, which their common interface makes easy (cf. the
program of section 13.2). Also, note that only `NetworkSimplex` takes the LEQ
form directly.

## 13.10 Exercises {#sec-13-10}

<span id="ex-13-1"></span>**Exercise 13.1.** Show that a flow of (13.1) with a
cycle of negative cost in its residual network is not optimal, and derive the
potentials of Theorem 13.2 from shortest path distances. Then check (13.4) on
the four answers of `manual/examples/13-min-cost-flow.cc`, which does it in
its function `optimal()`.

<span id="ex-13-2"></span>**Exercise 13.2.** Transform a problem in the LEQ
form (13.2) into the GEQ form with `ReverseDigraph` and `NegMap`, solve it
with `CostScaling`, and compare the cost with that of `NetworkSimplex` with
`supplyType(LEQ)`. (The supply types are checked by `checkSupplyTypes()` in
`test/min_cost_flow_test.cc`.)

<span id="ex-13-3"></span>**Exercise 13.3.** Solve the problem of
`manual/examples/13-floating-point.cc` with `tolerances(0, 0)` on supplies
that are exact in binary (e.g., $0.25$, $0.5$ and $-0.75$), and then on $0.1$,
$0.2$ and $-0.3$ with the default tolerances; explain the difference by
(13.5). (The tolerances are checked by `checkTolerances()` and
`checkToleranceGetters()` in `test/min_cost_flow_test.cc`.)

<span id="ex-13-4"></span>**Exercise 13.4.** Time the five pivot rules of
`NetworkSimplex` on random instances of a few thousand nodes, on an idle
machine and with repeated runs, and compare the default with `BEST_ELIGIBLE`;
repeat with `arc_mixing` set to `false`.

<span id="ex-13-5"></span>**Exercise 13.5.** Run `CostScaling` with the three
methods and with the factors 2, 16 and 64, and `CapacityScaling` with the
factors 1 and 4, on the same instances, and check that the costs agree. (The
battery `test/min_cost_flow_battery_test.cc` runs every algorithm and method
on limit cases and random instances, with integer and floating-point data.)

<span id="ex-13-6"></span>**Exercise 13.6.** Compute the bound $B$ of (13.6)
for the problem of `manual/examples/13-min-cost-flow.cc` with all the upper
bounds infinite, and verify that an optimal flow of that problem sends less
than $B$ on each arc.
