// A new graph, a new map and an observer (appendix A): the directed cycle
// on n nodes written as a base class that computes everything from the
// indices, completed by DigraphExtender and checked against the Digraph
// concept; a read map computed on the fly, checked against ReadMap; an
// observer of the nodes of a ListDigraph; and Dijkstra run on the new
// digraph with the new map.

#include <iostream>
#include <vector>
#include <lemon/bits/graph_extender.h>
#include <lemon/concept_check.h>
#include <lemon/concepts/digraph.h>
#include <lemon/concepts/maps.h>
#include <lemon/dijkstra.h>
#include <lemon/list_graph.h>

using namespace lemon;

// The base: node i and arc i, the arc i going from node i to node i + 1
// (mod n); the extender derives the iterators, the maps and the notifiers
class CycleDigraphBase {
 public:
  class Node {
    friend class CycleDigraphBase;

   protected:
    int _id;
    explicit Node(int id) : _id(id) {}

   public:
    Node() : _id(-1) {}
    Node(Invalid) : _id(-1) {}
    bool operator==(const Node &n) const { return _id == n._id; }
    bool operator!=(const Node &n) const { return _id != n._id; }
    bool operator<(const Node &n) const { return _id < n._id; }
  };

  class Arc {
    friend class CycleDigraphBase;

   protected:
    int _id;
    explicit Arc(int id) : _id(id) {}

   public:
    Arc() : _id(-1) {}
    Arc(Invalid) : _id(-1) {}
    bool operator==(const Arc &a) const { return _id == a._id; }
    bool operator!=(const Arc &a) const { return _id != a._id; }
    bool operator<(const Arc &a) const { return _id < a._id; }
  };

  typedef True NodeNumTag;  // countNodes() calls nodeNum()
  typedef True ArcNumTag;
  int nodeNum() const { return _n; }
  int arcNum() const { return _n; }

  Node source(Arc a) const { return Node(a._id); }
  Node target(Arc a) const { return Node((a._id + 1) % _n); }

  // the items from the last index down to 0
  void first(Node &v) const { v._id = _n - 1; }
  static void next(Node &v) { --v._id; }
  void first(Arc &a) const { a._id = _n - 1; }
  static void next(Arc &a) { --a._id; }
  // one arc out of each node and one into it
  void firstOut(Arc &a, const Node &v) const { a._id = v._id; }
  static void nextOut(Arc &a) { a._id = -1; }
  void firstIn(Arc &a, const Node &v) const { a._id = (v._id + _n - 1) % _n; }
  static void nextIn(Arc &a) { a._id = -1; }

  static int id(Node v) { return v._id; }
  static int id(Arc a) { return a._id; }
  static Node nodeFromId(int id) { return Node(id); }
  static Arc arcFromId(int id) { return Arc(id); }
  int maxNodeId() const { return _n - 1; }
  int maxArcId() const { return _n - 1; }

 protected:
  CycleDigraphBase() : _n(0) {}
  int _n;
};

typedef DigraphExtender<CycleDigraphBase> ExtendedCycleDigraphBase;

class CycleDigraph : public ExtendedCycleDigraphBase {
 public:
  explicit CycleDigraph(int n) { _n = n; }
  // the maps of the digraph are told that every item has gone, and then
  // that the new ones are there, as FullDigraph::resize() does
  void resize(int n) {
    notifier(Arc()).clear();
    notifier(Node()).clear();
    _n = n;
    notifier(Node()).build();
    notifier(Arc()).build();
  }
  Node operator()(int i) const { return nodeFromId(i); }
};

// A read map computed on the fly: the length of arc i is 1 + i % 3
class LengthByIndex {
 public:
  typedef CycleDigraph::Arc Key;
  typedef int Value;
  explicit LengthByIndex(const CycleDigraph &g) : _g(g) {}
  Value operator[](const Key &a) const { return 1 + _g.id(a) % 3; }

 private:
  const CycleDigraph &_g;
};

// An observer of the nodes of a ListDigraph that counts them
class NodeCounter : public ListDigraph::NodeNotifier::ObserverBase {
 public:
  explicit NodeCounter(const ListDigraph &g) : _count(0) {
    attach(g.notifier(ListDigraph::Node()));
    build();
  }
  int count() const { return _count; }

 protected:
  virtual void add(const ListDigraph::Node &) { ++_count; }
  virtual void add(const std::vector<ListDigraph::Node> &v) {
    _count += static_cast<int>(v.size());
  }
  virtual void erase(const ListDigraph::Node &) { --_count; }
  virtual void erase(const std::vector<ListDigraph::Node> &v) {
    _count -= static_cast<int>(v.size());
  }
  virtual void build() {
    _count = 0;
    ListDigraph::Node v;
    for (notifier()->first(v); v != INVALID; notifier()->next(v)) ++_count;
  }
  virtual void clear() { _count = 0; }

 private:
  int _count;
};

int main() {
  int errors = 0;

  checkConcept<concepts::Digraph, CycleDigraph>();
  checkConcept<concepts::ReadMap<CycleDigraph::Arc, int>, LengthByIndex>();

  CycleDigraph g(7);
  if (countNodes(g) != 7 || countArcs(g) != 7) ++errors;
  int out = 0;
  for (CycleDigraph::OutArcIt a(g, g(3)); a != INVALID; ++a) ++out;
  if (out != 1 || g.runningNode(CycleDigraph::OutArcIt(g, g(6))) != g(0))
    ++errors;

  // Dijkstra on the new digraph with the new map: the distance of node k
  // from node 0 is the sum of 1 + i % 3 for i < k
  LengthByIndex len(g);
  Dijkstra<CycleDigraph, LengthByIndex> dij(g, len);
  dij.run(g(0));
  int expected = 0;
  for (int k = 0; k < 7; ++k) {
    if (dij.dist(g(k)) != expected) ++errors;
    expected += 1 + k % 3;
  }

  // a node map follows a resize() through the notifier
  CycleDigraph::NodeMap<int> label(g, 5);
  g.resize(10);
  if (label[g(9)] != 0) ++errors;  // rebuilt, with Value() everywhere
  label[g(9)] = 1;

  // the observer counts the nodes as they come and go
  ListDigraph lg;
  lg.addNode();
  NodeCounter counter(lg);
  ListDigraph::Node v = lg.addNode();
  lg.addNode();
  lg.erase(v);
  if (counter.count() != 2 || counter.count() != countNodes(lg)) ++errors;

  std::cout << "a new digraph, map and observer: " << errors << " errors"
            << std::endl;
  return errors == 0 ? 0 : 1;
}
