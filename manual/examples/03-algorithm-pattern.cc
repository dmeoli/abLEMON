// The pattern of the algorithm classes (section 3.6), on Dijkstra: a named
// template parameter that replaces a map of the traits, run() split into
// init(), addSource() and start(), the queries, and the function-type
// interface of the same algorithm.

#include <iostream>
#include <lemon/dijkstra.h>
#include <lemon/smart_graph.h>

using namespace lemon;

int main() {
  int errors = 0;
  SmartDigraph g;
  SmartDigraph::Node s = g.addNode(), a = g.addNode(), b = g.addNode(),
                     t = g.addNode(), u = g.addNode();
  SmartDigraph::ArcMap<double> len(g);
  len[g.addArc(s, a)] = 2.0;
  len[g.addArc(s, b)] = 5.0;
  len[g.addArc(a, b)] = 1.0;
  len[g.addArc(a, t)] = 4.0;
  len[g.addArc(b, t)] = 2.0;
  len[g.addArc(u, s)] = 1.0;  // u is not reachable from s

  // the distances go into a map of the caller, whose type is set by the
  // named template parameter SetDistMap
  typedef SmartDigraph::NodeMap<double> DistMap;
  typedef Dijkstra<SmartDigraph, SmartDigraph::ArcMap<double> >::
      SetDistMap<DistMap>::Create Dij;
  DistMap dist(g);
  Dij dij(g, len);
  dij.distMap(dist);

  // run(s) is init(), addSource(s) and start(); start(t) stops at t
  dij.init();
  dij.addSource(s);
  dij.start(t);
  if (!dij.processed(t) || dist[t] != 5.0) ++errors;
  if (dij.predNode(t) != b || dij.predNode(b) != a) ++errors;
  if (dij.reached(u)) ++errors;

  // two sources at different initial distances
  dij.init();
  dij.addSource(s, 0.0);
  dij.addSource(b, -1.0);
  dij.start();
  if (dij.dist(t) != 1.0 || dij.dist(a) != 2.0) ++errors;

  // the function-type interface: the optional outputs are set by name
  DistMap d2(g);
  bool found = dijkstra(g, len).distMap(d2).run(s, t);
  if (!found || d2[t] != 5.0) ++errors;

  std::cout << "algorithm pattern: " << errors << " errors" << std::endl;
  return errors == 0 ? 0 : 1;
}
