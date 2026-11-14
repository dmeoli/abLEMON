// The (sub)graph isomorphism of section 18.7: the automorphisms of the
// cycle on five nodes, the embeddings of a path of three nodes into the
// cycle on four nodes and into K4 (as a subgraph and as an induced
// subgraph), the same counts with node labels, by Vf2 and by Vf2pp, and the
// enumeration of the mappings one by one with find().

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/vf2.h>
#include <lemon/vf2pp.h>

using namespace lemon;
typedef ListGraph Graph;

void cycle(Graph &g, int n) {
  std::vector<Graph::Node> v;
  for (int i = 0; i < n; ++i) v.push_back(g.addNode());
  for (int i = 0; i < n; ++i) g.addEdge(v[i], v[(i + 1) % n]);
}

int main() {
  int errors = 0;

  Graph c5, c4, k4, p3;
  cycle(c5, 5);
  cycle(c4, 4);
  for (int i = 0; i < 4; ++i) k4.addNode();
  for (Graph::NodeIt a(k4); a != INVALID; ++a)
    for (Graph::NodeIt b(k4); b != INVALID; ++b)
      if (a < b) k4.addEdge(a, b);
  Graph::Node x = p3.addNode(), y = p3.addNode(), z = p3.addNode();
  p3.addEdge(x, y);
  p3.addEdge(y, z);

  // The automorphisms of C5 are its 10 rotations and reflections
  int a1 = vf2(c5, c5).iso().count(), a2 = vf2pp(c5, c5).iso().count();
  std::cout << "automorphisms of C5: " << a1 << ' ' << a2 << std::endl;
  if (a1 != 10 || a2 != 10) ++errors;

  // A path of three nodes into C4 and into K4
  int sc4 = vf2(p3, c4).count(), ic4 = vf2(p3, c4).induced().count();
  int sk4 = vf2pp(p3, k4).count(), ik4 = vf2pp(p3, k4).induced().count();
  std::cout << "P3 into C4: " << sc4 << " subgraph, " << ic4
            << " induced; into K4: " << sk4 << " subgraph, " << ik4
            << " induced" << std::endl;
  if (sc4 != 8 || ic4 != 8 || sk4 != 24 || ik4 != 0) ++errors;

  // Labels: the middle node of the path must go to node 0 of K4
  Graph::NodeMap<int> lp(p3, 0), lk(k4, 0);
  lp[y] = 1;
  lk[Graph::NodeIt(k4)] = 1;
  int l1 = vf2(p3, k4).nodeLabels(lp, lk).count();
  int l2 = vf2pp(p3, k4).nodeLabels(lp, lk).count();
  std::cout << "with labels: " << l1 << ' ' << l2 << std::endl;
  if (l1 != 6 || l2 != 6) ++errors;

  // All the mappings one by one, each checked to be an embedding
  Graph::NodeMap<Graph::Node> m(p3);
  Vf2<Graph, Graph> alg(p3, c4, m);
  int found = 0;
  while (alg.find()) {
    ++found;
    for (Graph::EdgeIt e(p3); e != INVALID; ++e)
      if (findEdge(c4, m[p3.u(e)], m[p3.v(e)]) == INVALID) ++errors;
    if (m[x] == m[y] || m[y] == m[z] || m[x] == m[z]) ++errors;
  }
  if (found != 8) ++errors;

  // The same through the wizard, which allocates the object
  Vf2pp<Graph, Graph> *p = vf2pp(p3, c4).mapping(m).getPtrToVf2ppObject();
  found = 0;
  while (p->find()) ++found;
  delete p;
  if (found != 8) ++errors;

  return errors == 0 ? 0 : 1;
}
