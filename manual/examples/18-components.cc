// The components of section 18.1 to 18.3: an undirected graph of four
// blocks (two triangles sharing node 2, the bridge 4-5 and a triangle), its
// connected, bi-node-connected and bi-edge-connected components with their
// cut nodes and cut edges, and a digraph with its strongly connected
// components and the topological order of a DAG.

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/connectivity.h>

using namespace lemon;

int main() {
  int errors = 0;

  // The graph of Figure 18.1
  ListGraph g;
  ListGraph::Node v[8];
  for (int i = 0; i < 8; ++i) v[i] = g.addNode();
  const int ends[10][2] = {{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4},
                           {4, 2}, {4, 5}, {5, 6}, {6, 7}, {7, 5}};
  ListGraph::Edge bridge;
  for (int i = 0; i < 10; ++i) {
    ListGraph::Edge e = g.addEdge(v[ends[i][0]], v[ends[i][1]]);
    if (i == 6) bridge = e;
  }

  ListGraph::NodeMap<int> comp(g);
  int cc = connectedComponents(g, comp);
  ListGraph::EdgeMap<int> block(g);
  int bn = biNodeConnectedComponents(g, block);
  ListGraph::NodeMap<bool> cutNode(g, false);  // must start false
  int cn = biNodeConnectedCutNodes(g, cutNode);
  ListGraph::NodeMap<int> twoEdge(g);
  int be = biEdgeConnectedComponents(g, twoEdge);
  ListGraph::EdgeMap<bool> cutEdge(g, false);  // must start false
  int ce = biEdgeConnectedCutEdges(g, cutEdge);

  std::cout << "connected components: " << cc << ", blocks: " << bn
            << ", cut nodes: " << cn << ", 2-edge-connected components: "
            << be << ", bridges: " << ce << std::endl;
  if (!connected(g) || cc != 1) ++errors;
  if (bn != 4 || countBiNodeConnectedComponents(g) != 4) ++errors;
  if (cn != 3 || !cutNode[v[2]] || !cutNode[v[4]] || !cutNode[v[5]])
    ++errors;
  if (be != 2 || twoEdge[v[0]] != twoEdge[v[3]] ||
      twoEdge[v[4]] == twoEdge[v[5]])
    ++errors;
  if (ce != 1 || !cutEdge[bridge]) ++errors;
  if (biNodeConnected(g) || biEdgeConnected(g)) ++errors;
  if (acyclic(g) || tree(g) || bipartite(g) || !simpleGraph(g)) ++errors;

  // An even cycle is bipartite, and a path is a tree
  ListGraph h;
  ListGraph::Node w[4];
  for (int i = 0; i < 4; ++i) w[i] = h.addNode();
  for (int i = 0; i < 3; ++i) h.addEdge(w[i], w[i + 1]);
  if (!tree(h) || !acyclic(h)) ++errors;
  h.addEdge(w[3], w[0]);
  ListGraph::NodeMap<bool> side(h);
  if (!bipartitePartitions(h, side) || side[w[0]] == side[w[1]] ||
      side[w[0]] != side[w[2]] || tree(h) || !biNodeConnected(h))
    ++errors;

  // A digraph with three strongly connected components
  ListDigraph d;
  ListDigraph::Node u[6];
  for (int i = 0; i < 6; ++i) u[i] = d.addNode();
  const int arcs[8][2] = {{0, 1}, {1, 2}, {2, 0}, {2, 3},
                          {3, 4}, {4, 3}, {4, 5}, {1, 0}};
  for (int i = 0; i < 8; ++i) d.addArc(u[arcs[i][0]], u[arcs[i][1]]);
  ListDigraph::NodeMap<int> scc(d);
  int sn = stronglyConnectedComponents(d, scc);
  ListDigraph::ArcMap<bool> cutArc(d, false);
  int an = stronglyConnectedCutArcs(d, cutArc);
  std::cout << "strongly connected components: " << sn
            << ", arcs between them: " << an << std::endl;
  if (sn != 3 || an != 2 || stronglyConnected(d) || dag(d)) ++errors;
  // the numbering of the components is a topological order of them
  for (ListDigraph::ArcIt a(d); a != INVALID; ++a)
    if (scc[d.source(a)] > scc[d.target(a)]) ++errors;

  // The topological order of a DAG
  ListDigraph t;
  ListDigraph::Node x[4];
  for (int i = 0; i < 4; ++i) x[i] = t.addNode();
  t.addArc(x[3], x[1]);
  t.addArc(x[3], x[2]);
  t.addArc(x[1], x[0]);
  t.addArc(x[2], x[0]);
  ListDigraph::NodeMap<int> order(t);
  if (!checkedTopologicalSort(t, order)) ++errors;
  for (ListDigraph::ArcIt a(t); a != INVALID; ++a)
    if (order[t.source(a)] >= order[t.target(a)]) ++errors;
  t.addArc(x[0], x[3]);  // closes a directed cycle
  if (dag(t) || checkedTopologicalSort(t, order)) ++errors;

  return errors == 0 ? 0 : 1;
}
