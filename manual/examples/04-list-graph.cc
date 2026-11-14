// ListDigraph (section 4.2): the modifications that only the list-based
// graphs offer (erase, changeTarget, reverseArc, contract, split), and a
// Snapshot that removes what was added after it.

#include <iostream>
#include <lemon/list_graph.h>

using namespace lemon;

int main() {
  int errors = 0;
  ListDigraph g;
  ListDigraph::Node a = g.addNode(), b = g.addNode(), c = g.addNode(),
                    d = g.addNode();
  ListDigraph::Arc ab = g.addArc(a, b), bc = g.addArc(b, c),
                   cd = g.addArc(c, d), da = g.addArc(d, a),
                   ac = g.addArc(a, c);
  ListDigraph::ArcMap<int> cost(g, 1);
  cost[ac] = 7;

  // reverseArc() and changeTarget() keep the arc, with its id and its value
  g.reverseArc(ac);  // now c -> a
  if (g.source(ac) != c || g.target(ac) != a || cost[ac] != 7) ++errors;
  g.changeTarget(ac, b);  // now c -> b
  if (countInArcs(g, b) != 2) ++errors;

  // contract(a, b): b is erased, its arcs are joined to a, and the loop
  // a -> a made of ab is removed (r = true by default)
  g.contract(a, b);
  if (g.valid(b) || g.valid(ab)) ++errors;
  if (g.source(bc) != a || g.target(ac) != a) ++errors;
  if (countNodes(g) != 3 || countArcs(g) != 4) ++errors;

  // split(c): a new node takes the arcs out of c, joined to c by a new arc
  ListDigraph::Node c2 = g.split(c);
  if (g.source(cd) != c2 || g.source(ac) != c2) ++errors;
  if (countOutArcs(g, c) != 1 || findArc(g, c, c2) == INVALID) ++errors;

  // a snapshot: what is added after save() goes away at restore()
  ListDigraph::Snapshot snap(g);
  ListDigraph::Node e = g.addNode();
  g.addArc(e, a);
  g.addArc(d, e);
  if (countNodes(g) != 5 || countArcs(g) != 7) ++errors;
  snap.restore();
  if (countNodes(g) != 4 || countArcs(g) != 5 || g.valid(e)) ++errors;
  if (snap.valid()) ++errors;  // restore() invalidates the snapshot

  // erasing a node erases its arcs
  g.erase(d);
  if (g.valid(cd) || g.valid(da) || countArcs(g) != 3) ++errors;

  std::cout << "ListDigraph: " << errors << " errors" << std::endl;
  return errors == 0 ? 0 : 1;
}
