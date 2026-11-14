// The Euler tours of section 18.4: the tour of the complete graph on five
// nodes by EulerIt and that of a digraph made of two directed cycles by
// DiEulerIt, each checked to be a closed walk through every edge or arc
// once, and the test eulerian() on graphs with and without an isolated node.

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/full_graph.h>
#include <lemon/euler.h>

using namespace lemon;

int main() {
  int errors = 0;

  // K5: every degree is 4, hence an Euler tour exists
  FullGraph k5(5);
  FullGraph::EdgeMap<int> used(k5, 0);
  FullGraph::Node first = INVALID, last = INVALID;
  int length = 0;
  for (EulerIt<FullGraph> e(k5); e != INVALID; ++e) {
    FullGraph::Arc a = e;  // an arc gives the direction of the walk
    if (first == INVALID) first = k5.source(a);
    if (last != INVALID && k5.source(a) != last) ++errors;  // not a walk
    last = k5.target(a);
    ++used[FullGraph::Edge(e)];
    ++length;
  }
  std::cout << "Euler tour of K5: " << length << " edges" << std::endl;
  if (length != 10 || first != last || !eulerian(k5)) ++errors;
  for (FullGraph::EdgeIt e(k5); e != INVALID; ++e)
    if (used[e] != 1) ++errors;

  // Two directed triangles sharing node 0, started at node 1
  ListDigraph d;
  ListDigraph::Node n[5];
  for (int i = 0; i < 5; ++i) n[i] = d.addNode();
  d.addArc(n[0], n[1]);
  d.addArc(n[1], n[2]);
  d.addArc(n[2], n[0]);
  d.addArc(n[0], n[3]);
  d.addArc(n[3], n[4]);
  d.addArc(n[4], n[0]);
  ListDigraph::ArcMap<int> seen(d, 0);
  std::cout << "Euler tour of the digraph:";
  ListDigraph::Node at = n[1];
  for (DiEulerIt<ListDigraph> e(d, n[1]); e != INVALID; ++e) {
    if (d.source(e) != at) ++errors;
    at = d.target(e);
    ++seen[e];
    std::cout << ' ' << d.id(d.source(e)) << "->" << d.id(d.target(e));
  }
  std::cout << std::endl;
  if (at != n[1] || !eulerian(d)) ++errors;
  for (ListDigraph::ArcIt a(d); a != INVALID; ++a)
    if (seen[a] != 1) ++errors;

  // An isolated node makes the digraph not Eulerian (it is not connected),
  // although the tour is still there
  d.addNode();
  if (eulerian(d)) ++errors;
  int arcs = 0;
  for (DiEulerIt<ListDigraph> e(d); e != INVALID; ++e) ++arcs;
  if (arcs != 6) ++errors;

  // A path of two edges has two nodes of odd degree
  ListGraph p;
  ListGraph::Node a = p.addNode(), b = p.addNode(), c = p.addNode();
  p.addEdge(a, b);
  p.addEdge(b, c);
  if (eulerian(p)) ++errors;

  return errors == 0 ? 0 : 1;
}
