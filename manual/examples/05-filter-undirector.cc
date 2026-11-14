// Graph adaptors (section 5.2 and 5.3): the arcs of cost at most a
// threshold kept by FilterArcs, the result seen as an undirected graph by
// Undirector, and the nodes that reach a target found by a search on
// ReverseDigraph; the original digraph is never copied nor changed.

#include <iostream>
#include <lemon/adaptors.h>
#include <lemon/bfs.h>
#include <lemon/connectivity.h>
#include <lemon/list_graph.h>

using namespace lemon;

int main() {
  ListDigraph g;
  ListDigraph::Node v[6];
  for (int i = 0; i < 6; ++i) v[i] = g.addNode();
  ListDigraph::ArcMap<int> cost(g);
  // two triangles 0-1-2 and 3-4-5 of cheap arcs, joined by an expensive one
  const int arcs[7][3] = {{0, 1, 1}, {1, 2, 2}, {2, 0, 1}, {2, 3, 9},
                          {3, 4, 1}, {4, 5, 2}, {5, 3, 1}};
  for (int k = 0; k < 7; ++k)
    cost[g.addArc(v[arcs[k][0]], v[arcs[k][1]])] = arcs[k][2];

  // the arc filter: true on the arcs of cost at most 5
  ListDigraph::ArcMap<bool> cheap(g);
  for (ListDigraph::ArcIt a(g); a != INVALID; ++a) cheap[a] = cost[a] <= 5;

  typedef FilterArcs<ListDigraph> Cheap;
  Cheap sub(g, cheap);
  typedef Undirector<const Cheap> UCheap;
  UCheap usub(sub);

  int comps = countConnectedComponents(usub);   // 2: the two triangles
  int all = countConnectedComponents(undirector(g));   // 1
  std::cout << "components with the cheap arcs: " << comps
            << ", with all the arcs: " << all << std::endl;

  // enabling the expensive arc again joins the two components
  sub.enable(findArc(g, v[2], v[3]));
  int joined = countConnectedComponents(usub);

  // the nodes that reach v[0] are those that v[0] reaches backwards
  sub.disable(findArc(g, v[2], v[3]));
  ReverseDigraph<const ListDigraph> rg(g);
  Bfs<ReverseDigraph<const ListDigraph> > bfs(rg);
  bfs.run(v[0]);
  int reach = 0;
  for (ListDigraph::NodeIt n(g); n != INVALID; ++n)
    if (bfs.reached(n)) ++reach;
  std::cout << "nodes that reach node 0: " << reach << std::endl;

  return (comps == 2 && all == 1 && joined == 1 && reach == 3) ? 0 : 1;
}
