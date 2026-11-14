// The Bellman-Ford algorithm of section 10.4: distances with negative
// lengths and no negative cycle, the distances of the paths of at most k
// arcs, and a negative cycle detected and returned.

#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/bellman_ford.h>
#include <lemon/path.h>

using namespace lemon;

typedef ListDigraph::Node Node;
typedef ListDigraph::Arc Arc;
typedef ListDigraph::ArcMap< int > LengthMap;

int main() {
  // s -> a (4), s -> b (2), b -> a (-3), a -> c (2), b -> c (5), c -> d (-1)
  ListDigraph g;
  Node s = g.addNode() , a = g.addNode() , b = g.addNode() ,
       c = g.addNode() , d = g.addNode();
  LengthMap len( g );
  len[ g.addArc( s , a ) ] = 4;
  len[ g.addArc( s , b ) ] = 2;
  len[ g.addArc( b , a ) ] = -3;
  len[ g.addArc( a , c ) ] = 2;
  len[ g.addArc( b , c ) ] = 5;
  len[ g.addArc( c , d ) ] = -1;
  int bad = 0;

  // the distances, and the conditions (10.2) they satisfy
  BellmanFord< ListDigraph , LengthMap > bf( g , len );
  bf.init();
  bf.addSource( s );
  if( ! bf.checkedStart() )  // no negative cycle
    ++bad;
  if( bf.dist( a ) != -1 || bf.dist( c ) != 1 || bf.dist( d ) != 0 ||
      bf.predNode( a ) != b )
    ++bad;
  for( ListDigraph::ArcIt e( g ) ; e != INVALID ; ++e )
    if( len[ e ] + bf.dist( g.source( e ) ) < bf.dist( g.target( e ) ) )
      ++bad;
  if( bf.path( d ).length() != 4 )  // s -> b -> a -> c -> d
    ++bad;

  // the paths of at most two arcs: d is not reached, c by s -> a -> c
  BellmanFord< ListDigraph , LengthMap > two( g , len );
  two.run( s , 2 );
  if( two.reached( d ) || two.dist( c ) != 6 || two.dist( a ) != -1 )
    ++bad;

  // the function-type interface
  int dist_d = 0;
  if( ! bellmanFord( g , len ).dist( dist_d ).run( s , d ) || dist_d != 0 )
    ++bad;

  // the arc d -> b (1) closes the cycle b -> a -> c -> d -> b of length -1
  len[ g.addArc( d , b ) ] = 1;
  BellmanFord< ListDigraph , LengthMap > neg( g , len );
  neg.init();
  neg.addSource( s );
  if( neg.checkedStart() )
    ++bad;
  Path< ListDigraph > cycle = neg.negativeCycle();
  int total = 0;
  for( Path< ListDigraph >::ArcIt e( cycle ) ; e != INVALID ; ++e )
    total += len[ e ];
  if( cycle.empty() || ! checkPath( g , cycle ) || total >= 0 ||
      pathSource( g , cycle ) != pathTarget( g , cycle ) )
    ++bad;

  std::cout << "negative cycle of " << cycle.length() << " arcs, length "
            << total << "; " << bad << " errors" << std::endl;
  return( bad == 0 ? 0 : 1 );
}
