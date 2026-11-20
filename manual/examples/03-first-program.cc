// The first program of the manual (section 3.1): a digraph of four nodes
// and five arcs, the cost of each arc in an ArcMap, and the shortest path
// from s to t found by Dijkstra.

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/dijkstra.h>

using namespace lemon;

int main()
{
 ListDigraph g;
 ListDigraph::Node s = g.addNode() , a = g.addNode() , b = g.addNode() ,
		   t = g.addNode();
 ListDigraph::ArcMap< int > cost( g );
 cost[ g.addArc( s , a ) ] = 2;
 cost[ g.addArc( s , b ) ] = 5;
 cost[ g.addArc( a , b ) ] = 1;
 cost[ g.addArc( a , t ) ] = 4;
 cost[ g.addArc( b , t ) ] = 2;

 Dijkstra< ListDigraph , ListDigraph::ArcMap< int > > dijkstra( g , cost );
 dijkstra.run( s );

 std::cout << "distance of t: " << dijkstra.dist( t ) << std::endl;
 std::cout << "path, backwards:";
 for( ListDigraph::Node v = t ; v != s ; v = dijkstra.predNode( v ) )
  std::cout << ' ' << g.id( v );
 std::cout << ' ' << g.id( s ) << std::endl;
 return( dijkstra.dist( t ) == 5 ? 0 : 1 );
 }
