// Copies and arc sets (sections 4.7 and 4.9): a ListGraph copied into a
// SmartGraph with the references of its nodes, the cross references of its
// edges, an edge map and the image of one arc; then a ListEdgeSet and a
// SmartArcSet on the nodes of a ListDigraph, and what an erased node does
// to them.

#include <iostream>
#include <lemon/edge_set.h>
#include <lemon/list_graph.h>
#include <lemon/smart_graph.h>

using namespace lemon;

int main() {
  int errors = 0;

  ListGraph lg;
  ListGraph::Node a = lg.addNode(), b = lg.addNode(), c = lg.addNode();
  ListGraph::Edge ab = lg.addEdge(a, b), bc = lg.addEdge(b, c);
  lg.addEdge(c, c);  // a loop
  ListGraph::EdgeMap<double> w(lg);
  w[ab] = 1.5;
  w[bc] = 2.5;

  SmartGraph sg;
  sg.addNode();  // the copy clears the target first
  ListGraph::NodeMap<SmartGraph::Node> nref(lg);
  SmartGraph::EdgeMap<ListGraph::Edge> ecross(sg);
  SmartGraph::EdgeMap<double> sw(sg);
  ListGraph::Arc ba = lg.direct(ab, false);
  SmartGraph::Arc sba;
  graphCopy(lg, sg)
      .nodeRef(nref)
      .edgeCrossRef(ecross)
      .edgeMap(w, sw)
      .arc(ba, sba)
      .run();
  if (countNodes(sg) != 3 || countEdges(sg) != 3) ++errors;
  for (SmartGraph::EdgeIt e(sg); e != INVALID; ++e)
    if (sw[e] != w[ecross[e]]) ++errors;
  // the image of an arc has the images of its end nodes
  if (sg.source(sba) != nref[b] || sg.target(sba) != nref[a]) ++errors;

  // an edge set and an arc set sharing the nodes of a ListDigraph
  ListDigraph base;
  ListDigraph::Node x = base.addNode(), y = base.addNode(),
                    z = base.addNode();
  ListEdgeSet<ListDigraph> es(base);
  ListEdgeSet<ListDigraph>::Edge xy = es.addEdge(x, y);
  es.addEdge(y, z);
  es.addEdge(z, x);
  SmartArcSet<ListDigraph> as(base);
  as.addArc(x, y);
  as.addArc(x, z);
  if (countEdges(es) != 3 || countArcs(es) != 6 || countArcs(as) != 2)
    ++errors;
  es.erase(xy);
  if (countEdges(es) != 2) ++errors;

  // erasing z from the base graph erases its edges from the ListEdgeSet,
  // but it invalidates the SmartArcSet, which has an arc at z
  base.erase(z);
  if (countEdges(es) != 0 || countNodes(es) != 2) ++errors;
  if (as.valid()) ++errors;

  std::cout << "copies and arc sets: " << errors << " errors" << std::endl;
  return errors == 0 ? 0 : 1;
}
