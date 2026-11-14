// The static and the implicit graphs (sections 4.4 to 4.6): StaticDigraph
// and CompactDigraph built from a list of arcs sorted by source, StaticGraph
// from a list of edges in any order, a ListDigraph copied into a
// StaticDigraph with one of its maps, and the full, grid and hypercube
// graphs with the counts and the indices they compute.

#include <iostream>
#include <utility>
#include <vector>
#include <lemon/compact_graph.h>
#include <lemon/full_graph.h>
#include <lemon/grid_graph.h>
#include <lemon/hypercube_graph.h>
#include <lemon/list_graph.h>
#include <lemon/static_graph.h>

using namespace lemon;

int main() {
  int errors = 0;

  // arcs sorted by source: arc k joins the k-th pair
  std::vector<std::pair<int, int> > arcs;
  arcs.push_back(std::make_pair(0, 1));
  arcs.push_back(std::make_pair(0, 2));
  arcs.push_back(std::make_pair(1, 3));
  arcs.push_back(std::make_pair(1, 2));
  arcs.push_back(std::make_pair(3, 0));
  StaticDigraph sd;
  sd.build(4, arcs.begin(), arcs.end());
  if (sd.nodeNum() != 4 || sd.arcNum() != 5) ++errors;
  if (sd.source(sd.arc(3)) != sd.node(1) || sd.target(sd.arc(3)) != sd.node(2))
    ++errors;
  CompactDigraph cd;
  cd.build(4, arcs.begin(), arcs.end());
  if (countInArcs(cd, cd.node(2)) != 2) ++errors;  // a scan of the arcs

  // edges in any order: edge k joins the k-th pair, u() first
  std::vector<std::pair<int, int> > edges;
  edges.push_back(std::make_pair(2, 0));
  edges.push_back(std::make_pair(0, 1));
  edges.push_back(std::make_pair(1, 2));
  StaticGraph sg;
  sg.build(3, edges.begin(), edges.end());
  if (sg.u(sg.edge(0)) != sg.node(2) || sg.v(sg.edge(0)) != sg.node(0))
    ++errors;
  if (countIncEdges(sg, sg.node(0)) != 2 || sg.arcNum() != 6) ++errors;

  // a ListDigraph copied into a StaticDigraph, with a map and the node
  // references; the arcs out of each node come sorted by target
  ListDigraph lg;
  ListDigraph::Node x = lg.addNode(), y = lg.addNode(), z = lg.addNode();
  ListDigraph::ArcMap<int> cap(lg);
  cap[lg.addArc(x, z)] = 3;
  cap[lg.addArc(x, y)] = 4;
  cap[lg.addArc(y, z)] = 5;
  StaticDigraph st;
  StaticDigraph::ArcMap<int> scap(st);
  ListDigraph::NodeMap<StaticDigraph::Node> ref(lg);
  digraphCopy(lg, st).nodeRef(ref).arcMap(cap, scap).run();
  int sum = 0, prev = -1;
  for (StaticDigraph::OutArcIt a(st, ref[x]); a != INVALID; ++a) {
    sum += scap[a];
    if (st.index(st.target(a)) < prev) ++errors;
    prev = st.index(st.target(a));
  }
  if (sum != 7 || st.arcNum() != 3) ++errors;

  // the implicit graphs: everything is computed from the indices
  FullDigraph fd(4);
  FullGraph fg(4);
  if (fd.arcNum() != 16 || fg.edgeNum() != 6 || fg.arcNum() != 12) ++errors;
  if (fg.edge(fg(1), fg(1)) != INVALID) ++errors;  // no loops in FullGraph
  if (fd.source(fd.arc(fd(2), fd(3))) != fd(2)) ++errors;
  GridGraph grid(4, 3);  // 4 columns, 3 rows
  if (grid.nodeNum() != 12 || grid.edgeNum() != 2 * 12 - 4 - 3) ++errors;
  if (grid.target(grid.right(grid(1, 2))) != grid(2, 2)) ++errors;
  if (grid.up(grid(1, 2)) != INVALID) ++errors;  // the top row
  if (grid.col(grid(3, 1)) != 3 || grid.row(grid(3, 1)) != 1) ++errors;
  HypercubeGraph cube(3);
  if (cube.nodeNum() != 8 || cube.edgeNum() != 3 * 4) ++errors;
  HypercubeGraph::Edge e = findEdge(cube, cube(5), cube(7));  // 101, 111
  if (e == INVALID || cube.dimension(e) != 1) ++errors;

  std::cout << "static and implicit graphs: " << errors << " errors"
            << std::endl;
  return errors == 0 ? 0 : 1;
}
