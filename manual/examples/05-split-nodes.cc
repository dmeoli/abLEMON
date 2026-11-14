// Splitting the nodes (section 5.5): the largest number of s-t paths that
// share no inner node, as a maximum flow on SplitNodes with capacity 1 on
// the bind arcs of the inner nodes (Menger's theorem).

#include <iostream>
#include <lemon/adaptors.h>
#include <lemon/list_graph.h>
#include <lemon/preflow.h>

using namespace lemon;

int main() {
  ListDigraph g;
  ListDigraph::Node s = g.addNode(), a = g.addNode(), b = g.addNode(),
                    c = g.addNode(), d = g.addNode(), t = g.addNode();
  // every s-t path goes through a, b or c, and those through b and c
  // also go through d
  g.addArc(s, a); g.addArc(s, b); g.addArc(s, c);
  g.addArc(a, t); g.addArc(b, d); g.addArc(c, d); g.addArc(d, t);
  g.addArc(a, d);

  ListDigraph::ArcMap<int> arc_cap(g, 2);    // arcs: no limit that binds
  ListDigraph::NodeMap<int> node_cap(g, 1);  // inner nodes: capacity 1
  node_cap[s] = node_cap[t] = 2;

  typedef SplitNodes<ListDigraph> Split;
  Split split(g);
  typedef Split::CombinedArcMap<ListDigraph::ArcMap<int>,
                                ListDigraph::NodeMap<int> > CapMap;
  CapMap cap(arc_cap, node_cap);

  // the paths leave from the out-node of s and end in the in-node of t
  Preflow<Split, CapMap> preflow(split, cap, split.outNode(s),
                                 split.inNode(t));
  preflow.run();

  int nodes = countNodes(split), arcs = countArcs(split);
  std::cout << "node-disjoint paths: " << preflow.flowValue() << " (split "
            << "digraph of " << nodes << " nodes and " << arcs << " arcs)"
            << std::endl;
  return (preflow.flowValue() == 2 && nodes == 12 && arcs == 14) ? 0 : 1;
}
