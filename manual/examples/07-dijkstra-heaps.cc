// The heap of Dijkstra (section 7.5): the same shortest path problem on a
// grid digraph solved with each heap of the library through the named
// parameter SetStandardHeap, and the distances compared with those of the
// default binary heap.

#include <iostream>
#include <lemon/bin_heap.h>
#include <lemon/binomial_heap.h>
#include <lemon/bucket_heap.h>
#include <lemon/dheap.h>
#include <lemon/dijkstra.h>
#include <lemon/fib_heap.h>
#include <lemon/pairing_heap.h>
#include <lemon/quad_heap.h>
#include <lemon/radix_heap.h>
#include <lemon/smart_graph.h>

using namespace lemon;

typedef SmartDigraph Digraph;
typedef Digraph::ArcMap<int> LengthMap;
typedef Digraph::NodeMap<int> CrossRef;

// true if Dijkstra with the heap H gives the distances dist from s
template <typename H>
bool agrees(const Digraph &g, const LengthMap &len, Digraph::Node s,
            const Digraph::NodeMap<int> &dist) {
  typename Dijkstra<Digraph, LengthMap>::template SetStandardHeap<H>::Create
    dijkstra(g, len);
  dijkstra.run(s);
  for (Digraph::NodeIt n(g); n != INVALID; ++n)
    if (dijkstra.dist(n) != dist[n]) return false;
  return true;
}

int main() {
  // a 30 x 30 grid, with arcs in both directions and lengths from 1 to 9
  const int W = 30;
  Digraph g;
  Digraph::Node node[W][W];
  for (int i = 0; i < W; ++i)
    for (int j = 0; j < W; ++j) node[i][j] = g.addNode();
  LengthMap len(g);
  unsigned seed = 12345;
  for (int i = 0; i < W; ++i)
    for (int j = 0; j < W; ++j) {
      if (i + 1 < W) {
        seed = seed * 1103515245u + 12345u;
        len[g.addArc(node[i][j], node[i + 1][j])] = 1 + (seed >> 16) % 9;
        seed = seed * 1103515245u + 12345u;
        len[g.addArc(node[i + 1][j], node[i][j])] = 1 + (seed >> 16) % 9;
      }
      if (j + 1 < W) {
        seed = seed * 1103515245u + 12345u;
        len[g.addArc(node[i][j], node[i][j + 1])] = 1 + (seed >> 16) % 9;
        seed = seed * 1103515245u + 12345u;
        len[g.addArc(node[i][j + 1], node[i][j])] = 1 + (seed >> 16) % 9;
      }
    }
  Digraph::Node s = node[0][0];

  Dijkstra<Digraph, LengthMap> reference(g, len);   // a BinHeap
  reference.run(s);
  Digraph::NodeMap<int> dist(g);
  for (Digraph::NodeIt n(g); n != INVALID; ++n) dist[n] = reference.dist(n);

  bool ok = agrees<QuadHeap<int, CrossRef> >(g, len, s, dist) &&
            agrees<DHeap<int, CrossRef> >(g, len, s, dist) &&
            agrees<FibHeap<int, CrossRef> >(g, len, s, dist) &&
            agrees<PairingHeap<int, CrossRef> >(g, len, s, dist) &&
            agrees<BinomialHeap<int, CrossRef> >(g, len, s, dist) &&
            agrees<RadixHeap<CrossRef> >(g, len, s, dist) &&
            agrees<BucketHeap<CrossRef> >(g, len, s, dist);

  std::cout << "distance to the far corner: " << dist[node[W - 1][W - 1]]
            << (ok ? ", the same with every heap" : ", a heap disagrees")
            << std::endl;
  return ok ? 0 : 1;
}
