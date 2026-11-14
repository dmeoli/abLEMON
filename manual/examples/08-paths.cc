// The path structures of section 8.1: a path built at both ends, copied
// into the other three types, split and spliced, walked node by node, and
// the path of a Dijkstra search (a path dumper) copied into a Path.

#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/path.h>
#include <lemon/dijkstra.h>

using namespace lemon;

int main() {
  typedef ListDigraph::Node Node;
  typedef ListDigraph::Arc Arc;
  ListDigraph g;
  std::vector< Node > v;
  for( int i = 0 ; i < 5 ; ++i )
    v.push_back( g.addNode() );
  std::vector< Arc > a;  // the chain v0 -> v1 -> v2 -> v3 -> v4
  for( int i = 0 ; i < 4 ; ++i )
    a.push_back( g.addArc( v[ i ] , v[ i + 1 ] ) );
  Arc shortcut = g.addArc( v[ 0 ] , v[ 3 ] );

  int bad = 0;

  // a Path grows at both ends and is indexed in O(1)
  Path< ListDigraph > p;
  p.addBack( a[ 2 ] );
  p.addFront( a[ 1 ] );
  p.addFront( a[ 0 ] );
  p.addBack( a[ 3 ] );
  if( p.length() != 4 || p.nth( 0 ) != a[ 0 ] || p[ 3 ] != a[ 3 ] )
    ++bad;
  if( ! checkPath( g , p ) || pathSource( g , p ) != v[ 0 ] ||
      pathTarget( g , p ) != v[ 4 ] )
    ++bad;

  // the other three types are built from it by their copy constructors
  SimplePath< ListDigraph > sp( p );
  StaticPath< ListDigraph > stp( p );
  ListPath< ListDigraph > lp( p );
  if( sp.length() != 4 || stp.back() != a[ 3 ] || lp.nth( 2 ) != a[ 2 ] )
    ++bad;

  // a ListPath is split before its third arc and spliced back in O(1)
  ListPath< ListDigraph > tail;
  lp.split( lp.nthIt( 2 ) , tail );
  if( lp.length() != 2 || tail.length() != 2 || tail.front() != a[ 2 ] )
    ++bad;
  lp.spliceBack( tail );
  if( lp.length() != 4 || ! tail.empty() || ! checkPath( g , lp ) )
    ++bad;

  // the nodes of a path, its source included
  int count = 0;
  for( Node u : pathNodes( g , p ) ) {
    if( u != v[ count ] )
      ++bad;
    ++count;
  }
  if( count != 5 )
    ++bad;

  // Dijkstra gives a path dumper, read backwards, copied into a Path
  ListDigraph::ArcMap< int > len( g , 1 );
  len[ shortcut ] = 10;
  Dijkstra< ListDigraph > dijkstra( g , len );
  dijkstra.run( v[ 0 ] );
  Path< ListDigraph > q = dijkstra.path( v[ 4 ] );
  if( q.length() != 4 || q.front() != a[ 0 ] || q.back() != a[ 3 ] )
    ++bad;

  std::cout << "path of " << q.length() << " arcs from " << g.id( v[ 0 ] )
            << " to " << g.id( v[ 4 ] ) << ", " << bad << " errors"
            << std::endl;
  return( bad == 0 ? 0 : 1 );
}
