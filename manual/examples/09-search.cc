// The searches of sections 9.2 to 9.5: the distances of a breadth-first
// search, a path by the function-type interface, a topological order by
// the finishing order of a depth-first search, and a directed cycle found
// by a visitor of DfsVisit.

#include <iostream>
#include <iterator>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/bfs.h>
#include <lemon/dfs.h>
#include <lemon/maps.h>
#include <lemon/path.h>

using namespace lemon;

typedef ListDigraph::Node Node;
typedef ListDigraph::Arc Arc;

// a visitor that keeps the nodes on the stack of the search and records
// an arc that enters one of them, i.e., an arc that closes a cycle
struct CycleVisitor : public DfsVisitor< ListDigraph > {
  const ListDigraph & g;
  ListDigraph::NodeMap< bool > & onStack;
  Arc & back;
  CycleVisitor( const ListDigraph & gr , ListDigraph::NodeMap< bool > & s ,
                Arc & b )
      : g( gr ) , onStack( s ) , back( b ) { }
  void reach( const Node & n ) { onStack[ n ] = true; }
  void leave( const Node & n ) { onStack[ n ] = false; }
  void examine( const Arc & a ) {
    if( onStack[ g.target( a ) ] && back == INVALID )
      back = a;
  }
};

int main() {
  // a DAG on six nodes: 0 -> 1, 0 -> 2, 1 -> 3, 2 -> 3, 3 -> 4, 2 -> 5
  ListDigraph g;
  std::vector< Node > v;
  for( int i = 0 ; i < 6 ; ++i )
    v.push_back( g.addNode() );
  const int arcs[][ 2 ] = { { 0 , 1 } , { 0 , 2 } , { 1 , 3 } ,
                            { 2 , 3 } , { 3 , 4 } , { 2 , 5 } };
  for( const auto & a : arcs )
    g.addArc( v[ a[ 0 ] ] , v[ a[ 1 ] ] );

  int bad = 0;

  // breadth-first search: the number of arcs of a shortest path
  Bfs< ListDigraph > bfs( g );
  bfs.run( v[ 0 ] );
  const int expected[] = { 0 , 1 , 1 , 2 , 3 , 2 };
  for( int i = 0 ; i < 6 ; ++i )
    if( ! bfs.reached( v[ i ] ) || bfs.dist( v[ i ] ) != expected[ i ] )
      ++bad;

  // the function-type interface, stopped at the target
  Path< ListDigraph > p;
  int d = -1;
  bool found = lemon::bfs( g ).path( p ).dist( d ).run( v[ 0 ] , v[ 4 ] );
  if( ! found || d != 3 || p.length() != 3 || ! checkPath( g , p ) )
    ++bad;

  // depth-first search over all the nodes: the order in which they are
  // processed (finished), reversed, is a topological order
  std::vector< Node > finished;
  dfs( g ).processedMap( loggerBoolMap( std::back_inserter( finished ) ) )
    .run();
  ListDigraph::NodeMap< int > pos( g );
  for( int i = 0 ; i < int( finished.size() ) ; ++i )
    pos[ finished[ i ] ] = int( finished.size() ) - 1 - i;
  for( ListDigraph::ArcIt a( g ) ; a != INVALID ; ++a )
    if( pos[ g.source( a ) ] >= pos[ g.target( a ) ] )
      ++bad;
  if( finished.size() != 6 )
    ++bad;

  // the DAG has no cycle; the arc 4 -> 1 makes one
  for( int round = 0 ; round < 2 ; ++round ) {
    if( round == 1 )
      g.addArc( v[ 4 ] , v[ 1 ] );
    ListDigraph::NodeMap< bool > onStack( g , false );
    Arc back = INVALID;
    CycleVisitor visitor( g , onStack , back );
    DfsVisit< ListDigraph , CycleVisitor > visit( g , visitor );
    visit.run();
    if( ( back != INVALID ) != ( round == 1 ) )
      ++bad;
    // the arc found closes the cycle 1 -> 3 -> 4 -> 1, which one of its
    // three arcs depends on the order of the out-arcs
    if( round == 1 ) {
      int s = g.id( g.source( back ) ) , t = g.id( g.target( back ) );
      if( ! ( ( s == 1 && t == 3 ) || ( s == 3 && t == 4 ) ||
              ( s == 4 && t == 1 ) ) )
        ++bad;
    }
  }

  std::cout << "topological order:";
  for( int i = int( finished.size() ) - 1 ; i >= 0 ; --i )
    std::cout << ' ' << g.id( finished[ i ] );
  std::cout << "; " << bad << " errors" << std::endl;
  return( bad == 0 ? 0 : 1 );
}
